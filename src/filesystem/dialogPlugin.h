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
#ifndef FILESYSTEM_DIALOGPLUGIN_H
#define FILESYSTEM_DIALOGPLUGIN_H

#include <QObject>
#include "dialog.h"

namespace filesystem {

class DialogPlugin : public QObject
{
    Q_OBJECT

public:
    DialogPlugin(QObject *parent = nullptr);
    virtual ~DialogPlugin();

    virtual QString title() const;
    virtual void setTitle(const QString &title);

    virtual QString defaultSuffix() const;
    virtual void setDefaultSuffix(const QString &defaultSuffix);

    virtual QStringList nameFilters() const;
    virtual void setNameFilters(const QStringList &nameFilters);

    virtual Dialog::Mode mode() const;
    virtual void setMode(Dialog::Mode mode);

    virtual StandardFolder folder() const;
    virtual QUrl folderURL() const;
    virtual void setFolder(StandardFolder folder);

    virtual void open(const QQuickWindow *window) = 0;

signals:
    void accepted(QList<QUrl>, QString);
    void rejected();

private:
    QString m_title;
    StandardFolder m_startFolder;
    Dialog::Mode m_mode;
    QString m_defaultSuffix;
    QStringList m_nameFilters;
};

} // namespace filesystem

#endif // FILESYSTEM_DIALOGPLUGIN_H
