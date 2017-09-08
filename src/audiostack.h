#ifndef AUDIOSTACK_H
#define AUDIOSTACK_H
#include <QDebug>
#include <math.h>
#include <qglobal.h>

class AudioStack
{

public:
    struct Cell {
        qreal value;
        Cell * next = nullptr;
        Cell * pre = nullptr;
    };

protected:
    unsigned long _size = 0, sizeLimit = 0;
    Cell * firstdata = nullptr, * lastdata = nullptr;
    Cell * pointer = nullptr;
    AudioStack *subStack = nullptr;
    int subParts = 1;
    QVector<qreal> parts;


public:
    AudioStack(unsigned long size);
    void setSize(unsigned long size);
    unsigned long size();
    void add(qreal data);
    bool halfAdd(qreal data);

    void setParts(int size) {subParts = size;}
    bool partAdd(qreal data);

    void reset(void);
    bool next(void);
    qreal first(void);
    qreal current(void);
    qreal shift(void);
    void fill(qreal value);

    void setSubStack(AudioStack *stack) {subStack = stack;}
};

#endif // AUDIOSTACK_H
