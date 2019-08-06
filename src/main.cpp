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
#include <QFontDatabase>
#include "settings.h"

#include "src/generator.h"
#include "src/measurement.h"
#include "src/sourcemodel.h"
#include "src/sourcelist.h"
#include "src/chart/variablechart.h"

#ifndef APP_GIT_VERSION
#define APP_GIT_VERSION "unknow"
#endif

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QQuickStyle::setStyle("Material");
    QFontDatabase::addApplicationFont(":/fonts/osm.ttf");
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    QCoreApplication::setApplicationName("Open Sound Meter");
    QCoreApplication::setApplicationVersion(APP_GIT_VERSION);
    QCoreApplication::setOrganizationName("psmokotnin");
    QCoreApplication::setOrganizationDomain("psmokotnin.github.io");

    Settings settings;
    Generator g(settings.getGroup("generator"));
    SourceList sourceList;

    qmlRegisterType<Fftchart::VariableChart>("FftChart", 1, 0, "VariableChart");
    qmlRegisterUncreatableMetaObject(Filter::staticMetaObject, "Measurement", 1, 0, "FilterFrequency", "Error: only enums");
    qmlRegisterType<Measurement>("Measurement", 1, 0, "Measurement");
    qmlRegisterType<SourceModel>("SourceModel", 1, 0, "SourceModel");
    qmlRegisterUncreatableType<SourceList>("SourceModel", 1, 0, "SourceList", QStringLiteral("SourceList should not be created in QML"));
    qmlRegisterType<Settings>("Settings", 1, 0, "Settings");

    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty("appVersion", QString(APP_GIT_VERSION));
    engine.rootContext()->setContextProperty("applicationSettings", &settings);
    engine.rootContext()->setContextProperty("sourceList", &sourceList);
    engine.rootContext()->setContextProperty("generatorModel", &g);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    return QApplication::exec();
}
