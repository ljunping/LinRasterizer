//
// Created by Lin on 2024/12/4.
//

#ifndef MATERIAL_H
#define MATERIAL_H

#include "L_math.h"
#include "Color.h"
#include "CommonMacro.h"
#include "Object.h"
#include "Resource.h"




class FragShader;

class Material : public Resource
{
    INIT_TYPE(Material, Resource)
public:
    DEFINE_UNIFORM(bool)
    DEFINE_UNIFORM(float)
    DEFINE_UNIFORM(int)
    DEFINE_UNIFORM(Color)
    DEFINE_UNIFORM(Vec4)
    DEFINE_UNIFORM(Vec3)
    DEFINE_UNIFORM(Vec2)
    DEFINE_UNIFORM(Mat44)
    DEFINE_UNIFORM(Mat33)
};




#endif //MATERIAL_H
