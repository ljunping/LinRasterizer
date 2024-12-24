//
// Created by Lin on 2024/12/4.
//

#include "MeshRender.h"
#if __linux__
#include <bits/ranges_algo.h>
#endif
#include "Context.h"
#include "Transform.h"


bool compare_transparent_render_node(const RenderNode& a, const RenderNode& b)
{
    return a.model_matrix[3][2] < b.model_matrix[3][2];
}

bool less_compare_render_node(const RenderNode& a, const RenderNode& b)
{
    return a.render_order < b.render_order
        || a.texture < b.texture
        || a.material < b.material
        || a.frag_shader < b.frag_shader;
}

bool equal_compare_render_node(const RenderNode& a, const RenderNode& b)
{
    return
    a.render_order == b.render_order
        && a.texture == b.texture
        && a.material == b.material
        && a.frag_shader == b.frag_shader;
}

bool DrawCallData::try_add_render_node(RenderNode& node)
{
    init(node);
    if (!equal_compare_render_node(node, pass_node))
    {
        return false;
    }
    meshes.emplace_back(node.mesh, node.model_matrix);
    return true;
}

void DrawCallData::init(const RenderNode& node)
{
    if (!is_init)
    {
        is_init = true;
        pass_node = node;
        this->ctx = get_current_ctx();
        this->texture0 = Resource::get_resource<Texture>(node.texture);
        this->frag_shader0 = Resource::get_resource<FragShader>(node.frag_shader);
        this->material0 = Resource::get_resource<Material>(node.material);
        this->frame_buff = ctx->get_frame_buffer(node.frame_buff_index);
    }
}

DrawCallData::~DrawCallData()
{
    free(this->tri_pool);
    delete bvh_tree;
}

void DrawCallData::assign_triangle_primitives(int size)
{
    if (primitives.size() != size)
    {
        primitives.resize(size);
        free(this->tri_pool);
        this->tri_pool = static_cast<TrianglePrimitive*>(malloc(size * sizeof(TrianglePrimitive)));
    }
}

void DrawCallData::build_bvh_tree()
{
    this->bvh_tree = new BVHTree();
    this->bvh_tree->build(this->primitives);
}

void DrawCallData::draw_call_begin()
{
    this->frag_shader0->begin_draw_call(this);
}

void DrawCallData::draw_call_end()
{
    this->frag_shader0->end_draw_call(this);
}

void DrawCallData::get_model_matrix(const Mesh* mesh, L_MATH::Mat<float, 4, 4>& m) const
{
    for (auto _mesh : meshes)
    {
        if (_mesh.first == mesh->get_resource_id())
        {
            m = _mesh.second;
            return;
        }
    }
}

void DrawCallData::get_view_matrix(L_MATH::Mat<float, 4, 4>& m) const
{
    m = this->pass_node.camera->get_view_mat();
}

void DrawCallData::get_proj_matrix(L_MATH::Mat<float, 4, 4>& m) const
{
    m = this->pass_node.camera->projection_mat;
}


void RenderComponent::on_create()
{
    this->Component::on_create();
    get_current_ctx()->render_manager->on_create_obj(this);
}

void RenderComponent::on_delete()
{
    this->Component::on_delete();
    get_current_ctx()->render_manager->on_delete_obj(this);
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
    render_node.texture = this->texture;
    render_node.material = this->material;
}

void RenderManager::collection_render_node(Camera* camera, std::vector<RenderNode>& render_nodes, bool transparent)
{
    for (auto object : objects)
    {
        if (object->transparent == transparent)
        {
            object->collect_render_node(camera, render_nodes);
        }
    }
}


void RenderManager::calculate_render_pass(Camera* camera, std::vector<DrawCallData>& render_passes,bool transparent)
{
    std::vector<RenderNode> render_nodes;
    collection_render_node(camera, render_nodes, transparent);
    if (render_nodes.empty())
    {
        return;
    }
    if (!transparent)
    {
        render_passes.emplace_back();
        std::ranges::sort(render_nodes, less_compare_render_node);
        for (auto render_node : render_nodes)
        {
            if (!render_passes.back().try_add_render_node(render_node))
            {
                render_passes.emplace_back();
                render_passes.back().try_add_render_node(render_node);
            }
        }
    }else
    {
        std::ranges::sort(render_nodes, compare_transparent_render_node);
        for (auto render_node : render_nodes)
        {
            render_passes.emplace_back();
            render_passes.back().try_add_render_node(render_node);
        }
    }

}