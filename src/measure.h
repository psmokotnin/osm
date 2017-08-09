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

private:
    Source *source;
    QQueue<Sample> buffer;
    QList<fftData> data;
    FFT *fft;

protected:
    int fftPower;
    int fftSize;

public:
    explicit Measure(QObject *parent = nullptr);
    void setSource(Source *s);

signals:
    void readyRead();

public slots:
    void reciveData();
    void updateRTASeries(QAbstractSeries *series);
};

#endif // MEASURE_H
