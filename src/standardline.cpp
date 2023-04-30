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

const std::map<StandardLine::Mode, QString>StandardLine::m_modeMap = {
    {StandardLine::ELC,         "ELC"},
    {StandardLine::WEIGHTING_A, "Weighting A"},
    {StandardLine::WEIGHTING_B, "Weighting B"},
    {StandardLine::WEIGHTING_C, "Weighting C"},
};

StandardLine::StandardLine(QObject *parent) : chart::Source(parent), m_mode(ELC), m_loudness(80.f)
{
    setObjectName("StandardLine");
    setActive(true);
    setName("ELC");
    update();
}

chart::Source *StandardLine::clone() const
{
    auto cloned = new StandardLine(parent());
    cloned->setMode(mode());
    cloned->setLoudness(loudness());
    cloned->setName(name());
    cloned->setActive(active());
    return cloned;
}

QJsonObject StandardLine::toJSON(const SourceList *list) const noexcept
{
    auto object = Source::toJSON(list);
    object["mode"]      = mode();
    object["loudness"]  = loudness();

    return object;
}

void StandardLine::fromJSON(QJsonObject data, const SourceList *list) noexcept
{
    setMode(data["mode"].toInt(mode()));
    setLoudness(data["loudness"].toDouble());
    Source::fromJSON(data, list);
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
    static const std::vector<double> fs {20, 25, 31.5, 40, 50, 63, 80, 100, 125, 160, 200, 250, 315, 400, 500, 630, 800, 1000, 1250, 1600, 2000,
                                         2500, 3150, 4000, 5000, 6300, 8000, 10000, 12500};

    static const std::vector<double> af {0.532, 0.506, 0.480, 0.455, 0.432, 0.409, 0.387, 0.367, 0.349, 0.330, 0.315, 0.301, 0.288, 0.276, 0.267,
                                         0.259, 0.253, 0.250, 0.246, 0.244, 0.243, 0.243, 0.243, 0.242, 0.242, 0.245, 0.254, 0.271, 0.301};

    static const std::vector<double> Lu {-31.6, -27.2, -23.0, -19.1, -15.9, -13.0, -10.3, -8.1, -6.2, -4.5, -3.1, -2.0, -1.1, -0.4, 0.0, 0.3, 0.5,
                                         0.0, -2.7, -4.1, -1.0, 1.7, 2.5, 1.2, -2.1, -7.1, -11.2, -10.7, -3.1};

    static const std::vector<double> Tf {78.5, 68.7, 59.5, 51.1, 44.0, 37.5, 31.5, 26.5, 22.1, 17.9, 14.4, 11.4, 8.6, 6.2, 4.4, 3.0, 2.2, 2.4, 3.5,
                                         1.7, -1.3, -4.2, -6.0, -5.4, -1.5, 6.0, 12.6, 13.9, 12.3};

    m_deconvolutionSize = 0;
    m_dataLength = fs.size();
    m_ftdata.resize(fs.size());

    for (size_t i = 0; i < fs.size(); ++i) {
        m_ftdata[i].frequency   = fs[i];
        m_ftdata[i].phase       = -INFINITY;
        m_ftdata[i].module      = -INFINITY;
        m_ftdata[i].coherence   = 1.f;

        auto Af = 4.47 * pow(10, -3) *
                  (pow(10.0, (0.025 * loudness())) - 1.14) +
                  pow((0.4 * pow(10.0, (((Tf[i] + Lu[i]) / 10) - 9 ))), af[i]);

        auto Lp = ((10 / af[i]) * log10(Af)) - Lu[i] + 94;

        m_ftdata[i].module = powf(10, (Lp - 140/*dB*/) / 20) * 10;
        m_ftdata[i].magnitude = pow(10, (Lp - loudness()) / 20);
    }
}

void StandardLine::createWeighting()
{
    m_dataLength = 34;
    m_deconvolutionSize = 0;

    static const auto f1 = 20.598997;
    static const auto f2 = 107.65265;
    static const auto f3 = 737.86;
    static const auto f4 = 12194.217;
    static const auto f5 = 158.489032;

    m_ftdata.resize(m_dataLength);

    //i = 10...43
    for (size_t i = 0; i < m_dataLength; ++i) {
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
