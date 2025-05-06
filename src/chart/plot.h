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
#include "abstract/source.h"
#include "palette.h"
#include "cursorhelper.h"
#include "common/settings.h"
#include "chart/seriesesitem.h"
#include "plotpadding.h"

namespace Chart {

class Plot : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QList<QUuid> selected READ selected WRITE setSelected NOTIFY selectedChanged)
    Q_PROPERTY(QString xLabel READ xLabel NOTIFY xLabelChanged)
    Q_PROPERTY(QString yLabel READ yLabel NOTIFY yLabelChanged)
    Q_PROPERTY(QString rendererError READ rendererError NOTIFY rendererErrorChanged)

    friend class SeriesesItem;

public:
    explicit Plot(Settings *settings, QQuickItem *parent);
    void connectSources(SourceList *sourceList);

    void clear();
    void disconnectFromParent();
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override;

    bool appendDataSource(const Shared::Source &source);
    void removeDataSource(const Shared::Source &source);
    void setSourceZIndex(const QUuid &source, int index);
    void setHighlighted(const QUuid &source);

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

    bool selectAppended() const;
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
    virtual SeriesItem *createSeriesFromSource(const Shared::Source &source) = 0;

    CursorHelper *cursorHelper() const noexcept;

    const Padding m_padding { 50.f, 10.f, 10.f, 20.f };
    SeriesesItem m_seriesesItem;

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
