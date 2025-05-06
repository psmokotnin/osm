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

#include "frequencybasedplot.h"

namespace Chart {

class CoherenceThresholdLine : public PaintedItem
{
public:
    CoherenceThresholdLine(QQuickItem *parent);
    virtual void paint(QPainter *painter) override;
    void parentWidthChanged();
    void parentHeightChanged();
};

class CoherencePlot : public FrequencyBasedPlot
{
    Q_OBJECT
    Q_PROPERTY(unsigned int pointsPerOctave READ pointsPerOctave WRITE setPointsPerOctave NOTIFY
               pointsPerOctaveChanged)
    Q_PROPERTY(Type type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(QColor thresholdColor READ thresholdColor WRITE setThresholdColor NOTIFY thresholdColorChanged)
    Q_PROPERTY(float threshold READ threshold WRITE setThreshold NOTIFY thresholdChanged)
    Q_PROPERTY(bool showThreshold READ showThreshold WRITE setShowThreshold NOTIFY showThresholdChanged)

public:
    enum Type {Normal, Squared, SNR};
    Q_ENUMS(Type)
    static constexpr float THRESHOLD_NORMAL     = 0.95;
    static constexpr float THRESHOLD_SQUARED    = 0.91;
    static constexpr float THRESHOLD_SNR        = 10;

    CoherencePlot(Settings *settings, QQuickItem *parent = Q_NULLPTR);

    unsigned int pointsPerOctave() const;
    void setPointsPerOctave(unsigned int p);

    Type type() const;
    void setType(const Type &);
    void setType(const QVariant &type);

    virtual void setSettings(Settings *settings) noexcept override;
    virtual void storeSettings() noexcept override;

    float threshold() const;
    void setThreshold(const float &coherenceThreshold);

    QColor thresholdColor() const;
    void setThresholdColor(const QColor &thresholdColor);

    bool showThreshold() const;
    void setShowThreshold(const bool &showThreshold);

signals:
    void pointsPerOctaveChanged(unsigned int);
    void typeChanged(Type);
    void thresholdChanged(float);
    void thresholdColorChanged(QColor);
    void showThresholdChanged(bool);

protected:
    float m_threshold;
    bool m_showThreshold;
    QColor m_thresholdColor;
    CoherenceThresholdLine m_thresholdLine;
    Type m_type;
    virtual SeriesItem *createSeriesFromSource(const Shared::Source &source) override;
};
}
#endif // COHERENCEPLOT_H
