//
// Created by Lin on 2024/12/4.
//

#ifndef MATERIAL_H
#define MATERIAL_H

#define DEFINE_UNIFORM(FORM_TYPE)\
    private:std::unordered_map<std::string, FORM_TYPE> FORM_TYPE##_uniform;\
    public:FORM_TYPE get_##FORM_TYPE##_uniform(std::string uniform_name)\
    {\
        return FORM_TYPE##_uniform[uniform_name];\
    }\
    public:FORM_TYPE set_##FORM_TYPE##_uniform(std::string uniform_name,FORM_TYPE value)\
    {\
        FORM_TYPE##_uniform[uniform_name] = value;\
    }

#include "L_math.h"
#include "Color.h"


class  Material
{
public:
    int id;
    DEFINE_UNIFORM(float)
    DEFINE_UNIFORM(int)
    DEFINE_UNIFORM(Color)
    DEFINE_UNIFORM(Vec3)
    DEFINE_UNIFORM(Vec2)
    DEFINE_UNIFORM(Mat44)
    DEFINE_UNIFORM(Mat33)
};

class MaterialManager
{
    int material_id = 0;
    std::unordered_map<int,Material*> materials;
public:
    int create_material();
    int create_material(const char* mtx_file);
    void delete_material(int id);
    Material* get_material(int id);
};

inline MaterialManager MATERIAL_MANAGER;

#endif //MATERIAL_H
