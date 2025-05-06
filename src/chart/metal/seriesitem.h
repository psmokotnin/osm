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
#ifndef CHART_SERIESITEM_H
#define CHART_SERIESITEM_H

#include <QObject>
#include <QQuickItem>
#include "abstract/source.h"
#include "seriesnode.h"

namespace Chart {

//! represents a single series in a plot as QQuickItem
class SeriesItem : public QQuickItem
{
    Q_OBJECT

    using NodeConstructor = std::function<SeriesNode *(SeriesItem *)>;
public:
    explicit SeriesItem(const Shared::Source &source, QQuickItem *parent, NodeConstructor nodeConstructor);
    const Shared::Source &source() const;

    void setZIndex(int index);
    bool highlighted() const;
    void setHighlighted(const bool &highlighted);

signals:
    void preSourceDeleted();
    void updated();

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override;
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;

    Shared::Source m_source;
    bool m_highlighted;
    NodeConstructor m_nodeConstructor;
};

} // namespace chart

#endif // CHART_SERIESITEM_H
