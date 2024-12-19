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



Vec4 FragShader::run(int frag_index )
{
    auto frag = &(*this->fragment_map)[frag_index];
    auto& alpha = frag->alpha;
    return v_color4(GREEN);
}

Vec4 TextureFragShader::run(int frag_index)
{
    if (texture && texture->channels == 3)
    {
        Vec3 frag_color;
        this->sample_texture(frag_index, texture, frag_color);
        return Vec4(frag_color, 1);
    }
    if (texture && texture->channels == 4)
    {
        Vec4 frag_color;
        this->sample_texture(frag_index, texture, frag_color);
        return frag_color;
    }
}





