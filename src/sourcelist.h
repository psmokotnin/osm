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

class SourceList : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int count READ count)
    Q_PROPERTY(QUrl currentFile READ currentFile)

private:
    QVector<Fftchart::Source*> mItems;
    QUrl m_currentFile;
    const QList<QColor> colors {
        "#F44336", "#FFEB3B", "#9C27B0", "#673AB7",
        "#3F51B5", "#2196F3", "#03A9F4", "#00BCD4",
        "#009688", "#4CAF50", "#8BC34A", "#CDDC39",
        "#E91E63", "#FFC107", "#FF9800", "#FF5722",
        "#795548", "#9E9E9E", "#607D8B"
    };
    int colorIndex;

    bool loadList(const QJsonDocument &document) noexcept;
    bool loadMeasurement(const QJsonObject &data) noexcept;
    bool loadStored(const QJsonObject &data) noexcept;
public:
    explicit SourceList(QObject *parent = nullptr);

    const QVector<Fftchart::Source *> &items() const;

    int count() const noexcept;
    QUrl currentFile() const noexcept {return m_currentFile;}
    Q_INVOKABLE Fftchart::Source *get(int i) const noexcept;
    Q_INVOKABLE void clean() noexcept;
    Q_INVOKABLE void reset() noexcept;
    Q_INVOKABLE bool save(const QUrl &fileName) const noexcept;
    Q_INVOKABLE bool load(const QUrl &fileName) noexcept;

signals:
    void preItemAppended();
    void postItemAppended(Fftchart::Source *);

    void preItemRemoved(int index);
    void postItemRemoved();

public slots:
    Measurement *addMeasurement();
    void appendItem(Fftchart::Source *item, bool autocolor = false);
    void removeItem(Fftchart::Source *item);
    QColor nextColor();
};

#endif // SOURCELIST_H
