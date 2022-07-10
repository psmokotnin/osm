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
#ifndef REMOTE_MEASUREMENTITEM_H
#define REMOTE_MEASUREMENTITEM_H

#include <item.h>
#include "meta/metameasurement.h"

namespace remote {

class MeasurementItem : public remote::Item, public meta::Measurement
{
    Q_OBJECT
    //meta properties
    Q_PROPERTY(bool polarity READ polarity WRITE setPolarity NOTIFY polarityChanged)
    Q_PROPERTY(float gain READ gain WRITE setGain NOTIFY gainChanged)
    Q_PROPERTY(int dataChanel READ dataChanel WRITE setDataChanel NOTIFY dataChanelChanged)
    Q_PROPERTY(int referenceChanel READ referenceChanel WRITE setReferenceChanel NOTIFY
               referenceChanelChanged)
    Q_PROPERTY(int delay READ delay WRITE setDelay NOTIFY delayChanged)
    Q_PROPERTY(int average READ average WRITE setAverage NOTIFY averageChanged)
    Q_PROPERTY(meta::Measurement::Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(meta::Measurement::AverageType averageType READ averageType WRITE setAverageType NOTIFY averageTypeChanged)
    Q_PROPERTY(Filter::Frequency filtersFrequency READ filtersFrequency WRITE setFiltersFrequency NOTIFY
               filtersFrequencyChanged)
    Q_PROPERTY(WindowFunction::Type window READ windowFunctionType WRITE setWindowFunctionType NOTIFY
               windowFunctionTypeChanged)

    //constant meta properties
    Q_PROPERTY(QVariant modes READ getAvailableModes CONSTANT)
    Q_PROPERTY(QVariant windows READ getAvailableWindowTypes CONSTANT)

    Q_PROPERTY(int sampleRate READ sampleRate WRITE setSampleRate NOTIFY sampleRateChanged)

    Q_PROPERTY(int estimated READ estimated WRITE setEstimated NOTIFY estimatedChanged)
    Q_PROPERTY(int estimatedDelta READ estimatedDelta WRITE setEstimatedDelta NOTIFY estimatedChanged)

public:
    MeasurementItem(QObject *parent = nullptr);

    long estimated() const;
    void setEstimated(long estimated);

    long estimatedDelta() const;
    void setEstimatedDelta(long estimatedDelta);

    Q_INVOKABLE void resetAverage() noexcept override;
    Q_INVOKABLE chart::Source *store() noexcept override;

signals:
    void polarityChanged(bool) override;
    void gainChanged(float)  override;
    void modeChanged(meta::Measurement::Mode)  override;
    void averageChanged(unsigned int)  override;
    void dataChanelChanged(unsigned int)  override;
    void referenceChanelChanged(unsigned int)  override;
    void averageTypeChanged(meta::Measurement::AverageType)  override;
    void windowFunctionTypeChanged(WindowFunction::Type)  override;
    void filtersFrequencyChanged(Filter::Frequency) override;
    void delayChanged(int) override;
    void sampleRateChanged(unsigned int) override;

    void estimatedChanged();

private:
    long m_estimated, m_estimatedDelta;
};

} // namespace remote

#endif // REMOTE_MEASUREMENTITEM_H
