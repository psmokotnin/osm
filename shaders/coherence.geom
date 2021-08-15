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

layout(points) in;
//same as magnitude and phase geom
layout(triangle_strip, max_vertices = 64) out;

uniform vec4 m_color;
uniform vec2 screen;
uniform vec4 minmax;
uniform float width;

in vData
{
    vec4 splineData;
    vec2 frequency;
} vertices[];

vec2 spline(float t);
void setVertexData();

void main(void)
{
    float
        x, y, t,
        xs = vertices[0].frequency[0],
        xe = vertices[0].frequency[1],
        kx = screen[0] / 2.,
        ky = screen[1] / 2.
        ;
    vec2 p1, p2;
    vec2 d, dn, n;
    mat2 r = mat2(
        0.0, -1.0,
        1.0, 0.0
    );

    x = xs;
    float dx = max((xe - xs) / 16, 1.); //not more that 40 steps, see max_vertices
    do {
        p1 = spline(x);
        x += dx;
        p2 = spline(x);
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
        p1 -= d;
        p2 += d;

        setVertexData();
        gl_Position.xy = p1.xy + n;
        EmitVertex();

        setVertexData();
        gl_Position.xy = p2.xy + n;
        EmitVertex();

        setVertexData();
        gl_Position.xy = p1.xy - n;
        EmitVertex();

        setVertexData();
        gl_Position.xy = p2.xy - n;
        EmitVertex();

        EndPrimitive();
    } while (x <= xe);
}
void setVertexData()
{
    gl_Position.z = gl_in[0].gl_Position.z;
    gl_Position.w = gl_in[0].gl_Position.w;
}
vec2 spline(float x)
{
    vec2 r;

    r.x = (x * 2.) / screen[0] - 1.;
    float t = (x - vertices[0].frequency[0]) / (vertices[0].frequency[1] - vertices[0].frequency[0]);
    float v =
            vertices[0].splineData[0] +
            vertices[0].splineData[1] * t +
            vertices[0].splineData[2] * t*t +
            vertices[0].splineData[3] * t*t*t
    ;

    r.y = 1. - 2. * (v - minmax[2]) / (minmax[3] - minmax[2]);
    return r;
}
