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
    AudioStack *subStack = nullptr;
    int subParts = 1;
    QVector<float> parts;


public:
    AudioStack(unsigned long size);
    AudioStack(AudioStack *original);
    void setSize(unsigned long size);
    unsigned long size();
    void add(const float data);
    bool halfAdd(float data);

    void setParts(int size) {subParts = size;}
    bool partAdd(float data);

    void reset(void);
    bool next(void);
    bool isNext(void);
    float first(void);

    float current(void);
    float shift(void);
    void fill(float value);

    void rewind(int delta);

    void setSubStack(AudioStack *stack) {subStack = stack;}
};

#endif // AUDIOSTACK_H
