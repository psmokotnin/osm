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
    calcRange();
    calcSampleCount();
    calcLogMax();
    QObject::connect(qobject_cast<GeneratorThread*>(parent), &GeneratorThread::startFrequencyChanged, this, &SinSweep::setStart);
    QObject::connect(qobject_cast<GeneratorThread*>(parent), &GeneratorThread::endFrequencyChanged, this, &SinSweep::setEnd);
    QObject::connect(qobject_cast<GeneratorThread*>(parent), &GeneratorThread::sweepTypeChanged, this, &SinSweep::setType);
    QObject::connect(qobject_cast<GeneratorThread*>(parent), &GeneratorThread::durationChanged, this, &SinSweep::setDuration);

    QObject::connect(this, &OutputDevice::sampleRateChanged, this, &SinSweep::calcSampleCount);
}

Sample SinSweep::sample()
{
    m_phase += (2.0 * M_PI * static_cast<double>(m_frequency) / static_cast<double>(m_sampleRate));

    if (m_phase >= 2.0 * M_PI)
        m_phase -= 2.0 * M_PI;

    Sample output = {m_gain * static_cast<float>(sin(m_phase))};

    switch (m_type) {
    case Linear:
        m_frequency = (((m_currentSample-1)/m_sampleCount)*m_range)+m_start;
        break;
    case Logarithmic:
        m_frequency = m_start*(powf(10, m_logMax*(m_currentSample/m_sampleCount)));
        break;
    }
    m_currentSample++;

    if(m_frequency >= m_end){
        m_frequency = m_start;
        m_currentSample = 0;
    }
    return output;
}

void SinSweep::setDuration(int duration)
{
    m_duration = duration;
    m_frequency = m_start;
    m_currentSample = 0;
    calcSampleCount();
}

void SinSweep::setEnd(int end)
{

    m_end = static_cast<float>(end);
    m_frequency = m_start;
    m_currentSample = 0;
    calcRange();
    calcLogMax();
}

void SinSweep::setFrequency(int f)
{
    m_frequency = static_cast<float>(f);
}

void SinSweep::setType(int t)
{
    m_type = static_cast<Type>(t);
    m_frequency = m_start;
    m_currentSample = 0;
}

void SinSweep::setStart(int start)
{
    m_start = static_cast<float>(start);
    m_frequency = m_start;
    m_currentSample = 0;
    calcRange();
    calcLogMax();
}

