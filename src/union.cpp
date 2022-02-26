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
#include "sourcelist.h"
#include <QJsonArray>
#include <cmath>

Union::Union(Settings *settings, QObject *parent): chart::Source(parent),
    m_settings(settings),
    m_sources(2),
    m_timer(nullptr), m_timerThread(nullptr),
    m_operation(Summation),
    m_type(Vector),
    m_autoName(true)
{
    m_name = "Union";
    m_sources.fill(nullptr);
    setObjectName(m_name);

    m_timer.setInterval(80);//12.5 per sec
    m_timer.setSingleShot(true);
    m_timer.moveToThread(&m_timerThread);
    connect(this, &Union::operationChanged, &Union::applyAutoName);
    connect(this, &Union::typeChanged, &Union::applyAutoName);
    connect(&m_timer, SIGNAL(timeout()), SLOT(calc()), Qt::DirectConnection);
    connect(this, SIGNAL(needUpdate()), &m_timer, SLOT(start()));
    connect(&m_timerThread, SIGNAL(finished()), &m_timer, SLOT(stop()), Qt::DirectConnection);
    m_timerThread.start();
    init();
    applyAutoName();
}
Union::~Union()
{
    m_timerThread.quit();
    m_timerThread.wait();
}

chart::Source *Union::clone() const
{
    auto cloned = new Union(nullptr, parent());

    cloned->setCount(count());
    cloned->setOperation(operation());
    cloned->setType(type());
    cloned->setName(name());
    cloned->setActive(active());
    for (int i = 0; i < count(); ++i) {
        cloned->setSource(i, getSource(i));
    }
    return cloned;
}

int Union::count() const noexcept
{
    return m_sources.count();
}

void Union::setCount(int count) noexcept
{
    std::lock_guard<std::mutex> guard(m_dataMutex);
    if (m_sources.count() != count) {
        m_sources.resize(count);
        emit countChanged(count);
        update();
    }
}
void Union::setOperation(Operation operation) noexcept
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
void Union::setType(Type type)
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
        emit modelChanged();
    }
}
void Union::update() noexcept
{
    if (!m_timer.isActive()) {
        emit needUpdate();
    }
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
    if (count < 2) {
        setActive(false);
        return;
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
                case Summation:
                case Avg:
                    magnitude += (*it)->magnitudeRaw(i);
                    phase += (*it)->phase(i);
                    module += (*it)->module(i);
                    break;
                case Subtract:
                    magnitude = std::abs(magnitude - (*it)->magnitudeRaw(i));
                    auto p = phase.real * (*it)->phase(i).real + phase.imag * (*it)->phase(i).imag;
                    auto sign = (phase.imag - (*it)->phase(i).imag > 0 ? 1 : -1);
                    phase.real = p / (phase.abs() * (*it)->phase(i).abs());
                    phase.imag = sign * std::sqrt(1 - phase.real * phase.real);
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
        if (std::isnan(phase.real) || std::isnan(phase.imag)) {
            phase = {1, 0};
        }

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
    complex a, m, p;

    for (unsigned int i = 0; i < primary->size(); i++) {
        a = primary->phase(i) * primary->module(i);
        p = primary->phase(i) * primary->peakSquared(i);
        m = primary->phase(i) * primary->magnitudeRaw(i);

        coherence = std::abs(primary->module(i) * primary->coherence(i));
        coherenceWeight = std::abs(primary->module(i));

        for (auto it = m_sources.begin(); it != m_sources.end(); ++it) {
            if (*it && it != m_sources.begin()) {
                switch (m_operation) {
                case Summation:
                case Avg:
                    a += (*it)->phase(i) * (*it)->module(i);
                    p += (*it)->phase(i) * (*it)->peakSquared(i);
                    m += (*it)->phase(i) * (*it)->magnitudeRaw(i);
                    break;
                case Subtract:
                    a -= (*it)->phase(i) * (*it)->module(i);
                    p -= (*it)->phase(i) * (*it)->peakSquared(i);
                    m -= (*it)->phase(i) * (*it)->magnitudeRaw(i);
                    break;
                }

                coherence       += std::abs((*it)->module(i) * (*it)->coherence(i));
                coherenceWeight += std::abs((*it)->module(i));
            }
        }
        if (m_operation == Avg) {
            a /= count;
            p /= count;
            m /= count;
        }
        coherence /= coherenceWeight;

        m_ftdata[i].frequency  = primary->frequency(i);
        m_ftdata[i].module     = a.abs();
        m_ftdata[i].phase      = m.normalize();
        m_ftdata[i].magnitude  = m.abs();
        m_ftdata[i].coherence  = coherence;
        m_ftdata[i].peakSquared = p.abs();
    }
}

void Union::calcdB(unsigned int count, chart::Source *primary) noexcept
{
    float magnitude, module, coherence, coherenceWeight;
    complex phase;

    for (unsigned int i = 0; i < primary->size(); i++) {
        magnitude = primary->magnitude(i);
        phase = primary->phase(i);
        module = 20.f * std::log10((primary)->module(i));
        coherence = std::abs(primary->module(i) * primary->coherence(i));
        coherenceWeight = std::abs(primary->module(i));

        for (auto it = m_sources.begin(); it != m_sources.end(); ++it) {
            if (*it && it != m_sources.begin()) {
                switch (m_operation) {
                case Summation:
                case Avg:
                    magnitude += (*it)->magnitude(i);
                    phase += (*it)->phase(i);
                    module += 20.f * std::log10((*it)->module(i));
                    break;
                case Subtract:
                    magnitude -= (*it)->magnitude(i);
                    auto p = phase.real * (*it)->phase(i).real + phase.imag * (*it)->phase(i).imag;
                    auto sign = (phase.imag - (*it)->phase(i).imag > 0 ? 1 : -1);
                    phase.real = p / (phase.abs() * (*it)->phase(i).abs());
                    phase.imag = sign * std::sqrt(1 - phase.real * phase.real);
                    module -= 20.f * std::log10((*it)->module(i));
                    break;
                }

                coherence       += std::abs((*it)->module(i) * (*it)->coherence(i));
                coherenceWeight += std::abs((*it)->module(i));
            }
        }

        if (m_operation == Avg) {
            magnitude /= count;
            phase /= count;
            module /= count;
        }
        coherence /= coherenceWeight;
        if (std::isnan(phase.real) || std::isnan(phase.imag)) {
            phase = {1, 0};
        }

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
                case Summation:
                case Avg:
                    magnitude += std::pow((*it)->magnitudeRaw(i), 2);
                    phase += (*it)->phase(i);
                    module += std::pow((*it)->module(i), 2);
                    break;
                case Subtract:
                    magnitude -= std::pow((*it)->magnitudeRaw(i), 2);
                    auto p = phase.real * (*it)->phase(i).real + phase.imag * (*it)->phase(i).imag;
                    auto sign = (phase.imag - (*it)->phase(i).imag > 0 ? 1 : -1);
                    phase.real = p / (phase.abs() * (*it)->phase(i).abs());
                    phase.imag = sign * std::sqrt(1 - phase.real * phase.real);
                    module -= std::pow((*it)->module(i), 2);
                    break;
                }

                coherence       += std::abs((*it)->module(i) * (*it)->coherence(i));
                coherenceWeight += std::abs((*it)->module(i));
            }
        }
        if (m_operation == Avg) {
            magnitude /= count;
            phase /= count;
            module /= count;
        } else if (m_operation == Subtract) {
            magnitude = std::abs(magnitude);
            module    = std::abs(module);
        }
        coherence /= coherenceWeight;
        if (std::isnan(phase.real) || std::isnan(phase.imag)) {
            phase = {1, 0};
        }

        magnitude = std::sqrt(magnitude);
        module    = std::sqrt(module);

        m_ftdata[i].frequency  = primary->frequency(i);
        m_ftdata[i].module     = module;
        m_ftdata[i].phase      = phase.normalize();
        m_ftdata[i].magnitude  = magnitude;
        m_ftdata[i].coherence  = coherence;
    }
}

bool Union::autoName() const
{
    return m_autoName;
}

void Union::setAutoName(bool autoName)
{
    m_autoName = autoName;
    emit autoNameChanged();
}

void Union::applyAutoName() noexcept
{
    if (!m_autoName) {
        return;
    }
    setName(typeMap.at(m_type) + " " + operationMap.at(m_operation));
}

QJsonObject Union::toJSON(const SourceList *list) const noexcept
{
    QJsonObject object;
    object["active"]    = active();
    object["name"]      = name();

    object["count"]     = count();
    object["type"]      = type();
    object["operation"] = operation();
    object["autoName"]  = autoName();

    QJsonObject color;
    color["red"]    = m_color.red();
    color["green"]  = m_color.green();
    color["blue"]   = m_color.blue();
    color["alpha"]  = m_color.alpha();
    object["color"] = color;

    QJsonArray sources;
    if (list) {
        for (int i = 0; i < count(); ++i) {
            auto source = getSource(i);
            auto index = list->indexOf(source);
            sources.append(index);
        }
    }
    object["sources"] = sources;

    return object;
}
void Union::fromJSON(QJsonObject data, const SourceList *list) noexcept
{
    setType(static_cast<Type>(data["type"].toInt()));
    setOperation(static_cast<Operation>(data["operation"].toInt()));

    auto jsonColor = data["color"].toObject();
    QColor c(
        jsonColor["red"  ].toInt(0),
        jsonColor["green"].toInt(0),
        jsonColor["blue" ].toInt(0),
        jsonColor["alpha"].toInt(1));
    setColor(c);

    setName(data["name"].toString(name()));
    setAutoName(data["autoName"].toBool(true));

    auto connection = std::make_shared<QMetaObject::Connection>();
    auto connection_ptr = connection.get();
    auto fillSources = [ = ]() {
        setCount(data["count"].toInt(count()));

        auto sources = data["sources"].toArray();
        for (int i = 0; i < sources.count(); i++) {
            auto index = sources[i].toInt();

            auto source = list->get(index);
            if (source) {
                setSource(i, source);
            }
        }

        setActive(data["active"].toBool(active()));

        if (connection) {
            disconnect(*connection.get());
        }
    };
    *connection_ptr = connect(list, &SourceList::loaded, fillSources);
}

Union::Operation Union::operation() const noexcept
{
    return m_operation;
}
QObject *Union::store()
{
    auto *store = new Stored();
    store->build(this);
    store->autoName(name());

    QString notes = operationMap.at(m_operation) + " " + typeMap.at(m_type) + ":\n";
    for (auto &s : m_sources) {
        if (s) {
            notes += s->name() + "\t";
        }
    }
    store->setNotes(notes);
    return store;
}
