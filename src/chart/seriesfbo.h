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
#ifndef SERIESFBO_H
#define SERIESFBO_H

#include <QQuickFramebufferObject>
#include "source.h"

namespace Fftchart {

typedef std::function<QQuickFramebufferObject::Renderer* (void)> RendererCreator;

class SeriesFBO : public QQuickFramebufferObject
{
    Q_OBJECT

public:
    explicit SeriesFBO(Source* s, RendererCreator rc, QQuickItem *parent = nullptr);
    Source *source() const { return m_source;}
    QQuickFramebufferObject::Renderer *createRenderer() const override;

protected:
    RendererCreator rendererCreator;
    Source *m_source;

protected slots:
};
}
#endif // SERIESFBO_H
