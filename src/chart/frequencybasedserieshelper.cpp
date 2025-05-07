/**
 *  OSM
 *  Copyright (C) 2021  Pavel Smokotnin

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
#include "frequencybasedserieshelper.h"

namespace Chart {

FrequencyBasedSeriesHelper::FrequencyBasedSeriesHelper()
{

}

void FrequencyBasedSeriesHelper::iterate(const unsigned int &pointsPerOctave,
                                         const std::function<void (const unsigned int &)> &accumulate,
                                         const std::function<void (const float &, const float &, const unsigned int &)> &collected)
{
    unsigned int count = 0;

    constexpr const float startFrequency = 24000.f / 2048.f;//pow(2, 11);

    float _frequencyFactor = powf(2.f, 1.f / pointsPerOctave);
    float bandStart = startFrequency,
          bandEnd   = bandStart * _frequencyFactor,
          lastBandEnd = bandStart,
          frequency;

    if (!source() || _frequencyFactor < 1) {
        return;
    }

    for (unsigned int i = 1; i < source()->frequencyDomainSize(); ++i) {
        frequency = source()->frequency(i);
        if (frequency < bandStart) continue;

        if (pointsPerOctave > 0) {
            while (frequency > bandEnd) {

                if (count) {
                    collected(lastBandEnd, bandEnd, count);
                    count = 0;

                    //extend current band to the end of the pervious collected
                    lastBandEnd = bandEnd;
                }

                bandStart = bandEnd;
                bandEnd   *= _frequencyFactor;
            }
        } else {
            if (count) {
                auto delta = (frequency - lastBandEnd) / 2;
                collected(frequency - delta, frequency + delta, count);
            }
            lastBandEnd = frequency;
            count = 0;
        }
        count ++;
        accumulate(i);
    }
    if (count) {
        collected(lastBandEnd, bandEnd, count);
    }
}

} // namespace chart
