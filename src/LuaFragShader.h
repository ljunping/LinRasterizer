//
// Created by Lin on 2024/11/16.
//

#ifndef LUAFRAGSHADER_H
#define LUAFRAGSHADER_H
#pragma once

#include "Color.h"
#include "Rasterizer.h"
#include "Texture.h"
class LuaFragShader
{
public:
    LuaFragShader() = default;
    std::vector<Fragment>* fragment_map;
    int ox,sx,oy,sy;
    int width, height;
    virtual Color run(int frag_index);
    virtual ~LuaFragShader() = default;
    template<int N>
    void ddx(int frag_index, int attribute_index, L_MATH::Vec<float, N>& result);
    template <int N>
    void ddy(int frag_index, int attribute_index, L_MATH::Vec<float, N> &result);
    template<int N>
    void df(int frag_index_l,int frag_index_r, int attribute_index, L_MATH::Vec<float, N>& result);
    template<int N>
    void sample_texture(int frag_index, int texture_id, int uv_attribute_index, L_MATH::Vec<float, N> &result);
};

class TextureFragShader : public LuaFragShader
{
public:
    Color run(int frag_index) override;
};
template <int N>
void LuaFragShader::ddx(int frag_index, int attribute_index, L_MATH::Vec<float, N>& result)
{
    L_MATH::Vec<float, N> result2;
    df(frag_index, frag_index + 1, attribute_index, result);
    df(frag_index + width, frag_index + width + 1, attribute_index, result2);
    result += result2;
    result /= 2;
}

template <int N>
void LuaFragShader::ddy(int frag_index, int attribute_index, L_MATH::Vec<float, N>& result)
{
    L_MATH::Vec<float, N> result2;
    df(frag_index, frag_index + width, attribute_index, result);
    df(frag_index + 1, frag_index + 1 + width, attribute_index, result2);
    result += result2;
    result /= 2;
}

template <int N>
void LuaFragShader::df(int frag_index_l, int frag_index_r, int attribute_index, L_MATH::Vec<float, N> &result)
{
    auto &fragments = *fragment_map;
    if (frag_index_l < 0 || frag_index_l >= fragments.size() || frag_index_r < 0 || frag_index_r >= fragments.size())
    {
        return;
    }
    auto &fragment_l = fragments[frag_index_l];
    auto &fragment_r = fragments[frag_index_r];
    if (fragment_l.triangle==nullptr|| fragment_r.triangle==nullptr)
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
void LuaFragShader::sample_texture(int frag_index,int texture_id, int uv_attribute_index, L_MATH::Vec<float, N> &result)
{
    Vec2 dx, dy;
    Vec2 uv;
    auto& fragment = (*fragment_map)[frag_index];
    ddx(frag_index, uv_attribute_index, dx);
    ddy(frag_index, uv_attribute_index, dy);
    fragment.vertex_attribute.get_attribute_value(uv_attribute_index, uv);
    auto texture = TEXTURE_MANAGER.get_texture(texture_id);
    if(!texture)
    {
        return;
    }
    Vec2 lod;
    unsigned char _res[N];
    lod[0]= L_MATH::dot(dx, dx);
    lod[1] = L_MATH::dot(dy, dy);
    TEXTURE_MANAGER.sample(texture_id, uv, lod, _res);
    for (int i = 0; i < N; ++i)
    {
        result[i] = float(_res[i]) / float(255);
    }
}

#endif //LUAFRAGSHADER_H
