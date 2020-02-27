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
varying vec4 currentVertexPosition;
varying vec4 preVertexPosition;

attribute highp vec4 posAttr;
uniform highp vec4 prePosition;
uniform highp vec4 postPosition;
uniform highp mat4 matrix;

void main() {
    vec4 p = posAttr;
    p.x = log(p.x);
    gl_Position = matrix * p;

    p = prePosition;
    p.x = log(p.x);
    preVertexPosition = matrix * p;

    p = postPosition;
    p.x = log(p.x);
    currentVertexPosition = matrix * p;
}
