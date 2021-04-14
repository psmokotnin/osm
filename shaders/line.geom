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

layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

uniform vec2 screen;
uniform float width;

void setVertexData()
{
    gl_Position.z = gl_in[0].gl_Position.z;
    gl_Position.w = gl_in[0].gl_Position.w;
}

void main(void)
{
    float
        kx = screen[0] / 2.,
        ky = screen[1] / 2.
        ;
    vec2 p1, p2;
    vec2 d, n;
    mat2 r = mat2(
        0.0, -1.0,
        1.0, 0.0
    );

        p1 = gl_in[0].gl_Position.xy;
        p2 = gl_in[1].gl_Position.xy;

        d = p2.xy - p1.xy;
        d.x *= kx;
        d.y *= ky;
        d /= sqrt(d.x * d.x + d.y * d.y);

        n = d * width / 2.;
        n = r * n;
        n.x /= kx;
        n.y /= ky;
        d.x /= kx;
        d.y /= ky;

        setVertexData();
        gl_Position.xy = p1.xy + n - d;
        EmitVertex();

        setVertexData();
        gl_Position.xy = p2.xy + n + d;
        EmitVertex();

        setVertexData();
        gl_Position.xy = p1.xy - n - d;
        EmitVertex();

        setVertexData();
        gl_Position.xy = p2.xy - n + d;
        EmitVertex();

        EndPrimitive();
}
