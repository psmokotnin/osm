/**
 *  OSM
 *  Copyright (C) 2022  Pavel Smokotnin

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

#include "workingfolder.h"
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QApplication>

QString workingfolder::logFilePath()
{
    auto common = commonPath() ;
    return common.isEmpty() ? "" : common + "/log.txt";
}

QString workingfolder::autosaveFilePath()
{
    auto common = commonPath() ;
    return common.isEmpty() ? "" : common + "/autosave.osm";
}

QString workingfolder::settingsFilePath()
{
    auto common = commonPath() ;
    return common.isEmpty() ? "" : common + "/settings.sosm";
}

QString workingfolder::commonPath()
{
    auto static path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);

    if ( path.indexOf(QApplication::applicationName()) == -1) {
        path += "/" + QApplication::applicationName();
    }
    if (!QDir(path).exists()) {
        QDir("/").mkpath(path);
    }

    return path;
}

