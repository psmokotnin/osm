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
#ifndef FOURIERTRANSFORM_H
#define FOURIERTRANSFORM_H

#include "complex.h"
#include "windowfunction.h"
#include "container/array.h"

#if defined(Q_PROCESSOR_X86_64)
#include "ssemath.h"
#endif
#if defined(Q_PROCESSOR_ARM)
#include "armmath.h"
#endif

class FourierTransform
{
public:
    enum Type {Fast, Log};

private:
    unsigned int m_size;
    unsigned int m_pointer;
    Type m_type;
    WindowFunction m_window;

    //! income data channel
    container::array<float> m_inA, m_inB;

    //! fft swap map
    container::array<unsigned int> m_swapMap;

    struct LogBasisVector {
        unsigned int N;
        float frequency;
        std::vector<v4sf> w;
    };
    container::array<LogBasisVector> m_logBasis;

    //! containers for fast transform
    container::array<complex> m_fastA, m_fastB, m_wlen;

public:
    FourierTransform(unsigned int size = 2);

    //! set input buffer size
    void setSize(unsigned int size);
    unsigned int size() const
    {
        return m_size;
    }

    //! set transform type
    void setType(Type type);

    //! set type of applied window function
    void setWindowFunctionType(WindowFunction::Type type);

    //! return vector with frequency list for current type
    std::vector<float> getFrequencies(unsigned int sampleRate);

    //! add sample to the end of transformed buffer
    void add(float sampleA, float sampleB);

    //! set data in tranformed data
    void set(unsigned int i, const complex &a, const complex &b);

    //! run FFT ultrafast - speed up the FFT, but result can't be used for reverse fft
    void fast(bool reverse = false, bool ultrafast = false);

    //! run FFT with setted ultrafast
    void ufast()
    {
        fast(false, true);
    }

    //! run transform
    void transform(bool ultra = false);

    //! run reverse transform
    void reverse();

    //! run log transform
    void log();

    //! prepare transform for current type
    void prepare();

    //! prepare buffers for FFT. Must be called after setting size and before fast.
    void prepareFast();

    //! prepare log transform
    void prepareLog();

    //! return i in fast transform for given frequency and sampleRate
    long f2i(double frequency, int sampleRate) const;

    unsigned long pointer() const
    {
        return m_pointer;
    }

    //! return fast transform result for channel A
    complex af(unsigned int i) const;
    //! return fast transform result for channel B
    complex bf(unsigned int i) const;
};

#endif // FOURIERTRANSFORM_H
