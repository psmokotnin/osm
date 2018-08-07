#ifndef WINDOWFUNCTION_H
#define WINDOWFUNCTION_H

#include <QDebug>
#include <QVariant>
#include <math.h>


class WindowFunction
{
public:
    enum class Type {rectangular, hann, hamming, flat_top, blackman_harris, HFT223D};

    static const std::map<Type, QString> TypeMap;

private:
    Type _type = Type::rectangular;
    unsigned long _size;
    float *_data;
    float _gain;

    //calculate data for current type
    void calculate();

public:
    WindowFunction(unsigned long size);
    ~WindowFunction();

    void setSize(unsigned long size);
    unsigned long size() const {return _size;}

    Type type() const {return _type;}
    void setType(Type t);
    QVariant getTypes();

    float gain() const {return _gain;}
    float get(unsigned long k) const { return _data[k]; }

};
QDebug operator<<(QDebug dbg, const WindowFunction::Type &t);
#endif // WINDOWFUNCTION_H
