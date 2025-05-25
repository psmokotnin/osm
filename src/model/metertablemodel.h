/**
 *  OSM
 *  Copyright (C) 2023  Pavel Smokotnin

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
#ifndef METERTABLEMODEL_H
#define METERTABLEMODEL_H

#include <QAbstractListModel>
#include <QtQml>

#include "common/settings.h"
#include "meterplot.h"

class MeterTableModel : public QAbstractTableModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int columns READ columns WRITE setColumns NOTIFY columnsChanged)
    Q_PROPERTY(int rows READ rows WRITE setRows NOTIFY rowsChanged)
    Q_PROPERTY(SourceList *sourceList READ sourceList WRITE setSourceList NOTIFY sourceListChanged)
    Q_PROPERTY(Settings *settings READ settings WRITE setSettings NOTIFY settingsChanged)

public:
    explicit MeterTableModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex & = QModelIndex()) const override;
    int columnCount(const QModelIndex & = QModelIndex()) const override;

    enum {
        MeterRole   = Qt::UserRole,
    };
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int columns() const;
    void setColumns(int newColumns);

    int rows() const;
    void setRows(int newRows);

    SourceList *sourceList() const;
    void setSourceList(SourceList *newSourceList);

    Settings *settings() const;
    void setSettings(Settings *newSettings);

signals:
    void columnsChanged(int);
    void rowsChanged(int);

    void sourceListChanged();

    void settingsChanged();

private:
    void resize();

    int m_columns = 3;
    int m_rows = 1;

    QVector<QVector<std::shared_ptr<Chart::MeterPlot>>> m_data;
    SourceList *m_sourceList;
    Settings   *m_settings;
};


#endif // METERTABLEMODEL_H
