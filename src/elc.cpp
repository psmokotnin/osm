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
#include "elc.h"

ELC::ELC(QObject *parent) : Fftchart::Source(parent), m_loudness(80.f)
{
    setObjectName("ELC");
    setActive(true);
    setName("ELC");
    update();
}

QJsonObject ELC::toJSON() const noexcept
{
    QJsonObject object;
    object["loudness"] = loudness();
    object["active"]    = active();
    object["name"]      = name();

    QJsonObject color;
    color["red"]    = m_color.red();
    color["green"]  = m_color.green();
    color["blue"]   = m_color.blue();
    color["alpha"]  = m_color.alpha();
    object["color"] = color;

    return object;
}

void ELC::fromJSON(QJsonObject data) noexcept
{
    auto jsonColor = data["color"].toObject();
    QColor c(
                jsonColor["red"  ].toInt(0),
                jsonColor["green"].toInt(0),
                jsonColor["blue" ].toInt(0),
                jsonColor["alpha"].toInt(1));
    setColor(c);
    setLoudness(data["loudness"].toDouble());
    setName(data["name"].toString());
    setActive(data["active"].toBool(active()));
}

void ELC::setLoudness(float loudness)
{
    if (!qFuzzyCompare(loudness, m_loudness)) {
        m_loudness = loudness;
        update();
        emit loudnessChanged(m_loudness);
    }
}

void ELC::update()
{
    std::lock_guard<std::mutex> guard(m_dataMutex);

    static const std::vector<double> fs {20, 25, 31.5, 40, 50, 63, 80, 100, 125, 160, 200, 250, 315, 400, 500, 630, 800, 1000, 1250, 1600, 2000,
                           2500, 3150, 4000, 5000, 6300, 8000, 10000, 12500};

    static const std::vector<double> af {0.532, 0.506, 0.480, 0.455, 0.432, 0.409, 0.387, 0.367, 0.349, 0.330, 0.315, 0.301, 0.288, 0.276, 0.267,
                           0.259, 0.253, 0.250, 0.246, 0.244, 0.243, 0.243, 0.243, 0.242, 0.242, 0.245, 0.254, 0.271, 0.301};

    static const std::vector<double> Lu {-31.6, -27.2, -23.0, -19.1, -15.9, -13.0, -10.3, -8.1, -6.2, -4.5, -3.1, -2.0, -1.1, -0.4, 0.0, 0.3, 0.5,
                           0.0, -2.7, -4.1, -1.0, 1.7, 2.5, 1.2, -2.1, -7.1, -11.2, -10.7, -3.1};

    static const std::vector<double> Tf {78.5, 68.7, 59.5, 51.1, 44.0, 37.5, 31.5, 26.5, 22.1, 17.9, 14.4, 11.4, 8.6, 6.2, 4.4, 3.0, 2.2, 2.4, 3.5,
                           1.7, -1.3, -4.2, -6.0, -5.4, -1.5, 6.0, 12.6, 13.9, 12.3};

    m_dataLength = fs.size();
    m_deconvolutionSize = 1;

    delete[] m_ftdata;
    m_ftdata = new FTData[fs.size()];

    for (size_t i = 0; i < fs.size(); ++i) {
        m_ftdata[i].frequency = fs[i];
        m_ftdata[i].phase = 1;
        m_ftdata[i].module = -INFINITY;
        m_ftdata[i].coherence = 1.f;

        auto Af = 4.47 * pow(10, -3) *
                (pow(10.0, (0.025 * loudness())) - 1.14) +
                pow((0.4 * pow(10.0, (((Tf[i] + Lu[i]) / 10) - 9 ))), af[i]);

        auto Lp = ((10 / af[i]) * log10(Af)) - Lu[i] + 94;

        m_ftdata[i].module = powf(10, (Lp - 140/*dB*/) / 20) * 10;
        m_ftdata[i].magnitude = pow(10, (Lp - loudness()) / 20);
    }

    guard.~lock_guard();
    emit readyRead();
}
