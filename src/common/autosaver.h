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
#ifndef AUTOSAVER_H
#define AUTOSAVER_H

#include <QTimer>
#include <QThread>
#include <QtQml>

class SourceList;
class Settings;
class QSettings;

class AutoSaver : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    const QString FILE_KEY = "filename";

public:
    explicit AutoSaver(Settings *settings, SourceList *parent);
    ~AutoSaver();

    SourceList *list() const;
    QUrl fileName() const;

    Q_INVOKABLE void save();

signals:

private:
    void load();

    Settings *m_settings;
    QTimer m_timer;
    QThread m_timerThread;
};

#endif // AUTOSAVER_H
