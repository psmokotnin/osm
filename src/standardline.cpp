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
};

StandardLine::StandardLine(QObject *parent) : Abstract::Source(parent), m_mode(ELC), m_loudness(80.f)
{
    setObjectName("StandardLine");
    setActive(true);
    setName("ELC");
    update();
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
            Q_UNREACHABLE();
            break;
        }
    }
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
