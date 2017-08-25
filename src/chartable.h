#ifndef CHARTABLE_H
#define CHARTABLE_H

#include <QtCharts/QXYSeries>
QT_CHARTS_USE_NAMESPACE
#include <QIODevice>
#include <QColor>

#include "fft.h"
#include "audiostack.h"

class Chartable : public QIODevice
{
    Q_OBJECT

    //Active state of measurement
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)

    //Name of the measurement
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)

    //Chart color
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

    //How many points per octave is used. 0 is no grouping
    Q_PROPERTY(unsigned int pointsPerOctave READ pointsPerOctave WRITE setPointsPerOctave NOTIFY pointsPerOctaveChanged)

protected:
    bool _active         = true;
    QString _name        = "My measure";
    QColor _color        = QColor("#209fdf");
    int _pointsPerOctave = 12;
    int _fftSize;
    AudioStack *dataStack,
               *referenceStack;
    complex *data, *referenceData, *impulseData;

    void alloc();

public:
    Chartable(QObject *parent);

    virtual int sampleRate() = 0;

    qint64 readData(char *data, qint64 maxlen);
    virtual qint64 writeData(const char *data, qint64 len);

    bool active() {return _active;}
    virtual void setActive(bool active);

    QString name() {return _name;}
    void setName(QString name) {_name = name; emit nameChanged();}

    QColor color() {return _color;}
    void setColor(QColor color) {_color = color; emit colorChanged();}

    unsigned int pointsPerOctave() {return _pointsPerOctave;}
    void setPointsPerOctave(unsigned int p) {_pointsPerOctave = p;}

    int fftSize() {return _fftSize;}
    void setFftSize(int size) {_fftSize = size;}

    void copyData(AudioStack *toDataStack,
                  AudioStack *toReferenceStack,
                  complex *toData,
                  complex *toReferenceData,
                  complex *toImpulseData);

signals:
    void activeChanged();
    void nameChanged();
    void colorChanged();
    void pointsPerOctaveChanged();

public slots:
    virtual void updateSeries(QAbstractSeries *series, QString type);
    void scopeSeries(QAbstractSeries *series);
    void impulseSeries(QAbstractSeries *series);
};

#endif // CHARTABLE_H
