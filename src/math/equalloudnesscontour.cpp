/**
 *  OSM
 *  Copyright (C) 2024  Pavel Smokotnin

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

#include <cmath>
#include "math/equalloudnesscontour.h"
#include <QDebug>
namespace Math {

EqualLoudnessContour::EqualLoudnessContour()
{
    reset();
}

void EqualLoudnessContour::reset()
{
    m_index = 0;
}

double EqualLoudnessContour::phone(double frequency, double loudness)
{

    for (; m_index < fs.size() - 1; ++m_index) {
        if (fs[ m_index + 1 ] > frequency) {
            break;
        }
    }

    auto f1     = fs[m_index];
    auto f2     = fs[m_index + 1];
    auto phone1 = phone(m_index,     loudness);
    auto phone2 = phone(m_index + 1, loudness);

    auto k = (phone2 - phone1) / (f2 - f1);
    auto b = phone1 - k * f1;

    if ((frequency >= f1) && (frequency <= f2)) {
        return k * frequency + b;
    }
    return NAN;
}

std::size_t EqualLoudnessContour::size()
{
    return fs.size();
}

double EqualLoudnessContour::frequency(std::size_t i)
{
    return fs[i];
}

double EqualLoudnessContour::loudness(std::size_t i, double phone)
{
    auto Af = 4.47 * pow(10, -3) *
              (pow(10.0, (0.025 * phone)) - 1.14) +
              pow((0.4 * pow(10.0, (((Tf[i] + Lu[i]) / 10) - 9 ))), af[i]);

    return ((10 / af[i]) * log10(Af)) - Lu[i] + 94;
}

double EqualLoudnessContour::magnitude(std::size_t i)
{
    return Lu[i];
}

double EqualLoudnessContour::phone(std::size_t i, double loudness)
{
    loudness += Lu[i] - 94;
    loudness *= af[i] / 10;

    auto Af = std::pow(10, loudness);
    auto C = pow((0.4 * pow(10.0, (((Tf[i] + Lu[i]) / 10) - 9 ))), af[i]);

    Af -= C;
    Af /= 4.47 * pow(10, -3);
    Af += 1.14;

    return std::log10(Af) / 0.025;
}

const std::vector<double> EqualLoudnessContour::fs {20, 25, 31.5, 40, 50, 63, 80, 100, 125, 160, 200, 250, 315, 400, 500, 630, 800, 1000, 1250, 1600, 2000,
          2500, 3150, 4000, 5000, 6300, 8000, 10000, 12500};

const std::vector<double> EqualLoudnessContour::af {0.532, 0.506, 0.480, 0.455, 0.432, 0.409, 0.387, 0.367, 0.349, 0.330, 0.315, 0.301, 0.288, 0.276, 0.267,
          0.259, 0.253, 0.250, 0.246, 0.244, 0.243, 0.243, 0.243, 0.242, 0.242, 0.245, 0.254, 0.271, 0.301};

const std::vector<double> EqualLoudnessContour::Lu {-31.6, -27.2, -23.0, -19.1, -15.9, -13.0, -10.3, -8.1, -6.2, -4.5, -3.1, -2.0, -1.1, -0.4, 0.0, 0.3, 0.5,
          0.0, -2.7, -4.1, -1.0, 1.7, 2.5, 1.2, -2.1, -7.1, -11.2, -10.7, -3.1};

const std::vector<double> EqualLoudnessContour::Tf {78.5, 68.7, 59.5, 51.1, 44.0, 37.5, 31.5, 26.5, 22.1, 17.9, 14.4, 11.4, 8.6, 6.2, 4.4, 3.0, 2.2, 2.4, 3.5,
          1.7, -1.3, -4.2, -6.0, -5.4, -1.5, 6.0, 12.6, 13.9, 12.3};

} // namespace Math
