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
#ifndef XYPLOT_H
#define XYPLOT_H

#include "plot.h"

namespace Chart {
class XYPlot : public Plot
{
    Q_OBJECT

    Q_PROPERTY(float xmin READ xmin() WRITE setXMin() NOTIFY xminChanged)
    Q_PROPERTY(float xmax READ xmax() WRITE setXMax() NOTIFY xmaxChanged)
    Q_PROPERTY(float ymin READ ymin() WRITE setYMin() NOTIFY yminChanged)
    Q_PROPERTY(float ymax READ ymax() WRITE setYMax() NOTIFY ymaxChanged)

    Q_PROPERTY(float xLowLimit  READ xLowLimit() CONSTANT)
    Q_PROPERTY(float xHighLimit READ xHighLimit() CONSTANT)
    Q_PROPERTY(float yLowLimit  READ yLowLimit() CONSTANT)
    Q_PROPERTY(float yHighLimit READ yHighLimit() CONSTANT)

    Q_PROPERTY(float xScale READ xScale() CONSTANT)
    Q_PROPERTY(float yScale READ yScale() CONSTANT)

protected:
    Axis m_x, m_y;

    float xLowLimit()  const
    {
        return m_x.lowLimit();
    }
    float xHighLimit() const
    {
        return m_x.highLimit();
    }
    float yLowLimit()  const
    {
        return m_y.lowLimit();
    }
    float yHighLimit() const
    {
        return m_y.highLimit();
    }

    float xmin() const
    {
        return m_x.min();
    }
    float xmax() const
    {
        return m_x.max();
    }
    float ymin() const
    {
        return m_y.min();
    }
    float ymax() const
    {
        return m_y.max();
    }

    float xScale() const
    {
        return m_x.scale();
    }
    float yScale() const
    {
        return m_y.scale();
    }

    void setXMin(float v);
    void setXMax(float v);
    void setYMin(float v);
    void setYMax(float v);

public:
    XYPlot(Settings *settings, QQuickItem *parent);

    Axis *xAxis()
    {
        return &m_x;
    }
    Axis *yAxis()
    {
        return &m_y;
    }

    Q_INVOKABLE virtual void setHelper(qreal x, qreal y) noexcept override;
    Q_INVOKABLE virtual void unsetHelper() noexcept override;
    Q_INVOKABLE qreal x2v(qreal mouseX) const noexcept override;
    Q_INVOKABLE qreal y2v(qreal mouseY) const noexcept override;
    Q_INVOKABLE virtual void beginGesture();
    Q_INVOKABLE virtual void applyGesture(QPointF base, QPointF move, QPointF scale);
    Q_INVOKABLE virtual void resetAxis();

    QString xLabel() const override;
    QString yLabel() const override;
    virtual void setSettings(Settings *settings) noexcept override;
    virtual void storeSettings() noexcept override;
    virtual void inheritSettings(const Plot *source) override;

signals:
    void xminChanged(float);
    void xmaxChanged(float);
    void yminChanged(float);
    void ymaxChanged(float);
};
}

#endif // XYPLOT_H
