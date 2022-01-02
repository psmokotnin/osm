/**
 *  OSM
 *  Copyright (C) 2019  Pavel Smokotnin

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
#ifndef SOURCELIST_H
#define SOURCELIST_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QUrl>
#include "chart/source.h"


class Measurement;
class Union;
class Stored;
class ELC;

class SourceList : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int count READ count)
    Q_PROPERTY(QUrl currentFile READ currentFile)
    Q_PROPERTY(chart::Source *first READ firstSource)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelected NOTIFY selectedChanged)
    Q_PROPERTY(chart::Source *selected READ selected NOTIFY selectedChanged)
    using iterator = QVector<chart::Source *>::iterator;

public:
    explicit SourceList(QObject *parent = nullptr, bool appendMeasurement = true);
    SourceList *clone(QObject *parent, bool filtered = false) const noexcept;

    int count() const noexcept;
    chart::Source *firstSource() const noexcept;
    const QVector<chart::Source *> &items() const;
    SourceList::iterator begin() noexcept;
    SourceList::iterator end() noexcept;

    QUrl currentFile() const noexcept;

    Q_INVOKABLE chart::Source *get(int i) const noexcept;
    Q_INVOKABLE void clean() noexcept;
    Q_INVOKABLE void reset() noexcept;
    Q_INVOKABLE bool save(const QUrl &fileName) const noexcept;
    Q_INVOKABLE bool load(const QUrl &fileName) noexcept;
    Q_INVOKABLE bool import(const QUrl &fileName, const int &type) noexcept;
    Q_INVOKABLE bool importImpulse(const QUrl &fileName, QString separator) noexcept;
    Q_INVOKABLE bool importWav(const QUrl &fileName) noexcept;
    Q_INVOKABLE bool move(int from, int to) noexcept;
    Q_INVOKABLE int indexOf(chart::Source *) const noexcept;

    int selectedIndex() const;
    chart::Source *selected() const noexcept;
    void setSelected(int selected);

    enum {
        TRANSFER_TXT    = 0,
        TRANSFER_CSV    = 1,
        IMPULSE_TXT     = 2,
        IMPULSE_CSV     = 3,
        IMPULSE_WAV     = 4
    };

public slots:
    Q_INVOKABLE QColor nextColor();
    Q_INVOKABLE Measurement *addMeasurement();
    Q_INVOKABLE Union *addUnion();
    Q_INVOKABLE ELC *addElc();
    Q_INVOKABLE void appendItem(chart::Source *item, bool autocolor = false);
    Q_INVOKABLE void removeItem(chart::Source *item, bool deleteItem = true);
    Q_INVOKABLE void cloneItem(chart::Source *item);
    void appendNone();

signals:
    void preItemAppended();
    void postItemAppended(chart::Source *);

    void preItemRemoved(int index);
    void postItemRemoved();

    void preItemMoved(int from, int to);
    void postItemMoved();

    void selectedChanged();
    void loaded(QUrl fileName);

private:
    bool loadList(const QJsonDocument &document, const QUrl &fileName) noexcept;
    template<typename T> bool loadObject(const QJsonObject &data) noexcept;
    template<typename T> T *add() noexcept;
    bool importFile(const QUrl &fileName, QString separator) noexcept;

    QVector<chart::Source *> m_items;
    QUrl m_currentFile;
    const QList<QColor> m_colors {
        "#F44336", "#FFEB3B", "#9C27B0", "#673AB7",
        "#3F51B5", "#2196F3", "#03A9F4", "#00BCD4",
        "#009688", "#4CAF50", "#8BC34A", "#CDDC39",
        "#E91E63", "#FFC107", "#FF9800", "#FF5722",
        "#795548", "#9E9E9E", "#607D8B"
    };
    int m_colorIndex;
    int m_selected;
};

#endif // SOURCELIST_H
