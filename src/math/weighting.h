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
#include <map>
#include <QtMath>
#include <QVariant>
#include "math/biquad.h"

class Weighting : public math::Filter
{
public:
    enum Curve {
        A = 0,
        B = 1,
        C = 2,
        K = 3,
        Z = 4
    };

    //! ANSI 1.43-1997
    constexpr static const double F1 = 20.598997;
    constexpr static const double F2 = 107.65265;
    constexpr static const double F3 = 737.86223;
    constexpr static const double F4 = 12194.217;
    constexpr static const double F5 = 158.489032;

    constexpr static const double A_GAIN = 1.9997 + 0.8;
    constexpr static const double B_GAIN = 0.1696 + 1.2;
    constexpr static const double C_GAIN = 0.0619 + 1.3;

    Weighting(Curve curve = C, unsigned int sampleRate = 48000);
    virtual ~Weighting() = default;

    float operator() (const float &value) override;

    unsigned int sampleRate() const;
    void setSampleRate(unsigned int sampleRate);

    Curve curve() const;
    void setCurve(const Curve &curve);

    static constexpr Curve allCurves[] = {A, B, C, /*K,*/ Z};
    static QVariant availableCurves();
    static QString curveName(Curve time);
    static Curve curveByName(QString name);

private:
    void updateCoefficients();

    Curve m_curve;
    unsigned int m_sampleRate;

    float m_gain;

    struct WeghtingFilter : math::BiQuad {
        enum Mode {
            //! s / (s+a)
            Exponential,

            //! s / (s+a)^2
            TimeExponential
        };

        explicit WeghtingFilter(double frequency, Mode mode = Exponential, double numerator = 0.5 / M_PI);

        void calculate(unsigned int sampleRate);

        Mode m_mode;
        double m_numerator;
        double m_frequency;

    } m_filter1, m_filter2, m_filter3, m_filter4, m_filter5;

    static const std::map<Curve, QString> m_curveMap;
};

#endif // WEIGHTING_H
