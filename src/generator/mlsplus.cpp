/**
 *  OSM
 *  Copyright (C) 2023  Pavel Smokotnin

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
#include "mlsplus.h"

MLSPlus::MLSPlus(QObject *parent)
    : OutputDevice{parent}, m_state{}
{
    m_name = "MLS+";
    std::fill(m_state.begin(), m_state.end(), false);
    m_state[0] = true;
}

Sample MLSPlus::sample()
{
    static std::size_t counter = 0;
    if (++counter == 65'536) {
        counter = 0;
        return { 0 };
    }
    /*
     * W. Stahnke.
     * Primitive binary polynomials.
     * Mathematics of Computation, 27:977-980, 1973.
     *
     * @url https://www.ams.org/journals/mcom/1973-27-124/S0025-5718-1973-0327722-7/S0025-5718-1973-0327722-7.pdf
     */
    bool input  = m_state[16];
    m_state[16] = m_state[15];
    m_state[15] = m_state[14];
    m_state[14] = m_state[13];
    m_state[13] = m_state[12];
    m_state[12] = m_state[11];
    m_state[11] = m_state[10];
    m_state[10] = m_state[9];
    m_state[9]  = m_state[8];
    m_state[8]  = m_state[7];
    m_state[7]  = m_state[6];
    m_state[6]  = m_state[5];
    m_state[5]  = m_state[4] != input;
    m_state[4]  = m_state[3];
    m_state[3]  = m_state[2] != input;
    m_state[2]  = m_state[1] != input;
    m_state[1]  = m_state[0];
    m_state[0]  = input;

    Sample s = { m_state[16] ? 1.f : -1.f };

    // -18dB to align rms level with pink noise
    s.f *= m_gain / 8;

    return s;
}
