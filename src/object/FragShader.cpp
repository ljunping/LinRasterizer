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
    return (*fragment_map)[frag_index];
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
    auto fragment = (*fragment_map)[frag_index];
    auto mesh = fragment.vertex_attribute.attributes;
    Mat44 model_mat;
    auto& rot_mat = static_cast<Mat33&>(model_mat);
    Vec4 texture_color={0.5,0.5,0.5,1};
    texture_color[3] = 1;
    sample_texture(0, frag_index, texture_color);
    this->draw_call->get_model_matrix(mesh, model_mat);
    Vec3 pos, normal;
    Vec4& pos4 = static_cast<L_MATH::Vec<float, 4>&>(pos);
    pos[3] = 1;
    fragment.vertex_attribute.get_attribute_value(POS, pos);
    calculate_normal(frag_index, normal);
    normal[2]*=-1;
    pos4 = pos4.mul_transpose(model_mat);
    normal = normal.mul_transpose(rot_mat);
    normal.normalized();
    Vec3 view_dir = (this->draw_call->pass_node.camera->scene_node->get_global_pos() - pos).normalize();
    auto kd = this->draw_call->materials[0]->get_Vec3_uniform(MATERIAL_LIGHT_KD);
    auto ks = this->draw_call->materials[0]->get_Vec3_uniform(MATERIAL_LIGHT_KS);
    Vec3& color =static_cast<L_MATH::Vec<float, 3>&>(texture_color);
    for (auto light : ctx->light_manager->get_objects())
    {
        auto light_color = light->color * light->intensity;
        Vec3 light_dir = (light->scene_node->get_global_pos() - pos).normalize();
        auto h = ((light_dir + view_dir) / 2).normalize();
        color += kd * std::max(0.f, light_dir.dot(normal))*light_color;
        color += ks * std::pow(std::max(0.f, h.dot(normal)), 200) * light_color;
    }
    L_MATH::clamp(texture_color,Vec4::ZERO,Vec4::ONE);
    return texture_color;
}

void LightFragShader::calculate_normal(int frag_index, L_MATH::Vec<float, 3>& res)
{
    get_fragment(frag_index).vertex_attribute.get_attribute_value(NORMAL, res);
}

void NormalTextureLightFragShader::calculate_normal(int frag_index, L_MATH::Vec<float, 3>& res)
{
    sample_texture(frag_index, this->draw_call->textures[1], res);
    res *= 2.0;
    res -= 1.0;
}











