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
#ifndef CHART_FREQUENCYBASEDSERIESHELPER_H
#define CHART_FREQUENCYBASEDSERIESHELPER_H

#include <QtCore>
#include "abstract/source.h"

namespace Chart {

class FrequencyBasedSeriesHelper
{
public:
    explicit FrequencyBasedSeriesHelper();

protected:
    constexpr const static float LEVEL_NORMALIZATION = 0;

    virtual const Shared::Source &source() const = 0;
    void iterate(const unsigned int &pointsPerOctave,
                 const std::function<void(const unsigned int &)> &accumulate,
                 const std::function<void(const float &start, const float &end, const unsigned int &count)> &collected
                );

    template<typename T, typename TSpline = T> void iterateForSpline(const unsigned int &pointsPerOctave,
                                                                     T *value, float *coherence,
                                                                     const std::function<void (const unsigned int &)> &accumulate,
                                                                     const std::function<void(const float &f1, const float &f2, const TSpline *a, const float *c)> &collected,
                                                                     const std::function<TSpline(const T *value, const float *f, const unsigned int &count)> &beforeSpline = {}
                                                                    )
    {
        bool bCollected = false;
        unsigned int bCount = 0;
        TSpline splinePoint[4], a[4];
        float csplinePoint[4], f[4], c[4];
        const T resetValue = *value;

        auto it = [value, coherence, &collected, &splinePoint, &csplinePoint,
                          &f, &a, &c, &bCollected, &beforeSpline, &bCount, &resetValue]
        (const float & bandStart, const float & bandEnd, const unsigned int &count) {

            *coherence /= count;

            if (bCollected) {
                splinePoint[0]  = splinePoint[1];
                f[0] = f[1];
                splinePoint[1]  = splinePoint[2];
                f[1] = f[2];
                splinePoint[2]  = splinePoint[3];
                f[2] = f[3];

                csplinePoint[0]  = csplinePoint[1];
                csplinePoint[1]  = csplinePoint[2];
                csplinePoint[2]  = csplinePoint[3];
            }
            f[bCount] = (bandStart + bandEnd) / 2.f;
            if (beforeSpline) {
                splinePoint[bCount] = beforeSpline(value, f, count);
            } else if constexpr (std::is_same<T, TSpline>::value) {
                splinePoint[bCount] = (*value) / count;
            } else {
                qDebug() << "could not convert TSpline to T";
            }
            csplinePoint[bCount] = *coherence;

            if (bCount == 3) {
                a[0] = (splinePoint[0]      + splinePoint[1] * 4 + splinePoint[2]) / 6;
                a[1] = (splinePoint[0] * -1 +                      splinePoint[2]) / 2;
                a[2] = (splinePoint[0]      - splinePoint[1] * 2 + splinePoint[2]) / 2;
                a[3] = (splinePoint[0] * -1 + splinePoint[1] * 3 - splinePoint[2] * 3 + splinePoint[3]) / 6;

                c[0] = (     csplinePoint[0] + 4 * csplinePoint[1] +     csplinePoint[2]) / 6;
                c[1] = (-1 * csplinePoint[0] +                           csplinePoint[2]) / 2;
                c[2] = (     csplinePoint[0] - 2 * csplinePoint[1] +     csplinePoint[2]) / 2;
                c[3] = (-1 * csplinePoint[0] + 3 * csplinePoint[1] - 3 * csplinePoint[2] + csplinePoint[3]) / 6;
                bCollected = true;
                collected(f[1], f[2], a, c);
            } else {
                ++bCount;
                *value = resetValue;
            }
        };
        iterate(pointsPerOctave, accumulate, it);

        if (bCount == 3) {
            splinePoint[0]  = splinePoint[1];
            splinePoint[1]  = splinePoint[2];
            splinePoint[2]  = splinePoint[3];
            splinePoint[3] = a[0] + a[1] * 2 + a[2] * 4 + a[3] * 8;

            csplinePoint[0]  = csplinePoint[1];
            csplinePoint[1]  = csplinePoint[2];
            csplinePoint[2]  = csplinePoint[3];
            csplinePoint[3] = c[0] + c[1] * 2 + c[2] * 4 + c[3] * 8;

            a[0] = (splinePoint[0]      + splinePoint[1] * 4 + splinePoint[2]) / 6;
            a[1] = (splinePoint[0] * -1 +                      splinePoint[2]) / 2;
            a[2] = (splinePoint[0]      - splinePoint[1] * 2 + splinePoint[2]) / 2;
            a[3] = (splinePoint[0] * -1 + splinePoint[1] * 3 - splinePoint[2] * 3 + splinePoint[3]) / 6;

            c[0] = (     csplinePoint[0] + 4 * csplinePoint[1] +     csplinePoint[2]) / 6;
            c[1] = (-1 * csplinePoint[0] +                           csplinePoint[2]) / 2;
            c[2] = (     csplinePoint[0] - 2 * csplinePoint[1] +     csplinePoint[2]) / 2;
            c[3] = (-1 * csplinePoint[0] + 3 * csplinePoint[1] - 3 * csplinePoint[2] + csplinePoint[3]) / 6;

            collected(f[2], f[3], a, c);
        }
    }


};

} // namespace chart

#endif // CHART_FREQUENCYBASEDSERIESHELPER_H
