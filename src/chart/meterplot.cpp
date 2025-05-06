/**
 *  OSM
 *  Copyright (C) 2022  Pavel Smokotnin

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
#include "meterplot.h"
#include "sourcelist.h"
#include "measurement.h"//TODO: meta::measurement

namespace Chart {

const std::map<MeterPlot::Type, QString> MeterPlot::m_typesMap = {
    {MeterPlot::Type::RMS,   "RMS"  },
    {MeterPlot::Type::Peak,  "Peak" },
    {MeterPlot::Type::Crest, "Crest"},
    {MeterPlot::Type::THDN,  "THD+N"},
    {MeterPlot::Type::Time,  "Time" },
    {MeterPlot::Type::Leq,   "Leq"  },
    {MeterPlot::Type::Gain,  "Gain" },
    {MeterPlot::Type::Delay, "Delay"},
};

MeterPlot::MeterPlot(QObject *parent) : QObject(parent), LevelObject(),
    m_source(nullptr), m_sourceList(nullptr), m_settings(nullptr), m_timer(this), m_type(RMS), m_threshold(0),
    m_peakHold(false)
{
    m_timer.setInterval(1000);
    connect(this, &MeterPlot::curveChanged, this, &MeterPlot::updateThreshold);
    connect(&m_timer, &QTimer::timeout, this, &MeterPlot::timeReadyRead);

    connect(this, &MeterPlot::typeChanged, this, &MeterPlot::titleChanged);
    connect(this, &MeterPlot::modeChanged, this, &MeterPlot::titleChanged);
    connect(this, &MeterPlot::curveChanged, this, &MeterPlot::titleChanged);
    connect(this, &MeterPlot::timeChanged, this, &MeterPlot::titleChanged);

    connect(this, &MeterPlot::sourceChanged, this, &MeterPlot::sourceNameChanged);
    connect(this, &MeterPlot::typeChanged, this, &MeterPlot::sourceNameChanged);

    reset();
    updateThreshold();
}

QUuid MeterPlot::source() const
{
    return m_source ? m_source->uuid() : QUuid{};
}

void MeterPlot::setSource(QUuid sourceId)
{
    auto source = m_sourceList ? m_sourceList->getByUUid(sourceId) : Shared::Source{};
    if (m_source != source) {
        disconnect(m_sourceConnection);

        m_source = source;

        if (m_source) {
            m_sourceConnection = connect(m_source.get(), &Abstract::Source::readyRead, this, &MeterPlot::sourceReadyRead);
            connect(m_source.get(), &Abstract::Source::beforeDestroy, this, &MeterPlot::resetSource, Qt::DirectConnection);
            connect(m_source.get(), &Abstract::Source::nameChanged, this, &MeterPlot::sourceNameChanged);
        }
        emit sourceChanged(sourceId);
    }
}

QString MeterPlot::title() const
{
    switch (m_type) {
    case Time:
    case Gain:
    case Delay:
    case THDN:
        return typeName() + (m_peakHold ? " max" : "");;
    case Crest:
        return typeName() + " " + curveName() + " " + timeName();
    case Leq:
        return "L" + curveName() + "eq " + (m_peakHold ? "Max" : "") + timeName();
    default:
        return typeName() + " " + modeName() + " " + curveName() + " " + timeName();
    }
}

QString MeterPlot::value() const
{
    if (!m_source) {
        return "";
    }
    switch (m_type) {
    case Time:
        return timeValue();
    case THDN:
        return thdnValue();
    case Delay:
        return delayValue();
    default:
        return dBValue();
    }
}

QString MeterPlot::sourceName() const
{
    switch (m_type) {
    case Time:
        return "System";
    default:
        return (m_source ? m_source->name() : "");
    }
}

QString MeterPlot::dBValue() const
{
    float level = 0;
    switch (m_type) {
    case RMS:
        level = (mode() == SPL ? SPL_OFFSET : 0 ) + m_source->level(curve(), time());
        break;
    case Peak:
        level = (mode() == SPL ? SPL_OFFSET : 0 ) + m_source->peak(curve(), time());
        break;
    case Crest:
        level = m_source->peak(curve(), time()) - m_source->level(curve(), time());
        break;
    case Leq:
        level = m_leq.value() + SPL_OFFSET;
        break;
    case Gain:
        level = m_source->level(Weighting::Z, Meter::Slow) - m_source->referenceLevel();
        break;
    default:
        qDebug() << "not db value";
        Q_ASSERT(false);
        return "";
    }

    if (m_peakHold) {
        m_peakLevel = std::max(m_peakLevel, level);
        level = m_peakLevel;
    }

    return QString("%1").arg(level, 0, 'f', 1);
}

QString MeterPlot::timeValue() const
{
    return QTime::currentTime().toString("HH:mm");
}

QString MeterPlot::thdnValue() const
{
    auto level = m_source->level(Weighting::Curve::Z, Meter::Time::Slow) - m_source->referenceLevel();//Ref: Z Slow
    level = 100 * std::pow(10, level / 20);
    if (level < 1) {
        return QString("%1"   ).arg(level * 1000, 0, 'f', 0) + QString::fromUtf8("‰");
    } else {
        return QString("%1\%" ).arg(level, 0, 'f', 1);
    }
}

QString MeterPlot::delayValue() const
{
    if (auto measurement = std::dynamic_pointer_cast<Measurement>(m_source)) {
        auto delay = measurement->estimatedDelta();
        return QString("%1" ).arg(delay * 1000.f / measurement->sampleRate(), 0, 'f', 1);
    }
    return QString("N/A");
}

bool MeterPlot::peakHold() const
{
    return m_peakHold;
}

void MeterPlot::setPeakHold(bool newPeakHold)
{
    if (m_peakHold == newPeakHold)
        return;
    m_peakHold = newPeakHold;
    reset();
    emit peakHoldChanged();
    emit titleChanged();
}

void MeterPlot::reset()
{
    m_peakLevel = -std::numeric_limits<float>::infinity();
    emit valueChanged();
}

void MeterPlot::setSettings(Settings *newSettings)
{
    if (m_settings == newSettings)
        return;
    m_settings = newSettings;

    setSource(
        m_settings->reactValue<MeterPlot, QUuid>("source", this, &MeterPlot::sourceChanged, source()).toUuid()
    );
    setCurve(
        m_settings->reactValue<MeterPlot, QString>("curve", this, &MeterPlot::curveChanged, curveName()).toString()
    );
    setType(
        m_settings->reactValue<MeterPlot, QString>("type", this, &MeterPlot::typeChanged, typeName()).toString()
    );
    setMode(
        m_settings->reactValue<MeterPlot, Chart::LevelObject::Mode>("mode", this, &MeterPlot::modeChanged, mode()).toInt()
    );
    setThreshold(
        m_settings->reactValue<MeterPlot, float>("threshold", this, &MeterPlot::thresholdChanged, threshold()).toFloat()
    );
    setPause(
        m_settings->reactValue<MeterPlot, bool>("pause", this, &MeterPlot::pauseChanged, pause()).toBool()
    );
}

SourceList *MeterPlot::sourceList() const
{
    return m_sourceList;
}

void MeterPlot::setSourceList(SourceList *sourceList)
{
    if (m_sourceList == sourceList) {
        return;
    }
    m_sourceList = sourceList;
    emit sourceListChanged();

    if (sourceList && source().isNull()) {
        setSource(sourceList->firstSource());
    }
}

float MeterPlot::threshold() const
{
    return m_threshold;
}

void MeterPlot::setThreshold(float threshold)
{
    if (!qFuzzyCompare(m_threshold, threshold)) {
        m_threshold = threshold;
        emit thresholdChanged(m_threshold);
    }
}

QVariant MeterPlot::getAvailableTypes() const
{
    QStringList typeList;
    for (const auto &type : m_typesMap) {
        typeList << type.second;
    }
    return typeList;
}

QVariant MeterPlot::getAvailableTimes() const
{
    switch (m_type) {
    case Type::Leq:
        return math::Leq::availableTimes();
    default:
        break;
    }
    return LevelObject::getAvailableTimes();
}

QString MeterPlot::timeName() const
{
    switch (m_type) {
    case Type::Leq:
        return m_leq.timeName();
    default:
        break;
    }
    return LevelObject::timeName();
}

void MeterPlot::setTime(const QString &time)
{
    switch (m_type) {
    case Leq:
        m_leq.setTime(time);
        emit timeChanged(timeName());
        break;
    default:
        LevelObject::setTime(time);
        break;
    }
}

void MeterPlot::updateThreshold()
{
    switch (curve()) {

    case Weighting::A:
    case Weighting::B:
    case Weighting::K:
        setThreshold(99);
        break;

    case Weighting::C:
    case Weighting::Z:
        setThreshold(135);
        break;
    }
}

void MeterPlot::resetSource()
{
    setSource(nullptr);
}

void MeterPlot::sourceReadyRead()
{
    switch (m_type) {
    case RMS:
    case Peak:
    case Crest:
    case THDN:
    case Gain:
    case Delay:
        emit valueChanged();
        break;
    case Leq:
    case Time:
    {/*ignore*/}
    }
}

void MeterPlot::timeReadyRead()
{
    if (m_source && m_source->active() && m_type == Leq) {
        m_leq.addOneSecondValue(m_source->level(curve(), Meter::Time::Fast));
        emit valueChanged();
    } else if (m_type == Time) {
        emit valueChanged();
    }
}

const MeterPlot::Type &MeterPlot::type() const
{
    return m_type;
}

QString MeterPlot::typeName() const noexcept
{
    try {
        return m_typesMap.at(m_type);
    } catch (std::out_of_range) {}

    return "";
}

void MeterPlot::setType(const Type &type)
{
    if (m_type == type)
        return;
    m_type = type;

    emit typeChanged(typeName());
    emit valueChanged();
    emit timeChanged(timeName());

    if (m_type == Time || m_type == Leq) {
        m_timer.start();
    } else {
        m_timer.stop();
    }
}

void MeterPlot::setType(const QString &type)
{
    std::find_if(m_typesMap.cbegin(), m_typesMap.cend(),
    [&type, this](auto & e) {
        if (e.second == type) {
            setType(e.first);
            return true;
        }
        return false;
    });
}

} // namespace chart
