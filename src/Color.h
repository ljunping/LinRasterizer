//
// Created by Lin on 2024/11/18.
//

#ifndef COLOR_H
#define COLOR_H
#include "L_math.h"

typedef int Color;

inline Color l_color(int r, int g, int b)
{
    return 0xFF << 24 | r << 16 | g << 8 | b;
}

inline Color l_color(const Vec3& c)
{
    return l_color((c[0] * 255), c[1] * 255, c[2] * 255);
}

inline Color l_color4(const Vec4& c)
{
    return l_color((c[0] * 255), c[1] * 255, c[2] * 255);
}

inline Vec3 v_color(Color c)
{
    int b = (c >> 0) & (0xFF);
    int g = (c >> 8) & (0xFF);
    int r = (c >> 16) & (0xFF);
    return {r / 255.f, g / 255.f, b / 255.f};
}

inline Vec4 v_color4(Color c)
{
    int b = (c >> 0) & (0xFF);
    int g = (c >> 8) & (0xFF);
    int r = (c >> 16) & (0xFF);
    return {r / 255.f, g / 255.f, b / 255.f, 1};
}

inline Color blend(const Vec3& src, const Vec3& dst, float alpha)
{
    return l_color(src * alpha + dst * (1 - alpha));
}

inline Color blend4(const Vec4& src, const Vec4& dst, float alpha)
{
    return l_color4(src * alpha + dst * (1 - alpha));
}

inline Vec4 blend4v(const Vec4& src, const Vec4& dst, float alpha)
{
    return (src * alpha + dst * (1 - alpha));
}

inline Color RED = l_color(255, 0, 0);
inline Color GREEN = l_color(0, 255, 0);
inline Color BLUE = l_color(0, 0, 255);
inline Color WHITE = l_color(255, 255, 255);
inline Color BLACK = l_color(0, 0, 0);

inline Vec3 RED_VEC = Vec3{255, 0, 0};
inline Vec3 GREEN_VEC = Vec3{0, 255, 0};
inline Vec3 BLUE_VEC = Vec3{0, 0, 255};
inline Vec3 WHITE_VEC = Vec3{255, 255, 255};
inline Vec3 BLACK_VEC = Vec3{0, 0, 0};



#endif //COLOR_H
