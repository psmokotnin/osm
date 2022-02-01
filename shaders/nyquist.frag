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
#version 330
#ifdef GL_ES
precision mediump float;
#endif

uniform vec4 m_color;
uniform float coherenceThreshold;
uniform bool coherenceAlpha;

in fData
{
    float t;
    vec4 coherenceSpline;
} fragmentData;

out vec4 fragColor;

void main() {
    fragColor = m_color;
    if (!coherenceAlpha) {
        return;
    }

    float
            alpha, k, b,
            t = fragmentData.t;

    alpha = fragmentData.coherenceSpline[0] +
            fragmentData.coherenceSpline[1] * t +
            fragmentData.coherenceSpline[2] * t * t +
            fragmentData.coherenceSpline[3] * t * t * t;
    if (alpha >= 1.0) {
        return;
    }
    if (alpha < coherenceThreshold) {
        fragColor.a = 0.0;
    } else {
        k = 1.0 / (1.0 - coherenceThreshold);
        b = -k * coherenceThreshold;
        fragColor.a = sqrt(k * alpha + b);
    }
}
