#ifndef STORED_H
#define STORED_H

#include "chart/source.h"

class Stored: public Fftchart::Source
{
    Q_OBJECT

public:
    explicit Stored(QObject *parent = nullptr);
    void build (Fftchart::Source *source);
};
#endif // STORED_H
