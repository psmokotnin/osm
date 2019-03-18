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
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQmlContext>

#include "src/generator.h"
#include "src/measurement.h"
#include "src/chart/variablechart.h"
#include "src/osmsettings.h"

#ifndef APP_GIT_VERSION
#define APP_GIT_VERSION "unknow"
#endif

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QQuickStyle::setStyle("Material");
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    QCoreApplication::setApplicationName("Open Sound Meter");
    QCoreApplication::setApplicationVersion(APP_GIT_VERSION);
    QCoreApplication::setOrganizationName("psmokotnin");
    QCoreApplication::setOrganizationDomain("psmokotnin.github.io");

    //set path for config file
    OsmSettings::setPath(QSettings::IniFormat,QSettings::UserScope,QCoreApplication::applicationDirPath());
    //make settings instante
    OsmSettings s(SETTING_FILE_NAME,QSettings::IniFormat);
    Generator g;
    Measurement m;

    qmlRegisterType<Fftchart::VariableChart>("FftChart", 1, 0, "VariableChart");

    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty("appVersion", QString(APP_GIT_VERSION));
    engine.rootContext()->setContextProperty("generatorModel", &g);
    engine.rootContext()->setContextProperty("measurementModel", &m);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    //Set poiner to Qml Engine
    s.engine = &engine;
    if (engine.rootObjects().isEmpty())
        return -1;

    //Store setting before exit:
    QObject::connect(&app,&QApplication::aboutToQuit,&s,&OsmSettings::storeSettings);

    //Load settings
    s.loadSettings();
    return QApplication::exec();
}
