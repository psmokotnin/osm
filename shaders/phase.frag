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
uniform vec4 m_color;
uniform vec4 splineRe;
uniform vec4 splineIm;
uniform float frequency1;
uniform float frequency2;
uniform float width;
uniform vec2 screen;
uniform vec4 minmax;
uniform vec4 coherenceSpline;

float spline(float t) {
    float re =
            splineRe[0] +
            splineRe[1] * t +
            splineRe[2] * t*t +
            splineRe[3] * t*t*t
    ;
    float im =
            splineIm[0] +
            splineIm[1] * t +
            splineIm[2] * t*t +
            splineIm[3] * t*t*t
    ;
    float alpha = atan(im, re);

    return screen.y - (alpha - minmax[2]) * screen.y / (minmax[3] - minmax[2]);
}

void main() {

    vec4 color = m_color;
    float st = 0.0;
    float
            currentY, t,
            xs = frequency1,
            xe = frequency2;
    float dist;
    vec2 currentDist, coord = gl_FragCoord.xy;

    for (float i = -1. * width; i < width; i += 0.1) {
        t = (gl_FragCoord.x + i - xs) / (xe - xs);
        currentY = spline(t);
        currentDist.x = gl_FragCoord.x + i;
        currentDist.y = currentY;

        dist = distance(coord, currentDist);
        if (dist < width / 2.0) {
            gl_FragColor = m_color;
            gl_FragColor.a = coherenceSpline[0] +
                    coherenceSpline[1] * i +
                    coherenceSpline[2] * i*i +
                    coherenceSpline[3] * i*i*i;
            return;
        }
    }

    gl_FragColor = vec4(0.0);
}
