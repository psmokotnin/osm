/**
 *  OSM
 *  Copyright (C) 2022  Pavel Smokotnin
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
attribute highp vec2 position;
attribute vec4 color;
varying vec4 frag_color;
uniform highp mat4 matrix;

void main() {
    vec4 p;
    p.x = position.x;
    p.y = position.y;
    p.z = 0.0;
    p.w = 1.0;

    gl_Position = matrix * p;
    frag_color = color;
}
