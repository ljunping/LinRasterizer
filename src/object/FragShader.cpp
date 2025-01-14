//
// Created by Lin on 2024/11/16.
//

#include "FragShader.h"

#include "Camera.h"
#include "Context.h"
#include "DrawCallContext.h"
#include "Light.h"
#include "Material.h"
#include "Mesh.h"
#include "MeshRender.h"
#include "Transform.h"
#include "WindowHandle.h"

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
    auto size = width * height;
    int msaa_index = frag_index_l / (size);
    if (draw_call->setting.msaa_factor <= msaa_index)
    {
        return;
    }
    int l_size = msaa_index * (size);
    int r_size = l_size + (size);
    if (frag_index_l < l_size || frag_index_l >= r_size || frag_index_r < l_size || frag_index_r >= r_size)
    {
        return;
    }
    auto &fragment_l = fragment_map[frag_index_l];
    auto &fragment_r = fragment_map[frag_index_r];
    if (fragment_l.triangle == nullptr || fragment_r.triangle == nullptr)
    {
        return;
    }
    if (fragment_l.interpolation_data.mesh_ptr != fragment_r.interpolation_data.mesh_ptr)
    {
        return;
    }
    L_MATH::Vec<float, N> a1,a2;
    fragment_l.interpolation_data.get_attribute_value(attribute_index, a1);
    fragment_r.interpolation_data.get_attribute_value(attribute_index, result);
    result -= a1;
}

template <int N>
void FragShader::sample_texture(int frag_index, SHARE_PTR<Texture>& texture, L_MATH::Vec<float, N>& result)
{
    Vec2 uv;
    auto ctx = get_current_ctx();
    auto& fragment = fragment_map[frag_index];
    if (!ctx->setting.enable_mipmap)
    {
        fragment.interpolation_data.get_attribute_value(UV, uv);
        texture->sample(uv, 0, static_cast<Vec4&>(result));
        return;
    }
    Vec2 dx, dy;
    fragment.interpolation_data.get_attribute_value(UV, uv);
    ddx(frag_index, UV, dx);
    ddy(frag_index, UV, dy);
    if (!texture)
    {
        return;
    }
    Vec2 lod = {
        L_MATH::dot(dx, dx),
        L_MATH::dot(dy, dy)
    };

    texture->sample(uv, lod, static_cast<Vec4&>(result));
}

template void FragShader::ddx(int frag_index, int attribute_index, L_MATH::Vec<float, 1>& result);
template void FragShader::ddx(int frag_index, int attribute_index, L_MATH::Vec<float, 2>& result);
template void FragShader::ddx(int frag_index, int attribute_index, L_MATH::Vec<float, 3>& result);
template void FragShader::ddx(int frag_index, int attribute_index, L_MATH::Vec<float, 4>& result);

template void FragShader::ddy(int frag_index, int attribute_index, L_MATH::Vec<float, 1>& result);
template void FragShader::ddy(int frag_index, int attribute_index, L_MATH::Vec<float, 2>& result);
template void FragShader::ddy(int frag_index, int attribute_index, L_MATH::Vec<float, 3>& result);
template void FragShader::ddy(int frag_index, int attribute_index, L_MATH::Vec<float, 4>& result);

template void FragShader::df(int frag_index_l, int frag_index_r, int attribute_index, L_MATH::Vec<float, 1> &result);
template void FragShader::df(int frag_index_l, int frag_index_r, int attribute_index, L_MATH::Vec<float, 2> &result);
template void FragShader::df(int frag_index_l, int frag_index_r, int attribute_index, L_MATH::Vec<float, 3> &result);
template void FragShader::df(int frag_index_l, int frag_index_r, int attribute_index, L_MATH::Vec<float, 4>& result);


template void FragShader::sample_texture(int frag_index, SHARE_PTR<Texture>&, L_MATH::Vec<float, 1>& result);
template void FragShader::sample_texture(int frag_index, SHARE_PTR<Texture>&, L_MATH::Vec<float, 2>& result);
template void FragShader::sample_texture(int frag_index, SHARE_PTR<Texture>&, L_MATH::Vec<float, 3>& result);
template void FragShader::sample_texture(int frag_index, SHARE_PTR<Texture>&, L_MATH::Vec<float, 4>& result);


void FragShader::begin_draw_call(DrawCallContext* draw_call_context)
{
    fragment_map = draw_call_context->fragment_map;
    draw_call = draw_call_context;
    this->width = draw_call_context->w;
    this->height = draw_call_context->h;
}

void FragShader::end_draw_call(DrawCallContext* pass)
{
    fragment_map=nullptr;
    draw_call = nullptr;
    this->width = 0;
    this->height = 0;
}





Vec4 FragShader::run(int frag_index )
{
    auto material = this->draw_call->materials[0];
    auto color = material->get_Vec4_uniform(MATERIAL_COLOR1);
    auto blend_color = material->get_Vec4_uniform(MATERIAL_BLEND_CONST_COLOR1);
    color = blend4v(blend_color, color, blend_color[3]);
    return color;
}

Fragment& FragShader::get_fragment(int frag_index) const
{
    return fragment_map[frag_index];
}

void FragShader::sample_texture(int texture_index, int index, L_MATH::Vec<float, 4>& color)
{
    auto texture = this->draw_call->textures[texture_index];
    if (texture && texture->channels == 3)
    {
        color[3] = 1;
        Vec3& frag_color = static_cast<L_MATH::Vec<float, 3>&>(color);
        this->sample_texture(index, texture, frag_color);
    }
    if (texture && texture->channels == 4)
    {
        this->sample_texture(index, texture, color);
    }
}


L_MATH::Vec<float, 4> LightShadowMapFragShader::run(int frag_index)
{
    return {1, 1, 1, 1};
}

Vec4 TextureFragShader::run(int frag_index)
{
    Vec4 color;
    sample_texture(0, frag_index, color);
    auto material = this->draw_call->materials;
    auto blend_color = material[0]->get_Vec4_uniform(MATERIAL_BLEND_CONST_COLOR1);
    color = blend4v(blend_color, color, blend_color[3]);
    return color;
}

L_MATH::Vec<float, 4> LightFragShader::run(int frag_index)
{
    auto& fragment = fragment_map[frag_index];
    auto fix_outputs = fragment.interpolation_data.output.fix_outputs;
    auto& view_light_dirs = fragment.interpolation_data.output.view_light_dirs;
    Vec3 view_dir = static_cast<L_MATH::Vec<float, 3>&>(fix_outputs[VIEW_VIEW_DIR]);
    Vec3 view_normal = static_cast<L_MATH::Vec<float, 3>&>(fix_outputs[VIEW_NORMAL]);
    Vec3 world_pos = static_cast<L_MATH::Vec<float, 3>&>(fix_outputs[WORLD_POS]);
    view_dir.normalized();
    view_normal.normalized();
    auto kd = this->draw_call->materials[0]->get_Vec3_uniform(MATERIAL_LIGHT_KD);
    auto ks = this->draw_call->materials[0]->get_Vec3_uniform(MATERIAL_LIGHT_KS);
    auto ke = this->draw_call->materials[0]->get_Vec3_uniform(MATERIAL_LIGHT_KE);

    Vec4 texture_color;
    sample_texture(0, frag_index, texture_color);
    Vec4 frag_color;
    frag_color[3] = 1;
    Vec3& frag_color_v3 = static_cast<L_MATH::Vec<float, 3>&>(frag_color);
    int light_index = 0;
    frag_color_v3 += texture_color * ke * std::max(L_MATH::dot(view_dir, view_normal), 0.0f);
    for (auto light : draw_call->ctx->light_manager->get_objects())
    {
        auto calculate_shadow = light->calculate_shadow(draw_call->camera, world_pos);
        if (!L_MATH::is_zero(calculate_shadow))
        {
            continue;
        }
        Vec3 light_dir = static_cast<L_MATH::Vec<float, 3>&>(view_light_dirs[light_index]);
        auto light_dis = light_dir.sqrt_magnitude();
        light_dir /= light_dis;
        auto light_color = texture_color * light->calculate_radiance(world_pos);
        auto h = ((light_dir + view_dir) / 2).normalize();
        frag_color_v3 += kd * std::max(0.f, light_dir.dot(view_normal)) * light_color;
        frag_color_v3 += ks * std::pow(std::max(0.f, h.dot(view_normal)), 200) * light_color;
        light_index++;
    }
    L_MATH::clamp(frag_color,Vec4::ZERO,Vec4::ONE);

    return frag_color;
}


L_MATH::Vec<float, 4> NormalTextureLightFragShader::run(int frag_index)
{
    auto& fragment = fragment_map[frag_index];
    auto fix_outputs = fragment.interpolation_data.output.fix_outputs;
    auto& tbn_light_dirs = fragment.interpolation_data.output.tbn_light_dirs;
    Vec3 tbn_view_dir, tbn_view_normal;
    tbn_view_dir = static_cast<L_MATH::Vec<float, 3>&>(fix_outputs[TBN_VIEW_DIR]);
    Vec3 world_pos = static_cast<L_MATH::Vec<float, 3>&>(fix_outputs[WORLD_POS]);
    tbn_view_dir.normalized();
    sample_texture(1, frag_index, static_cast<L_MATH::Vec<float, 4>&>(tbn_view_normal));
    tbn_view_normal = (tbn_view_normal * 2 - 1);
    auto bump_factor = this->draw_call->materials[0]->get_float_uniform(MATERIAL_NORMAL_BUMP_FACTOR);
    tbn_view_normal[0] *= bump_factor;
    tbn_view_normal[1] *= bump_factor;
    tbn_view_normal.normalized();
    auto kd = this->draw_call->materials[0]->get_Vec3_uniform(MATERIAL_LIGHT_KD);
    auto ks = this->draw_call->materials[0]->get_Vec3_uniform(MATERIAL_LIGHT_KS);
    auto ke = this->draw_call->materials[0]->get_Vec3_uniform(MATERIAL_LIGHT_KE);

    Vec4 texture_color;
    sample_texture(0, frag_index, texture_color);
    Vec4 frag_color;
    Vec3& frag_color_v3 = static_cast<L_MATH::Vec<float, 3>&>(frag_color);
    int light_index = 0;
    frag_color_v3[3] = 1;
    frag_color_v3 += texture_color * ke * std::max(L_MATH::dot(tbn_view_dir, tbn_view_normal), 0.f);
    for (auto light : draw_call->ctx->light_manager->get_objects())
    {
        auto calculate_shadow = 1 - light->calculate_shadow(draw_call->camera, world_pos);
        Vec3 light_dir = static_cast<L_MATH::Vec<float, 3>&>(tbn_light_dirs[light_index]);
        auto light_dis = light_dir.sqrt_magnitude();
        light_dir /= light_dis;
        auto light_color = texture_color * light->calculate_radiance(world_pos) * calculate_shadow;
        auto h = ((light_dir + tbn_view_dir) / 2).normalize();
        frag_color_v3 += kd * std::max(0.f, light_dir.dot(tbn_view_normal)) * light_color;
        frag_color_v3 += ks * std::pow(std::max(0.f, h.dot(tbn_view_normal)), 200) * light_color;
        light_index++;
    }
    L_MATH::clamp(frag_color,Vec4::ZERO,Vec4::ONE);
    return frag_color;
}

L_MATH::Vec<float, 4> MaterialBRDFFragShader::run(int frag_index)
{
    auto& fragment = fragment_map[frag_index];
    auto material = this->draw_call->materials[0];
    auto fix_outputs = fragment.interpolation_data.output.fix_outputs;
    auto& tbn_light_dirs = fragment.interpolation_data.output.tbn_light_dirs;
    Vec3 tbn_view_dir, tbn_view_normal;
    tbn_view_dir = static_cast<L_MATH::Vec<float, 3>&>(fix_outputs[TBN_VIEW_DIR]);
    Vec3 world_pos = static_cast<L_MATH::Vec<float, 3>&>(fix_outputs[WORLD_POS]);
    tbn_view_dir.normalized();
    sample_texture(1, frag_index, static_cast<L_MATH::Vec<float, 4>&>(tbn_view_normal));
    tbn_view_normal = (tbn_view_normal * 2 - 1);
    auto bump_factor = material->get_float_uniform(MATERIAL_NORMAL_BUMP_FACTOR);
    tbn_view_normal[0] *= bump_factor;
    tbn_view_normal[1] *= bump_factor;
    tbn_view_normal.normalized();
    Vec4 texture_color;
    sample_texture(0, frag_index, texture_color);
    Vec4 frag_color;
    Vec3& frag_color_v3 = static_cast<L_MATH::Vec<float, 3>&>(frag_color);
    int light_index = 0;
    frag_color_v3[3] = 1;
    frag_color_v3 += texture_color * material->get_Vec4_uniform(MATERIAL_ENV_LIGHT) * std::max(
        L_MATH::dot(tbn_view_dir, tbn_view_normal), 0.3f);
    for (auto light : draw_call->ctx->light_manager->get_objects())
    {
        auto calculate_shadow = light->calculate_shadow(draw_call->camera, world_pos);
        if (!L_MATH::is_zero(calculate_shadow))
        {
            continue;
        }
        Vec3 light_dir = static_cast<L_MATH::Vec<float, 3>&>(tbn_light_dirs[light_index]);
        auto light_dis = light_dir.sqrt_magnitude();
        light_dir /= light_dis;
        auto light_color = texture_color * light->calculate_radiance(world_pos) * std::max(
            dot(light_dir, tbn_view_normal), 0.f);

        frag_color_v3 += light_color * material->f(tbn_view_normal, light_dir, tbn_view_dir) * PI;
        light_index++;
    }
    L_MATH::clamp(frag_color,Vec4::ZERO,Vec4::ONE);
    return frag_color;
}












