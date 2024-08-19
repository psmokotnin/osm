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

#ifndef MATH_EQUALLOUDNESSCONTOUR_H
#define MATH_EQUALLOUDNESSCONTOUR_H

#include <vector>

namespace Math {

class EqualLoudnessContour
{
public:
    EqualLoudnessContour();
    void reset();
    double phone(double frequency, double loudness);

    static std::size_t size();
    static double frequency(std::size_t i);
    static double loudness(std::size_t i, double phone);
    static double magnitude(std::size_t i);
    static double phone(std::size_t i, double loudness);


private:
    std::size_t m_index = 0;

    static const std::vector<double> fs;
    static const std::vector<double> af;
    static const std::vector<double> Lu;
    static const std::vector<double> Tf;
};

} // namespace Math

#endif // MATH_EQUALLOUDNESSCONTOUR_H
