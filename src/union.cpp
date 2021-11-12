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

int Union::count() const noexcept
{
    return m_sources.count();
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
        calcVector(count, primary);
        break;
    case Polar:
        calcPolar(count, primary);
        break;
    case dB:
        calcdB(count, primary);
        break;
    case Power:
        calcPower(count, primary);
        break;
    }

    for (auto &s : sources) {
        if (s) s->unlock();
    }
    emit readyRead();
}
void Union::calcPolar(unsigned int count, chart::Source *primary) noexcept
{
    float magnitude, module, coherence, coherenceWeight;
    complex phase;

    for (unsigned int i = 0; i < primary->size(); i++) {
        magnitude = primary->magnitudeRaw(i);
        phase = primary->phase(i);
        module = primary->module(i);
        coherence = std::abs(primary->module(i) * primary->coherence(i));
        coherenceWeight = std::abs(primary->module(i));

        for (auto it = m_sources.begin(); it != m_sources.end(); ++it) {
            if (*it && it != m_sources.begin()) {
                switch (m_operation) {
                case Sum:
                case Avg:
                    magnitude += (*it)->magnitudeRaw(i);
                    phase += (*it)->phase(i);
                    module += (*it)->module(i);
                    break;
                case Diff:
                    magnitude -= (*it)->magnitudeRaw(i);
                    phase -= (*it)->phase(i);
                    module -= (*it)->module(i);
                    break;
                }

                coherence += std::abs((*it)->module(i) * (*it)->coherence(i));
                coherenceWeight += std::abs((*it)->module(i));
            }
        }
        if (m_operation == Avg) {
            magnitude /= count;
            phase /= count;
            module /= count;
        }
        coherence /= coherenceWeight;

        m_ftdata[i].frequency  = primary->frequency(i);
        m_ftdata[i].module     = module;
        m_ftdata[i].phase      = phase.normalize();
        m_ftdata[i].magnitude  = magnitude;
        m_ftdata[i].coherence  = coherence;
    }
}
void Union::calcVector(unsigned int count, chart::Source *primary) noexcept
{
    float coherence, coherenceWeight;
    complex a, m;

    for (unsigned int i = 0; i < primary->size(); i++) {

        a = primary->phase(i) * primary->module(i);
        m = primary->phase(i) * primary->magnitudeRaw(i);

        coherence = std::abs(primary->module(i) * primary->coherence(i));
        coherenceWeight = std::abs(primary->module(i));

        for (auto it = m_sources.begin(); it != m_sources.end(); ++it) {
            if (*it && it != m_sources.begin()) {
                switch (m_operation) {
                case Sum:
                case Avg:
                    a += (*it)->phase(i) * (*it)->module(i);
                    m += (*it)->phase(i) * (*it)->magnitudeRaw(i);
                    break;
                case Diff:
                    a -= (*it)->phase(i) * (*it)->module(i);
                    m -= (*it)->phase(i) * (*it)->magnitudeRaw(i);
                    break;
                }

                coherence += std::abs((*it)->module(i) * (*it)->coherence(i));
                coherenceWeight += std::abs((*it)->module(i));
            }
        }
        if (m_operation == Avg) {
            a /= count;
            m /= count;
        }
        coherence /= coherenceWeight;

        m_ftdata[i].frequency  = primary->frequency(i);
        m_ftdata[i].module     = a.abs();
        m_ftdata[i].phase      = m.normalize();
        m_ftdata[i].magnitude  = m.abs();
        m_ftdata[i].coherence  = coherence;
    }
}

void Union::calcdB(unsigned int count, chart::Source *primary) noexcept
{
    float magnitude, module, coherence, coherenceWeight;
    complex phase;

    for (unsigned int i = 0; i < primary->size(); i++) {
        magnitude = primary->magnitude(i);
        phase = primary->phase(i);
        module = 20.f * std::log10f((primary)->module(i));
        coherence = std::abs(primary->module(i) * primary->coherence(i));
        coherenceWeight = std::abs(primary->module(i));

        for (auto it = m_sources.begin(); it != m_sources.end(); ++it) {
            if (*it && it != m_sources.begin()) {
                switch (m_operation) {
                case Sum:
                case Avg:
                    magnitude += (*it)->magnitude(i);
                    phase += (*it)->phase(i);
                    module += 20.f * std::log10f((*it)->module(i));
                    break;
                case Diff:
                    magnitude -= (*it)->magnitude(i);
                    phase -= (*it)->phase(i);
                    module -= 20.f * std::log10f((*it)->module(i));
                    break;
                }

                coherence += std::abs((*it)->module(i) * (*it)->coherence(i));
                coherenceWeight += std::abs((*it)->module(i));
            }
        }

        if (m_operation == Avg) {
            magnitude /= count;
            phase /= count;
            module /= count;
        }
        coherence /= coherenceWeight;

        magnitude = std::pow(10, magnitude / 20.f);
        module    = std::pow(10, module / 20.f);

        m_ftdata[i].frequency  = primary->frequency(i);
        m_ftdata[i].module     = module;
        m_ftdata[i].phase      = phase.normalize();
        m_ftdata[i].magnitude  = magnitude;
        m_ftdata[i].coherence  = coherence;
    }
}

void Union::calcPower(unsigned int count, chart::Source *primary) noexcept
{
    float magnitude, module, coherence, coherenceWeight;
    complex phase;

    for (unsigned int i = 0; i < primary->size(); i++) {
        magnitude = std::pow(primary->magnitudeRaw(i), 2);
        phase = primary->phase(i);
        module = std::pow((primary)->module(i), 2);
        coherence = std::abs(primary->module(i) * primary->coherence(i));
        coherenceWeight = std::abs(primary->module(i));

        for (auto it = m_sources.begin(); it != m_sources.end(); ++it) {
            if (*it && it != m_sources.begin()) {
                switch (m_operation) {
                case Sum:
                case Avg:
                    magnitude += std::pow((*it)->magnitudeRaw(i), 2);
                    phase += (*it)->phase(i);
                    module += std::pow((*it)->module(i), 2);
                    break;
                case Diff:
                    magnitude -= std::pow((*it)->magnitudeRaw(i), 2);
                    phase -= (*it)->phase(i);
                    module -= std::pow((*it)->module(i), 2);
                    break;
                }

                coherence += std::abs((*it)->module(i) * (*it)->coherence(i));
                coherenceWeight += std::abs((*it)->module(i));
            }
        }
        if (m_operation == Avg) {
            magnitude /= count;
            phase /= count;
            module /= count;
        } else if (m_operation == Diff) {
            magnitude = std::abs(magnitude);
            module    = std::abs(module);
        }
        coherence /= coherenceWeight;

        magnitude = std::sqrt(magnitude);
        module    = std::sqrt(module);

        m_ftdata[i].frequency  = primary->frequency(i);
        m_ftdata[i].module     = module;
        m_ftdata[i].phase      = phase.normalize();
        m_ftdata[i].magnitude  = magnitude;
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

Union::Operation Union::operation() const noexcept
{
    return m_operation;
}
QObject *Union::store()
{
    auto *store = new Stored();
    store->build(this);

    QString notes = operationMap.at(m_operation) + " " + typeMap.at(m_type) + ":\n";
    for (auto &s : m_sources) {
        if (s) {
            notes += s->name() + "\t";
        }
    }
    store->setNotes(notes);
    return store;
}
