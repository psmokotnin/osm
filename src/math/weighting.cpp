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
#include "weighting.h"
#include <cmath>
#include <numeric>

Weighting::Weighting() : m_curve(A), m_sampleRate(48000), m_gain(1.0),
    m_filter1(F1, Filter::TimeExponential, F4),
    m_filter2(F2), m_filter3(F3),
    m_filter4(F4, Filter::TimeExponential, F4), m_filter5(F5)
{
    updateCoefficients();
}

float Weighting::operator()(const double &value)
{
    double result = m_gain * value;
    switch (m_curve) {
    case A:
        result = m_filter1(result);
        result = m_filter4(result);
        result = m_filter2(result);
        result = m_filter3(result);
        break;
    case B:
        result = m_filter1(result);
        result = m_filter4(result);
        result = m_filter5(result);
        break;
    case C:
        result = m_filter1(result);
        result = m_filter4(result);
        break;
    case K:
        //TODO:
        break;
    case Z:
        //flat response
        break;
    }

    return result;
}

unsigned int Weighting::sampleRate() const
{
    return m_sampleRate;
}

void Weighting::setSampleRate(unsigned int sampleRate)
{
    m_sampleRate = sampleRate;
}

Weighting::Curve Weighting::curve() const
{
    return m_curve;
}

void Weighting::setCurve(const Curve &curve)
{
    m_curve = curve;
}

void Weighting::updateCoefficients()
{
    m_filter1.calculate(m_sampleRate);
    m_filter2.calculate(m_sampleRate);
    m_filter3.calculate(m_sampleRate);
    m_filter4.calculate(m_sampleRate);
    m_filter5.calculate(m_sampleRate);

    switch (m_curve) {
    case A:
        m_gain = std::pow(10.0, A_GAIN / 20);
        break;
    case B:
        m_gain = std::pow(10.0, B_GAIN / 20);
        break;
    case C:
        m_gain = std::pow(10.0, C_GAIN / 20);
        break;
    case K:
        m_gain = 1.0;
        break;
    case Z:
        m_gain = 1.0;
        break;
    }
}

Weighting::Filter::Filter(double frequency, Mode mode, double numerator) :
    m_mode(mode), m_numerator(numerator), m_frequency(frequency),
    m_a(), m_b(), m_x(), m_y()
{
}

void Weighting::Filter::calculate(unsigned int sampleRate)
{
    std::fill(m_x.begin(), m_x.end(), 0);
    std::fill(m_y.begin(), m_y.end(), 0);

    double T = 1.0 / sampleRate;
    double w = 2.0 * M_PI * m_frequency;
    w = 2 * sampleRate * std::atan(w / (2 * sampleRate));

    switch (m_mode) {
    case Exponential: {
        m_b[0] =  1.0;
        m_b[1] = -1.0;
        m_b[2] =  0.0;

        m_a[0] =  1.0;
        m_a[1] = -std::pow(M_E, -w * T);
        m_a[2] =  0.0;
    }
    break;

    case TimeExponential: {
        double k = std::pow(M_E, -w * T);
        double g = k * T;
        double w_numerator = 2.0 * M_PI * m_numerator;
        w_numerator = 2 * sampleRate * std::atan(w_numerator / (2 * sampleRate));
        g *= w_numerator;

        m_b[0] =  0.0 * g;
        m_b[1] =  1.0 * g;
        m_b[2] = -1.0 * g;

        m_a[0] =  1.0;
        m_a[1] = -2.0 * k;
        m_a[2] =  1.0 * std::pow(k, 2);
    }
    break;
    }
}

float Weighting::Filter::operator()(const double &value)
{
    for (int i = 2; i > 0; --i) {
        m_x[i] = m_x[i - 1];
        m_y[i] = m_y[i - 1];
    }
    m_x[0] = value;
    m_y[0] = 0;

    auto y_iter = m_y.begin() + 1;
    float y = std::accumulate(m_a.cbegin() + 1, m_a.cend(), 0.f, [&](float acc, float value) {
        acc += value * (*y_iter);
        y_iter++;
        return acc;
    });

    auto x_iter = m_x.begin();
    float x = std::accumulate(m_b.cbegin(), m_b.cend(), 0.f, [&](float acc, float value) {
        acc += value * (*x_iter);
        x_iter++;
        return acc;
    });

    m_y[0] = (x - y) / m_a[0];
    return m_y[0];
}
