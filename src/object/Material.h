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
    virtual Vec3 f(const Vec3& normal,const Vec3& l,const Vec3& v);
};

class LambertianMaterial:public Material
{
    INIT_TYPE(LambertianMaterial, Material)
public:
    Vec3 c_diffuse{};
    L_MATH::Vec<float, 3> f(const L_MATH::Vec<float, 3>& normal, const L_MATH::Vec<float, 3>& l, const L_MATH::Vec<float, 3>& v) override;
};

class BliPhongMaterial:public Material
{
    INIT_TYPE(BliPhongMaterial, Material)
public:
    Vec3 c_diffuse{};
    Vec3 c_specular{};
    float specular_factor{};
    L_MATH::Vec<float, 3> f(const L_MATH::Vec<float, 3>& normal, const L_MATH::Vec<float, 3>& l, const L_MATH::Vec<float, 3>& v) override;
};




#endif //MATERIAL_H
