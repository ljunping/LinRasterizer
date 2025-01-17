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
#include "Mesh.h"
#include "VertShader.h"

template<class T>
static bool less_compare_array(const T* a, const T* b, int count)
{
    for (int i = 0; i < count; ++i)
    {
        if (a[i] < b[i]) return true;
    }
    return false;
}

template<class T>
static bool equal_compare_array(const T* a, const T* b, int count)
{
    for (int i = 0; i < count; ++i)
    {
        if (a[i] != b[i])
        {
            return false;
        }
    }
    return true;
}

bool less_compare_render_node(const RenderNode& a, const RenderNode& b)
{
    return a.render_order < b.render_order
        || less_compare_array(a.materials, b.materials,MAX_MATERIAL_COUNT)
        || less_compare_array(a.textures, b.textures,MAX_TEXTURES_COUNT)
        || a.frag_shader < b.frag_shader ||
        a.vert_shader < b.vert_shader;
}

bool equal_compare_render_node(const RenderNode& a, const RenderNode& b)
{
    return
        a.render_order == b.render_order
        && equal_compare_array(a.materials, b.materials,MAX_MATERIAL_COUNT)
        && equal_compare_array(a.textures, b.textures,MAX_TEXTURES_COUNT)
        && a.frag_shader == b.frag_shader
        && a.vert_shader == b.vert_shader;


}


void DrawCallContext::assign_geometry_primitives(GeometryType type, int count)
{
    auto _count = geometry_count[type];
    auto& geometrie_pool = geometrie_pools[type];
    if (_count != count)
    {
        free(geometrie_pool);
        geometrie_pool = static_cast<Geometry*>(malloc(count * geometry_size(type)));
        geometry_invalid[type] = static_cast<bool*>(malloc(count));
        geometry_count[type] = count;
    }
}

void DrawCallContext::assign_vert(int vert_count)
{
    if (vert_count != this->gl_vert_count)
    {
        this->gl_vert_count = vert_count;
        free(this->gl_verts);
        this->gl_verts = static_cast<GlVert*>(malloc(sizeof(GlVert) * vert_count));
        this->vert_invalid = static_cast<bool*>(malloc(sizeof(bool) * vert_count));
    }
}

void DrawCallContext::raycaster_process_geometry(int mesh_tri_count)
{
    int geometry_index = mesh_tri_count;
    std::vector<int> render_nodes[GeometryCount];
    int geometry_count[GeometryCount]{};
    int _geometry_sum = mesh_tri_count;
    for (int i = 0; i < this->nodes.size(); ++i)
    {
        auto& node = this->nodes[i];
        if (node.local_geometry)
        {
            render_nodes[node.local_geometry->geometry_type].emplace_back(i);
            geometry_count[node.local_geometry->geometry_type]++;
            _geometry_sum++;
        }
    }
    this->geometry_2_node.resize(_geometry_sum);
    this->geometries.resize(_geometry_sum);
    for (int i = 0; i < GeometryCount; ++i)
    {
        auto _GeometryType = static_cast<GeometryType>(i);
        int offset = i == TRI ? mesh_tri_count : 0;
        int _geometry_count = geometry_count[i] + offset;
        this->assign_geometry_primitives(static_cast<GeometryType>(i), _geometry_count);
        for (int j = offset; j < _geometry_count; ++j)
        {
            auto render_node_indx = render_nodes[i][j - offset];
            auto& render_node = this->nodes[render_node_indx];
            auto _Geometry = reinterpret_cast<Geometry*>(
                reinterpret_cast<char*>(this->geometrie_pools[i]) +
                j * geometry_size(_GeometryType));
            this->geometry_invalid[_GeometryType][j] = true;
            build_geometry(_GeometryType, _Geometry);
            render_node.local_geometry->clone(_Geometry);
            _Geometry->id = geometry_index;
            this->geometry_2_node[_Geometry->id] = &render_node;
            this->geometries[geometry_index] = _Geometry;
            _Geometry->transform(render_node.model_matrix);
            render_node.transform_geometry = _Geometry;
            geometry_index++;
        }
    }
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

RenderNodeComponent::~RenderNodeComponent()
{

}


MeshProvider::~MeshProvider()
{
}



void MeshProvider::locate_centroid(Camera* camera) const
{
    if (!mesh)
    {
        return;
    }
    auto mesh_centroid = mesh->get_mesh_centroid();
    Box<3> box = mesh->get_box();
    auto vp = camera->get_view_mat();
    Mat44 vp_inv = L_MATH::inverse(vp);
    Vec4 target_proj_center = {0, 0, -2 * camera->view_frustum.near, 1};
    Vec3 target_world_center = Vec3((vp_inv * target_proj_center)[0]);

    auto _tar_l = camera->view_frustum.near * tan(L_MATH::deg2rad(camera->view_frustum.fov / 2));
    auto _offset = (box.max - box.min);
    float scale = _tar_l * 2 / _offset[1];

    auto _locate_mat = L_MATH::translate(target_world_center) * L_MATH::scale({scale, scale, scale}) *
        L_MATH::translate(mesh_centroid * -1);
    this->scene_node->set_local_to_global_mat(_locate_mat);
}


std::shared_ptr<Mesh> MeshRender::get_mesh()
{
    auto mesh_provider = scene_node->get_component<MeshProvider>();
    if (mesh_provider)
    {
        return mesh_provider->mesh;
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
    render_node.mesh = scene_node->get_component<MeshProvider>()->mesh;
    render_node.model_matrix = this->scene_node->get_local_to_global_mat();
    render_node.render_order = this->render_order;
    render_node.transparent = this->transparent;
    render_node.camera = camera;
    render_node.vert_shader = this->vert_shader;
    render_node.shadow_caster = this->shader_caster;
    render_node.emit = false;
    for (int i = 0; i < MAX_MATERIAL_COUNT; ++i)
    {
        render_node.materials[i] = this->materials[i];
    }
    for (int i = 0; i < MAX_TEXTURES_COUNT; ++i)
    {
        render_node.textures[i] = this->textures[i];
    }
    render_node.transform = this->scene_node;
}



void GeometryRender::collect_render_node(Camera* camera, std::vector<RenderNode>& render_nodes)
{
    if (!camera->is_render_layer(this->render_layer))
    {
        return;
    }
    if (geometry && get_current_ctx()->setting.enable_global_path_trace)
    {
        auto& render_node = render_nodes.emplace_back();
        render_node.model_matrix = this->scene_node->get_local_to_global_mat();
        render_node.local_geometry = this->geometry;
        for (int i = 0; i < MAX_MATERIAL_COUNT; ++i)
        {
            render_node.materials[i] = this->materials[i];
        }
        for (int i = 0; i < MAX_TEXTURES_COUNT; ++i)
        {
            render_node.textures[i] = this->textures[i];
        }
        render_node.transform = this->scene_node;
        render_node.emit = this->emit;

    }
}

void RenderNodeManager::collection_render_node(Camera* camera, std::vector<RenderNode>& render_nodes)
{
    for (auto object : objects)
    {
        object->collect_render_node(camera, render_nodes);
    }
}


MeshRender* add_mesh_render(Transform* node, const SHARE_PTR<VertShader>& vert_shader,
                            const SHARE_PTR<FragShader>& frag_shader,
                            const SHARE_PTR<Material>& material,
                            const SHARE_PTR<Texture>& texture, int render_layer, bool transparent)
{

    auto meshrender = node->add_component<MeshRender>();
    meshrender->frag_shader = Resource::copy_resource(frag_shader);
    meshrender->vert_shader = Resource::copy_resource(vert_shader);
    (meshrender->materials[0] = material);
    meshrender->textures[0] = texture;
    meshrender->frame_buff_index = 0;
    meshrender->transparent = transparent;
    meshrender->render_layer = render_layer;

    return meshrender;
}


GeometryRender* add_geometry_render(Transform* node,const SHARE_PTR<Geometry>& geo,
                                    const SHARE_PTR<Material>& material,
                                    const SHARE_PTR<Texture>& texture, int render_layer)
{
    auto meshrender = node->add_component<GeometryRender>();
    meshrender->frame_buff_index = 0;
    meshrender->geometry = geo;
    meshrender->render_layer = render_layer;
    meshrender->materials[0] = material;
    meshrender->textures[0] = texture;
    return meshrender;
}


Transform* create_mesh_provider(const SHARE_PTR<Mesh>& mesh)
{
    //mesh
    auto node = CREATE_OBJECT_BY_TYPE(Transform);
    auto mesh_provider = node->add_component<MeshProvider>();
    mesh_provider->mesh = mesh;
    return node;
}






