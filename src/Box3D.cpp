//
// Created by Lin on 2024/11/14.
//

#include "Box3D.h"

Box3D::Box3D()
= default;

Box3D::Box3D(const L_MATH::Vec<float, 3>& p)
{
    min = p;
    max = p;
}

Box3D::Box3D(const L_MATH::Vec<float, 3>& p, const L_MATH::Vec<float, 3>& q)
{
    min = p;
    max = q;
    expand(q);
}

void Box3D::expand(const L_MATH::Vec<float, 3>& v)
{
    min = fmin(min, v);
    max = fmax(max, v);
}

bool Box3D::inside_2d(const L_MATH::Vec<float, 3>& point) const
{
    return point.data[0] > min.data[0] && point.data[0] < max.data[0]
        && point.data[1] > min.data[1] && point.data[1] <max.data[1];
}

bool Box3D::inside_3d(const L_MATH::Vec<float, 3>& point) const
{
    return point.data[0] > min.data[0] && point.data[0] < max.data[0]
        && point.data[1] > min.data[1] && point.data[1] < max.data[1]
        && point.data[2] > min.data[2] && point.data[2] < max.data[2];
}
