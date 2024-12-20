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

#define MATERIAL_LIGHT_KD 100
#define MATERIAL_LIGHT_KS 101
#define MATERIAL_LIGHT_KA 102
#define MATERIAL_LIGHT_KE 103
#define MATERIAL_LIGHT_NS 104
#define MATERIAL_LIGHT_NI 105
#define MATERIAL_LIGHT_D 106

#define COLOR1 201
#define COLOR2 202
#define COLOR3 203



class FragShader;

class Material : public Resource
{
    INIT_TYPE(Material, Resource)
public:
    DEFINE_UNIFORM(float)
    DEFINE_UNIFORM(int)
    DEFINE_UNIFORM(Color)
    DEFINE_UNIFORM(Vec3)
    DEFINE_UNIFORM(Vec2)
    DEFINE_UNIFORM(Mat44)
    DEFINE_UNIFORM(Mat33)
};




#endif //MATERIAL_H
