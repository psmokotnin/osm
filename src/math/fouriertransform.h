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
    enum Type { Fast, Log};
    enum Norm { Sqrt, Lin};
    enum Align { Right, Center};

    FourierTransform(unsigned int size = 2);

    //! set input buffer size
    void setSize(unsigned int size);
    unsigned int size() const;

    //! set transform type
    void setType(Type type);
    Type type() const;

    //! set type of applied window function
    void setWindowFunctionType(WindowFunction::Type type);

    //! return vector with frequency list for current type
    std::vector<float> getFrequencies();

    //! add sample to the end of transformed buffer
    void add(float sampleA, float sampleB);

    //! set data in tranformed data
    void set(unsigned int i, const Complex &a, const Complex &b);

    //! run FFT ultrafast - speed up the FFT, but result can't be used for reverse fft
    void fast(bool reverse = false, bool ultrafast = false);

    //! run FFT with setted ultrafast
    void ufast();

    //! run transform
    void transform(bool ultra = false);

    //! run reverse transform
    void reverse();

    //! run reverse only for A transform
    void transformSingleChannel(bool reverse = true);

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

    //! return current position in circular buffer
    unsigned long pointer() const;

    //! return value in input buffer at current position
    float aIn() const;
    float bIn() const;

    //! return fast transform result for channel A
    Complex af(unsigned int i) const;
    //! return fast transform result for channel B
    Complex bf(unsigned int i) const;

    unsigned int sampleRate() const;
    void setSampleRate(unsigned int sampleRate);

    void reset();

    void setNorm(Norm newNorm);

    void setAlign(Align newAlign);


    void setLogWindowDenominator(unsigned int newLogWindowDenominator);

private:
    unsigned int m_size;
    unsigned int m_pointer;
    unsigned int m_sampleRate;
    unsigned int m_logWindowDenominator;

    Type m_type;
    Norm m_norm = Sqrt;
    Align m_align = Right;
    WindowFunction m_window;

    //! income data channel
    Container::array<float> m_inA, m_inB;

    //! fft swap map
    Container::array<unsigned int> m_swapMap;

    struct LogBasisVector {
        unsigned int N;
        float frequency;
        std::vector<v4sf> w;
    };
    Container::array<LogBasisVector> m_logBasis;

    //! containers for fast transform
    Container::array<Complex> m_fastA, m_fastB, m_wlen;
};

#endif // FOURIERTRANSFORM_H
