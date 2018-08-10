#include "stored.h"

Stored::Stored(QObject *parent) : Fftchart::Source(parent)
{

}
void Stored::build (Fftchart::Source *source)
{
    _dataLength = source->size();
    _deconvolutionSize = source->impulseSize();
    setFftSize(source->fftSize());
    _ftdata = new FTData[_dataLength];
    _impulseData = new TimeData[_deconvolutionSize];
    source->copy(_ftdata, _impulseData);
    emit readyRead();
}
