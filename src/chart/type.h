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
#ifndef TYPE_H
#define TYPE_H

#include <map>
#include <QString>

namespace Chart {

enum Type {RTA, Magnitude, Phase, Scope, Impulse, Step, Coherence, GroupDelay, PhaseDelay, Spectrogram, CrestFactor, Nyquist, Level, SPL};
static std::map<Type, QString> typeMap = {
    {RTA,       "Spectrum"},
    {Magnitude, "Magnitude"},
    {Phase,     "Phase"},
    {Scope,     "Scope"},
    {Impulse,   "Impulse"},
    {Step,      "Step"},
    {Coherence, "Coherence"},
    {GroupDelay, "Group Delay"},
    {PhaseDelay, "Phase Delay"},
    {Spectrogram, "Spectrogram"},
    {CrestFactor, "Crest Factor"},
    {Nyquist,   "Nyquist"},
    {Level,     "Level"},
    {SPL,       "Numeric"}
};
}
#endif // TYPE_H
