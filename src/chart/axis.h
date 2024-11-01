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
#ifndef CHART_AXIS_H
#define CHART_AXIS_H
#include "painteditem.h"
#include "palette.h"

#include <optional>
#include <cmath>

namespace Chart {

enum AxisType {Linear, Logarithmic};
enum AxisDirection {Vertical, Horizontal};

class Axis : public PaintedItem
{
    Q_OBJECT
    static std::vector<float> ISO_LABELS;

public:
    Axis(AxisDirection d, const Palette &palette, QQuickItem *parent = Q_NULLPTR);
    void paint(QPainter *painter) noexcept override;

    float convert(float value, float size) const;
    float reverse(float value, float size, float max = NAN, float min = NAN) const noexcept;
    float coordToValue(float coord) const noexcept;
    qreal coordToValue(qreal coord) const noexcept;

    void configure(AxisType type, float min, float max, unsigned int ticks = 0, float scale = 1.0f);
    void reset();
    void setReset(float min, float max);

    void beginGesture();
    bool applyGesture(qreal base, qreal move, qreal scale);

    float lowLimit() const noexcept;
    float highLimit() const noexcept;

    void setMin(float v);
    float min() const noexcept;

    void setScale(float v) noexcept;
    float scale() const noexcept;

    void setMax(float v) noexcept;
    float max() const noexcept;

    void setISOLabels() noexcept;
    void autoLabels(unsigned int ticks);
    void setLabels(std::vector<float> labels) noexcept;

    void setType(AxisType t) noexcept;
    AxisType type() const noexcept;

    float offset() const noexcept;
    void setOffset(float offset) noexcept;

    float centralLabel() const noexcept;
    void setCentralLabel(float central) noexcept;

    void setPeriodic(float p);
    float period() const noexcept;

    void setPeriodStart(float start);
    float periodStart() const noexcept;

    QString unit() const;
    void setUnit(const QString &unit);

    float helperValue() const;
    void setHelperValue(float helperValue);

public slots:
    void needUpdate();
    void parentWidthChanged();
    void parentHeightChanged();

signals:
    void minChanged(float);
    void maxChanged(float);
    void unitChanged(QString);

private:
    float fromPeriodicValue(const float &value) const noexcept;

    AxisType m_type = Linear;
    AxisDirection m_direction;
    const Palette &m_palette;
    std::vector<float> m_labels;
    float m_min, m_max, m_scale;
    float m_lowLimit, m_highLimit;
    float m_offset, m_centralLabel;
    float m_helperValue;
    std::optional<float> m_period;
    float m_periodStart;
    QString m_unit;

    struct {
        float min = 0;
        float max = 0;
    } m_store, m_reset;
};
}
#endif // CHART_AXIS_H
