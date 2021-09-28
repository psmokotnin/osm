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
uniform highp mat4 matrix;

layout (location = 0) in vec4 splineRe;
layout (location = 1) in vec4 splineIm;
layout (location = 2) in vec4 coherenceSpline;

out vData
{
    vec4 splineRe;
    vec4 splineIm;
    vec4 coherenceSpline;
} vertex;

void main() {
    vertex.splineRe = splineRe;
    vertex.splineIm = splineIm;
    vertex.coherenceSpline = coherenceSpline;
}
