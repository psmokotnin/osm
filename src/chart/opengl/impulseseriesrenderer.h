/**
 *  OSM
 *  Copyright (C) 2021  Pavel Smokotnin

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
#ifndef IMPULSESERIESRENDERER_H
#define IMPULSESERIESRENDERER_H

#include "xyseriesrenderer.h"
#include "../impulseplot.h"

namespace Chart {
class ImpulseSeriesRenderer : public XYSeriesRenderer
{
public:
    ImpulseSeriesRenderer();
    void init() override;
    void renderSeries() override;
    virtual void synchronize(QQuickFramebufferObject *item) override;

protected:
    virtual void updateMatrix() override;

private:
    int m_widthUniform, m_screenUniform;
    bool m_normalized;
    ImpulsePlot::Mode m_mode;
};

}
#endif // IMPULSESERIESRENDERER_H
