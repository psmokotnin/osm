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

namespace chart {

MeterPlot::MeterPlot(QObject *parent) : QObject(parent), LevelObject(), m_source(nullptr), m_threshold(0)
{
    connect(this, &MeterPlot::curveChanged, this, &MeterPlot::updateThreshold);
    updateThreshold();
}

chart::Source *MeterPlot::source() const
{
    return m_source;
}

void MeterPlot::setSource(chart::Source *source)
{
    if (m_source != source) {
        disconnect(m_sourceConnection);

        m_source = source;
        m_sourceConnection = connect(m_source, &chart::Source::readyRead, this, &MeterPlot::valueChanged);

        emit sourceChanged(m_source);
    }
}

float MeterPlot::value() const
{
    if (!m_source) {
        return NAN;
    }
    switch (mode()) {
    case dBfs:
        return m_source->level(curve(), time());

    case SPL:
        return SPL_OFFSET +  m_source->level(curve(), time());
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

} // namespace chart
