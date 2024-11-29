//
// Created by Lin on 2024/11/14.
//

#ifndef BOX3D_H
#define BOX3D_H
#include"L_math.h"

class alignas(16) Box3D
{
public:
    Vec3 min;
    Vec3 max;
    Box3D();
    explicit Box3D(const Vec3& p);

    Box3D(const Vec3& p, const Vec3& q);

    void expand(const Vec3& v);

    bool inside_2d(const Vec3& point) const;
    bool inside_3d(const Vec3& point) const;

};

#endif //BOX3D_H
