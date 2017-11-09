#include "stored.h"

Stored::Stored(QObject *parent) : Chartable(parent)
{

}
void Stored::build(Chartable *source)
{
    _fftSize = source->fftSize();
    _deconvolutionSize = source->deconvolutionSize();
    alloc();

    //source->copyData(dataStack, referenceStack, &data, impulseData);
}
