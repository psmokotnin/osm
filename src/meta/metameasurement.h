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
#ifndef META_MEASUREMENT_H
#define META_MEASUREMENT_H

#include <QObject>
#include <QtQml>
#include "metabase.h"
#include "math/filter.h"
#include "math/windowfunction.h"
#include "chart/source.h"

namespace meta {

class Measurement : public Base
{
    Q_GADGET
    QML_ELEMENT

public:
    enum AverageType {Off, LPF, FIFO};
    Q_ENUM(AverageType)

    enum Mode {FFT10, FFT11, FFT12, FFT13, FFT14, FFT15, FFT16, LFT};
    Q_ENUM(Mode)
    Q_ENUM(Filter::Frequency)

    Measurement();

    QVariant getAvailableModes() const;
    QVariant getAvailableWindowTypes() const;

    bool polarity() const;
    void setPolarity(bool polarity);

    float gain() const;
    void setGain(float gain);

    meta::Measurement::Mode mode() const;
    void setMode(const Mode &mode);
    void setMode(QVariant mode);
    QString modeName() const;

    unsigned int dataChanel() const;
    void setDataChanel(unsigned int dataChanel);

    unsigned int referenceChanel() const;
    void setReferenceChanel(unsigned int referenceChanel);

    meta::Measurement::AverageType averageType() const;
    void setAverageType(const AverageType &averageType);
    void setAverageType(QVariant type);

    unsigned int average() const;
    void setAverage(unsigned int average);

    WindowFunction::Type windowFunctionType() const;
    void setWindowFunctionType(const WindowFunction::Type &windowFunctionType);
    void setWindowFunctionType(QVariant type);

    Filter::Frequency filtersFrequency() const;
    void setFiltersFrequency(const Filter::Frequency &filtersFrequency);
    void setFiltersFrequency(QVariant frequency);

    unsigned int delay() const;
    void setDelay(unsigned int delay);


    unsigned int sampleRate() const;
    void setSampleRate(unsigned int sampleRate);

    Q_INVOKABLE virtual void resetAverage() noexcept = 0;
    Q_INVOKABLE virtual chart::Source *store() noexcept = 0;

//virtual signals:
    virtual void polarityChanged(bool) = 0;
    virtual void gainChanged(float)  = 0;
    virtual void modeChanged(Mode)  = 0;
    virtual void averageChanged(unsigned int)  = 0;
    virtual void dataChanelChanged(unsigned int)  = 0;
    virtual void referenceChanelChanged(unsigned int)  = 0;
    virtual void averageTypeChanged(Measurement::AverageType)  = 0;
    virtual void windowFunctionTypeChanged(WindowFunction::Type)  = 0;
    virtual void filtersFrequencyChanged(Filter::Frequency) = 0;
    virtual void delayChanged(unsigned int) = 0;
    virtual void sampleRateChanged(unsigned int) = 0;

protected:
    bool m_polarity;
    float m_gain;
    unsigned int m_dataChanel, m_referenceChanel;
    unsigned int m_delay;
    std::atomic<unsigned int> m_average;
    unsigned int m_sampleRate;
    Mode m_mode;
    AverageType m_averageType;
    Filter::Frequency m_filtersFrequency;
    WindowFunction::Type m_windowFunctionType;

    static const std::map<Mode, QString> m_modeMap;
    static const std::map<Mode, int> m_FFTsizes;

};

} // namespace meta

#endif // META_MEASUREMENT_H
