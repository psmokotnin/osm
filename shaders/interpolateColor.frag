#version 120
/**
 *  OSM
 *  Copyright (C) 2020  Pavel Smokotnin

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

uniform vec4 m_colorLeft;
uniform vec4 m_colorRight;
uniform vec2 screen;

varying vec4 currentVertexPosition;
varying vec4 preVertexPosition;

void main(void)
{
    float k = 1.0 / (currentVertexPosition.x - preVertexPosition.x);
    float b = 1.0 - currentVertexPosition.x * k;

    gl_FragColor = mix(
                m_colorLeft,
                m_colorRight,
                k * (2.0 * gl_FragCoord.x / screen.x - 1.0) + b
    );
}
