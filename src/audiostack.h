#ifndef AUDIOSTACK_H
#define AUDIOSTACK_H
#include <QDebug>
#include <math.h>
#include <qglobal.h>

class AudioStack
{

public:
    struct Cell {
        float value;
        Cell * next = nullptr;
        Cell * pre = nullptr;
    };

protected:
    unsigned long _size = 0, sizeLimit = 0;
    Cell * firstdata = nullptr, * lastdata = nullptr;
    Cell * pointer = nullptr;


public:
    AudioStack(unsigned long size);
    AudioStack(AudioStack *original);
    virtual ~AudioStack() = default;

    void setSize(unsigned long size);
    unsigned long size();
    virtual void add(const float data);
    virtual void dropFirst();

    void reset(void);
    bool next(void);
    bool isNext(void);
    float first(void);

    float current(void);
    float shift(void);
    void fill(float value);

    void rewind(long delta);
};

#endif // AUDIOSTACK_H
