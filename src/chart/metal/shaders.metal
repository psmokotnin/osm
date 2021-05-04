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

#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;

struct LineVertexIn {
    float2 position [[ attribute(0) ]];
    float2 second   [[ attribute(1) ]];
    float direction [[ attribute(2) ]];
};

struct VertexIn {
    float2 position [[ attribute(0) ]];
};

struct VertexColorIn {
    float2 position [[ attribute(0) ]];
    float4 color [[ attribute(1) ]];
};

struct VertexOut {
    float4 position [[position]];
    float alpha;
};

struct VertexColorOut {
    float4 position [[position]];
    float4 color;
};

vertex VertexOut clearVertex()
{
    VertexOut out;
    return out;
};

vertex VertexOut lineVertex(
    const LineVertexIn vertexIn [[ stage_in ]],
    unsigned int vid [[vertex_id]],
    constant float4x4 &matrix [[buffer(1)]],
    constant float2 &size [[buffer(2)]],
    constant float &width  [[buffer(3)]]
)
{
    VertexOut out;
    out.alpha = abs(vertexIn.direction);
    uint8_t vertexId = vid % 6;

    float4 currentPoint = float4(
                              vertexIn.position.x,
                              vertexIn.position.y,
                              0,
                              1) * matrix;

    float4 secondPoint = float4(
                             vertexIn.second.x,
                             vertexIn.second.y,
                             0,
                             1) * matrix;

    float2 direction = secondPoint.xy - currentPoint.xy;
    if (vertexIn.direction < 0) {
        direction *= -1;
    }
    direction.x *= size.x;
    direction.y *= size.y;
    direction = normalize(direction);
    direction *= (-2.0 * (vid % 2) + 1.0) * width;

    float2 norm = float2(-direction.y, direction.x);

    direction.x /= size.x;
    direction.y /= size.y;
    norm.x /= size.x;
    norm.y /= size.y;

    float2 offset = norm + (vertexId == 0 || vertexId == 5 ? -1 : 1) * direction;
    currentPoint.xy += offset;

    out.position = currentPoint;
    return out;
}

vertex VertexOut logVertex(
    const VertexIn vertexIn [[ stage_in ]],
    unsigned int vid [[vertex_id]],
    constant float4x4 &matrix [[buffer(1)]]
)
{
    VertexOut out;
    out.alpha = 1.0;

    float4 currentPoint = float4(
                              log(vertexIn.position.x),
                              vertexIn.position.y,
                              0,
                              1) * matrix;

    out.position = currentPoint;
    return out;
}

vertex VertexColorOut logColorVertex(
    const VertexColorIn vertexIn [[ stage_in ]],
    unsigned int vid [[vertex_id]],
    constant float4x4 &matrix [[buffer(1)]]
)
{
    VertexColorOut out;
    out.color = vertexIn.color;
    out.position = float4(
                       log(vertexIn.position.x),
                       vertexIn.position.y,
                       0,
                       1) * matrix;
    return out;
}

vertex VertexOut logLineVertex(
    const LineVertexIn vertexIn [[ stage_in ]],
    unsigned int vid [[vertex_id]],
    constant float4x4 &matrix [[buffer(1)]],
    constant float2 &size [[buffer(2)]],
    constant float &width  [[buffer(3)]]
)
{
    VertexOut out;
    out.alpha = abs(vertexIn.direction);
    uint8_t vertexId = vid % 6;

    float4 currentPoint = float4(
                              log(vertexIn.position.x),
                              vertexIn.position.y,
                              0,
                              1) * matrix;

    float4 secondPoint = float4(
                             log(vertexIn.second.x),
                             vertexIn.second.y,
                             0,
                             1) * matrix;

    float2 direction = secondPoint.xy - currentPoint.xy;
    if (vertexIn.direction < 0) {
        direction *= -1;
    }
    direction.x *= size.x;
    direction.y *= size.y;
    direction = normalize(direction);
    direction *= (-2.0 * (vid % 2) + 1.0) * width;

    float2 norm = float2(-direction.y, direction.x);

    direction.x /= size.x;
    direction.y /= size.y;
    norm.x /= size.x;
    norm.y /= size.y;

    float2 offset = norm + (vertexId == 0 || vertexId == 5 ? -1 : 1) * direction;
    currentPoint.xy += offset;

    out.position = currentPoint;
    return out;
}

fragment float4 fillColor (
    constant float4 &color [[buffer(1)]],
    VertexOut in [[stage_in]]
)
{
    float4 pointColor = color;
    pointColor.a = in.alpha;
    return pointColor;
}

fragment float4 fillPointColor (
    VertexColorOut in [[stage_in]]
)
{
    return in.color;
}
