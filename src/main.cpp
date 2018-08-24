#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQmlContext>

#include "src/generator.h"
#include "src/measurement.h"
#include "src/chart/chart.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QQuickStyle::setStyle("Material");

    Generator g;
    Measurement *m;
    m = new Measurement(nullptr);

    qmlRegisterType<Fftchart::Chart>("FftChart", 1, 0, "FftChart");

    QQmlApplicationEngine engine;
    //qmlRegisterType<Complex>();
    //Q_DECLARE_METATYPE(QQmlListProperty<Complex>)

    engine.rootContext()->setContextProperty("generatorModel", &g);
    engine.rootContext()->setContextProperty("measurementModel", m);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));


    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
