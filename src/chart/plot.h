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
#include "cursorhelper.h"
#include "common/settings.h"

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
    Q_PROPERTY(QList<QUuid> selected READ selected WRITE setSelected NOTIFY selectedChanged)
    Q_PROPERTY(QString xLabel READ xLabel NOTIFY xLabelChanged)
    Q_PROPERTY(QString yLabel READ yLabel NOTIFY yLabelChanged)
    Q_PROPERTY(QString rendererError READ rendererError NOTIFY rendererErrorChanged)

public:
    explicit Plot(Settings *settings, QQuickItem *parent);
    void clear();
    void disconnectFromParent();
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *);

    virtual void appendDataSource(Source *source);
    virtual void removeDataSource(Source *source);
    virtual void setSourceZIndex(Source *source, int index);
    virtual void setHighlighted(Source *source);

    Q_INVOKABLE virtual void setHelper(qreal x, qreal y) noexcept = 0;
    Q_INVOKABLE virtual void unsetHelper() noexcept = 0;
    Q_INVOKABLE virtual qreal x2v(qreal x) const noexcept = 0;
    Q_INVOKABLE virtual qreal y2v(qreal y) const noexcept = 0;
    virtual QString xLabel() const = 0;
    virtual QString yLabel() const = 0;

    virtual void setSettings(Settings *settings) noexcept;
    virtual void storeSettings() noexcept = 0;

    virtual void inheritSettings(const Plot *source) = 0;

    const Palette &palette() const noexcept;
    bool darkMode() const noexcept;
    void setDarkMode(bool darkMode) noexcept;

    QString rendererError() const;
    void setRendererError(QString error);

    QList<QUuid> selected() const;
    void select(const QUuid &source);
    void setSelected(const QList<QUuid> &selected);
    bool isSelected(const QUuid &source) const;

    void setSelectAppended(bool selectAppended);

signals:
    void updated();
    void rendererErrorChanged();
    void xLabelChanged();
    void yLabelChanged();
    void selectedChanged();

public slots:
    void parentWidthChanged();
    void parentHeightChanged();
    void update();

protected:
    virtual SeriesItem *createSeriesFromSource(Source *source) = 0;
    void applyWidthForSeries(SeriesItem *s);
    void applyHeightForSeries(SeriesItem *s);

    CursorHelper *cursorHelper() const noexcept;

    const struct Padding {
        float   left    = 50.f,
                right   = 10.f,
                top     = 10.f,
                bottom  = 20.f;
    } m_padding;

    QList<SeriesItem *> m_serieses;
    Settings *m_settings;
    Palette m_palette;
    QList<QUuid> m_selected;
    QString m_rendererError;

    static CursorHelper *s_cursorHelper;

private:
    bool m_selectAppended;
};
}
#endif // PLOT_H
