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
#ifndef OSMSETTINGS_H
#define OSMSETTINGS_H

#include <QObject>
#include <QSettings>
#include <QCoreApplication>
#include <QQmlApplicationEngine>
#include <QDebug>
#include <QQmlProperty>

//choose config  filename here:
#ifndef SETTING_FILE_NAME
#define SETTING_FILE_NAME  "opensoundmeter.conf"
#endif

class OsmSettings : public QSettings
{
    Q_OBJECT

public:
    //Default constructor for next use, like store setting in standart system path
    explicit OsmSettings(QObject *parent = nullptr);
    //Constructor for specific falepath and filename
    explicit OsmSettings(const QString &fileName, Format format, QObject *parent = nullptr);
    //pointer to qml engine
    QQmlApplicationEngine *engine;
public slots:
    void storeSettings();
    void loadSettings();
};

#endif // OSMSETTINGS_H
