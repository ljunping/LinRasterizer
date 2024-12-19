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


class Context;
struct RenderPass;
class Material;

struct Fragment
{
    TrianglePrimitive* triangle = nullptr;
    Vec3 alpha;
    Vec3 frag_coord;
    Vec2 resolution;
    RenderPass* pass;
    VertexAttribute vertex_attribute;
};
struct DrawCall
{


};

class FragShader : public Resource
{
    INIT_TYPE(FragShader,Resource)
public:
    std::vector<Fragment>* fragment_map;
    int width, height;
    Material* material = nullptr;
    Texture* texture = nullptr;
    Color* frame_buff = nullptr;
    void begin_render_pass(Context* ctx,RenderPass* pass);
    void end_render_pass(Context* ctx,RenderPass* pass);
    virtual Vec4 run(int frag_index);
    ~FragShader() override = default;
    template <int N>
    void ddx(int frag_index, int attribute_index, L_MATH::Vec<float, N>& result);
    template <int N>
    void ddy(int frag_index, int attribute_index, L_MATH::Vec<float, N>& result);
    template <int N>
    void df(int frag_index_l, int frag_index_r, int attribute_index, L_MATH::Vec<float, N>& result);
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
    auto &fragments = *fragment_map;
    if (frag_index_l < 0 || frag_index_l >= fragments.size() || frag_index_r < 0 || frag_index_r >= fragments.size())
    {
        return;
    }
    auto &fragment_l = fragments[frag_index_l];
    auto &fragment_r = fragments[frag_index_r];
    if (fragment_l.triangle == nullptr || fragment_r.triangle == nullptr)
    {
        return;
    }
    if (fragment_l.vertex_attribute.attributes!=fragment_r.vertex_attribute.attributes)
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
    Vec2 dx, dy;
    Vec2 uv;
    auto& fragment = (*fragment_map)[frag_index];
    ddx(frag_index, UV, dx);
    ddy(frag_index, UV, dy);
    fragment.vertex_attribute.get_attribute_value(UV, uv);
    if (!texture)
    {
        return;
    }
    Vec2 lod;
    unsigned char _res[N];
    lod[0] = L_MATH::dot(dx, dx);
    lod[1] = L_MATH::dot(dy, dy);
    texture->sample(uv, lod, _res);
    for (int i = 0; i < N; ++i)
    {
        result[i] = float(_res[i]) / float(255);
    }
}


#endif //LUAFRAGSHADER_H