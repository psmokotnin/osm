/**
 *  OSM
 *  Copyright (C) 2024  Pavel Smokotnin
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef FIRDESIGN_H
#define FIRDESIGN_H

#include <vector>
#include "complex.h"

//! Design of correction FIR filters from a measured transfer function.
//! Pure DSP, no Qt object dependencies, so it can be tested in isolation.
namespace FirDesign {

enum PhaseMode {
    LinearPhase  = 0,   //!< magnitude correction only, symmetric taps, latency taps/2
    MinimumPhase = 1,   //!< magnitude correction only, minimum phase, no bulk latency
    FullPhase    = 2    //!< magnitude and excess phase correction, latency taps/2
};

//! Measured response, ascending frequency, all frequencies > 0.
struct Input {
    std::vector<float>   frequency;
    std::vector<float>   magnitude;     //!< linear
    std::vector<Complex> phase;         //!< unit phasor, a delay tau is exp(-j*2pi*f*tau)
    std::vector<float>   coherence;     //!< 0..1, empty or all zero = not available
    float                delay = 0.f;   //!< seconds, removed from the phase in FullPhase mode
};

struct Parameters {
    unsigned int sampleRate = 48000;
    unsigned int taps       = 512;
    PhaseMode    phaseMode  = LinearPhase;
    int          smoothing  = 0;        //!< 1/N octave, 0 = off
    float        lowLimit   = 0.f;      //!< Hz, correction fades out below, 0 = lowest frequency the taps resolve
    float        highLimit  = 0.f;      //!< Hz, correction fades out above, 0 = end of measured data
    float        transition = 3.f;      //!< exponent of the fade outside the limits, larger is steeper
    float        maxBoost   = 6.f;      //!< dB
    float        maxCut     = 24.f;     //!< dB
};

struct Result {
    std::vector<float>   taps;
    unsigned int         latency = 0;   //!< bulk delay of the taps in samples
    std::vector<Complex> response;      //!< realised response at Input::frequency, bulk delay removed
};

//! lowest frequency a filter of the given length can resolve
float minFrequency(unsigned int sampleRate, unsigned int taps);

Result design(const Input &input, const Parameters &parameters);

}

#endif // FIRDESIGN_H
