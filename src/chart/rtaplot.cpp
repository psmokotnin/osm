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

#include <QPainter>
#include <QPainterPath>

#include "rtaplot.h"
#include "targettrace.h"

using namespace Chart;

RTAPlot::RTAPlot(Settings *settings, QQuickItem *parent): FrequencyBasedPlot(settings, parent),
    m_mode(Chart::RTAPlot::Mode::Line), m_scale(Scale::DBfs), m_spline(false), m_showPeaks(true),
    m_targetTrace(new TargetTraceItem(m_palette, this))
{
    qRegisterMetaType<Chart::RTAPlot::Mode>();
    qRegisterMetaType<Chart::RTAPlot::Scale>();

    m_pointsPerOctave = 0;

    m_y.configure(AxisType::Linear, -140.f, 140.f,  15);
    m_y.setCentralLabel(m_y.min() - 1.f);
    m_y.setUnit("dB");
    updateAxis();
    setFlag(QQuickItem::ItemHasContents);

    connect(this, &RTAPlot::modeChanged, this, &RTAPlot::update);
    connect(this, &RTAPlot::showPeaksChanged, this, &RTAPlot::update);
    connect(this, &RTAPlot::pointsPerOctaveChanged, this, &RTAPlot::update);
    connect(this, &RTAPlot::scaleChanged, this, &RTAPlot::update);
    connect(this, &RTAPlot::scaleChanged, this, &RTAPlot::updateAxis);
}

void RTAPlot::updateAxis()
{
    switch (m_scale) {
    case DBfs:
        m_y.configure(AxisType::Linear, -140.f, 40.f,  29);
        m_y.setCentralLabel(m_y.min() - 1.f);
        m_y.setMax(0.f);
        m_y.setMin(-140.f);
        m_y.setUnit("dB");
        break;

    case SPL:
        m_y.configure(AxisType::Linear, 40.f, 142.f,  34);
        m_y.setCentralLabel(m_y.min() - 1.f);
        m_y.setMax(100);
        m_y.setMin(60);
        m_y.setUnit("dB");
        break;

    case Phon:
        m_y.configure(AxisType::Linear, 40.f, 142.f,  34);
        m_y.setCentralLabel(m_y.min() - 1.f);
        m_y.setMax(100);
        m_y.setMin(60);
        m_y.setUnit("ph");
        break;
    }
}

void RTAPlot::setMode(Chart::RTAPlot::Mode mode)
{
    if (m_mode == mode)
        return;
    m_mode = mode;
    emit modeChanged(m_mode);
}

void RTAPlot::setMode(unsigned int mode)
{
    setMode(static_cast<Chart::RTAPlot::Mode>(mode));
}

RTAPlot::Mode RTAPlot::mode()
{
    return m_mode;
}

void RTAPlot::setSettings(Settings *settings) noexcept
{
    if (settings && (settings->value("type") == "Spectrum")) {

        FrequencyBasedPlot::setSettings(settings);
        setScale(m_settings->reactValue<RTAPlot, Chart::RTAPlot::Scale>("scale", this, &RTAPlot::scaleChanged,
                                                                        m_scale).toUInt());
        setMode(m_settings->reactValue<RTAPlot, Chart::RTAPlot::Mode>("mode", this, &RTAPlot::modeChanged, m_mode).toUInt());
        setShowPeaks(m_settings->reactValue<RTAPlot, bool>("showPeaks", this, &RTAPlot::showPeaksChanged, m_mode).toBool());
    }
}
void RTAPlot::storeSettings() noexcept
{
    if (!m_settings)
        return;

    FrequencyBasedPlot::storeSettings();
    m_settings->setValue("mode", static_cast<int>(m_mode));
    m_settings->setValue("showPeaks", m_showPeaks);
    m_settings->setValue("scale", static_cast<int>(m_scale));
}

bool RTAPlot::showPeaks() const
{
    return m_showPeaks;
}

void RTAPlot::setShowPeaks(bool showPeaks)
{
    if (m_showPeaks != showPeaks) {
        m_showPeaks = showPeaks;
        emit showPeaksChanged(m_showPeaks);
    }
}

bool RTAPlot::isPointsPerOctaveValid(unsigned int &value) const
{
    return value >= 0 && value <= 48;
}

RTAPlot::Scale RTAPlot::scale() const
{
    return m_scale;
}

void RTAPlot::setScale(Scale newScale)
{
    if (m_scale == newScale)
        return;
    m_scale = newScale;
    emit scaleChanged(newScale);
}

void RTAPlot::setScale(unsigned int newScale)
{
    setScale(static_cast<Chart::RTAPlot::Scale>(newScale));
}


RTAPlot::TargetTraceItem::TargetTraceItem(const Palette &palette, QQuickItem *parent) : PaintedItem(parent),
    m_palette(palette)
{
    connect(parent, &QQuickItem::widthChanged, this, [this, parent]() {
        setWidth(parent->width());
    });
    connect(parent, &QQuickItem::heightChanged, this, [this, parent]() {
        setHeight(parent->height());
    });
    auto updateSlot = [this] () {
        update();
    };
    connect(reinterpret_cast<Plot *>(parent), &Plot::updated, this, updateSlot);
    connect(TargetTrace::getInstance(), &TargetTrace::changed, this, updateSlot);
    connect(TargetTrace::getInstance(), &TargetTrace::showChanged, this, updateSlot);
    connect(TargetTrace::getInstance(), &TargetTrace::activeChanged, this, updateSlot);
    connect(TargetTrace::getInstance(), &TargetTrace::offsetChanged, this, updateSlot);
    setWidth(parent->width());
    setHeight(parent->height());
    setZ(1);
}

void RTAPlot::TargetTraceItem::paint(QPainter *painter) noexcept
{
    auto target = TargetTrace::getInstance();
    if (!target->show() || !target->active() || !painter) {
        return;
    }

    std::lock_guard<std::mutex> guard(target->mutex());

    auto plot = static_cast<RTAPlot *>(parent());
    if (plot->scale() == RTAPlot::Scale::Phon) {
        return;
    }
    auto yOffset = heightf() - padding.bottom;
    auto offset = QPointF(0, target->width() / 2);
    auto scaleOffset = target->offset();
    if (plot->scale() == RTAPlot::Scale::DBfs) {
        scaleOffset -= 140;
    }

    QPen linePen(m_palette.lineColor(), 1);
    QColor color = target->color();
    color.setAlphaF(0.215);
    painter->setPen(linePen);
    painter->setBrush(color);

    auto convert = [this, plot, yOffset, scaleOffset] (const QPointF & values) {
        return QPointF(
                   plot->xAxis()->convert(values.x(), pwidth()) + padding.left,
                   yOffset - plot->yAxis()->convert(values.y() + scaleOffset, pheight())
               );
    };

    try {
        QPainterPath path;

        bool first = true;
        auto &points = target->points();
        for (auto &point : points ) {
            if (first) {
                path.moveTo(convert(point  + offset));
                first = false;
            } else {

                path.lineTo(convert(point + offset));
            }
        }

        for (auto it = points.crbegin(); it < points.crend(); ++it) {
            path.lineTo(convert(*it - offset));
        }
        path.lineTo(convert(points[0]  + offset));
        painter->drawPath(path);
    } catch (std::invalid_argument) {}
}
