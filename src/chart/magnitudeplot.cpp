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
#include "magnitudeplot.h"
#include "targettrace.h"
#include <QPainter>
#include <QPainterPath>

using namespace chart;

MagnitudePlot::MagnitudePlot(Settings *settings, QQuickItem *parent) :
    FrequencyBasedPlot(settings, parent), m_invert(false), m_mode(Linear)
{
    setMode(dB);
    setFlag(QQuickItem::ItemHasContents);
    m_targetTrace = new TargetTraceItem(m_palette, this);
}
void MagnitudePlot::setSettings(Settings *settings) noexcept
{
    if (settings && (settings->value("type") == "Magnitude")) {
        FrequencyBasedPlot::setSettings(settings);

        setMode(m_settings->reactValue<MagnitudePlot, MagnitudePlot::Mode>(
                    "mode", this, &MagnitudePlot::modeChanged, m_mode).toInt());
    }
}
void MagnitudePlot::storeSettings() noexcept
{
    if (!m_settings)
        return;

    FrequencyBasedPlot::storeSettings();
    m_settings->setValue("mode", m_mode);
}
bool MagnitudePlot::invert() const
{
    return m_invert;
}
void MagnitudePlot::setInvert(bool invert)
{
    if (m_invert == invert)
        return;

    m_invert = invert;
    emit invertChanged(m_invert);
    update();
}

MagnitudePlot::Mode MagnitudePlot::mode() const
{
    return m_mode;
}

void MagnitudePlot::setMode(const MagnitudePlot::Mode &mode)
{
    if (m_mode != mode) {
        m_mode = mode;

        switch (m_mode) {
        case dB:
            m_y.configure(AxisType::Linear, -36.f, 36.f,  25);
            m_y.setReset(-18.f, 18.f);
            m_y.reset();
            m_y.setUnit("dB");
            break;

        case Linear:
            m_y.configure(AxisType::Linear, 0.f, 200.f,  20);
            m_y.setReset(0.f, 100.f);
            m_y.reset();
            m_y.setUnit("");
            break;
        }
        update();
        emit modeChanged(m_mode);
    }
}

void MagnitudePlot::setMode(const int &mode)
{
    setMode(static_cast<Mode>(mode));
}

MagnitudePlot::TargetTraceItem::TargetTraceItem(const Palette &palette, QQuickItem *parent) : PaintedItem(parent),
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
    setWidth(parent->width());
    setHeight(parent->height());
    setZ(1);
}

void MagnitudePlot::TargetTraceItem::paint(QPainter *painter) noexcept
{
    auto target = TargetTrace::getInstance();
    if (!target->show() || !target->active()) {
        return;
    }
    auto plot = static_cast<MagnitudePlot *>(parent());
    auto yOffset = heightf() - padding.bottom;
    auto offset = QPointF(0, target->width() / 2);
    QPen linePen(m_palette.lineColor(), 1);
    QColor color = target->color();
    color.setAlphaF(0.215);
    painter->setPen(linePen);
    painter->setBrush(color);

    std::array<QPair<QPointF, QPointF>, TargetTrace::SEGMENT_COUNT> segments = {
        {
            {QPointF(20,               target->start(0)), QPointF(target->point(0), target->end(0))},
            {QPointF(target->point(0), target->start(1)), QPointF(target->point(1), target->end(1))},
            {QPointF(target->point(1), target->start(2)), QPointF(20'000,           target->end(2))}
        }
    };

    auto convert = [this, plot, &yOffset] (const QPointF & values) {
        return QPointF(
                   plot->xAxis()->convert(values.x(), pwidth()) + padding.left,
                   yOffset - plot->yAxis()->convert(values.y(), pheight())
               );
    };

    for (auto &segment : segments) {
        QPainterPath path;
        path.moveTo(convert(segment.first  + offset));
        path.lineTo(convert(segment.second + offset));
        path.lineTo(convert(segment.second - offset));
        path.lineTo(convert(segment.first  - offset));
        path.lineTo(convert(segment.first  + offset));
        painter->drawPath(path);
    }
}
