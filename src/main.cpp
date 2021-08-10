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
#include <QtQuick/QQuickView>
#include <QQuickStyle>
#include <QQmlContext>
#include <QFontDatabase>
#include "settings.h"
#include "logger.h"
#include "notifier.h"
#include "src/generator.h"
#include "src/targettrace.h"
#include "src/measurement.h"
#include "src/union.h"
#include "src/elc.h"
#include "src/sourcemodel.h"
#include "src/sourcelist.h"
#include "src/chart/variablechart.h"

#include "src/audio/client.h"
#include "src/audio/devicemodel.h"
#include "src/appearance.h"
#include "src/filesystem/dialog.h"
#ifdef GRAPH_METAL
#include "src/chart/metal/seriesnode.h"
#endif

#ifndef APP_GIT_VERSION
#define APP_GIT_VERSION "unknow"
#endif

int main(int argc, char *argv[])
{
    qInstallMessageHandler(logger::messageHandler);

#ifdef GRAPH_METAL
    QQuickWindow::setSceneGraphBackend(chart::SeriesNode::chooseRhi());
#elif defined(GRAPH_OPENGL)
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
    QQuickWindow::setTextRenderType(QQuickWindow::NativeTextRendering);
    QSurfaceFormat surfaceFormat;
    surfaceFormat.setMajorVersion(3);
    surfaceFormat.setMinorVersion(3);
    surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(surfaceFormat);
#endif

    QApplication app(argc, argv);
    QQuickStyle::setStyle("Material");
    QFontDatabase::addApplicationFont(":/fonts/Roboto/Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Roboto/Bold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Roboto/BoldItalic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Roboto/Italic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/osm.ttf");
    QCoreApplication::setApplicationName("OpenSoundMeter");
    QCoreApplication::setApplicationVersion(APP_GIT_VERSION);
    QCoreApplication::setOrganizationName("opensoundmeter");
    QCoreApplication::setOrganizationDomain("opensoundmeter.com");

    Settings settings;
    Appearance appearence(&settings);
    audio::Client::getInstance();
    Generator g(settings.getGroup("generator"));
    SourceList sourceList;
    auto t = new TargetTrace(settings.getGroup("targettrace"));
    auto notifier = Notifier::getInstance();

    qmlRegisterType<audio::DeviceModel>("Audio", 1, 0, "DeviceModel");
    qmlRegisterType<chart::VariableChart>("OpenSoundMeter", 1, 0, "VariableChart");
    qmlRegisterUncreatableMetaObject(Filter::staticMetaObject, "Measurement", 1, 0, "FilterFrequency",
                                     "Error: only enums");
    qmlRegisterType<Measurement>("Measurement", 1, 0, "Measurement");
    qmlRegisterType<Union>("Union", 1, 0, "Union");
    qmlRegisterType<Stored>("Stored", 1, 0, "Stored");
    qmlRegisterType<ELC>("ELC", 1, 0, "ELC");
    qmlRegisterType<SourceModel>("SourceModel", 1, 0, "SourceModel");
    qmlRegisterUncreatableType<SourceList>("SourceModel", 1, 0, "SourceList",
                                           QStringLiteral("SourceList should not be created in QML"));
    qmlRegisterType<Settings>("Settings", 1, 0, "Settings");
    qmlRegisterType<Appearance>("OpenSoundMeter", 1, 0, "Appearance");
    qmlRegisterType<Notifier>("OpenSoundMeter", 1, 0, "Notifier");
#ifdef Q_OS_IOS
    //replace for QQuickControls2 FileDialog:
    qmlRegisterUncreatableMetaObject(filesystem::staticMetaObject, "OpenSoundMeter", 1, 0, "Filesystem",
                                     "Access to enums & flags only");
    qmlRegisterType<filesystem::Dialog>("OpenSoundMeter", 1, 0, "FileDialog");
#endif
    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty("appVersion", QString(APP_GIT_VERSION));
    engine.rootContext()->setContextProperty("applicationSettings", &settings);
    engine.rootContext()->setContextProperty("applicationAppearance", &appearence);
    engine.rootContext()->setContextProperty("sourceList", &sourceList);
    engine.rootContext()->setContextProperty("generatorModel", &g);
    engine.rootContext()->setContextProperty("targetTraceModel", t);
    engine.rootContext()->setContextProperty("notifier", notifier);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    return QApplication::exec();
}
