#ifndef SINSWEEP_H
#define SINSWEEP_H

#include <QtMath>
#include "outputdevice.h"

class SinSweep : public OutputDevice
{
    Q_OBJECT
public:

    enum Type {
        Linear,
        Logarithmic
    };
    Q_ENUM(Type);

    SinSweep(QObject *parent);

    float startFrequency() const
    {
        return m_start;
    }

public slots:
    void setStart(int start);
    void setEnd(int end);
    void setFrequency(int f);
    void setType(Type);

private:
    Sample sample();

    float m_frequency;
    double m_phase;
    float m_start;
    float m_end;
    //TODO: add user defined duration (ms? s? samples?)
    float m_duration;
    Type m_type;
signals:
    void frequencyChanged(int);
};

#endif // SINSWEEP_H
