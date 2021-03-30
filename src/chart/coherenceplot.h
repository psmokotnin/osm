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
#ifndef COHERENCEPLOT_H
#define COHERENCEPLOT_H

#include "xyplot.h"

namespace Fftchart {

class CoherenceThresholdLine : public PaintedItem
{
public:
    CoherenceThresholdLine(QQuickItem *parent);
    virtual void paint(QPainter *painter) override;
    void parentWidthChanged();
    void parentHeightChanged();
};

class CoherencePlot : public XYPlot
{
    Q_OBJECT
    Q_PROPERTY(unsigned int pointsPerOctave READ pointsPerOctave WRITE setPointsPerOctave NOTIFY
               pointsPerOctaveChanged)
    Q_PROPERTY(Type type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(QColor thresholdColor READ thresholdColor WRITE setThresholdColor NOTIFY thresholdColorChanged)
    Q_PROPERTY(float threshold READ threshold WRITE setThreshold NOTIFY
               thresholdChanged)

public:
    enum Type {Normal, Squared};
    Q_ENUMS(Type)

protected:
    unsigned int m_pointsPerOctave;
    float m_threshold;
    QColor m_thresholdColor;
    CoherenceThresholdLine m_thresholdLine;
    Type m_type;
    virtual SeriesFBO *createSeriesFromSource(Source *source) override;

public:
    CoherencePlot(Settings *settings, QQuickItem *parent = Q_NULLPTR);

    unsigned int pointsPerOctave()
    {
        return m_pointsPerOctave;
    }
    void setPointsPerOctave(unsigned int p);

    Type type()
    {
        return m_type;
    }
    void setType(Type);
    void setType(QVariant type)
    {
        setType(static_cast<Type>(type.toInt()));
    }

    virtual void setSettings(Settings *settings) noexcept override;
    virtual void storeSettings() noexcept override;

    float threshold() const;
    void setThreshold(const float &coherenceThreshold);

    QColor thresholdColor() const;
    void setThresholdColor(const QColor &thresholdColor);

signals:
    void pointsPerOctaveChanged(unsigned int);
    void typeChanged(Type);
    void thresholdChanged(float);
    void thresholdColorChanged(QColor);
};
}
#endif // COHERENCEPLOT_H
