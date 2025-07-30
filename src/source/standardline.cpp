/**
 *  OSM
 *  Copyright (C) 2018  Pavel Smokotnin

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
#include "standardline.h"
#include "math/equalloudnesscontour.h"

const std::map<StandardLine::Mode, QString>StandardLine::m_modeMap = {
    {StandardLine::ELC,         "ELC"},
    {StandardLine::WEIGHTING_A, "Weighting A"},
    {StandardLine::WEIGHTING_B, "Weighting B"},
    {StandardLine::WEIGHTING_C, "Weighting C"},
    {StandardLine::FLAT,        "Flat"},
};

StandardLine::StandardLine(QObject *parent) : Abstract::Source(parent), m_mode(ELC), m_loudness(80.f)
{
    setObjectName("StandardLine");
    setActive(true);
    setName("ELC");
    update();

    connect(this, &Abstract::Source::sampleRateChanged, this, &StandardLine::update);
    connect(this, &StandardLine::transformModeChanged, this, &StandardLine::update);
}

Shared::Source StandardLine::clone() const
{
    auto cloned = std::make_shared<StandardLine>();
    cloned->setMode(mode());
    cloned->setLoudness(loudness());
    cloned->setName(name());
    cloned->setActive(active());
    return std::static_pointer_cast<Abstract::Source>(cloned);
}

QJsonObject StandardLine::toJSON() const noexcept
{
    auto object = Abstract::Source::toJSON();
    object["mode"]      = mode();
    object["loudness"]  = loudness();

    return object;
}

void StandardLine::fromJSON(QJsonObject data, const SourceList *) noexcept
{
    setMode(data["mode"].toInt(mode()));
    setLoudness(data["loudness"].toDouble());
    Abstract::Source::fromJSON(data);
}

float StandardLine::loudness() const noexcept
{
    return m_loudness;
}

void StandardLine::setLoudness(float loudness)
{
    if (!qFuzzyCompare(loudness, m_loudness)) {
        m_loudness = loudness;
        update();
        emit loudnessChanged(m_loudness);
    }
}

void StandardLine::update()
{
    m_dataMutex.lock();

    switch (m_mode) {
    case ELC:
        createELC();
        break;

    case WEIGHTING_A:
    case WEIGHTING_B:
    case WEIGHTING_C:
        createWeighting();
        break;
    case FLAT:
        createFlat();
        break;
    }

    m_dataMutex.unlock();
    emit readyRead();
}

void StandardLine::createELC()
{
    auto size = Math::EqualLoudnessContour::size();
    setFrequencyDomainSize(size);
    setTimeDomainSize(0);
    for (size_t i = 0; i < size; ++i) {
        m_ftdata[i].frequency   = Math::EqualLoudnessContour::frequency(i);
        m_ftdata[i].phase       = -INFINITY;
        m_ftdata[i].module      = -INFINITY;
        m_ftdata[i].coherence   = 1.f;

        auto Lp = Math::EqualLoudnessContour::loudness(i, loudness());

        m_ftdata[i].module    = powf(10, (Lp - 140/*dB*/) / 20);
        m_ftdata[i].magnitude = powf(10, (Lp - loudness()) / 20);
    }
}

void StandardLine::createWeighting()
{
    setFrequencyDomainSize(34);
    setTimeDomainSize(0);

    static const auto f1 = 20.598997;
    static const auto f2 = 107.65265;
    static const auto f3 = 737.86;
    static const auto f4 = 12194.217;
    static const auto f5 = 158.489032;

    //i = 10...43
    for (size_t i = 0; i < frequencyDomainSize(); ++i) {
        auto f = std::pow(10, 0.1 * (i + 10));

        m_ftdata[i].frequency   = f;
        m_ftdata[i].phase       = -INFINITY;
        m_ftdata[i].module      = -INFINITY;
        m_ftdata[i].coherence   = 1.f;

        switch (m_mode) {
        case WEIGHTING_A:
            m_ftdata[i].magnitude = pow(10, 1.997 / 20) * (
                                        f4 * f4 * f * f * f * f /
                                        (
                                            (f * f + f1 * f1) *
                                            std::sqrt(f * f + f2 * f2) *
                                            std::sqrt(f * f + f3 * f3) *
                                            (f * f + f4 * f4)
                                        )
                                    );
            break;

        case WEIGHTING_B:
            m_ftdata[i].magnitude = pow(10, 0.1696 / 20) * (
                                        f4 * f4 * f * f * f /
                                        (
                                            (f * f + f1 * f1) *
                                            std::sqrt(f * f + f5 * f5) *
                                            (f * f + f4 * f4)
                                        )
                                    );
            break;

        case WEIGHTING_C:
            m_ftdata[i].magnitude = pow(10, 0.0619 / 20) * (
                                        f4 * f4 * f * f /
                                        (
                                            (f * f + f1 * f1) *
                                            (f * f + f4 * f4)
                                        )
                                    );
            break;

        case ELC:
        case FLAT:
            Q_UNREACHABLE();
            break;
        }
    }
}

void StandardLine::createFlat()
{
    m_dataFT.setSampleRate(sampleRate());
    try {
        using M = Meta::Measurement;
        switch (transformMode()) {
        case M::Mode::LFT:
            m_dataFT.setType(FourierTransform::Log);
            setTimeDomainSize(pow(2, M::m_FFTsizes.at(M::FFT12)));
            break;

        default:
            m_dataFT.setType(FourierTransform::Fast);
            m_dataFT.setSize(pow(2, M::m_FFTsizes.at(transformMode())));
            setTimeDomainSize(pow(2, M::m_FFTsizes.at(transformMode())));
        }
    } catch (std::exception &e) {
        qDebug() << __FILE__ << ":" << __LINE__  << e.what();
        setTimeDomainSize(0);
        setFrequencyDomainSize(0);
        return;
    }

    m_dataFT.prepare();

    auto frequencyList = m_dataFT.getFrequencies();
    setFrequencyDomainSize(frequencyList.size());

    unsigned int i = 0;
    for (auto &frequency : frequencyList) {
        m_ftdata[i].frequency   = frequency;
        m_ftdata[i].module      = 1.f;
        m_ftdata[i].coherence   = 1.f;
        m_ftdata[i].magnitude   = 1.f;
        m_ftdata[i].phase       = {1.f, 0.f};
        ++i;
    }

    int t = 0;
    float kt = 1000.f / sampleRate();
    for (unsigned int i = 0, j = timeDomainSize() / 2 - 1; i < timeDomainSize(); i++, j++, t++) {
        if (t > static_cast<int>(timeDomainSize() / 2)) {
            t -= static_cast<int>(timeDomainSize());
            j -= timeDomainSize();
        }

        m_impulseData[j].value = (t == 0 ? 1.f : 0.f);
        m_impulseData[j].time  = t * kt;
    }

}

Meta::Measurement::Mode StandardLine::transformMode() const
{
    return m_transformMode;
}

void StandardLine::setTransformMode(const Meta::Measurement::Mode &newTransformMode)
{
    if (m_transformMode == newTransformMode)
        return;
    m_transformMode = newTransformMode;
    emit transformModeChanged();
}

QVariant StandardLine::getAvailableTransformModes()
{
    return Meta::Measurement::getAvailableModes();
}

StandardLine::Mode StandardLine::mode() const
{
    return m_mode;
}

void StandardLine::setMode(const int &mode)
{
    setMode(static_cast<Mode>(mode));
}

void StandardLine::setMode(const Mode &mode)
{
    if (m_mode != mode) {
        m_mode = mode;
        try {
            setName(m_modeMap.at(m_mode));
        } catch (std::exception &e) {
            qDebug() << __FILE__ << ":" << __LINE__  << e.what();
        }

        update();
        emit modeChanged(m_mode);
    }
}

QVariant StandardLine::getAvailableModes() const
{
    QStringList typeList;
    for (const auto &type : m_modeMap) {
        typeList << type.second;
    }
    return typeList;
}
