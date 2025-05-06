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
#ifndef RTAPLOT_H
#define RTAPLOT_H

#include "frequencybasedplot.h"

namespace Chart {
class RTAPlot : public FrequencyBasedPlot
{
public:
    enum Mode {
        Line  = 0,
        Bars  = 1,
        Lines = 2
    };
    enum Scale { DBfs, SPL, Phon };

    Q_OBJECT
    Q_ENUM(Mode)

    Q_ENUM(Scale)

    Q_PROPERTY(Chart::RTAPlot::Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(bool showPeaks READ showPeaks WRITE setShowPeaks NOTIFY showPeaksChanged)
    Q_PROPERTY(Chart::RTAPlot::Scale scale READ scale WRITE setScale NOTIFY scaleChanged)

public:
    RTAPlot(Settings *settings, QQuickItem *parent = Q_NULLPTR);

    Chart::RTAPlot::Mode mode();
    void setMode(Chart::RTAPlot::Mode mode);
    void setMode(unsigned int mode);

    virtual void setSettings(Settings *settings) noexcept override;
    virtual void storeSettings() noexcept override;

    bool showPeaks() const;
    void setShowPeaks(bool showPeaks);

    Scale scale() const;
    void setScale(Scale newScale);
    void setScale(unsigned int newScale);


signals:
    void showPeaksChanged(bool);
    void modeChanged(Chart::RTAPlot::Mode);
    void scaleChanged(Chart::RTAPlot::Scale);

protected slots:
    void updateAxis();

protected:
    bool isPointsPerOctaveValid(unsigned int &value) const override;

private:
    virtual SeriesItem *createSeriesFromSource(const Shared::Source &source) override;
    Mode m_mode;
    Scale m_scale;
    bool m_spline, m_showPeaks;

    class TargetTraceItem : public PaintedItem
    {
    public:
        TargetTraceItem(const Palette &palette, QQuickItem *parent = Q_NULLPTR);
        void paint(QPainter *painter) noexcept override;

    private:
        const Palette &m_palette;

    } *m_targetTrace = nullptr;
};
}
#endif // RTAPLOT_H
