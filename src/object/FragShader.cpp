//
// Created by Lin on 2024/11/16.
//

#include "FragShader.h"

#include "Context.h"
#include "Material.h"
#include "MeshRender.h"
#include "WindowHandle.h"


void FragShader::begin_render_pass(Context* ctx,RenderPass* pass)
{
    material = Resource::get_resource<Material>(pass->pass_node.material);
    texture = Resource::get_resource<Texture>(pass->pass_node.texture);
    frame_buff = ctx->get_frame_buffer(pass->pass_node.frame_buff_index);
    fragment_map = &pass->pass_node.camera->fragment_map;
    this->width = ctx->window_handle->w;
    this->height = ctx->window_handle->h;
}
void FragShader::end_render_pass(Context* ctx,RenderPass* pass)
{
    material = nullptr;
    texture = nullptr;
    frame_buff=nullptr;
    fragment_map=nullptr;
    this->width = 0;
    this->height = 0;
}



Color FragShader::run(int frag_index )
{
    auto frag = &(*this->fragment_map)[frag_index];
    auto& alpha = frag->alpha;
    // auto& v1 = frag->triangle->vert[0];
    // auto& v2 = frag->triangle->vert[1];
    // auto& v3 = frag->triangle->vert[2];
    // Vec3 c1, c2, c3;
    // v1.get_attribute_value(1, c1);
    // v2.get_attribute_value(1, c2);
    // v3.get_attribute_value(1, c3);
    // Vec3 frag_color = c1 * alpha[0] + c2 * alpha[1] + c3 * alpha[2];
    return GREEN;
}

Color TextureFragShader::run(int frag_index)
{
    auto frag = &(*this->fragment_map)[frag_index];
    Vec4 frag_color;
    this->sample_texture(frag_index, texture, frag_color);
    return color(frag_color);
}





