//
// Created by Lin on 2024/11/16.
//

#include "FragShader.h"

#include "Context.h"
#include "Light.h"
#include "Material.h"
#include "MeshRender.h"
#include "WindowHandle.h"


void FragShader::begin_draw_call(DrawCallData* pass)
{
    fragment_map = &pass->pass_node.camera->fragment_map;
    draw_call = pass;
    this->ctx = pass->ctx;
    pass->ctx->get_screen_size(this->width, this->height);
}

void FragShader::end_draw_call(DrawCallData* pass)
{
    fragment_map=nullptr;
    draw_call = nullptr;
    ctx = nullptr;
    this->width = 0;
    this->height = 0;
}

void sample_texture_0(FragShader* shader, int index,Vec4& color)
{
    auto texture = shader->draw_call->texture0;
    if (texture && texture->channels == 3)
    {
        Vec3 frag_color;
        shader->sample_texture(index, texture, frag_color);
        color = Vec4(frag_color, 1);
    }
    if (texture && texture->channels == 4)
    {
        Vec4 frag_color;
        shader->sample_texture(index, texture, frag_color);
        color = frag_color;
    }
}



Vec4 FragShader::run(int frag_index )
{
    auto fragment = (*fragment_map)[frag_index];
    auto material = this->draw_call->material0;
    auto color = material->get_Vec4_uniform(MATERIAL_COLOR1);
    auto blend_color = material->get_Vec4_uniform(MATERIAL_BLEND_CONST_COLOR1);
    color = blend4v(blend_color, color, blend_color[3]);
    return color;
}



Vec4 TextureFragShader::run(int frag_index)
{
    Vec4 color;
    sample_texture_0(this, frag_index, color);
    auto material = this->draw_call->material0;
    auto blend_color = material->get_Vec4_uniform(MATERIAL_BLEND_CONST_COLOR1);
    color = blend4v(blend_color, color, blend_color[3]);
    return color;
}

L_MATH::Vec<float, 4> LightFragShader::run(int frag_index)
{
    auto fragment = (*fragment_map)[frag_index];
    auto mesh = fragment.vertex_attribute.attributes;
    Mat44 model_mat;
    Mat33 rot_mat;
    this->draw_call->get_model_matrix(mesh, model_mat);
    rot_mat.copy_from(0, 0, model_mat);
    Vec3 pos, normal, view_dir, light_dir;
    fragment.vertex_attribute.get_attribute_value(POS, pos);
    fragment.vertex_attribute.get_attribute_value(NORMAL, normal);
    pos = (Vec3)(model_mat * Vec4(pos, 1))[0];
    normal = (rot_mat * normal)[0];
    view_dir = (this->draw_call->pass_node.camera->scene_node->get_global_pos() - pos).normalize();
    auto kd = this->draw_call->material0->get_Vec3_uniform(MATERIAL_LIGHT_KD);
    auto ks = this->draw_call->material0->get_Vec3_uniform(MATERIAL_LIGHT_KS);
    Vec3 color;
    for (auto light : ctx->light_manager->get_objects())
    {
        auto light_color = light->color * light->intensity;
        light_dir = (light->scene_node->get_global_pos() - pos).normalize();
        auto h = ((light_dir + view_dir) / 2).normalize();
        color += kd * std::max(0.f, light_dir.dot(normal))*light_color;
        color += ks * std::pow(std::max(0.f, h.dot(normal)), 200) * light_color;
    }
    L_MATH::clamp(color,Vec3::ZERO,Vec3::ONE);
    return Vec4(color, 1);

}





