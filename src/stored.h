#ifndef STORED_H
#define STORED_H

#include "chartable.h"

class Stored : public Chartable
{
public:
    Stored(QObject *parent = nullptr);

    void build(Chartable *source);

    int sampleRate() {return 0;}
};

#endif // STORED_H
