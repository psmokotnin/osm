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
    {WindowFunction::Type::rectangular, "Rectangular"},
    {WindowFunction::Type::hann, "Hann"},
    {WindowFunction::Type::hamming, "Hamming"},
    {WindowFunction::Type::flat_top, "Flat Top"},
    {WindowFunction::Type::blackman_harris, "Blackman Harris"},
    {WindowFunction::Type::HFT223D, "HFT223D"}
};
void WindowFunction::setSize(unsigned int size)
{
    if (m_size != size) {
        m_size = size;
        m_data.resize(m_size);
        calculate();
    }
}
void WindowFunction::setType(Type t)
{
    m_type = t;
    calculate();
}
QVariant WindowFunction::getTypes() const
{
    QStringList typeList;
    for (const auto &type : TypeMap) {
        typeList << type.second;
    }
    return typeList;
}
void WindowFunction::calculate()
{
    float cg = 0.0;
    for (unsigned int i = 0; i < m_size; i++) {
        double z = 2.0 * M_PI * i / m_size, dataTemp;

        switch (m_type) {

            case Type::rectangular:
                dataTemp = 1.0;
                break;

            case Type::hann:
                dataTemp = pow(sin(M_PI * i / m_size), 2);
                break;

            case Type::hamming:
                dataTemp = 0.54 - 0.46 * cos(z);
                break;

            case Type::blackman_harris:
                dataTemp =
                        0.35875                - 0.48829 * cos(z      ) +
                        0.14128 * cos(2.0 * z) - 0.01168 * cos(3.0 * z);
                break;

            case Type::flat_top:
                dataTemp = 1 -
                        1.930 * cos(2 * z) + 1.290 * cos(4 * z) -
                        0.388 * cos(6 * z) + 0.028 * cos(8 * z);
                break;

            case Type::HFT223D:
                dataTemp = 1.0 -
                    1.98298997309 * cos(z    ) + 1.75556083063 * cos(2 * z) -
                    1.19037717712 * cos(3 * z) + 0.56155440797 * cos(4 * z) -
                    0.17296769663 * cos(5 * z) + 0.03233247087 * cos(6 * z) -
                    0.00324954578 * cos(7 * z) + 0.00013801040 * cos(8 * z) -
                    0.00000132725 * cos(9 * z);
                break;
        }
        m_data[i] = static_cast<float>(dataTemp);
        cg += m_data[i];
    }

    m_gain = cg / m_size;
}
QDebug operator<<(QDebug dbg, const WindowFunction::Type &t)
{
    dbg.nospace() << "WindowFunction type:" << WindowFunction::TypeMap.at(t);
    return dbg.maybeSpace();
}
