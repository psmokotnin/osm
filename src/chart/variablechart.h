#ifndef CHART_VARIABLECHART_H
#define CHART_VARIABLECHART_H
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
#include <QtQuick/QQuickItem>
#include "type.h"
#include "abstract/source.h"
#include "plot.h"
#include "common/settings.h"

class SourceList;

namespace Chart {

class VariableChart : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QString type READ typeString WRITE setTypeByString NOTIFY typeChanged)
    Q_PROPERTY(Settings *settings READ settings WRITE setSettings NOTIFY settingsChanged)
    Q_PROPERTY(QQuickItem *plot READ plot() NOTIFY typeChanged)
    Q_PROPERTY(SourceList *sources READ sources WRITE setSources NOTIFY sourcesChanged)
    Q_PROPERTY(bool darkMode READ darkMode WRITE setDarkMode NOTIFY darkModeChanged)

public:
    VariableChart(QQuickItem *parent = Q_NULLPTR);
    QString typeString() const;
    void setType(const Type &type);
    void setTypeByString(const QString &type);

    Settings *settings() const noexcept;
    void setSettings(Settings *settings) noexcept;

    Q_INVOKABLE void removeDataSource(const Shared::Source &source);
    Q_INVOKABLE void setSourceZIndex(const QUuid &source, int index);
    Q_INVOKABLE QString urlForGrab(QUrl url) const;

    Plot *plot() const noexcept;
    SourceList *sources() const noexcept;
    void setSources(SourceList *sourceList);

    const bool &darkMode() const noexcept;
    void setDarkMode(const bool &) noexcept;

public slots:
    void updateZOrders() noexcept;

signals:
    void typeChanged();
    void settingsChanged();
    void sourcesChanged();
    void darkModeChanged();

protected:
    void initType();

private:
    void connectSources(SourceList *sourceList);
    Plot *m_plot;
    SourceList *m_sources;
    Settings *m_settings;
    Type m_selected;
    bool m_darkMode;

};

}
#endif // CHART_VARIABLECHART_H
