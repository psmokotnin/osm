/**
 *  OSM
 *  Copyright (C) 2019  Pavel Smokotnin

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
//precision mediump float;

uniform vec4 m_color;
uniform vec4 splineA;
uniform float frequency1;
uniform float frequency2;
uniform float width;
uniform vec2 screen;
uniform vec4 minmax;
uniform vec4 coherenceSpline;
uniform float coherenceThreshold;
uniform bool coherenceAlpha;

float spline(float t) {
    float y =
            splineA[0] +
            splineA[1] * t +
            splineA[2] * t*t +
            splineA[3] * t*t*t
    ;
    return screen.y - (y - minmax[2]) * screen.y / (minmax[3] - minmax[2]);
}

void main() {

    vec4 color = m_color;
    float st = 0.0;
    float
            currentY, t,
            xs = frequency1,
            xe = frequency2;
    float dist, alpha;
    vec2 currentDist, coord = gl_FragCoord.xy;

    for (float i = -1. * width; i < width; i += 0.1) {
        t = (gl_FragCoord.x + i - xs) / (xe - xs);
        currentY = spline(t);
        currentDist.x = gl_FragCoord.x + i;
        currentDist.y = currentY;

        dist = distance(coord, currentDist);
        if (dist < width / 2.0) {
            gl_FragColor = m_color;
            if (coherenceAlpha) {
                alpha = coherenceSpline[0] +
                        coherenceSpline[1] * t +
                        coherenceSpline[2] * t*t +
                        coherenceSpline[3] * t*t*t;
                if (alpha < coherenceThreshold) {
                    discard;
                } else {
                    float k = 1.0 / (1.0 - coherenceThreshold),
                          b = -k * coherenceThreshold;
                    gl_FragColor.a = sqrt(k * alpha + b);
                }
            }
            return;
        }
    }

    gl_FragColor = vec4(0.0);
}
