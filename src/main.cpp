#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQmlContext>

#include "src/generator.h"
#include "src/source.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QQuickStyle::setStyle("Material");

    Generator g;
    Source s;

    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty("generatorModel", &g);
    engine.rootContext()->setContextProperty("sourceModel", &s);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));


    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
