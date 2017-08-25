#ifndef STORED_H
#define STORED_H

#include "chartable.h"

class Stored : public Chartable
{
protected:
    int _sampleRate;

public:
    Stored(QObject *parent = nullptr);

    void build(Chartable *source);

    int sampleRate() {return _sampleRate;}
    void setSampleRate(int sampleRate) {_sampleRate = sampleRate;}
};

#endif // STORED_H
