#ifndef MEASURE_H
#define MEASURE_H

#include <QObject>

#include <QtCharts/QAbstractSeries>
QT_CHARTS_USE_NAMESPACE

#include "source.h"
#include "fft.h"

class Measure : public QObject
{
    Q_OBJECT

    //Active state of measurement
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)

    //Name of the measurement
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)

    //Chart color
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

    //Current sound level
    Q_PROPERTY(qreal level READ level NOTIFY levelChanged)

    //How many points per octave is used. 0 is no grouping
    Q_PROPERTY(unsigned int pointsPerOctave READ pointsPerOctave WRITE setPointsPerOctave NOTIFY pointsPerOctaveChanged)

private:
    Source *source;
    QQueue<Sample> buffer;
    complex * data;
    FFT *fft;

    bool _active         = true;
    QString _name        = "My measure";
    QColor _color        = QColor("#209fdf");
    int _pointsPerOctave = 12;
    qreal _level         = 0.0;

protected:
    int fftPower;
    int fftSize;

public:
    explicit Measure(QObject *parent = nullptr);

    bool active() {return _active;}
    void setActive(bool active);

    QString name() {return _name;}
    void setName(QString name) {_name = name; emit nameChanged();}

    QColor color() {return _color;}
    void setColor(QColor color) {_color = color; emit colorChanged();}

    void setSource(Source *s);

    unsigned int pointsPerOctave() {return _pointsPerOctave;}
    void setPointsPerOctave(unsigned int p) {_pointsPerOctave = p;}

    qreal level() {return _level;}

signals:
    void activeChanged();
    void nameChanged();
    void colorChanged();

    void readyRead();
    void levelChanged();
    void pointsPerOctaveChanged();

public slots:
    void reciveData();
    void updateRTASeries(QAbstractSeries *series);
};

#endif // MEASURE_H
