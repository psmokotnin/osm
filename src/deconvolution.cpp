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
#include "deconvolution.h"
#include <complex>
Deconvolution::Deconvolution(unsigned int size) :
    m_size(size),
    fft(size),
    ifft(size)
{
    m_data.resize(m_size, 0.f);
    fft.prepareFast();
    ifft.prepareFast();
}
void Deconvolution::add(float in, float out)
{
    fft.add(in, out);
}
void Deconvolution::transform(WindowFunction *window)
{
    //direct
    fft.fast(window);

    //devision
    for (unsigned int i = 0; i < m_size; i++) {
        ifft.set(i, fft.bf(i) / fft.af(i), 0.f);
    }
    //reverse
    ifft.fast(nullptr, true, true);

    for (unsigned int i = 0; i < m_size; i++) {
        m_data[i] = ifft.af(i).real / m_size;
    }
}
float Deconvolution::get(const unsigned int i) const
{
#ifdef WIN64
    if (m_data[i]/0.f == m_data[i]) {
#else
    if (std::isnan(m_data[i])) {
#endif
        return 0.f;
    }
    return m_data[i];
}
void Deconvolution::setSize(unsigned int size)
{
    m_size = size;
    m_data.resize(m_size, 0.f);
    fft.setSize(m_size);
    ifft.setSize(m_size);
    fft.prepareFast();
    ifft.prepareFast();
}
