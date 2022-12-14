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

const std::map<MeterPlot::Type, QString> MeterPlot::m_typesMap = {
    {MeterPlot::Type::RMS,   "RMS"  },
    {MeterPlot::Type::Peak,  "Peak" },
    {MeterPlot::Type::Crest, "Crest"},
};

MeterPlot::MeterPlot(QObject *parent) : QObject(parent), LevelObject(),
    m_source(nullptr), m_type(RMS), m_threshold(0)
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

        if (m_source) {
            m_sourceConnection = connect(m_source, &chart::Source::readyRead, this, &MeterPlot::valueChanged);
            connect(m_source, &chart::Source::beforeDestroy, this, &MeterPlot::resetSource, Qt::DirectConnection);
        }
        emit sourceChanged(m_source);
    }
}

QString MeterPlot::value() const
{
    if (!m_source) {
        return "";
    }
    return dBValue();
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
    default:
        qDebug() << "not db value";
        Q_ASSERT(false);
        return "";
    }

    return QString("%1").arg(level, 0, 'f', 1);
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
    emit typeChanged();
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
