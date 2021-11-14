/**
 *  OSM
 *  Copyright (C) 2021  Pavel Smokotnin

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
#include "recentfilesmodel.h"
#include "settings.h"

RecentFilesModel::RecentFilesModel(QObject *parent) : QAbstractListModel(parent), m_settings(nullptr), m_fileurls(),
    m_projectFolder()
{
    connect(this, &QAbstractListModel::modelReset, this, &RecentFilesModel::countChanged);
}

QHash<int, QByteArray> RecentFilesModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[FileNameRole] = "fileName";
    names[UrlRole]      = "url";
    return names;
}

int RecentFilesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    if (!m_settings)
        return 0;
    return m_fileurls.count();
}

QVariant RecentFilesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !m_settings)
        return QVariant();

    QVariant r;
    switch (role) {
    case FileNameRole:
        r = m_fileurls[index.row()].toUrl().toString(QUrl::PreferLocalFile | QUrl::RemoveUserInfo);
        break;

    case UrlRole:
        r = m_fileurls[index.row()].toUrl();
        break;
    }

    return r;
}

void RecentFilesModel::clear()
{
    beginResetModel();
    m_fileurls.clear();
    store();
    endResetModel();
}

QString RecentFilesModel::projectFolder() const
{
    return m_projectFolder;
}

void RecentFilesModel::storeProjectFolder(QUrl url)
{
    QFileInfo file(url.toString());
    m_projectFolder = file.dir().path();
    emit projectFolderChanged();

    m_settings->setValue("projectFolder", m_projectFolder);
}

Settings *RecentFilesModel::settings() const
{
    return m_settings;
}

void RecentFilesModel::setSettings(Settings *settings)
{
    m_settings = settings;
    beginResetModel();
    m_fileurls = m_settings->value("filenames").toList();
    m_projectFolder = m_settings->value("projectFolder").toString();
    endResetModel();

    emit settingsChanged();
    emit projectFolderChanged();
}

int RecentFilesModel::count() const
{
    return m_fileurls.count();
}

void RecentFilesModel::addUrl(QUrl url)
{
    beginResetModel();

    while (m_fileurls.indexOf(url) != -1) {
        m_fileurls.removeOne(url);
    }
    m_fileurls.prepend(url);
    if (m_fileurls.count() > LIMIT) {
        m_fileurls.pop_back();
    }
    store();
    endResetModel();
}

void RecentFilesModel::store()
{
    if (!m_settings) {
        return;
    }
    m_settings->setValue("filenames", m_fileurls);
}
