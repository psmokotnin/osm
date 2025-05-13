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
#include "notifier.h"
#include <QJsonArray>
#include <cmath>

std::mutex Union::s_calcmutex = std::mutex();

Union::Union(QObject *parent): Abstract::Source(parent),
    m_sources(2),
    m_timer(nullptr), m_timerThread(nullptr),
    m_operation(Summation),
    m_type(Vector),
    m_autoName(true)
{
    setName("Union");
    setObjectName(name());

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

Shared::Source Union::clone() const
{
    auto cloned = std::make_shared<Union>();

    cloned->setCount(count());
    cloned->setOperation(operation());
    cloned->setType(type());
    cloned->setName(name());
    cloned->setActive(active());
    for (int i = 0; i < count(); ++i) {
        cloned->setSource(i, getSource(i));
    }
    return std::static_pointer_cast<Abstract::Source>(cloned);
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

void Union::setActive(bool newActive) noexcept
{
    if (newActive == active())
        return;

    if (newActive && checkLoop(this)) {
        return;
    }
    Abstract::Source::setActive(newActive);
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
    auto primary = m_sources.first();
    setFrequencyDomainSize(static_cast<unsigned int>(primary ? primary->frequencyDomainSize() : 1));
    setTimeDomainSize(     static_cast<unsigned int>(primary ? primary->timeDomainSize()      : 1));
}
Shared::Source Union::getSource(int index) const noexcept
{
    if (index < m_sources.count()) {
        return m_sources.at(index);
    }
    return {nullptr};
}

QUuid Union::getSourceId(int index) const noexcept
{
    auto source = getSource(index);
    if (source) {
        return source->uuid();
    }
    return {};
}
bool Union::setSource(int index, const Shared::Source &s) noexcept
{
    if (s == getSource(index))
        return true;

    auto unionSource = std::dynamic_pointer_cast<Union>(s);
    if (unionSource) {
        if (unionSource->checkLoop(this)) {
            setActive(false);
            return false;
        }
    }

    if (index < m_sources.count()) {
        if (m_sources[index]) {
            disconnect(m_sources[index].get(), &Abstract::Source::readyRead, this, &Union::update);
        }
        m_sources.replace(index, s);
        if (index == 0)
            init();

        if (s) {
            connect(s.get(), &::Abstract::Source::readyRead, this, &Union::update);
            connect(s.get(), &::Abstract::Source::beforeDestroy, this, &Union::sourceDestroyed, Qt::DirectConnection);
        }
        update();
        emit modelChanged();
    }
    return true;
}

void Union::update() noexcept
{
    if (!m_timer.isActive()) {
        emit needUpdate();
    }
}

void Union::calc() noexcept
{
    std::set<Shared::Source> sources;

    if (!active())
        return;

    {
        std::lock_guard<std::mutex> callGuard(s_calcmutex);
        std::lock_guard<std::mutex> guard(m_dataMutex);
        auto primary = m_sources.first();
        unsigned int count = 0;

        if (!primary) {
            setActive(false);
            return;
        }

        if (primary->frequencyDomainSize() != frequencyDomainSize()) {
            resize();
        }

        for (auto &s : m_sources) {
            if (s) {
                sources.insert(s);
                if (s->frequencyDomainSize() != primary->frequencyDomainSize()) {
                    if (0/*can resize*/) {
                        //resize
                    } else {
                        setActive(false);
                        emit Notifier::getInstance()->newMessage(name(),
                                                                 " Sources must have the same window size and sample rate: " + s->name());
                        return;
                    }
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
            if (s && s.get() != this) s->lock();
        }

        if (m_operation == Apply) {
            calcApply(primary);
        } else {
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
        }

        for (auto &s : sources) {
            if (s) s->unlock();
        }
    }
    emit readyRead();
}
void Union::calcPolar(unsigned int count, const Shared::Source &primary) noexcept
{
    float magnitude, module, coherence, coherenceWeight;
    Complex phase;

    for (unsigned int i = 0; i < primary->frequencyDomainSize(); i++) {
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
                case Diff:
                case Subtract: {
                    magnitude = std::abs(magnitude - (*it)->magnitudeRaw(i));
                    auto p = phase.real * (*it)->phase(i).real + phase.imag * (*it)->phase(i).imag;
                    auto sign = (phase.imag - (*it)->phase(i).imag > 0 ? 1 : -1);
                    phase.real = p / (phase.abs() * (*it)->phase(i).abs());
                    phase.imag = sign * std::sqrt(1 - phase.real * phase.real);
                    module -= (*it)->module(i);
                }
                break;
                case Min:
                    magnitude = std::min(magnitude, (*it)->magnitudeRaw(i));
                    module = std::min(module, (*it)->module(i));
                    phase.polar(std::min(phase.arg(), (*it)->phase(i).arg()));
                    break;
                case Max:
                    magnitude = std::max(magnitude, (*it)->magnitudeRaw(i));
                    module = std::max(module, (*it)->module(i));
                    phase.polar(std::max(phase.arg(), (*it)->phase(i).arg()));
                    break;
                case Apply:
                    Q_ASSERT(false);
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
        if (m_operation == Diff) {
            magnitude = std::abs(magnitude);
            phase.polar(std::abs(phase.arg()));
            module = std::abs(module);
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

    for (unsigned int i = 0; i < primary->timeDomainSize(); i++) {
        m_impulseData[i].time = primary->impulseTime(i);
        m_impulseData[i].value = NAN;
    }
}
void Union::calcVector(unsigned int count, const Shared::Source &primary) noexcept
{
    float coherence, coherenceWeight;
    Complex a, m, p;

    for (unsigned int i = 0; i < primary->frequencyDomainSize(); i++) {
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
                case Diff:
                case Subtract:
                    a -= (*it)->phase(i) * (*it)->module(i);
                    p -= (*it)->phase(i) * (*it)->peakSquared(i);
                    m -= (*it)->phase(i) * (*it)->magnitudeRaw(i);
                    break;
                case Max:
                    a = std::max(a, (*it)->phase(i) * (*it)->module(i));
                    p = std::max(p, (*it)->phase(i) * (*it)->peakSquared(i));
                    m = std::max(m, (*it)->phase(i) * (*it)->magnitudeRaw(i));
                    break;
                case Min:
                    a = std::min(a, (*it)->phase(i) * (*it)->module(i));
                    p = std::min(p, (*it)->phase(i) * (*it)->peakSquared(i));
                    m = std::min(m, (*it)->phase(i) * (*it)->magnitudeRaw(i));
                    break;
                case Apply: //work in calcApply
                    Q_ASSERT(false);
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

    if (primary->timeDomainSize() < 2) {
        return;
    }

    for (unsigned int i = 0; i < primary->timeDomainSize(); i++) {
        m_impulseData[i].time = primary->impulseTime(i);
        m_impulseData[i].value = primary->impulseValue(i);
    }
    float dt = m_impulseData[1].time - m_impulseData[0].time;

    for (unsigned int i = 0; i < primary->timeDomainSize(); i++) {
        for (auto it = m_sources.begin(); it != m_sources.end(); ++it) {
            if (!(*it)) {
                continue;
            }
            float st = (*it)->impulseTime(i);
            long offseted =  (long)i + (st - m_impulseData[i].time) / dt;

            if (*it && it != m_sources.begin() && offseted > 0 && offseted < timeDomainSize()) {
                switch (m_operation) {
                case Summation:
                case Avg:
                    m_impulseData[offseted].value += (*it)->impulseValue(i);
                    break;
                case Subtract:
                case Diff:
                    m_impulseData[offseted].value -= (*it)->impulseValue(i);
                    break;
                case Min:
                    m_impulseData[offseted].value = std::min(m_impulseData[offseted].value, Complex{(*it)->impulseValue(i)});
                    break;
                case Max:
                    m_impulseData[offseted].value = std::max(m_impulseData[offseted].value, Complex{(*it)->impulseValue(i)});
                    break;
                case Apply:
                    //calculated in calcApply
                    Q_ASSERT(false);
                    break;
                }
            }
        }

    }

    if (m_operation == Avg) {
        for (unsigned int i = 0; i < primary->timeDomainSize(); i++) {
            m_impulseData[i].value /= count;
        }
    }
}

void Union::calcdB(unsigned int count, const Shared::Source &primary) noexcept
{
    float magnitude, module, coherence, coherenceWeight;
    Complex phase;

    for (unsigned int i = 0; i < primary->frequencyDomainSize(); i++) {
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
                case Diff:
                case Subtract: {
                    magnitude -= (*it)->magnitude(i);
                    auto p = phase.real * (*it)->phase(i).real + phase.imag * (*it)->phase(i).imag;
                    auto sign = (phase.imag - (*it)->phase(i).imag > 0 ? 1 : -1);
                    phase.real = p / (phase.abs() * (*it)->phase(i).abs());
                    phase.imag = sign * std::sqrt(1 - phase.real * phase.real);
                    module -= 20.f * std::log10((*it)->module(i));
                }
                break;
                case Min:
                    magnitude = std::min(magnitude, (*it)->magnitude(i));
                    module = std::min(module, 20.f * std::log10((*it)->module(i)));
                    phase.polar(std::min(phase.arg(), (*it)->phase(i).arg()));
                    break;
                case Max:
                    magnitude = std::max(magnitude, (*it)->magnitude(i));
                    module = std::max(module, 20.f * std::log10((*it)->module(i)));
                    phase.polar(std::max(phase.arg(), (*it)->phase(i).arg()));
                    break;
                case Apply:
                    Q_ASSERT(false);
                    break;
                }

                coherence       += std::abs((*it)->module(i) * (*it)->coherence(i));
                coherenceWeight += std::abs((*it)->module(i));
            }
        }
        if (m_operation == Diff) {
            magnitude = std::abs(magnitude);
            phase.polar(std::abs(phase.arg()));
            module = std::abs(module);
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

    for (unsigned int i = 0; i < primary->timeDomainSize(); i++) {
        m_impulseData[i].time = primary->impulseTime(i);
        m_impulseData[i].value = NAN;
    }
}

void Union::calcPower(unsigned int count, const Shared::Source &primary) noexcept
{
    float magnitude, module, coherence, coherenceWeight;
    Complex phase;

    for (unsigned int i = 0; i < primary->frequencyDomainSize(); i++) {
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
                case Diff:
                case Subtract: {
                    magnitude -= std::pow((*it)->magnitudeRaw(i), 2);
                    auto p = phase.real * (*it)->phase(i).real + phase.imag * (*it)->phase(i).imag;
                    auto sign = (phase.imag - (*it)->phase(i).imag > 0 ? 1 : -1);
                    phase.real = p / (phase.abs() * (*it)->phase(i).abs());
                    phase.imag = sign * std::sqrt(1 - phase.real * phase.real);
                    module -= std::pow((*it)->module(i), 2);
                }
                break;
                case Min:
                    magnitude = std::min(magnitude, powf((*it)->magnitudeRaw(i), 2));
                    module = std::min(module, powf((*it)->module(i), 2));
                    phase.polar(std::min(phase.arg(), (*it)->phase(i).arg()));
                    break;
                case Max:
                    magnitude = std::max(magnitude, powf((*it)->magnitudeRaw(i), 2));
                    module = std::max(module, powf((*it)->module(i), 2));
                    phase.polar(std::max(phase.arg(), (*it)->phase(i).arg()));
                    break;
                case Apply:
                    Q_ASSERT(false);
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
        } else if (m_operation == Diff) {
            magnitude = std::abs(magnitude);
            phase.polar(std::abs(phase.arg()));
            module = std::abs(module);
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

    for (unsigned int i = 0; i < primary->timeDomainSize(); i++) {
        m_impulseData[i].time = primary->impulseTime(i);
        m_impulseData[i].value = NAN;
    }
}

void Union::calcApply(const Shared::Source &primary) noexcept
{
    float magnitude, module, coherence;
    Complex phase;

    for (unsigned int i = 0; i < primary->frequencyDomainSize(); i++) {
        magnitude   = primary->magnitudeRaw(i);
        phase       = primary->phase(i);
        module      = (primary)->module(i);
        coherence   = primary->coherence(i);

        for (auto it = m_sources.begin(); it != m_sources.end(); ++it) {
            if (*it && it != m_sources.begin()) {

                magnitude *= (*it)->magnitudeRaw(i);
                module    *= (*it)->magnitudeRaw(i);
                coherence  = std::min(coherence, (*it)->coherence(i));
                phase.polar(phase.arg() + (*it)->phase(i).arg());
            }
        }
        if (std::isnan(phase.real) || std::isnan(phase.imag)) {
            phase = {1, 0};
        }

        m_ftdata[i].frequency  = primary->frequency(i);
        m_ftdata[i].module     = module;
        m_ftdata[i].phase      = phase.normalize();
        m_ftdata[i].magnitude  = magnitude;
        m_ftdata[i].coherence  = coherence;
    }

    for (unsigned int i = 0; i < primary->timeDomainSize(); i++) {
        m_impulseData[i].time = primary->impulseTime(i);
        m_impulseData[i].value = NAN;
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

bool Union::checkLoop(Union *target) const
{
    if (target) {
        for (auto &source : sources()) {
            if (!source) {
                continue;
            }

            auto unionSource = std::dynamic_pointer_cast<Union>(source);
            if (!unionSource) {
                continue;
            }
            if ((unionSource.get() == target) || unionSource->checkLoop(target)) {
                emit Notifier::getInstance()->newMessage("Source loop detected:", target->name());
                return true;
            }
        }
    }
    return false;
}

const Union::SourceVector &Union::sources() const
{
    return m_sources;
}

void Union::applyAutoName() noexcept
{
    if (!m_autoName) {
        return;
    }
    try {
        setName(typeMap.at(m_type) + " " + operationMap.at(m_operation));
    } catch (std::exception &e) {
        qDebug() << __FILE__ << ":" << __LINE__  << e.what();
    }
}

void Union::sourceDestroyed(::Abstract::Source *source)
{
    std::lock_guard l(s_calcmutex);

    auto position = std::find_if(m_sources.begin(), m_sources.end(), [source](const auto & p) {
        return p.get() == source;
    });
    if (position != m_sources.end()) {
        auto index = std::distance(m_sources.begin(), position);
        setSource(index, Shared::Source{nullptr});
    }
}

QJsonObject Union::toJSON() const noexcept
{
    auto object = Abstract::Source::toJSON();

    object["count"]     = count();
    object["type"]      = type();
    object["operation"] = operation();
    object["autoName"]  = autoName();

    QJsonArray sources;
    for (int i = 0; i < count(); ++i) {
        auto source = getSource(i);
        if (source) {
            sources.append(source.uuid().toString());
        }
    }
    object["sources"] = sources;

    return object;
}
void Union::fromJSON(QJsonObject data, const SourceList *list) noexcept
{
    auto uuid = QUuid::fromString(data["uuid"].toString());
    if (!uuid.isNull()) {
        setUuid(uuid);
    }
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
            auto uuid = QUuid::fromString( sources[i].toString() );
            auto source = list->getByUUid(uuid);
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
Shared::Source Union::store()
{
    auto store = std::make_shared<Stored>();
    store->build( *this );
    store->autoName(name());

    try {
        QString notes = operationMap.at(m_operation) + " " + typeMap.at(m_type) + ":\n";
        for (auto &s : m_sources) {
            if (s) {
                notes += s->name() + "\t";
            }
        }
        store->setNotes(notes);
    } catch (std::exception &e) {
        qDebug() << __FILE__ << ":" << __LINE__  << e.what();
    }
    return { store };
}
