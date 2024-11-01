/**
 *  OSM
 *  Copyright (C) 2023  Pavel Smokotnin

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

#include "metawindowing.h"

namespace Meta {

const std::map<Windowing::Mode, QString>Windowing::m_modeMap = {
    {Windowing::Mode::FFT8,  "FFT 8"},
    {Windowing::Mode::FFT9,  "FFT 9"},
    {Windowing::Mode::FFT10, "FFT 10"},
    {Windowing::Mode::FFT11, "FFT 11"},
    {Windowing::Mode::FFT12, "FFT 12"},
    {Windowing::Mode::FFT13, "FFT 13"},
    {Windowing::Mode::FFT14, "FFT 14"},
    {Windowing::Mode::FFT15, "FFT 15"},
    {Windowing::Mode::FFT16, "FFT 16"},
    {Windowing::Mode::LTW1,  "LTW1"},
    {Windowing::Mode::LTW2,  "LTW2"},
    {Windowing::Mode::LTW3,  "LTW3"},
};
const std::map<Windowing::Mode, int>Windowing::m_FFTsizes = {
    {Windowing::Mode::FFT8,   8},
    {Windowing::Mode::FFT9,   9},
    {Windowing::Mode::FFT10, 10},
    {Windowing::Mode::FFT11, 11},
    {Windowing::Mode::FFT12, 12},
    {Windowing::Mode::FFT13, 13},
    {Windowing::Mode::FFT14, 14},
    {Windowing::Mode::FFT15, 15},
    {Windowing::Mode::FFT16, 16},
    {Windowing::Mode::LTW1,  16},
    {Windowing::Mode::LTW2,  16},
    {Windowing::Mode::LTW3,  16},
};

Windowing::Windowing() : m_wide(1), m_offset(0),
    m_minFrequency(20), m_maxFrequency(20000),
    m_mode(FFT10), m_domain(SourceDomain::Time),
    m_windowFunctionType(WindowFunction::Type::Hann)
{
}

QVariant Windowing::getAvailableModes()
{
    QStringList typeList;
    for (const auto &type : m_modeMap) {
        typeList << type.second;
    }
    return typeList;
}

QVariant Windowing::getAvailableWindowTypes()
{
    return WindowFunction::getTypes();
}

const WindowFunction::Type &Windowing::windowFunctionType() const
{
    return m_windowFunctionType;
}

void Windowing::setWindowFunctionType(const QVariant &newWindowFunctionType)
{
    setWindowFunctionType(newWindowFunctionType.value<WindowFunction::Type>());
}

void Windowing::setWindowFunctionType(const WindowFunction::Type &newWindowFunctionType)
{
    if (m_windowFunctionType == newWindowFunctionType)
        return;
    m_windowFunctionType = newWindowFunctionType;
    emit windowFunctionTypeChanged(m_windowFunctionType);
}

Windowing::SourceDomain Windowing::domain() const
{
    return m_domain;
}

void Windowing::setDomain(SourceDomain newDomain)
{
    if (m_domain == newDomain) {
        return;
    }
    m_domain = newDomain;
    emit domainChanged(m_domain);
}

float Windowing::minFrequency() const
{
    return m_minFrequency;
}

void Windowing::setMinFrequency(float newMinFrequency)
{
    if (qFuzzyCompare(m_minFrequency, newMinFrequency))
        return;
    m_minFrequency = newMinFrequency;
    emit minFrequencyChanged();
}

float Windowing::maxFrequency() const
{
    return m_maxFrequency;
}

void Windowing::setMaxFrequency(float newMaxFrequency)
{
    if (qFuzzyCompare(m_maxFrequency, newMaxFrequency))
        return;
    m_maxFrequency = newMaxFrequency;
    emit maxFrequencyChanged();
}

void Windowing::setDomain(QVariant newDomain)
{
    setDomain(newDomain.value<SourceDomain>());
}

Windowing::Mode Windowing::mode() const
{
    return m_mode;
}

QString Windowing::modeName() const
{
    return m_modeMap.at(mode());
}

void Windowing::setMode(Mode newMode)
{
    if (m_mode == newMode)
        return;
    m_mode = newMode;
    emit modeChanged(m_mode);
}

void Windowing::setMode(QVariant newMode)
{
    setMode(newMode.value<Mode>());
}

const QString &Windowing::tipName() const
{
    return m_tipName;
}

void Windowing::setTipName(const QString &newTipName)
{
    if (m_tipName == newTipName)
        return;
    m_tipName = newTipName;
    emit tipNameChanged(m_tipName);
}

float Windowing::offset() const
{
    return m_offset;
}

void Windowing::setOffset(float newOffset)
{
    if (qFuzzyCompare(m_offset, newOffset))
        return;
    m_offset = newOffset;
    emit offsetChanged(m_offset);
}

float Windowing::wide() const
{
    return m_wide;
}

void Windowing::setWide(float newWide)
{
    if (qFuzzyCompare(m_wide, newWide))
        return;
    m_wide = newWide;
    emit wideChanged(m_wide);
}

} // namespace meta
