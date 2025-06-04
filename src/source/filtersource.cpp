/**
 *  OSM
 *  Copyright (C) 2022  Pavel Smokotnin

 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "filtersource.h"
#include "stored.h"

FilterSource::FilterSource(QObject *parent) : Abstract::Source(parent), Meta::Filter(), m_autoName(true)
{
    setObjectName("Filter");
    setName("Filter");
    setActive(true);
    update();

    connect(this, &FilterSource::typeChanged, this, &FilterSource::update);
    connect(this, &FilterSource::modeChanged, this, &FilterSource::update);
    connect(this, &FilterSource::orderChanged, this, &FilterSource::update);
    connect(this, &FilterSource::gainChanged, this, &FilterSource::update);
    connect(this, &FilterSource::qChanged, this, &FilterSource::update);
    connect(this, &FilterSource::cornerFrequencyChanged, this, &FilterSource::update);
    connect(this, &FilterSource::sampleRateChanged, this, &FilterSource::update);
    connect(this, &FilterSource::polarityChanged, this, &FilterSource::update);

    connect(this, &FilterSource::typeChanged, this, &FilterSource::applyAutoName);
    connect(this, &FilterSource::orderChanged, this, &FilterSource::applyAutoName);

    applyAutoName();
}

Shared::Source FilterSource::clone() const
{
    auto cloned = std::make_shared<FilterSource>();
    cloned->setActive(active());
    cloned->setName(name());
    cloned->setMode(mode());
    cloned->setType(type());
    cloned->setCornerFrequency(cornerFrequency());
    cloned->setOrder(order());
    cloned->setQ(q());
    cloned->setPolarity(polarity());
    cloned->setSampleRate(sampleRate());

    return std::static_pointer_cast<Abstract::Source>(cloned);
}

QJsonObject FilterSource::toJSON() const noexcept
{
    auto object = Abstract::Source::toJSON();

    object["mode"]          = mode();
    object["type"]          = type();
    object["cornerFrequency"]     = cornerFrequency();
    object["gain"]          = gain();
    object["q"]             = q();
    object["polarity"]      = polarity();
    object["order"]         = static_cast<int>(order());

    return object;
}

void FilterSource::fromJSON(QJsonObject data, const SourceList *) noexcept
{
    Abstract::Source::fromJSON(data);

    auto variantMode = data["mode"].toVariant();
    if (variantMode.isValid()) {
        setMode(variantMode.value<Meta::Measurement::Mode>());
    }

    auto variantType = data["type"].toVariant();
    if (variantType.isValid()) {
        setType(variantType.value<Type>());
    }

    setOrder(           data["order"].toInt(            order()));
    setCornerFrequency( data["cornerFrequency"].toDouble(cornerFrequency()));
    setGain(    data["gain"].toDouble(cornerFrequency()));
    setQ(       data["q"].toDouble(cornerFrequency()));
    setPolarity(data["polarity"].toBool(polarity()));
}

Shared::Source FilterSource::store()
{
    auto store = std::make_shared<Stored>();
    store->build( *this );
    store->autoName(name());
    return { store };
}

void FilterSource::update()
{
    {
        std::lock_guard guard{m_dataMutex};

        m_dataFT.setSampleRate(sampleRate());
        m_inverse.setSampleRate(sampleRate());

        try {
            using M = Meta::Measurement;
            switch (mode()) {
            case M::Mode::LFT:
                m_dataFT.setType(FourierTransform::Log);
                setTimeDomainSize(pow(2, M::m_FFTsizes.at(M::FFT12)));
                break;

            default:
                m_dataFT.setType(FourierTransform::Fast);
                m_dataFT.setSize(pow(2, M::m_FFTsizes.at(mode())));
                setTimeDomainSize(pow(2, M::m_FFTsizes.at(mode())));
            }
        } catch (std::exception &e) {
            qDebug() << __FILE__ << ":" << __LINE__  << e.what();
            setTimeDomainSize(0);
            setFrequencyDomainSize(0);
            return;
        }

        m_inverse.setType(FourierTransform::Fast);
        m_inverse.setSize(timeDomainSize());
        m_inverse.prepare();
        m_dataFT.prepare();

        auto frequencyList = m_dataFT.getFrequencies();
        setFrequencyDomainSize(frequencyList.size());

        unsigned int i = 0;
        for (auto frequency : frequencyList) {
            auto H = calculate(frequency);

            m_ftdata[i].frequency   = frequency;
            m_ftdata[i].module      = H.abs();
            m_ftdata[i].coherence   = 1.f;
            m_ftdata[i].magnitude   = H.abs();
            m_ftdata[i].phase       = H.normalize();
            if (polarity()) {
                m_ftdata[i].phase.real *= -1;
                m_ftdata[i].phase.imag *= -1;
            }
            ++i;
        }

        frequencyList = m_inverse.getFrequencies();
        for (unsigned int i = 0; i < frequencyList.size(); ++i) {
            auto v = calculate(frequencyList[i]);
            if (std::isnan(v.real) || std::isnan(v.imag)) {
                v = 0;
            }
            m_inverse.set(i, v.conjugate(), 0.f);
            m_inverse.set(timeDomainSize() - i - 1, v, 0.f);
        }

        m_inverse.transformSingleChannel();

        int t = 0;
        float kt = 1000.f / sampleRate();
        auto norm = 1.f / timeDomainSize();
        for (unsigned int i = 0, j = timeDomainSize() / 2 - 1; i < timeDomainSize(); i++, j++, t++) {
            if (t > static_cast<int>(timeDomainSize() / 2)) {
                t -= static_cast<int>(timeDomainSize());
                j -= timeDomainSize();
            }

            m_impulseData[j].value = m_inverse.af(i).real * norm;
            m_impulseData[j].time  = t * kt;//ms
        }
    }
    emit readyRead();
}

void FilterSource::applyAutoName()
{
    if (!m_autoName) {
        return;
    }
    try {
        setName(m_typeShortMap.at(type()) + " " + QString::number(order()));
    } catch (std::exception &e) {
        qDebug() << __FILE__ << ":" << __LINE__  << e.what();
    }
}

Complex FilterSource::calculate(float frequency) const
{
    auto w = frequency / cornerFrequency();
    auto s = Complex::i * w;

    switch (type()) {
    case ButterworthHPF:
        return Butterworth(true, order(), s);
    case ButterworthLPF:
        return Butterworth(false, order(), s);
    case LinkwitzRileyHPF:
        return LinkwitzRiley(true, s);
    case LinkwitzRileyLPF:
        return LinkwitzRiley(false, s);
    case BesselHPF:
        return Bessel(true, s);
    case BesselLPF:
        return Bessel(false, s);
    case APF:
        return calculateAPF(s);
    case BPF:
        return calculateBPF(s);
    case Peak:
        return calculatePeak(s);
    case HighShelf:
        return calculateShelf(true, s);
    case LowShelf:
        return calculateShelf(false, s);
    case Notch:
        return calculateNotch(s);
    }
}

Complex FilterSource::Bessel(bool hpf, Complex s) const
{
    auto n  = order();

    auto factorial = [](unsigned long k) {
        unsigned long value = 1;
        for (unsigned long i = 2; i <= k; ++i) {
            value *= i;
        }
        return value;
    };

    auto a = [&n, &factorial](unsigned long k) -> float {
        return factorial(2 * n - k) / (std::pow(2.0, n - k) * factorial(k) * factorial(n - k));
    };

    Complex s_k = 1;
    Complex numerator = a(0);
    Complex denominator = 0;
    float orderK = 1;

    switch (order()) {
    case 2:
        orderK = 4 / 3.f;
        break;
    case 3:
        orderK = 1.755672389;
        break;
    case 4:
        orderK = 2.113917675;
        break;
    case 5:
        orderK = 2.427410702;
        break;
    case 6:
        orderK = 2.703395061;
        break;
    }

    if (hpf) {
        s = Complex{1, 0} / s;
    }
    s *= orderK;



    for (unsigned int k = 0; k <= n; ++k) {
        denominator += s_k * a(k);
        s_k *= s;
    }

    return numerator / denominator;
}

Complex FilterSource::calculateAPF(Complex s) const
{
    Complex numerator;
    Complex denominator;
    float q2 = 0.5f;
    //float q4 = 1.f / sqrt(2);

    switch (order()) {
    case 2:
        numerator   = s * s - 1;
        denominator = s * s + s / q2 + 1;
        break;
    case 4:
        numerator   = s * s - s / q() + 1;
        denominator = s * s + s / q() + 1;
        break;
    }

    return numerator / denominator;
}

Complex FilterSource::calculateBPF(Complex s) const
{
    Complex numerator;
    Complex denominator;

    numerator   =         s / q();
    denominator = s * s + s / q() + 1;

    return numerator / denominator;
}

Complex FilterSource::calculatePeak(Complex s) const
{
    float a;
    Complex numerator;
    Complex denominator;

    a = std::pow(10, gain() / 40);
    numerator   = s * s + (s * a) / q() + 1;
    denominator = s * s + s / (a * q()) + 1;

    return numerator / denominator;
}

Complex FilterSource::calculateNotch(Complex s) const
{
    Complex numerator;
    Complex denominator;

    numerator   = s * s           + 1;
    denominator = s * s + s / q() + 1;

    return numerator / denominator;
}

Complex FilterSource::calculateShelf(bool high, Complex s) const
{
    float a;
    Complex numerator;
    Complex denominator;

    a = std::pow(10, gain() / 40);
    if (high) {
        numerator   = s * s * a + s * sqrt(a) / q() + 1;
        denominator = s * s     + s * sqrt(a) / q() + a;
    } else {
        numerator   = s * s     + s * sqrt(a) / q() + a;
        denominator = s * s * a + s * sqrt(a) / q() + 1;
    }

    return numerator * a / denominator;
}

Complex FilterSource::Butterworth(bool hpf, unsigned int order, const Complex &s) const
{
    Complex numerator = 1;
    if (hpf) {
        for (unsigned int i = 0; i < order; ++i) {
            numerator *= s;
        }
    }

    bool odd = order % 2;
    Complex denominator = (odd ? (s + 1) : 1);
    unsigned int lastK = (order - (odd % 2 ? 1 : 0)) / 2;
    for (unsigned int k = 1; k <= lastK; ++k) {
        denominator *= ButterworthPolinom(k, order, s);
    }

    return numerator / denominator;
}

Complex FilterSource::ButterworthPolinom(unsigned int k, unsigned int order, const Complex &s) const
{
    float b =  -2 * cos((2 * k + order - 1) * M_PI / (2 * order));
    return s * s + s * b + 1;
}

Complex FilterSource::LinkwitzRiley(bool hpf, const Complex &s) const
{
    auto b = Butterworth(hpf, order() / 2, s);
    return b * b;
}

bool FilterSource::autoName() const
{
    return m_autoName;
}

void FilterSource::setAutoName(bool newAutoName)
{
    if (m_autoName == newAutoName)
        return;
    m_autoName = newAutoName;
    emit autoNameChanged();
}
