//
// Created by Lin on 2024/12/4.
//

#include "MeshRender.h"

#include "Context.h"
#include "Transform.h"



bool less_compare_render_node(const RenderNode& a, const RenderNode& b)
{
    return a.render_order < b.render_order
        || a.texture < b.texture
        || a.material < b.material
        || a.frag_shader < b.frag_shader;
}

bool equal_compare_render_node(const RenderNode& a, const RenderNode& b)
{
    return a.render_order == b.render_order
        && a.texture == b.texture
        && a.material == b.material
        && a.frag_shader == b.frag_shader;
}

bool RenderPass::try_add_render_node(RenderNode& node)
{
    if (!is_init)
    {
        is_init = true;
        pass_node = node;
    }
    if (!equal_compare_render_node(node, pass_node))
    {
        return false;
    }
    meshes.emplace_back(node.mesh, node.model_matrix);
    return true;
}

RenderPass::~RenderPass()
{
    delete[] tri_pool;
    delete bvh_tree;
}

void RenderPass::assign_triangle_primitives(int size)
{
    primitives.resize(size);
    this->tri_pool = new TrianglePrimitive[size];
}

void RenderPass::build_bvh_tree()
{
    this->bvh_tree = new BVHTree();
    this->bvh_tree->build<TrianglePrimitive>(this->primitives);
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



void MeshRender::collect_render_node(Camera* camera,std::vector<RenderNode>& render_nodes)
{
    if (!camera->is_render_layer(this->sort_layer))
    {
        return;
    }
    render_nodes.emplace_back();
    auto& render_node = render_nodes.back();
    render_node.frame_buff_index = this->frame_buff_index;
    render_node.frag_shader = frag_shader;
    render_node.mesh = mesh;
    render_node.model_matrix = this->scene_node->get_local_to_global_mat();
    render_node.render_order = this->render_order;
    render_node.camera = camera;
    render_node.texture = this->texture;
    render_node.material = this->material;
}

void RenderManager::collection_render_node(Camera* camera, std::vector<RenderNode>& render_nodes)
{
    for (auto object : objects)
    {
        object->collect_render_node(camera, render_nodes);
    }
}

void RenderManager::calculate_render_pass(Camera* camera, std::vector<RenderPass>& render_passes)
{
    std::vector<RenderNode> render_nodes;
    collection_render_node(camera, render_nodes);
    if (render_nodes.empty())
    {
        return;
    }
    render_passes.emplace_back();
    std::ranges::sort(render_nodes, less_compare_render_node);
    for (auto render_node : render_nodes)
    {
        if(!render_passes.back().try_add_render_node(render_node))
        {
            render_passes.emplace_back();
            render_passes.back().try_add_render_node(render_node);
        }
    }
}