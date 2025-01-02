//
// Created by Lin on 2024/11/18.
//

#include "Color.h"

Color l_color(int r, int g, int b)
{
    return 0xFF << 24 | r << 16 | g << 8 | b;
}

Color l_color(const L_MATH::Vec<float, 3>& c)
{
    return l_color((c[0] * 255), c[1] * 255, c[2] * 255);
}

Color l_color4(const L_MATH::Vec<float, 4>& c)
{
    return l_color((c[0] * 255), c[1] * 255, c[2] * 255);
}

L_MATH::Vec<float, 3> v_color(Color c)
{
    int b = (c >> 0) & (0xFF);
    int g = (c >> 8) & (0xFF);
    int r = (c >> 16) & (0xFF);
    return {r / 255.f, g / 255.f, b / 255.f};
}

L_MATH::Vec<float, 4> v_color4(Color c)
{
    int b = (c >> 0) & (0xFF);
    int g = (c >> 8) & (0xFF);
    int r = (c >> 16) & (0xFF);
    return {r / 255.f, g / 255.f, b / 255.f, 1};
}

Color blend(const L_MATH::Vec<float, 3>& src, const L_MATH::Vec<float, 3>& dst, float alpha)
{
    return l_color(src * alpha + dst * (1 - alpha));
}

Color blend4(const L_MATH::Vec<float, 4>& src, const L_MATH::Vec<float, 4>& dst, float alpha)
{
    return l_color4(src * alpha + dst * (1 - alpha));
}

L_MATH::Vec<float, 4> blend4v(const L_MATH::Vec<float, 4>& src, const L_MATH::Vec<float, 4>& dst, float alpha)
{
    return (src * alpha + dst * (1 - alpha));
}
