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
#include "seriesfbo.h"

using namespace Fftchart;

SeriesFBO::SeriesFBO(Source* s, RendererCreator rc, QQuickItem *parent):
    QQuickFramebufferObject(parent),
    rendererCreator(rc),
    m_source(s)
{
    setFlag(QQuickItem::ItemHasContents);
    connect(s, SIGNAL(colorChanged()),  SLOT(update()));
    connect(s, SIGNAL(readyRead()),     SLOT(update()));
    connect(s, SIGNAL(activeChanged()), SLOT(update()));

    //make measurements be on the top of the sources
    if (s->objectName() == "Measurement") {
        setZ(2.0);
    }
}
QQuickFramebufferObject::Renderer *SeriesFBO::createRenderer() const
{
    return rendererCreator();
}
