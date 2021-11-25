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
#ifndef RECENTFILESMODEL_H
#define RECENTFILESMODEL_H

#include <QAbstractListModel>
#include <QtQml>

class Settings;
class RecentFilesModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(Settings *settings READ settings WRITE setSettings NOTIFY settingsChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    QML_ELEMENT

    const int LIMIT = 10;
public:
    explicit RecentFilesModel(QObject *parent = nullptr);
    enum {
        FileNameRole    = Qt::UserRole + 1,
        UrlRole         = Qt::UserRole + 2
    };

    virtual QHash<int, QByteArray> roleNames() const override;
    Q_INVOKABLE int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    Q_INVOKABLE QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Q_INVOKABLE void clear();

    Q_INVOKABLE QString projectFolder() const;
    Q_INVOKABLE void storeProjectFolder(QUrl url);

    Settings *settings() const;
    void setSettings(Settings *settings);

    int count() const;

public slots:
    Q_INVOKABLE void addUrl(QUrl url);

signals:
    void settingsChanged();
    void countChanged();
    void projectFolderChanged();

private:
    void store();

    Settings *m_settings;
    QList<QVariant> m_fileurls;
    QString m_projectFolder;
};

#endif // RECENTFILESMODEL_H
