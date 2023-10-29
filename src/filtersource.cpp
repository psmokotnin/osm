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

FilterSource::FilterSource(QObject *parent) : chart::Source(parent), meta::Filter(), m_autoName(true)
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

    connect(this, &FilterSource::typeChanged, this, &FilterSource::applyAutoName);
    connect(this, &FilterSource::orderChanged, this, &FilterSource::applyAutoName);

    applyAutoName();
}

chart::Source *FilterSource::clone() const
{
    auto cloned = new FilterSource(parent());
    cloned->setActive(active());
    cloned->setName(name());
    cloned->setMode(mode());
    cloned->setType(type());
    cloned->setCornerFrequency(cornerFrequency());
    cloned->setOrder(order());
    cloned->setSampleRate(sampleRate());

    return cloned;
}

QJsonObject FilterSource::toJSON(const SourceList *list) const noexcept
{
    auto object = Source::toJSON(list);

    object["mode"]          = mode();
    object["type"]          = type();
    object["cornerFrequency"]     = cornerFrequency();
    object["gain"]          = gain();
    object["q"]             = q();
    object["order"]         = static_cast<int>(order());
    object["sampleRate"]    = static_cast<int>(sampleRate());

    return object;
}

void FilterSource::fromJSON(QJsonObject data, const SourceList *list) noexcept
{
    Source::fromJSON(data, list);

    auto variantMode = data["mode"].toVariant();
    if (variantMode.isValid()) {
        setMode(variantMode.value<meta::Measurement::Mode>());
    }

    auto variantType = data["type"].toVariant();
    if (variantType.isValid()) {
        setType(variantType.value<Type>());
    }


    setSampleRate(      data["sampleRate"].toInt(       sampleRate()));
    setOrder(           data["order"].toInt(            order()));
    setCornerFrequency( data["cornerFrequency"].toDouble(cornerFrequency()));
    setGain( data["gain"].toDouble(cornerFrequency()));
    setQ(    data["q"].toDouble(cornerFrequency()));
}

chart::Source *FilterSource::store() noexcept
{
    auto *store = new Stored();
    store->build(this);
    store->autoName(name());
    return store;
}

void FilterSource::update()
{
    {
        std::lock_guard guard{m_dataMutex};

        m_dataFT.setSampleRate(sampleRate());
        m_inverse.setSampleRate(sampleRate());

        try {
            using M = meta::Measurement;
            switch (mode()) {
            case M::Mode::LFT:
                m_dataFT.setType(FourierTransform::Log);
                m_deconvolutionSize = pow(2, M::m_FFTsizes.at(M::FFT12));
                break;

            default:
                m_dataFT.setType(FourierTransform::Fast);
                m_dataFT.setSize(pow(2, M::m_FFTsizes.at(mode())));
                m_deconvolutionSize = pow(2, M::m_FFTsizes.at(mode()));
            }
        } catch (std::exception &e) {
            qDebug() << __FILE__ << ":" << __LINE__  << e.what();
            m_deconvolutionSize = 0;
            m_dataLength = 0;
            return;
        }

        m_inverse.setType(FourierTransform::Fast);
        m_inverse.setSize(m_deconvolutionSize);
        m_inverse.prepare();
        m_dataFT.prepare();

        auto frequencyList = m_dataFT.getFrequencies();
        m_dataLength = frequencyList.size();
        m_ftdata.resize(m_dataLength);
        unsigned int i = 0;
        for (auto frequency : frequencyList) {
            auto H = calculate(frequency);

            m_ftdata[i].frequency   = frequency;
            m_ftdata[i].module      = H.abs();
            m_ftdata[i].coherence   = 1.f;
            m_ftdata[i].magnitude   = H.abs();
            m_ftdata[i].phase       = H.normalize();
            ++i;
        }

        m_impulseData.resize(m_deconvolutionSize);

        frequencyList = m_inverse.getFrequencies();
        for (unsigned int i = 0; i < frequencyList.size(); ++i) {
            auto v = calculate(frequencyList[i]);
            if (std::isnan(v.real) || std::isnan(v.imag)) {
                v = 0;
            }
            m_inverse.set(i, v.conjugate(), 0.f);
            m_inverse.set(m_deconvolutionSize - i - 1, v, 0.f);
        }

        m_inverse.transformSingleChannel();

        int t = 0;
        float kt = 1000.f / sampleRate();
        auto norm = 1.f / sqrtf(m_deconvolutionSize);
        for (unsigned int i = 0, j = m_deconvolutionSize / 2 - 1; i < m_deconvolutionSize; i++, j++, t++) {
            if (t > static_cast<int>(m_deconvolutionSize / 2)) {
                t -= static_cast<int>(m_deconvolutionSize);
                j -= m_deconvolutionSize;
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

complex FilterSource::calculate(float frequency) const
{
    auto w = frequency / cornerFrequency();
    auto s = complex::i * w;

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
    case Peak:
        return calculatePeak(s);
    }
}

complex FilterSource::Bessel(bool hpf, complex s) const
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

    complex s_k = 1;
    complex numerator = a(0);
    complex denominator = 0;
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
        s = complex{1, 0} / s;
    }
    s *= orderK;



    for (unsigned int k = 0; k <= n; ++k) {
        denominator += s_k * a(k);
        s_k *= s;
    }

    return numerator / denominator;
}

complex FilterSource::calculateAPF(complex s) const
{
    float q;
    complex numerator;
    complex denominator;

    switch (order()) {
    case 2:
        q = 1.f / 2;
        numerator = s * s - 1;
        denominator = s * s + s / q + 1;
        break;
    case 4:
        q = 1.f / sqrt(2);
        numerator = s * s - s / q + 1;
        denominator = s * s + s / q + 1;
        break;
    }

    return numerator / denominator;
}

complex FilterSource::calculatePeak(complex s) const
{
    float a;
    complex numerator;
    complex denominator;

    a = std::pow(10, gain() / 40);
    numerator   = s * s + (s * a) / q() + 1;
    denominator = s * s + s / (a * q()) + 1;

    return numerator / denominator;
}

complex FilterSource::Butterworth(bool hpf, unsigned int order, const complex &s) const
{
    complex numerator = 1;
    if (hpf) {
        for (unsigned int i = 0; i < order; ++i) {
            numerator *= s;
        }
    }

    bool odd = order % 2;
    complex denominator = (odd ? (s + 1) : 1);
    unsigned int lastK = (order - (odd % 2 ? 1 : 0)) / 2;
    for (unsigned int k = 1; k <= lastK; ++k) {
        denominator *= ButterworthPolinom(k, order, s);
    }

    return numerator / denominator;
}

complex FilterSource::ButterworthPolinom(unsigned int k, unsigned int order, const complex &s) const
{
    float b =  -2 * cos((2 * k + order - 1) * M_PI / (2 * order));
    return s * s + s * b + 1;
}

complex FilterSource::LinkwitzRiley(bool hpf, const complex &s) const
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
