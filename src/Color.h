//
// Created by Lin on 2024/11/18.
//

#ifndef COLOR_H
#define COLOR_H
#include "L_math.h"

typedef int Color;

Color l_color(int r, int g, int b);

Color l_color(const Vec3& c);

Color l_color4(const Vec4& c);

Vec3 v_color(Color c);

Vec4 v_color4(Color c);

Color blend(const Vec3& src, const Vec3& dst, float alpha);

Color blend4(const Vec4& src, const Vec4& dst, float alpha);

Color blend(const Vec4& src, const Vec3& dst);


Vec4 blend4v(const Vec4& src, const Vec4& dst, float alpha);

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
