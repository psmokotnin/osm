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
#ifndef FILESYSTEM_DIALOG_H
#define FILESYSTEM_DIALOG_H

#include <QQuickItem>
#include <QMetaType>
#include <QUrl>

namespace filesystem {

Q_NAMESPACE

enum class StandardFolder {Home, Documents, Images, Desktop, Music, Movies};
Q_ENUM_NS(StandardFolder)

class DialogPlugin;

class Dialog : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(filesystem::StandardFolder folder READ folder WRITE setFolder)
    Q_PROPERTY(QString defaultSuffix READ defaultSuffix WRITE setDefaultSuffix)
    Q_PROPERTY(QStringList nameFilters READ nameFilters WRITE setNameFilters)
    Q_PROPERTY(Mode mode READ mode WRITE setMode REQUIRED)

    //for QtQuick2 replacement:
    Q_PROPERTY(QUrl fileUrl READ fileUrl)
    Q_PROPERTY(QString selectedNameFilter READ selectedNameFilter)
    Q_PROPERTY(bool selectExisting READ selectExisting WRITE setSelectExisting)

public:
    Dialog(QQuickItem *parent = nullptr);

    enum Mode {OpenFile, SaveFile};
    Q_ENUM(Mode);

    QString title() const;
    void setTitle(const QString &title);

    QString defaultSuffix() const;
    void setDefaultSuffix(const QString &defaultSuffix);

    QStringList nameFilters() const;
    void setNameFilters(const QStringList &nameFilters);

    Mode mode() const;
    void setMode(Mode mode);

    StandardFolder folder() const;
    void setFolder(StandardFolder folder);

    Q_INVOKABLE void open();
    QUrl fileUrl() const;

    bool selectExisting() const;
    void setSelectExisting(const bool &existing);
    QString selectedNameFilter() const;

signals:
    void accepted();
    void rejected();

private:
    DialogPlugin *m_plugin;
    QList<QUrl> m_files;
    QString m_selectedFilter;
};

} // namespace filesystem

Q_DECLARE_METATYPE(filesystem::Dialog::Mode)

#endif // FILESYSTEM_DIALOG_H
