#include "stored.h"

Stored::Stored(QObject *parent) : Chartable(parent)
{

}
void Stored::build(Chartable *source)
{
    _fftSize = source->fftSize();
    alloc();
    source->copyData(dataStack, referenceStack, module, magnitude, phase, impulseData);
}
