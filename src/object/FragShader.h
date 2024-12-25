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


class Context;
struct DrawCallData;
class Material;

struct Fragment
{
    TrianglePrimitive* triangle{};
    Vec3 alpha;
    Vec3 frag_coord;
    Vec2 resolution;
    DrawCallData* draw_call{};
    VertexAttribute vertex_attribute;
};


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

    std::vector<Fragment>* fragment_map{};
    int width{}, height{};
    DrawCallData* draw_call = nullptr;
    Context* ctx = nullptr;
    void begin_draw_call(DrawCallData* pass);
    void end_draw_call(DrawCallData* pass);
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
    void sample_texture(int frag_index, Texture* texture, L_MATH::Vec<float, N>& result);
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
    virtual void calculate_normal(int frag_index,Vec3& res);
};

class NormalTextureLightFragShader : public LightFragShader
{
    INIT_TYPE(NormalTextureLightFragShader, LightFragShader)
public:
    void calculate_normal(int frag_index, L_MATH::Vec<float, 3>& res) override;
};
template <int N>
void FragShader::ddx(int frag_index, int attribute_index, L_MATH::Vec<float, N>& result)
{
    L_MATH::Vec<float, N> result2;
    df(frag_index, frag_index + 1, attribute_index, result);
    df(frag_index + width, frag_index + width + 1, attribute_index, result2);
    result += result2;
    result /= 2;
}

template <int N>
void FragShader::ddy(int frag_index, int attribute_index, L_MATH::Vec<float, N>& result)
{
    L_MATH::Vec<float, N> result2;
    df(frag_index, frag_index + width, attribute_index, result);
    df(frag_index + 1, frag_index + 1 + width, attribute_index, result2);
    result += result2;
    result /= 2;
}

template <int N>
void FragShader::df(int frag_index_l, int frag_index_r, int attribute_index, L_MATH::Vec<float, N> &result)
{
    int msaa_index = frag_index_l / (width * height);
    if (ctx->setting.msaa_factor <= msaa_index)
    {
        return;
    }
    int r_size = (msaa_index + 1) * (width * height);
    int l_size = msaa_index * (width * height);
    auto& fragments = *fragment_map;
    if (frag_index_l < l_size || frag_index_l >= r_size || frag_index_r < l_size || frag_index_r >= r_size)
    {
        return;
    }
    auto &fragment_l = fragments[frag_index_l];
    auto &fragment_r = fragments[frag_index_r];
    if (fragment_l.triangle == nullptr || fragment_r.triangle == nullptr)
    {
        return;
    }
    if (fragment_l.vertex_attribute.attributes != fragment_r.vertex_attribute.attributes)
    {
        return;
    }
    L_MATH::Vec<float, N> a1,a2;
    fragment_l.vertex_attribute.get_attribute_value(attribute_index, a1);
    fragment_r.vertex_attribute.get_attribute_value(attribute_index, a2);
    result = a2 - a1;
}

template <int N>
void FragShader::sample_texture(int frag_index, Texture* texture, L_MATH::Vec<float, N>& result)
{
    Vec2 uv;
    auto ctx = get_current_ctx();
    auto& fragment = (*fragment_map)[frag_index];
    if (!ctx->setting.enable_mipmap)
    {
        const unsigned char* _res;
        fragment.vertex_attribute.get_attribute_value(UV, uv);
        texture->texture_raw(uv, _res);
        for (int i = 0; i < N; ++i)
        {
            result[i] = float(_res[i]) / float(255);
        }
        return;
    }
    unsigned char _res[N];
    Vec2 dx, dy;
    fragment.vertex_attribute.get_attribute_value(UV, uv);
    ddx(frag_index, UV, dx);
    ddy(frag_index, UV, dy);
    if (!texture)
    {
        return;
    }
    Vec2 lod;

    lod[0] = L_MATH::dot(dx, dx);
    lod[1] = L_MATH::dot(dy, dy);
    texture->sample(uv, lod, _res);
    for (int i = 0; i < N; ++i)
    {
        result[i] = float(_res[i]) / float(255);
    }
}


#endif //LUAFRAGSHADER_H
