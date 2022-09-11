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
#include "source.h"

using namespace chart;
Source::Source(QObject *parent) : QObject(parent),
    m_dataMutex(), m_onReset(false),
    m_ftdata(nullptr),
    m_impulseData(nullptr),
    m_dataLength(0),
    m_deconvolutionSize(0),
    m_active(false),
    m_uuid(QUuid::createUuid())
{
}

bool Source::cloneable() const
{
    return true;
}
void Source::destroy()
{
    emit beforeDestroy(this);
    disconnect();
    deleteLater();   //Schedules ~Source() from qml
}
void Source::setActive(bool active)
{
    if (m_active != active) {
        m_active = active;
        emit activeChanged();
    }
}
void Source::setName(QString name)
{
    if (m_name != name) {
        m_name = name;
        emit nameChanged(m_name);
    }
}
void Source::setColor(QColor color)
{
    if (m_color != color) {
        m_color = color;
        emit colorChanged(m_color);
    }
}

const unsigned int &Source::size() const noexcept
{
    return m_dataLength;
}
void Source::setGlobalColor(int globalValue)
{
    if (globalValue < 19) {
        m_color = Qt::GlobalColor(globalValue);
        emit colorChanged(m_color);
    }
}

QUuid Source::uuid() const
{
    return m_uuid;
}
const float &Source::frequency(const unsigned int &i) const noexcept
{
    if (i >= m_dataLength)
        return m_zero;
    return m_ftdata[i].frequency;
}
float Source::module(const unsigned int &i) const noexcept {
    if (i >= m_dataLength)
        return m_zero;
    return m_ftdata[i].module;
}
float Source::magnitude(const unsigned int &i) const noexcept
{
    if (i >= m_dataLength)
        return m_zero;
    return 20.f * log10f(m_ftdata[i].magnitude);
}
float Source::magnitudeRaw(const unsigned int &i) const noexcept
{
    if (i >= m_dataLength)
        return m_zero;
    return m_ftdata[i].magnitude;
}
complex Source::phase(const unsigned int &i) const noexcept
{
    if (i <= m_dataLength)
        return m_ftdata[i].phase;

    return m_ftdata[0].phase;
}
const float &Source::coherence(const unsigned int &i) const noexcept
{
    if (i >= m_dataLength)
        return m_zero;

    return m_ftdata[i].coherence;
}

const float &Source::peakSquared(const unsigned int &i) const noexcept
{
    if (i >= m_dataLength)
        return m_zero;

    return m_ftdata[i].peakSquared;
}

float Source::crestFactor(const unsigned int &i) const noexcept
{
    if (i >= m_dataLength)
        return -INFINITY;

    return 10.f * std::log10(m_ftdata[i].peakSquared / m_ftdata[i].meanSquared);
}

unsigned int Source::impulseSize() const noexcept
{
    return m_deconvolutionSize;
}
float Source::impulseTime(const unsigned int &i) const noexcept
{
    if (i >= m_deconvolutionSize)
        return m_zero;
    return m_impulseData[i].time;
}
float Source::impulseValue(const unsigned int &i) const noexcept
{
    if (i >= m_deconvolutionSize)
        return m_zero;
    return m_impulseData[i].value.real;
}
void Source::copy(FTData *dataDist, TimeData *timeDist)
{
    std::copy_n(m_ftdata, size(), dataDist);
    std::copy_n(m_impulseData, impulseSize(), timeDist);
}

void Source::copyFrom(size_t dataSize, size_t timeSize, Source::FTData *dataSrc,
                      Source::TimeData *timeSrc)
{
    if (m_ftdata) {
        delete[] m_ftdata;
    }
    if (m_impulseData) {
        delete[] m_impulseData;
    }

    m_dataLength = dataSize;
    m_deconvolutionSize = timeSize;
    m_ftdata = new FTData[m_dataLength];
    m_impulseData = new TimeData[m_deconvolutionSize];

    std::copy_n(dataSrc, size(), m_ftdata);
    std::copy_n(timeSrc, impulseSize(), m_impulseData);
}

float Source::level(const Weighting::Curve curve, const Meter::Time time) const
{
    if (m_levelsData.m_data.find({curve, time}) == m_levelsData.m_data.end()) {
        Q_ASSERT(false);
        return 0;
    }
    return m_levelsData.m_data.at({curve, time});
}


Source::Levels::Levels()
{
    for (auto &curve : Weighting::allCurves) {
        for (auto &time : Meter::allTimes) {
            Key   key   {curve, time};
            Meter meter {curve, time};
            m_data[key] = -INFINITY;
        }
    }
}

auto Source::Levels::begin()
{
    return m_data.begin();
}

auto Source::Levels::end()
{
    return m_data.end();
}

QJsonObject Source::levels()
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

void Source::setLevels(const QJsonObject &data)
{
    for (auto &&[key, value] : m_levelsData) {

        auto curve = Weighting::curveName(key.curve);
        auto time = Meter::timeName(key.time);

        auto curveData = data[curve].toObject();
        value = curveData[time].toDouble();
    }
}
