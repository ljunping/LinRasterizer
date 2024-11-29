//
// Created by Lin on 2024/11/18.
//

#ifndef COLOR_H
#define COLOR_H
#include "L_math.h"

typedef int Color;
inline Color color(int r, int g, int b)
{
    return r << 16 | g << 8 | b;
}

inline Color color(const Vec3& c)
{
    return color((c[0] * 255), c[1] * 255, c[2] * 255);
}

inline Color color(const Vec4& c)
{
    return color((c[0] * 255), c[1] * 255, c[2] * 255);
}
inline Color RED = color(255, 0, 0);
inline Color GREEN = color(0, 255, 0);
inline Color BLUE = color(0, 0, 255);
inline Color WHITE = color(255, 255, 255);
inline Color BLACK = color(0, 0, 0);

inline Vec3 RED_VEC = Vec3{255, 0, 0};
inline Vec3 GREEN_VEC = Vec3{0, 255, 0};
inline Vec3 BLUE_VEC = Vec3{0, 0, 255};
inline Vec3 WHITE_VEC = Vec3{255, 255, 255};
inline Vec3 BLACK_VEC = Vec3{0, 0, 0};



#endif //COLOR_H
