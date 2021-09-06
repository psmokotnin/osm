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
    m_fft(size),
    m_ifft(size)
{
    m_data.resize(m_size, 0.f);
    m_fft.prepareFast();
    m_ifft.prepareFast();
}
void Deconvolution::add(float in, float out)
{
    m_fft.add(in, out);
}
void Deconvolution::transform()
{
    //direct
    m_fft.transform();

    //devision
    for (unsigned int i = 0; i < m_size; i++) {
        m_ifft.set(i, m_fft.bf(i) / m_fft.af(i), 0.f);
    }
    //reverse
    m_ifft.reverse();

    for (unsigned int i = 0; i < m_size; i++) {
        m_data[i] = m_ifft.af(i).real / sqrtf(m_size);
    }
}
float Deconvolution::get(const unsigned int i) const
{
#ifdef WIN64
    if (m_data[i] / 0.f == m_data[i]) {
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
    m_fft.setSize(m_size);
    m_ifft.setSize(m_size);
    m_fft.prepareFast();
    m_ifft.prepareFast();
}
void Deconvolution::setWindowFunctionType(WindowFunction::Type type)
{
    m_fft.setWindowFunctionType(type);
}
