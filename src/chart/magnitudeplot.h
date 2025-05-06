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
#ifndef MAGNITUDEPLOT_H
#define MAGNITUDEPLOT_H

#include "frequencybasedplot.h"

namespace Chart {
class MagnitudePlot : public FrequencyBasedPlot
{
public:
    enum Mode {
        dB      = 0x00,
        Linear  = 0x01,
        Impedance = 0x02,
    };

    Q_OBJECT
    Q_ENUM(Mode);
    Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(bool invert READ invert WRITE setInvert NOTIFY invertChanged)
    Q_PROPERTY(float sensor READ sensor WRITE setSensor NOTIFY sensorChanged)

public:
    MagnitudePlot(Settings *settings, QQuickItem *parent = Q_NULLPTR);

    virtual void setSettings(Settings *settings) noexcept override;
    virtual void storeSettings() noexcept override;

    bool invert() const;
    void setInvert(bool invert);

    Mode mode() const;
    void setMode(const Mode &mode);
    void setMode(const int &mode);

    float sensor() const;
    void setSensor(float sensor);

signals:
    void invertChanged(bool);
    void modeChanged(Mode);
    void sensorChanged(float);

protected:
    virtual SeriesItem *createSeriesFromSource(const Shared::Source &source) override;

private:
    bool m_invert;
    Mode m_mode;
    float m_sensor;

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

#endif // MAGNITUDEPLOT_H
