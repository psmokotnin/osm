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
//GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS / sizeof(fData) = 1024 / (2 + 4)
layout(triangle_strip, max_vertices = 64) out;

uniform vec4 m_color;
uniform highp mat4 matrix;
uniform vec2 screen;
uniform vec4 minmax;
uniform float width;

out vec4 fragSpline;
out vec2 frequency;
out vec4 coherenceSpline;

in vData
{
    vec4 splineRe;
    vec4 splineIm;
    vec4 coherenceSpline;
} vertices[];

out fData
{
    float t;
    vec4 coherenceSpline;
} fragmentData;

vec4 spline(float t);
void emitVertexData(float t);

void main(void)
{
    float
        kx = screen[0] / 2.,
        ky = screen[1] / 2.
        ;
    vec4 p1, p2;
    vec2 d, n;
    mat2 r = mat2(
        0.0, -1.0,
        1.0, 0.0
    );

    float t = 0, dt = 1.0 / 16.0;
    for (int i = 0; i < 16; ++i) {
        p1 = spline(t);
        t += dt;
        p2 = spline(t);

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

        gl_Position = vec4(p1.xy + n - d, 0.0, 1.0);
        emitVertexData(t);

        gl_Position = vec4(p2.xy + n + d, 0.0, 1.0);
        emitVertexData(t);

        gl_Position = vec4(p1.xy - n - d, 0.0, 1.0);
        emitVertexData(t);

        gl_Position = vec4(p2.xy - n + d, 0.0, 1.0);
        emitVertexData(t);

        EndPrimitive();
    }
}
void emitVertexData(float t)
{
    fragmentData.t = t;
    fragmentData.coherenceSpline = vertices[0].coherenceSpline;

    EmitVertex();
}
vec4 spline(float t)
{
    vec2 r;
    r.x =
            vertices[0].splineRe[0] +
            vertices[0].splineRe[1] * t +
            vertices[0].splineRe[2] * t*t +
            vertices[0].splineRe[3] * t*t*t
    ;
    r.y =
            vertices[0].splineIm[0] +
            vertices[0].splineIm[1] * t +
            vertices[0].splineIm[2] * t*t +
            vertices[0].splineIm[3] * t*t*t
    ;

    return matrix * vec4(r.x, r.y, 0.0, 1.0);
}
