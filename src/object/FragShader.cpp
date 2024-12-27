//
// Created by Lin on 2024/11/16.
//

#include "FragShader.h"

#include "Camera.h"
#include "Context.h"
#include "DrawCallContext.h"
#include "Light.h"
#include "Material.h"
#include "MeshRender.h"
#include "Transform.h"
#include "WindowHandle.h"


void FragShader::begin_draw_call(DrawCallContext* pass)
{
    fragment_map = pass->camera->fragment_map;
    draw_call = pass;
    this->ctx = pass->ctx;
    pass->ctx->get_screen_size(this->width, this->height);
}

void FragShader::end_draw_call(DrawCallContext* pass)
{
    fragment_map=nullptr;
    draw_call = nullptr;
    ctx = nullptr;
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

    Vec3 view_dir, view_normal;
    view_dir = static_cast<L_MATH::Vec<float, 3>&>(fix_outputs[VIEW_VIEW_DIR]);
    view_normal = static_cast<L_MATH::Vec<float, 3>&>(fix_outputs[VIEW_NORMAL]);
    view_dir.normalized();
    view_normal.normalized();
    auto kd = this->draw_call->materials[0]->get_Vec3_uniform(MATERIAL_LIGHT_KD);
    auto ks = this->draw_call->materials[0]->get_Vec3_uniform(MATERIAL_LIGHT_KS);
    Vec4 texture_color;
    sample_texture(0, frag_index, texture_color);
    Vec4 frag_color;
    frag_color[3] = 1;
    Vec3& frag_color_v3 = static_cast<L_MATH::Vec<float, 3>&>(frag_color);
    int light_index = 0;
    for (auto light : ctx->light_manager->get_objects())
    {
        Vec3 light_dir = static_cast<L_MATH::Vec<float, 3>&>(view_light_dirs[light_index]);
        auto light_dis = light_dir.sqrt_magnitude();
        light_dir /= light_dis;
        auto light_color = texture_color * light->color * light->intensity / (light_dis * light_dis);
        auto h = ((light_dir + view_dir) / 2).normalize();
        frag_color_v3 += kd * std::max(0.f, light_dir.dot(view_normal))*light_color;
        frag_color_v3 += ks * std::pow(std::max(0.f, h.dot(view_normal)), 200) * light_color;
        light_index++;
    }
    L_MATH::clamp(frag_color,Vec4::ZERO,Vec4::ONE);
    return frag_color;
}

void LightFragShader::calculate_normal(int frag_index, L_MATH::Vec<float, 3>& res)
{
    get_fragment(frag_index).interpolation_data.get_attribute_value(NORMAL, res);
}

L_MATH::Vec<float, 4> NormalTextureLightFragShader::run(int frag_index)
{
    auto& fragment = fragment_map[frag_index];
    auto fix_outputs = fragment.interpolation_data.output.fix_outputs;
    auto& tbn_light_dirs = fragment.interpolation_data.output.tbn_light_dirs;
    Vec3 tbn_view_dir, tbn_view_normal;
    tbn_view_dir = static_cast<L_MATH::Vec<float, 3>&>(fix_outputs[TBN_VIEW_DIR]);
    tbn_view_dir.normalized();
    sample_texture(1, frag_index, static_cast<L_MATH::Vec<float, 4>&>(tbn_view_normal));
    tbn_view_normal = (tbn_view_normal * 2 - 1);
    auto bump_factor = this->draw_call->materials[0]->get_float_uniform(MATERIAL_NORMAL_BUMP_FACTOR);
    tbn_view_normal[0] *= bump_factor;
    tbn_view_normal[1] *= bump_factor;
    tbn_view_normal.normalized();
    auto kd = this->draw_call->materials[0]->get_Vec3_uniform(MATERIAL_LIGHT_KD);
    auto ks = this->draw_call->materials[0]->get_Vec3_uniform(MATERIAL_LIGHT_KS);
    Vec4 texture_color;
    sample_texture(0, frag_index, texture_color);
    Vec4 frag_color;
    Vec3& frag_color_v3 = static_cast<L_MATH::Vec<float, 3>&>(frag_color);
    int light_index = 0;
    frag_color_v3[3] = 1;
    for (auto light : ctx->light_manager->get_objects())
    {
        Vec3 light_dir = static_cast<L_MATH::Vec<float, 3>&>(tbn_light_dirs[light_index]);
        auto light_dis = light_dir.sqrt_magnitude();
        light_dir /= light_dis;
        auto light_color = texture_color * light->color * light->intensity / (light_dis * light_dis);
        auto h = ((light_dir + tbn_view_dir) / 2).normalize();
        frag_color_v3 += kd * std::max(0.f, light_dir.dot(tbn_view_normal))*light_color;
        frag_color_v3 += ks * std::pow(std::max(0.f, h.dot(tbn_view_normal)), 200) * light_color;
        light_index++;
    }
    // frag_color_v3
    // color /= 6;
    L_MATH::clamp(frag_color,Vec4::ZERO,Vec4::ONE);
    return frag_color;
}












