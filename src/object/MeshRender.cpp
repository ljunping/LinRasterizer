//
// Created by Lin on 2024/12/4.
//

#include "MeshRender.h"
#if __linux__
#include <bits/ranges_algo.h>
#endif
#include "Camera.h"
#include "Context.h"
#include "Transform.h"
#include "DrawCallContext.h"


static bool less_compare_array(const int* a, const int* b, int count)
{
    for (int i = 0; i < count; ++i)
    {
        if (a[i] < b[i]) return true;
    }
    return false;
}


static bool equal_compare_array(const int* a, const int* b, int count)
{
    for (int i = 0; i < count; ++i)
    {
        if (a[i]!=b[i]) return false;
    }
    return true;
}

bool less_compare_render_node(const RenderNode& a, const RenderNode& b)
{
    return a.render_order < b.render_order
        || less_compare_array(a.materials, b.materials,MAX_MATERIAL_COUNT)
        || less_compare_array(a.textures, b.textures,MAX_TEXTURES_COUNT)
        || a.frag_shader < b.frag_shader;

}

bool equal_compare_render_node(const RenderNode& a, const RenderNode& b)
{
    return
        a.render_order == b.render_order
        && equal_compare_array(a.materials, b.materials,MAX_MATERIAL_COUNT)
        && equal_compare_array(a.textures, b.textures,MAX_TEXTURES_COUNT)
        && a.frag_shader == b.frag_shader;
}



void RenderNodeComponent::on_create()
{
    this->Component::on_create();
    get_current_ctx()->render_node_manager->on_create_obj(this);
}

void RenderNodeComponent::on_delete()
{
    this->Component::on_delete();
    get_current_ctx()->render_node_manager->on_delete_obj(this);
}



void MeshProvider::locate_centroid(Camera* camera) const
{
    auto mesh = Resource::get_resource<Mesh>(mesh_id);
    if (!mesh)
    {
        return;
    }
    auto mesh_centroid = mesh->get_mesh_centroid();
    Box<3> box = mesh->get_box();
    auto vp = camera->get_view_mat();
    Mat44 vp_inv;
    L_MATH::inverse(vp, vp_inv);
    Vec4 target_proj_center = {0, 0, -2 * camera->near, 1};
    Vec3 target_world_center = Vec3((vp_inv * target_proj_center)[0]);

    auto _tar_l = camera->near * tan(L_MATH::deg2rad(camera->fov / 2));
    auto _offset = (box.max - box.min);
    float scale = _tar_l * 2 / _offset[1];

    auto _locate_mat = L_MATH::translate(target_world_center) * L_MATH::scale({scale, scale, scale}) *
        L_MATH::translate(mesh_centroid * -1);
    this->scene_node->set_local_to_global_mat(_locate_mat);
}

Mesh* MeshProvider::get_mesh() const
{
    return Resource::get_resource<Mesh>(mesh_id);
}


Mesh* MeshRender::get_mesh()
{
    auto mesh_provider = scene_node->get_component<MeshProvider>();
    if (mesh_provider)
    {
        return mesh_provider->get_mesh();
    }
    return nullptr;
}

void MeshRender::collect_render_node(Camera* camera,std::vector<RenderNode>& render_nodes)
{
    if (!camera->is_render_layer(this->render_layer))
    {
        return;
    }
    render_nodes.emplace_back();
    auto& render_node = render_nodes.back();
    render_node.frame_buff_index = this->frame_buff_index;
    render_node.frag_shader = frag_shader;
    render_node.mesh = scene_node->get_component<MeshProvider>()->mesh_id;
    render_node.model_matrix = this->scene_node->get_local_to_global_mat();
    render_node.render_order = this->render_order;
    render_node.transparent = this->transparent;
    render_node.camera = camera;
    render_node.vert_shader = this->vert_shader;
    for (int i = 0; i < MAX_MATERIAL_COUNT; ++i)
    {
        render_node.materials[i] = this->materials[i];
    }
    for (int i = 0; i < MAX_TEXTURES_COUNT; ++i)
    {
        render_node.textures[i] = this->textures[i];
    }
}


void RenderNodeManager::collection_render_node(Camera* camera, std::vector<RenderNode>& render_nodes, bool transparent)
{
    for (auto object : objects)
    {
        if (object->transparent == transparent)
        {
            object->collect_render_node(camera, render_nodes);
        }
    }
}










