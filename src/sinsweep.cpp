#include "sinsweep.h"
#include "generatorthread.h"
#include <QDebug>

SinSweep::SinSweep(QObject *parent) : OutputDevice(parent),
    m_phase(0.0),
    m_start(1000.f),
    m_end(2000.f),
    m_duration(1),
    m_type(Linear)
{
    m_name = "SineSweep";
    m_frequency = m_start;
    QObject::connect(qobject_cast<GeneratorThread*>(parent), &GeneratorThread::startFrequencyChanged, this, &SinSweep::setStart);
    QObject::connect(qobject_cast<GeneratorThread*>(parent), &GeneratorThread::endFrequencyChanged, this, &SinSweep::setEnd);
    QObject::connect(qobject_cast<GeneratorThread*>(parent), &GeneratorThread::sweepTypeChanged, this, &SinSweep::setType);
}

Sample SinSweep::sample()
{
    m_phase += (2.0 * M_PI * static_cast<double>(m_frequency) / m_sampleRate);
    double fDelta;

    switch (m_type) {
    case Linear:
        fDelta = (static_cast<double>(m_end) - static_cast<double>(m_start)) / (m_sampleRate * m_duration);
        break;
    case Logarithmic:
        fDelta = (m_frequency / (10 * log(m_frequency))) / ((m_end - m_start) * m_duration);
        break;
    }

    if (m_phase >= 2.0 * M_PI)
        m_phase -= 2.0 * M_PI;

    Sample output = {m_gain * static_cast<float>(sin(m_phase))};
    m_frequency += fDelta;
    if(m_frequency >= m_end)
        m_frequency = m_start;
    return output;
}

void SinSweep::setEnd(int end)
{

    m_end = static_cast<float>(end);
    m_frequency = m_start;
}

void SinSweep::setFrequency(int f)
{
    m_frequency = static_cast<float>(f);
}

void SinSweep::setType(SinSweep::Type t)
{
    m_type = t;
}

void SinSweep::setStart(int start)
{
    m_start = static_cast<float>(start);
    m_frequency = m_start;
}

