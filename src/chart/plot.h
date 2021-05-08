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
#include "palette.h"
#include "../settings.h"

#ifdef GRAPH_METAL
#include "seriesitem.h"
using SeriesItem = chart::SeriesItem;

#elif defined(GRAPH_OPENGL)
#include "seriesfbo.h"
using SeriesItem = chart::SeriesFBO;

#else
#pragma message("GRAPH backend not setted")
#endif

namespace chart {
class Plot : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(chart::Source *filter READ filter WRITE setFilter NOTIFY filterChanged)
    Q_PROPERTY(QString xLabel READ xLabel CONSTANT)
    Q_PROPERTY(QString yLabel READ yLabel CONSTANT)
    Q_PROPERTY(bool openGLError READ openGLError NOTIFY openGLErrorChanged)

public:
    explicit Plot(Settings *settings, QQuickItem *parent);
    void clear();
    void disconnectFromParent();
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *);

    virtual void appendDataSource(Source *source);
    virtual void removeDataSource(Source *source);
    virtual void setSourceZIndex(Source *source, int index);
    virtual void setHighlighted(Source *source);

    Q_INVOKABLE virtual qreal x2v(qreal x) const noexcept = 0;
    Q_INVOKABLE virtual qreal y2v(qreal y) const noexcept = 0;
    virtual QString xLabel() const = 0;
    virtual QString yLabel() const = 0;

    virtual void setSettings(Settings *settings) noexcept;
    virtual void storeSettings() noexcept = 0;

    const Palette &palette() const noexcept;
    bool darkMode() const noexcept;
    void setDarkMode(bool darkMode) noexcept;

    chart::Source *filter() const noexcept;
    void setFilter(chart::Source *filter) noexcept;

    bool openGLError() const;
    void setOpenGLError(bool openGLError);

signals:
    void filterChanged(chart::Source *);
    void updated();
    void openGLErrorChanged();

public slots:
    void parentWidthChanged();
    void parentHeightChanged();
    void update();

protected:
    virtual SeriesItem *createSeriesFromSource(Source *source) = 0;
    void applyWidthForSeries(SeriesItem *s);
    void applyHeightForSeries(SeriesItem *s);

    const struct Padding {
        float   left    = 50.f,
                right   = 10.f,
                top     = 10.f,
                bottom  = 20.f;
    } m_padding;

    QList<SeriesItem *> m_serieses;
    Settings *m_settings;
    Palette m_palette;
    QPointer<chart::Source> m_filter;
    bool m_openGLError;
};
}
#endif // PLOT_H
