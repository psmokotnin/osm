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
#include "windowfunction.h"
#include <QtMath>

WindowFunction::WindowFunction(Type type, QObject *parent) : QObject(parent),
    m_type(type),
    m_size(0),
    m_gain(1.f)
{

}
const std::map<WindowFunction::Type, QString> WindowFunction::TypeMap = {
    {WindowFunction::Type::Rectangular, "Rectangular"},
    {WindowFunction::Type::Hann, "Hann"},
    {WindowFunction::Type::Hamming, "Hamming"},
    {WindowFunction::Type::FlatTop, "Flat Top"},
    {WindowFunction::Type::BlackmanHarris, "Blackman Harris"},
    {WindowFunction::Type::HFT223D, "HFT223D"},
    {WindowFunction::Type::Exponental, "Exponental"}
};
void WindowFunction::setSize(unsigned int size)
{
    if (m_size != size) {
        m_size = size;
        m_data.resize(m_size);
        calculate();
    }
}

unsigned int WindowFunction::size() const
{
    return m_size;
}

float WindowFunction::pointGain(float i, unsigned int N) const
{
    double z = 2.0 * M_PI * i / N;
    if (z < 0) {
        z = 0;
    }
    if (z > 2 * M_PI) {
        z = 2 * M_PI;
    }
    switch (m_type) {
    case Type::Rectangular:
        return 1.0;

    case Type::Hann:
        return pow(sin(z / 2), 2);

    case Type::Hamming:
        return 0.54 - 0.46 * cos(z);

    case Type::BlackmanHarris:
        return
            0.35875                - 0.48829 * cos(z      ) +
            0.14128 * cos(2.0 * z) - 0.01168 * cos(3.0 * z);

    case Type::FlatTop:
        return 1 -
               1.930 * cos(1 * z) + 1.290 * cos(2 * z) -
               0.388 * cos(3 * z) + 0.028 * cos(4 * z);

    case Type::HFT223D:
        return 1.0 -
               1.98298997309 * cos(z    ) + 1.75556083063 * cos(2 * z) -
               1.19037717712 * cos(3 * z) + 0.56155440797 * cos(4 * z) -
               0.17296769663 * cos(5 * z) + 0.03233247087 * cos(6 * z) -
               0.00324954578 * cos(7 * z) + 0.00013801040 * cos(8 * z) -
               0.00000132725 * cos(9 * z);

    case Type::Exponental: {
        auto t = (N / 2.0) * 8.69 / (7 * 8.69); //-140dB on edges
        auto power = - std::abs(i - N / 2) / t;
        return std::pow(M_E, power);
    }
    }
}

WindowFunction::Type WindowFunction::type() const
{
    return m_type;
}
void WindowFunction::setType(Type t)
{
    m_type = t;
    calculate();
}
QVariant WindowFunction::getTypes()
{
    QStringList typeList;
    for (const auto &type : TypeMap) {
        typeList << type.second;
    }
    return typeList;
}

const float &WindowFunction::get(unsigned int k) const
{
    return m_data[k];
}

QString WindowFunction::name(Type type) noexcept
{
    return WindowFunction::TypeMap.at(type);
}

float WindowFunction::gain() const
{
    return m_gain;
}

float WindowFunction::norm() const
{
    return m_norm;
}

void WindowFunction::calculate()
{
    float cg = 0.0;
    for (unsigned int i = 0; i < m_size; i++) {
        cg += pointGain(i, m_size);
    }
    m_gain = cg / m_size;
    m_norm = pointGain(1, 2);

    for (unsigned int i = 0; i < m_size; i++) {
        m_data[i] = pointGain(i, m_size) / m_gain;
    }
}
QDebug operator<<(QDebug dbg, const WindowFunction::Type &t)
{
    dbg.nospace() << "WindowFunction type:" << WindowFunction::TypeMap.at(t);
    return dbg.maybeSpace();
}
