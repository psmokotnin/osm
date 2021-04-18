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
#ifndef ARMMATH_H
#define ARMMATH_H

#include "arm_neon.h"
using v4sf = float32x4_t;

__attribute__((aligned(16))) inline v4sf _mm_set1_ps(const float &v)
{
    return vdupq_n_f32(v);
};

__attribute__((aligned(16))) inline v4sf _mm_set_ps(const float &v3,
                                                    const float &v2,
                                                    const float &v1,
                                                    const float &v0)
{
    v4sf r = vdupq_n_f32(0);
    r = vsetq_lane_f32(v0, r, 0);
    r = vsetq_lane_f32(v1, r, 1);
    r = vsetq_lane_f32(v2, r, 2);
    r = vsetq_lane_f32(v3, r, 3);
    return r;
}

__attribute__((aligned(16))) inline v4sf _mm_mul_ps(const v4sf &left, const v4sf &right)
{
    return vmulq_f32(left, right);
}

__attribute__((aligned(16))) inline v4sf _mm_add_ps(const v4sf &left, const v4sf &right)
{
    return vaddq_f32(left, right);
}

__attribute__((aligned(16))) inline void _mm_store_ps(float *dest, v4sf &source)
{
    vst1q_f32(dest, source);
}

__attribute__((aligned(16))) inline v4sf _mm_sub_ps(const v4sf &left, const v4sf &right)
{
    return vsubq_f32(left, right);
}

/*
//STD version for testing:
#include <array>
using v4sf = std::array<float, 4>;

__attribute__((aligned(16))) inline v4sf _mm_set1_ps(const float &v)
{
    v4sf d;
    d[0] = v;
    d[1] = v;
    d[2] = v;
    d[3] = v;
    return d;
};

__attribute__((aligned(16))) inline v4sf _mm_set_ps(const float &v3,
                                                    const float &v2,
                                                    const float &v1,
                                                    const float &v0)
{
    v4sf d;
    d[0] = v0;
    d[1] = v1;
    d[2] = v2;
    d[3] = v3;
    return d;
}

__attribute__((aligned(16))) inline v4sf _mm_mul_ps(const v4sf &left, const v4sf &right)
{
    v4sf d;
    d[0] = left[0] * right[0];
    d[1] = left[1] * right[1];
    d[2] = left[2] * right[2];
    d[3] = left[3] * right[3];
    return d;
}

__attribute__((aligned(16))) inline v4sf _mm_add_ps(const v4sf &left, const v4sf &right)
{
    v4sf d;
    d[0] = left[0] + right[0];
    d[1] = left[1] + right[1];
    d[2] = left[2] + right[2];
    d[3] = left[3] + right[3];
    return d;
}

__attribute__((aligned(16))) inline void _mm_store_ps(float *dest, v4sf &source)
{
    //dest = source.data();
    dest[0] = source[0];
    dest[1] = source[1];
    dest[2] = source[2];
    dest[3] = source[3];
}

__attribute__((aligned(16))) inline v4sf _mm_sub_ps(const v4sf &left, const v4sf &right)
{
    v4sf d;
    d[0] = left[0] - right[0];
    d[1] = left[1] - right[1];
    d[2] = left[2] - right[2];
    d[3] = left[3] - right[3];
    return d;
}*/
#endif // ARMMATH_H
