//
// Created by Lin on 2024/11/16.
//

#ifndef LUAFRAGSHADER_H
#define LUAFRAGSHADER_H
#pragma once

#include "Color.h"
#include "Material.h"
#include "Texture.h"
#include "Resource.h"
#include "CommonMacro.h"
#include "Context.h"
#include "DrawCallContext.h"


class Context;
class DrawCallContext;
class Material;




class FragShader : public Resource
{
    INIT_TYPE(FragShader,Resource)
public:
    DEFINE_UNIFORM(float)
    DEFINE_UNIFORM(int)
    DEFINE_UNIFORM(Color)
    DEFINE_UNIFORM(Vec3)
    DEFINE_UNIFORM(Vec4)
    DEFINE_UNIFORM(Vec2)
    DEFINE_UNIFORM(Mat44)
    DEFINE_UNIFORM(Mat33)
    Fragment* fragment_map{};
    int width{}, height{};
    DrawCallContext* draw_call = nullptr;
    void begin_draw_call(DrawCallContext* pass);
    void end_draw_call(DrawCallContext* pass);
    virtual Vec4 run(int frag_index);
    ~FragShader() override = default;
    template <int N>
    void ddx(int frag_index, int attribute_index, L_MATH::Vec<float, N>& result);
    template <int N>
    void ddy(int frag_index, int attribute_index, L_MATH::Vec<float, N>& result);
    template <int N>
    void df(int frag_index_l, int frag_index_r, int attribute_index, L_MATH::Vec<float, N>& result);
    Fragment& get_fragment(int frag_index) const;
    void sample_texture(int texture_index, int frag_index,Vec4& color);
    template <int N>
    void sample_texture(int frag_index, SHARE_PTR<Texture>& texture, L_MATH::Vec<float, N>& result);
};
class LightShadowMapFragShader:public FragShader
{
    INIT_TYPE(LightShadowMapFragShader, FragShader)
    public:
    Vec4 run(int frag_index) override;

};
class TextureFragShader : public FragShader
{
    INIT_TYPE(TextureFragShader, FragShader)
public:
    Vec4 run(int frag_index) override;
}
;

class LightFragShader : public FragShader
{
    INIT_TYPE(LightFragShader, FragShader)
public:
    Vec4 run(int frag_index) override;
};

class NormalTextureLightFragShader : public FragShader
{
    INIT_TYPE(NormalTextureLightFragShader, FragShader)
public:
    Vec4 run(int frag_index) override;
};

class MaterialBRDFFragShader : public FragShader
{
    INIT_TYPE(MaterialBRDFFragShader, FragShader)
public:
    Vec4 run(int frag_index) override;
};



#endif //LUAFRAGSHADER_H
