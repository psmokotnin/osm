/**
 *  OSM
 *  Copyright (C) 2024  Pavel Smokotnin

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

#ifndef CHART_SERIESESITEM_H
#define CHART_SERIESESITEM_H

#include <QQmlEngine>
#include <QList>

#include "chart/seriesitem.h"

namespace chart {

class Plot;

class SeriesesItem : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT

public:
    SeriesesItem(QQuickItem *parent = nullptr, Plot *plot = nullptr);
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override;

    void disconnectFromParent();
    void clear();

    bool appendDataSource(const Source::Shared &source);
    void removeDataSource(const Source::Shared &source);
    void setSourceZIndex(const QUuid &source, int index);
    void setHighlighted(const QUuid &source);

public slots:
    void parentWidthChanged();
    void parentHeightChanged();
    void update();

signals:

private:
    void applyWidthForSeries(QQuickItem *s);
    void applyHeightForSeries(QQuickItem *s);

    QList<SeriesItem *> m_serieses;
    Plot *m_plot = nullptr;
    bool m_selectAppended = false;
};

} // namespace chart

#endif // CHART_SERIESESITEM_H
