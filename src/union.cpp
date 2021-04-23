/**
 *  OSM
 *  Copyright (C) 2020  Pavel Smokotnin

 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "union.h"
#include "stored.h"

Union::Union(Settings *settings, QObject *parent): chart::Source(parent),
    m_settings(settings),
    m_sources(4),
    m_timer(nullptr), m_timerThread(nullptr),
    m_operation(Sum),
    m_type(Vector)
{
    m_name = "Union";
    m_sources.fill(nullptr);
    setObjectName(m_name);

    m_timer.setInterval(80);//12.5 per sec
    m_timer.setSingleShot(true);
    m_timer.moveToThread(&m_timerThread);
    connect(&m_timer, SIGNAL(timeout()), SLOT(calc()), Qt::DirectConnection);
    connect(this, SIGNAL(needUpdate()), &m_timer, SLOT(start()));
    connect(&m_timerThread, SIGNAL(finished()), &m_timer, SLOT(stop()), Qt::DirectConnection);
    m_timerThread.start();
    init();
}
Union::~Union()
{
    m_timerThread.quit();
    m_timerThread.wait();
}

chart::Source *Union::clone() const
{
    auto cloned = new Union(nullptr, parent());
    cloned->m_sources = m_sources;
    cloned->setOperation(operation());
    cloned->setType(type());
    cloned->setName(name());
    cloned->setActive(active());
    return cloned;
}
void Union::setOperation(const Operation &operation) noexcept
{
    if (m_operation != operation) {
        m_operation = operation;
        update();
        emit operationChanged(m_operation);
    }
}

void Union::setActive(bool active) noexcept
{
    if (active == m_active)
        return;
    chart::Source::setActive(active);
    update();
}
Union::Type Union::type() const
{
    return m_type;
}
void Union::setType(const Type &type)
{
    if (m_type != type) {
        m_type = type;
        emit typeChanged();
        update();
    }
}
void Union::init() noexcept
{
    std::lock_guard<std::mutex> guard(m_dataMutex);
    resize();
}

void Union::resize()
{
    chart::Source *primary = m_sources.first();
    m_dataLength         = static_cast<unsigned int>(primary ? primary->size() : 1);
    m_deconvolutionSize = static_cast<unsigned int>(primary ? primary->impulseSize() : 1);
    m_ftdata             = new FTData[m_dataLength];
    m_impulseData        = new TimeData[m_deconvolutionSize];
}
chart::Source *Union::getSource(int index) const noexcept
{
    if (index < m_sources.count()) {
        return m_sources.at(index);
    }
    return nullptr;
}
void Union::setSource(int index, chart::Source *s) noexcept
{
    if (s == getSource(index))
        return;

    if (index < m_sources.count()) {
        if (m_sources[index]) {
            disconnect(m_sources[index], &chart::Source::readyRead, this, &Union::update);
        }
        m_sources.replace(index, s);
        if (index == 0)
            init();

        if (s) connect(s, &chart::Source::readyRead, this, &Union::update);
        update();
    }
}
void Union::update() noexcept
{
    if (!m_timer.isActive())
        emit needUpdate();
}
void Union::calc() noexcept
{
    std::set<chart::Source *> sources;

    if (!active())
        return;

    std::lock_guard<std::mutex> guard(m_dataMutex);
    chart::Source *primary = m_sources.first();
    unsigned int count = 0;

    if (!primary) {
        setActive(false);
        return;
    }

    if (primary->size() != size()) {
        resize();
    }

    for (auto &s : m_sources) {
        if (s) {
            sources.insert(s);
            if (s->size() != primary->size()) {
                setActive(false);
                return;
            }
            count ++;
        }
    }

    //lock each unique source, prevent deadlock
    for (auto &s : sources) {
        if (s) s->lock();
    }

    switch (m_type) {
    case Vector:
        calcVector(count, sources, primary);
        break;
    case Polar:
        calcPolar(count, sources, primary);
        break;
    }

    for (auto &s : sources) {
        if (s) s->unlock();
    }
    emit readyRead();
}
void Union::calcPolar(unsigned int count, std::set<Source *> sources,
                      chart::Source *primary) noexcept
{
    float magnitude, phase, module, coherence;

    for (unsigned int i = 0; i < primary->size(); i++) {
        magnitude = primary->magnitudeRaw(i);
        phase = primary->phase(i).arg();
        module = primary->module(i);
        coherence = primary->coherence(i);
        for (auto it = sources.begin(); it != sources.end(); ++it) {
            if (*it && *it != primary) {
                switch (m_operation) {
                case Sum:
                case Avg:
                    magnitude += (*it)->magnitudeRaw(i);
                    phase += (*it)->phase(i).arg();
                    module += (*it)->module(i);
                    break;
                case Diff:
                    magnitude -= (*it)->magnitudeRaw(i);
                    phase -= (*it)->phase(i).arg();
                    module -= (*it)->module(i);
                    break;
                }

                coherence = std::max(coherence, (*it)->coherence(i));
            }
        }
        if (m_operation == Avg) {
            magnitude /= count;
            phase /= count;
            module /= count;
        }
        complex p;
        p.polar(phase);
        m_ftdata[i].frequency  = primary->frequency(i);
        m_ftdata[i].module     = module;
        m_ftdata[i].phase      = p;
        m_ftdata[i].magnitude  = magnitude;
        m_ftdata[i].coherence  = coherence;
    }
}
void Union::calcVector(unsigned int count, std::set<chart::Source *> sources,
                       chart::Source *primary) noexcept
{
    complex a, m;

    for (unsigned int i = 0; i < primary->size(); i++) {

        a = primary->phase(i) * primary->module(i);
        m = primary->phase(i) * primary->magnitudeRaw(i);
        float coherence = primary->coherence(i);
        for (auto it = sources.begin(); it != sources.end(); ++it) {
            if (*it && *it != primary) {
                switch (m_operation) {
                case Sum:
                case Avg:
                    m_ftdata[i].module += (*it)->module(i);
                    a += (*it)->phase(i) * (*it)->module(i);
                    m += (*it)->phase(i) * (*it)->magnitudeRaw(i);
                    break;
                case Diff:
                    m_ftdata[i].module -= (*it)->module(i);
                    a -= (*it)->phase(i) * (*it)->module(i);
                    m -= (*it)->phase(i) * (*it)->magnitudeRaw(i);
                    break;
                }

                coherence = std::max(coherence, (*it)->coherence(i));
            }
        }
        if (m_operation == Avg) {
            a /= count;
            m /= count;
        }
        m_ftdata[i].frequency  = primary->frequency(i);
        m_ftdata[i].module     = a.abs();
        m_ftdata[i].phase      = m;
        m_ftdata[i].magnitude  = m.abs();
        m_ftdata[i].coherence  = coherence;
    }
}
QJsonObject Union::toJSON() const noexcept
{
    QJsonObject data;
    //TODO: do
    return data;
}
void Union::fromJSON(QJsonObject data) noexcept
{
    //TODO: do
    Q_UNUSED(data)
}
QObject *Union::store()
{
    auto *store = new Stored();
    store->build(this);

    QString notes = operationMap.at(m_operation) + ":\n";
    for (auto &s : m_sources) {
        if (s) {
            notes += s->name() + "\t";
        }
    }
    store->setNotes(notes);
    return store;
}
