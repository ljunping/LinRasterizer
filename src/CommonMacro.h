//
// Created by Lin on 2024/12/5.
//

#ifndef COMMONMACRO_H
#define COMMONMACRO_H
#include <memory>
#define SHARE_PTR std::shared_ptr
#define UNIQUE_PTR std::unique_ptr
#define WEAK_PTR std::weak_ptr

#define DEFINE_UNIFORM(FORM_TYPE)\
    private:FORM_TYPE FORM_TYPE##_uniform[FormValueTypeCount]{};\
    public:FORM_TYPE get_##FORM_TYPE##_uniform(int uniform_name)\
    {\
        return FORM_TYPE##_uniform[uniform_name];\
    }\
    public:void set_##FORM_TYPE##_uniform(int uniform_name,FORM_TYPE value)\
    {\
        FORM_TYPE##_uniform[uniform_name] = value;\
    }

#define MAX_TEXTURES_COUNT 4
#define MAX_MATERIAL_COUNT 4
#define MAX_VERT_OUTPUT_COUNT 8

enum AttributeType
{
    POS,
    NORMAL,
    COLOR,
    UV,
    TANGENT,
    AttributeTypeCount,
};

enum FormValueType
{
    MATERIAL_LIGHT_KD,
    MATERIAL_LIGHT_KS,
    MATERIAL_LIGHT_KA,
    MATERIAL_LIGHT_KE,
    MATERIAL_LIGHT_NS,
    MATERIAL_LIGHT_NI,
    MATERIAL_LIGHT_D,
    MATERIAL_NORMAL_BUMP_FACTOR,
    MATERIAL_COLOR1,
    MATERIAL_COLOR2,
    MATERIAL_COLOR3,
    MATERIAL_BLEND_CONST_COLOR1,
    MATERIAL_BLEND_CONST_COLOR2,
    MATERIAL_BLEND_CONST_COLOR3,
    FormValueTypeCount,
};

#endif //COMMONMACRO_H




