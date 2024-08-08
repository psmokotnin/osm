/**
 *  OSM
 *  Copyright (C) 2018  Pavel Smokotnin

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
#include <algorithm>
#include "source/source_abstract.h"

namespace Source {

Abstract::Abstract(QObject *parent) : QObject(parent),
    m_dataMutex(), m_onReset(false),
    m_ftdata(),
    m_impulseData(),
    m_dataLength(0),
    m_deconvolutionSize(0),
    m_active(false),
    m_uuid(QUuid::createUuid())
{
    qRegisterMetaType<Source::Abstract *>("Source*");
}

Abstract::~Abstract()
{
}

bool Abstract::cloneable() const
{
    return true;
}
void Abstract::destroy()
{
    emit beforeDestroy(this);
    disconnect();
    //deleteLater();   //Schedules ~Source() from qml
}
void Abstract::setActive(bool active)
{
    if (m_active != active) {
        m_active = active;
        emit activeChanged();
    }
}
void Abstract::setName(QString name)
{
    if (m_name != name) {
        m_name = name;
        emit nameChanged(m_name);
    }
}
void Abstract::setColor(QColor color)
{
    if (m_color != color) {
        m_color = color;
        emit colorChanged(m_color);
    }
}

const unsigned int &Abstract::size() const noexcept
{
    return m_dataLength;
}
void Abstract::setGlobalColor(int globalValue)
{
    if (globalValue < 19) {
        m_color = Qt::GlobalColor(globalValue);
        emit colorChanged(m_color);
    }
}

void Abstract::setUuid(const QUuid &newUuid)
{
    m_uuid = newUuid;
}

QUuid Abstract::uuid() const
{
    return m_uuid;
}
const float &Abstract::frequency(const unsigned int &i) const noexcept
{
    if (i >= m_dataLength)
        return m_zero;
    return m_ftdata[i].frequency;
}
float Abstract::module(const unsigned int &i) const noexcept {
    if (i >= m_dataLength)
        return m_zero;
    return m_ftdata[i].module;
}
float Abstract::magnitude(const unsigned int &i) const noexcept
{
    if (i >= m_dataLength)
        return m_zero;
    return 20.f * log10f(m_ftdata[i].magnitude);
}
float Abstract::magnitudeRaw(const unsigned int &i) const noexcept
{
    if (i >= m_dataLength)
        return m_zero;
    return m_ftdata[i].magnitude;
}
complex Abstract::phase(const unsigned int &i) const noexcept
{
    if (i <= m_dataLength)
        return m_ftdata[i].phase;

    return m_ftdata[0].phase;
}
const float &Abstract::coherence(const unsigned int &i) const noexcept
{
    if (i >= m_dataLength)
        return m_zero;

    return m_ftdata[i].coherence;
}

const float &Abstract::peakSquared(const unsigned int &i) const noexcept
{
    if (i >= m_dataLength)
        return m_zero;

    return m_ftdata[i].peakSquared;
}

float Abstract::crestFactor(const unsigned int &i) const noexcept
{
    if (i >= m_dataLength)
        return -INFINITY;

    return 10.f * std::log10(m_ftdata[i].peakSquared / m_ftdata[i].meanSquared);
}

unsigned int Abstract::impulseSize() const noexcept
{
    return m_deconvolutionSize;
}
float Abstract::impulseTime(const unsigned int &i) const noexcept
{
    if (i >= m_deconvolutionSize)
        return m_zero;
    return m_impulseData[i].time;
}
float Abstract::impulseValue(const unsigned int &i) const noexcept
{
    if (i >= m_deconvolutionSize)
        return m_zero;
    return m_impulseData[i].value.real;
}
void Abstract::copy(FTData *dataDist, TimeData *timeDist)
{
    if (dataDist) {
        std::copy_n(m_ftdata.data(), size(), dataDist);
    }
    if (timeDist) {
        std::copy_n(m_impulseData.data(), impulseSize(), timeDist);
    }
}

void Abstract::copyFrom(size_t dataSize, size_t timeSize, Abstract::FTData *dataSrc,
                        Abstract::TimeData *timeSrc)
{
    m_dataLength = dataSize;
    m_deconvolutionSize = timeSize;
    m_ftdata.resize(m_dataLength);
    m_impulseData.resize(m_deconvolutionSize);

    std::copy_n(dataSrc, size(), m_ftdata.data());
    std::copy_n(timeSrc, impulseSize(), m_impulseData.data());
}

QJsonObject Abstract::toJSON(const SourceList *) const noexcept
{
    QJsonObject object;
    object["uuid"]      = uuid().toString();
    object["active"]    = active();
    object["name"]      = name();

    QJsonObject color;
    color["red"]    = m_color.red();
    color["green"]  = m_color.green();
    color["blue"]   = m_color.blue();
    color["alpha"]  = m_color.alpha();
    object["color"] = color;

    return object;
}

void Abstract::fromJSON(QJsonObject data, const SourceList *) noexcept
{
    auto uuid = QUuid::fromString(data["uuid"].toString());
    if (!uuid.isNull()) {
        setUuid(uuid);
    }
    setActive(data["active"].toBool(active()));
    setName(data["name"].toString());

    auto jsonColor = data["color"].toObject();
    QColor c(
        jsonColor["red"  ].toInt(0),
        jsonColor["green"].toInt(0),
        jsonColor["blue" ].toInt(0),
        jsonColor["alpha"].toInt(1));
    setColor(c);
}

float Abstract::level(const Weighting::Curve curve, const Meter::Time time) const
{
    if (m_levelsData.m_data.find({curve, time}) == m_levelsData.m_data.end()) {
        Q_ASSERT(false);
        return 0;
    }
    return m_levelsData.m_data.at({curve, time});
}

float Abstract::referenceLevel() const
{
    return m_levelsData.m_referenceLevel;
}

float Abstract::peak(const Weighting::Curve curve, const Meter::Time time) const
{
    if (m_levelsData.m_data.find({curve, time}) == m_levelsData.m_data.end()) {
        Q_ASSERT(false);
        return 0;
    }
    return m_levelsData.m_data.at({curve, time});//TODO: m_peakData ??
}

Abstract::Levels::Levels() : m_referenceLevel(0)
{
    for (auto &curve : Weighting::allCurves) {
        for (auto &time : Meter::allTimes) {
            Key   key   {curve, time};
            Meter meter {curve, time};
            m_data[key] = -INFINITY;
        }
    }
}

auto Abstract::Levels::begin()
{
    return m_data.begin();
}

auto Abstract::Levels::end()
{
    return m_data.end();
}

QJsonObject Abstract::levels()
{
    QJsonObject levels;
    for (auto &&[key, value] : m_levelsData) {

        auto curve = Weighting::curveName(key.curve);
        auto time = Meter::timeName(key.time);

        auto curveData = levels[curve].toObject();
        curveData[time] = level(key.curve, key.time);
        levels[curve] = curveData;
    }
    return levels;
}

void Abstract::setLevels(const QJsonObject &data)
{
    for (auto &&[key, value] : m_levelsData) {

        auto curve = Weighting::curveName(key.curve);
        auto time = Meter::timeName(key.time);

        auto curveData = data[curve].toObject();
        value = curveData[time].toDouble();
    }
}
}
