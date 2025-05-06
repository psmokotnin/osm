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
#include "measurementitem.h"

namespace remote {

MeasurementItem::MeasurementItem(QObject *parent) : Item(parent), Meta::Measurement(),
    m_estimated(0), m_estimatedDelta(0)
{
    setObjectName("RemoteMeasurement");
}

long MeasurementItem::estimated() const
{
    return m_estimated;
}

void MeasurementItem::setEstimated(long estimated)
{
    m_estimated = estimated;
}

long MeasurementItem::estimatedDelta() const
{
    return m_estimatedDelta;
}

void MeasurementItem::setEstimatedDelta(long estimatedDelta)
{
    m_estimatedDelta = estimatedDelta;
}

void MeasurementItem::resetAverage() noexcept
{
    emit sendCommand("resetAverage");
}

Shared::Source MeasurementItem::store()
{
    emit sendCommand("store");
    return { nullptr };
}

void MeasurementItem::applyAutoGain(const float reference)
{
    emit sendCommand("applyAutoGain", reference);
}

} // namespace remote
