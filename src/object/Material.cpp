//
// Created by Lin on 2024/12/4.
//

#include "Material.h"



Vec3 Material::f(const L_MATH::Vec<float, 3>& normal, const L_MATH::Vec<float, 3>& l, const L_MATH::Vec<float, 3>& v)
{
    return Vec3::ONE / PI;
}

Vec3 LambertianMaterial::f(const L_MATH::Vec<float, 3>& normal, const L_MATH::Vec<float, 3>& l, const L_MATH::Vec<float, 3>& v)

{
    return c_diffuse / PI;
}

Vec3 BliPhongMaterial::f(const L_MATH::Vec<float, 3>& normal, const L_MATH::Vec<float, 3>& l, const L_MATH::Vec<float, 3>& v)
{
    return c_specular * pow(dot(((l + v) / 2).normalize(), normal), specular_factor) + c_diffuse / PI;
}

