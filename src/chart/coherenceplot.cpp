/**
 *  OSM
 *  Copyright (C) 2019  Pavel Smokotnin

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
#include "coherenceplot.h"

#include <QPainter>

using namespace Chart;

CoherencePlot::CoherencePlot(Settings *settings, QQuickItem *parent): FrequencyBasedPlot(settings, parent),
    m_threshold(0.91f), m_showThreshold(true), m_thresholdColor("#FF5722"), m_thresholdLine(this),
    m_type(Type::SNR)
{
    setType(Type::Normal);
    setFlag(QQuickItem::ItemHasContents);
}

unsigned int CoherencePlot::pointsPerOctave() const
{
    return m_pointsPerOctave;
}
float CoherencePlot::threshold() const
{
    return m_threshold;
}

void CoherencePlot::setThreshold(const float &coherenceThreshold)
{
    if (!qFuzzyCompare(m_threshold, coherenceThreshold)) {
        m_threshold = coherenceThreshold;
        emit thresholdChanged(m_threshold);
        m_thresholdLine.update();
    }
}

QColor CoherencePlot::thresholdColor() const
{
    return m_thresholdColor;
}

void CoherencePlot::setThresholdColor(const QColor &thresholdColor)
{
    if (thresholdColor != m_thresholdColor) {
        m_thresholdColor = thresholdColor;
        emit thresholdColorChanged(m_thresholdColor);
        m_thresholdLine.update();
    }
}

bool CoherencePlot::showThreshold() const
{
    return m_showThreshold;
}

void CoherencePlot::setShowThreshold(const bool &showThreshold)
{
    if (m_showThreshold != showThreshold) {
        m_showThreshold = showThreshold;
        emit showThresholdChanged(m_showThreshold);
        m_thresholdLine.setVisible(m_showThreshold);
        m_thresholdLine.update();
    }
}

void CoherencePlot::setPointsPerOctave(unsigned int p)
{
    if (m_pointsPerOctave == p)
        return;

    m_pointsPerOctave = p;
    emit pointsPerOctaveChanged(m_pointsPerOctave);
    update();
}

CoherencePlot::Type CoherencePlot::type() const
{
    return m_type;
}

void CoherencePlot::setType(const CoherencePlot::Type &type)
{
    if (m_type != type) {
        m_type = type;
        switch (m_type) {
        case Normal:
        case Squared: {
            std::vector<float> labels {0.f, 0.2f, 0.4f, 0.6f, 0.8f, 1.f};
            m_y.configure(AxisType::Linear, 0.f, 1.f);
            m_y.setCentralLabel(m_y.min() - 1.f);
            m_y.setLabels(labels);
            m_y.setUnit("");
            emit typeChanged(m_type);
            setThreshold(m_type == Normal ? THRESHOLD_NORMAL : THRESHOLD_SQUARED);
            break;
        }

        case SNR:
            m_y.configure(AxisType::Linear, -50.f, 50.f, 11);
            m_y.setReset(0.f, 50.f);
            m_y.reset();
            m_y.setUnit("dB");
            emit typeChanged(m_type);
            setThreshold(THRESHOLD_SNR);
            break;
        }
        update();
    }
}

void CoherencePlot::setType(const QVariant &type)
{
    setType(static_cast<Type>(type.toInt()));
}
void CoherencePlot::setSettings(Settings *settings) noexcept
{
    if (settings && (settings->value("type") == "Coherence")) {
        XYPlot::setSettings(settings);

        setType(
            m_settings->reactValue<CoherencePlot, CoherencePlot::Type>("ctype", this, &CoherencePlot::typeChanged, m_type));
        setPointsPerOctave(
            m_settings->reactValue<CoherencePlot, unsigned int>("pointsPerOctave", this,
                                                                &CoherencePlot::pointsPerOctaveChanged, m_pointsPerOctave).toUInt());
        setThreshold(
            m_settings->reactValue<CoherencePlot, float>("threshold", this,
                                                         &CoherencePlot::thresholdChanged, m_threshold).toFloat());
    }
}
void CoherencePlot::storeSettings() noexcept
{
    if (!m_settings)
        return;

    XYPlot::storeSettings();
    m_settings->setValue("ctype", m_type);
    m_settings->setValue("pointsPerOctave", m_pointsPerOctave);
    m_settings->setValue("coherenceThreshold", m_threshold);
}

CoherenceThresholdLine::CoherenceThresholdLine(QQuickItem *parent) : PaintedItem(parent)
{
    connect(parent, &QQuickItem::widthChanged, this, &CoherenceThresholdLine::parentWidthChanged);
    connect(parent, &QQuickItem::heightChanged, this, &CoherenceThresholdLine::parentHeightChanged);
    connect(reinterpret_cast<Plot *>(parent), &Plot::updated, this, [this] () {
        update();
    });
    setWidth(parent->width());
    setHeight(parent->height());
}

void Chart::CoherenceThresholdLine::paint(QPainter *painter)
{
    try {
        auto plot = reinterpret_cast<CoherencePlot *>(parent());
        auto y = heightf() - padding.bottom - plot->yAxis()->convert(plot->threshold(), pheight());
        QColor color(plot->thresholdColor());
        QPen linePen(color, 1);
        QPoint p1(padding.left, y),
               p2(width() - padding.right, y);

        painter->setRenderHints(QPainter::Antialiasing, true);
        painter->setPen(linePen);
        painter->drawLine(p1, p2);
    } catch (std::invalid_argument) {}
}

void CoherenceThresholdLine::parentWidthChanged()
{
    setWidth(parentItem()->width());
}

void CoherenceThresholdLine::parentHeightChanged()
{
    setHeight(parentItem()->height());
}
