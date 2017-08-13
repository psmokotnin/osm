#ifndef MEASURE_H
#define MEASURE_H

#include <QObject>
#include <QAudioInput>

#include <QTimer>
#include <QtCharts/QAbstractSeries>
QT_CHARTS_USE_NAMESPACE

#include "sample.h"
#include "audiostack.h"
#include "fft.h"

class Measure : public QIODevice
{
    Q_OBJECT

    //Active state of measurement
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)

    //Name of the measurement
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)

    //Chart color
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

    //Current sound level
    Q_PROPERTY(float level READ level NOTIFY levelChanged)
    Q_PROPERTY(float referenceLevel READ referenceLevel NOTIFY referenceLevelChanged)

    //How many points per octave is used. 0 is no grouping
    Q_PROPERTY(unsigned int pointsPerOctave READ pointsPerOctave WRITE setPointsPerOctave NOTIFY pointsPerOctaveChanged)

private:
    QAudioInput* audio;
    QAudioFormat format;
    QTimer *timer;
    AudioStack *dataStack,
               *referenceStack;
    int
        _chanelCount = 2,
        _dataChanel = 1,
        _referenceChanel = 0;

    complex *data, *referenceData;
    FFT *fft;

    bool _active         = true;
    QString _name        = "My measure";
    QColor _color        = QColor("#209fdf");
    int _pointsPerOctave = 12;
    float _level         = 0.0,
         _referenceLevel = 0.0;

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

    unsigned int pointsPerOctave() {return _pointsPerOctave;}
    void setPointsPerOctave(unsigned int p) {_pointsPerOctave = p;}

    float level() {return _level;}
    float referenceLevel() {return _referenceLevel;}

    //IO methods
    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

signals:
    void activeChanged();
    void nameChanged();
    void colorChanged();

    void readyRead();
    void levelChanged();
    void referenceLevelChanged();
    void pointsPerOctaveChanged();

public slots:
    void transform();
    void updateRTASeries(QAbstractSeries *series);
};

#endif // MEASURE_H
