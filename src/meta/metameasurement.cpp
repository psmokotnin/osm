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
#include "metameasurement.h"

namespace Meta {

const std::map<Measurement::Mode, QString>Measurement::m_modeMap = {
    {Measurement::FFT10, "10"},
    {Measurement::FFT11, "11"},
    {Measurement::FFT12, "12"},
    {Measurement::FFT13, "13"},
    {Measurement::FFT14, "14"},
    {Measurement::FFT15, "15"},
    {Measurement::FFT16, "16"},
    {Measurement::LFT,   "LTW"}
};
const std::map<Measurement::InputFilter, QString>Measurement::m_inputFilterMap = {
    {Measurement::InputFilter::Z,     "Z"},
    {Measurement::InputFilter::A,     "A"},
    {Measurement::InputFilter::C,     "C"},
    {Measurement::InputFilter::Notch, "Notch"},
    {Measurement::InputFilter::BP100, "BPF 100"},
    {Measurement::InputFilter::LP200, "LPF 200"},
};
const std::map<Measurement::Mode, int>Measurement::m_FFTsizes = {
    {Measurement::FFT10, 10},
    {Measurement::FFT11, 11},
    {Measurement::FFT12, 12},
    {Measurement::FFT13, 13},
    {Measurement::FFT14, 14},
    {Measurement::FFT15, 15},
    {Measurement::FFT16, 16}
};

Measurement::Measurement() : Base(),
    m_polarity(false),
    m_gain(1.f), m_offset(1.f),
    m_dataChanel(0), m_referenceChanel(1), m_delay(0),
    m_average(1),
    m_mode(FFT14),
    m_inputFilter(InputFilter::Z),
    m_averageType(AverageType::LPF),
    m_filtersFrequency(Filter::Frequency::FourthHz),
    m_windowFunctionType(WindowFunction::Type::Hann)
{
    qRegisterMetaType<Filter::Frequency>();
    qRegisterMetaType<Meta::Measurement::Mode>();
    qRegisterMetaType<Meta::Measurement::AverageType>();
    qRegisterMetaType<Meta::Measurement::InputFilter>();
    qRegisterMetaType<WindowFunction::Type>();
}

bool Measurement::polarity() const
{
    return m_polarity;
}

void Measurement::setPolarity(bool polarity)
{
    if (m_polarity != polarity) {
        m_polarity = polarity;
        emit polarityChanged(m_polarity);
    }
}

float Measurement::gain() const
{
    return 20 * std::log10(m_gain);
}

void Measurement::setGain(float gain)
{
    gain = std::pow(10, gain / 20.f);
    if (!qFuzzyCompare(m_gain, gain)) {
        m_gain = gain;
        emit gainChanged(m_gain);
    }
}

float Measurement::offset() const
{
    return 20 * std::log10(m_offset);;
}

void Measurement::setOffset(float newOffset)
{
    newOffset = std::pow(10, newOffset / 20.f);
    if (qFuzzyCompare(m_offset, newOffset))
        return;
    m_offset = newOffset;
    emit offsetChanged(m_offset);
}

QVariant Measurement::getAvailableModes()
{
    QStringList typeList;
    for (const auto &type : m_modeMap) {
        typeList << type.second;
    }
    return typeList;
}

QVariant Measurement::getAvailableWindowTypes()
{
    return WindowFunction::getTypes();
}

QVariant Measurement::getAvailableInputFilters()
{
    QStringList typeList;
    for (const auto &type : m_inputFilterMap) {
        typeList << type.second;
    }
    return typeList;
}

Measurement::Mode Measurement::mode() const
{
    return m_mode;
}

void Measurement::setMode(const Mode &mode)
{
    if (m_mode != mode) {
        m_mode = mode;
        emit modeChanged(m_mode);
    }
}

void Measurement::setMode(QVariant mode)
{
    setMode(mode.value<Mode>());
}

QString Measurement::modeName() const
{
    try {
        return m_modeMap.at(mode());
    } catch (std::exception &e) {
        qDebug() << __FILE__ << ":" << __LINE__  << e.what();
    }
    Q_ASSERT(false);
    return "";
}

unsigned int Measurement::dataChanel() const
{
    return m_dataChanel;
}

void Measurement::setDataChanel(unsigned int dataChanel)
{
    if (dataChanel != m_dataChanel) {
        m_dataChanel = dataChanel;
        emit dataChanelChanged(m_dataChanel);
    }
}

unsigned int Measurement::referenceChanel() const
{
    return m_referenceChanel;
}

void Measurement::setReferenceChanel(unsigned int referenceChanel)
{
    if (referenceChanel != m_referenceChanel) {
        m_referenceChanel = referenceChanel;
        emit referenceChanelChanged(m_referenceChanel);
    }
}

Measurement::AverageType Measurement::averageType() const
{
    return m_averageType;
}

void Measurement::setAverageType(const AverageType &averageType)
{
    if (m_averageType != averageType) {
        m_averageType = averageType;
        emit averageTypeChanged(m_averageType);
    }
}

void Measurement::setAverageType(QVariant type)
{
    setAverageType(static_cast<AverageType>(type.toInt()));
}

unsigned int Measurement::average() const
{
    return m_average;
}

void Measurement::setAverage(unsigned int average)
{
    if (m_average != average) {
        m_average = average;
        emit averageChanged(m_average);
    }
}

WindowFunction::Type Measurement::windowFunctionType() const
{
    return m_windowFunctionType;
}

void Measurement::setWindowFunctionType(const WindowFunction::Type &windowFunctionType)
{
    if (m_windowFunctionType != windowFunctionType) {
        m_windowFunctionType = windowFunctionType;
        emit windowFunctionTypeChanged(m_windowFunctionType);
    }
}

void Measurement::setWindowFunctionType(QVariant type)
{
    setWindowFunctionType(static_cast<WindowFunction::Type>(type.toInt()));
}

Filter::Frequency Measurement::filtersFrequency() const
{
    return m_filtersFrequency;
}

void Measurement::setFiltersFrequency(const Filter::Frequency &filtersFrequency)
{
    if (m_filtersFrequency != filtersFrequency) {
        m_filtersFrequency = filtersFrequency;
        emit filtersFrequencyChanged(m_filtersFrequency);
    }
}

void Measurement::setFiltersFrequency(QVariant frequency)
{
    setFiltersFrequency(static_cast<Filter::Frequency>(frequency.toInt()));
}

int Measurement::delay() const
{
    return m_delay;
}

void Measurement::setDelay(int delay)
{
    if (m_delay != delay) {
        m_delay = delay;
        emit delayChanged(m_delay);
    }
}

Meta::Measurement::Measurement::InputFilter Measurement::inputFilter() const
{
    return m_inputFilter;
}

void Measurement::setInputFilter(InputFilter inputFilter)
{
    if (m_inputFilter == inputFilter) {
        return;
    }

    m_inputFilter = inputFilter;
    emit inputFilterChanged(m_inputFilter);
}

void Measurement::setInputFilter(QVariant inputFilter)
{
    setInputFilter(static_cast<InputFilter>(inputFilter.toInt()));
}

} // namespace meta
