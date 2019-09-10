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
#ifndef PLOT_H
#define PLOT_H

#include <QtQuick/QQuickItem>
#include "axis.h"
#include "source.h"
#include "seriesfbo.h"
#include "../settings.h"

namespace Fftchart {
class Plot : public QQuickItem
{
    Q_OBJECT

protected:
    QList<SeriesFBO*> series;
    virtual SeriesFBO* createSeriesFromSource(Source *source) = 0;

    const struct Padding {
        float   left    = 50.f,
                right   = 10.f,
                top     = 10.f,
                bottom  = 20.f;
    } padding;

    void applyWidthForSeries(SeriesFBO *s);
    void applyHeightForSeries(SeriesFBO *s);
    Settings *m_settings;

    QColor m_backgroundColor;

public:
    explicit Plot(Settings *settings, QQuickItem *parent);
    void clear();
    void disconnectFromParent();
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *);

    virtual void appendDataSource(Source *source);
    virtual void removeDataSource(Source *source);

    Q_INVOKABLE virtual qreal x2v(qreal x) const noexcept = 0;
    Q_INVOKABLE virtual qreal y2v(qreal y) const noexcept = 0;

    virtual void setSettings(Settings *settings) noexcept {m_settings = settings;}
    virtual void storeSettings() noexcept = 0;

    void setBackgroundColor(QColor);

signals:

protected slots:
    void parentWidthChanged();
    void parentHeightChanged();

public slots:
    void update();
};
}
#endif // PLOT_H
