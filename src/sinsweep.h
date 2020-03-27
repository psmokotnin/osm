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
    Q_ENUM(Type)

    SinSweep(QObject *parent);

public slots:
    void setStart(int start);
    void setEnd(int end);
    void setDuration(int duration);
    void setFrequency(int f);
    void setType(int);
    void calcSampleCount(){m_sampleCount = m_sampleRate*m_duration;}

private:
    Sample sample();

    float m_frequency, m_start, m_end, m_range, m_sampleCount, m_logMax;
    double m_phase;
    //duration in seconds
    int m_duration;
    Type m_type;
    void calcRange(){m_range = m_end-m_start;}
    void calcLogMax(){m_logMax = log10f(m_end/m_start);}
    int m_currentSample{0};

signals:
    void frequencyChanged(int);
};
Q_DECLARE_METATYPE(SinSweep::Type)

#endif // SINSWEEP_H
