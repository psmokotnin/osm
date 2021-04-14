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

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 frequencyRange;
layout (location = 2) in vec4 splineData;

out vData
{
    vec4 splineData;
    vec2 frequency;
} vertex;

void main() {
    gl_Position.x = matrix[0][0] * log(position.x) + matrix[3][0];
    gl_Position.y = matrix[0][0] * log(position.y) + matrix[3][0];
    gl_Position.z = 0;
    gl_Position.w = 1;

    vertex.splineData = splineData;
    vertex.frequency = frequencyRange;
}
