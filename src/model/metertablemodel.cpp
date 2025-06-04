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
#include "model/metertablemodel.h"
#include "sourcelist.h"

MeterTableModel::MeterTableModel(QObject *parent) : QAbstractTableModel(parent),
    m_sourceList(nullptr), m_settings(nullptr)
{
    resize();
}

void MeterTableModel::resize()
{
    m_data.resize(m_rows);
    int r = 0;
    for (auto &row : m_data) {
        row.resize(m_columns);
        int c = 0;
        for (auto &cell : row) {
            if (!cell) {
                cell = std::make_shared<Chart::MeterPlot>(this);
                if (sourceList()) {
                    cell->setSourceList(sourceList());
                    cell->setSource(sourceList()->firstSource());
                }
                if (m_settings) {
                    cell->setSettings(m_settings->getSubGroup(QString("/") + r + "/" + c));
                }
            }
            c++;
        }
        r++;
    }
}

Settings *MeterTableModel::settings() const
{
    return m_settings;
}

void MeterTableModel::setSettings(Settings *newSettings)
{
    if (m_settings == newSettings)
        return;
    m_settings = newSettings;
    emit settingsChanged();

    setRows(
        m_settings->reactValue<MeterTableModel, int>("rows", this, &MeterTableModel::rowsChanged, m_rows).toInt()
    );
    setColumns(
        m_settings->reactValue<MeterTableModel, int>("columns", this, &MeterTableModel::columnsChanged, m_columns).toInt()
    );

    int r = 0;
    for (auto &row : m_data) {
        row.resize(m_columns);
        int c = 0;
        for (auto &cell : row) {
            if (cell) {
                cell->setSettings(m_settings->getSubGroup(QString("/") + r + "/" + c));
            }
            c++;
        }
        r++;
    }
}

SourceList *MeterTableModel::sourceList() const
{
    return m_sourceList;
}

void MeterTableModel::setSourceList(SourceList *newSourceList)
{
    if (m_sourceList == newSourceList)
        return;

    m_sourceList = newSourceList;

    for (auto &row : m_data) {
        for (auto &cell : row) {
            if (cell) {
                cell->setSourceList(sourceList());
            }
        }
    }

    emit sourceListChanged();
}

int MeterTableModel::rowCount(const QModelIndex &) const
{
    return m_rows;
}

int MeterTableModel::columnCount(const QModelIndex &) const
{
    return m_columns;
}

QVariant MeterTableModel::data(const QModelIndex &index, int role) const
{
    QVariant v;

    switch (role) {
    case MeterRole:
        v.setValue(m_data[index.row()][index.column()].get());
    default:
        break;
    }

    return v;
}

QHash<int, QByteArray> MeterTableModel::roleNames() const
{
    return { {MeterRole, "meter"} };
}

int MeterTableModel::columns() const
{
    return m_columns;
}

void MeterTableModel::setColumns(int newColumns)
{
    if (m_columns == newColumns)
        return;
    m_columns = newColumns;
    resize();
    emit columnsChanged(m_columns);
}

int MeterTableModel::rows() const
{
    return m_rows;
}

void MeterTableModel::setRows(int newRows)
{
    if (m_rows == newRows)
        return;
    m_rows = newRows;
    resize();
    emit rowsChanged(m_rows);
}
