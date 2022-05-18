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
#ifndef WEIGHTING_H
#define WEIGHTING_H

#include <array>

class Weighting
{
public:
    enum Curve {A, B, C, K, Z};

    //! ANSI 1.43-1997
    constexpr static const double F1 = 20.598997;
    constexpr static const double F2 = 107.65265;
    constexpr static const double F3 = 737.86223;
    constexpr static const double F4 = 12194.217;
    constexpr static const double F5 = 158.489032;

    constexpr static const double A_GAIN = 1.9997 + 0.5;
    constexpr static const double B_GAIN = 0.1696 + 1.2;
    constexpr static const double C_GAIN = 0.0619 + 1.5;

    Weighting();

    float operator() (const double &value);

    unsigned int sampleRate() const;
    void setSampleRate(unsigned int sampleRate);

    Curve curve() const;
    void setCurve(const Curve &curve);

private:
    void updateCoefficients();

    Curve m_curve;
    unsigned int m_sampleRate;

    double m_gain;

    struct Filter {
        enum Mode {
            //! s / (s+a)
            Exponential,

            //! s / (s+a)^2
            TimeExponential
        };

        explicit Filter(double frequency, Mode mode = Exponential, double numerator = 0.5 / M_PI);

        void calculate(unsigned int sampleRate);
        float operator() (const double &value);

        Mode m_mode;
        double m_numerator;
        double m_frequency;

        std::array<double, 3> m_a, m_b, m_x, m_y;

    } m_filter1, m_filter2, m_filter3, m_filter4, m_filter5;
};

#endif // WEIGHTING_H
