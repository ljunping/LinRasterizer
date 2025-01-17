//
// Created by Lin on 2024/12/26.
//

#include "DrawCallContext.h"
#include "BVHTree.h"
#include "Geometry.h"
#include "Context.h"
#include "CommonMacro.h"
#include "Mesh.h"
#include "Transform.h"
#include "Material.h"
#include "Texture.h"
#include "FragShader.h"
#include "Light.h"
#include "VertShader.h"



DrawCallContext::~DrawCallContext()
{
    for (int i = 0; i < GeometryCount; ++i)
    {
        free(this->geometrie_pools[i]);
        free(this->geometry_invalid[i]);

    }
    free(this->vert_invalid);
    free(this->gl_verts);
    delete bvh_tree;
}

int DrawCallContext::get_mesh_vert_count() const
{
    return mesh_vert_count[mesh_vert_count.size() - 1];
}

int DrawCallContext::get_mesh_tri_count() const
{
    return mesh_tri_count[mesh_tri_count.size() - 1];
}


const RenderNode* DrawCallContext::get_render_node(int vert_index) const
{
    return gl_verts[vert_index].render_node;
}


void DrawCallContext::set_mesh_render_params(const RenderNode& node)
{
    for (int i = 0; i < MAX_MATERIAL_COUNT; ++i)
    {
        this->materials[i] = node.materials[i];
    }
    for (int i = 0; i < MAX_TEXTURES_COUNT; ++i)
    {
        this->textures[i] = node.textures[i];
    }
    this->frag_shader = node.frag_shader;
    this->vert_shader = node.vert_shader;
}


void DrawCallContext::add_render_node(RenderNode& node)
{
    auto resource = node.mesh;
    this->nodes.emplace_back(node);
    if (node.emit)
    {
        emit_render_node.emplace_back(this->nodes.size() - 1);
    };
    if (resource != nullptr)
    {
        this->mesh_2_node[resource.get()] = this->nodes.size() - 1;
        this->meshes.emplace_back(resource);
        this->mesh_2_index[resource.get()] = this->meshes.size() - 1;
        this->mesh_vert_count.push_back(resource->vert_count + this->mesh_vert_count.back());
        this->mesh_tri_count.push_back(resource->tri_count() + this->mesh_tri_count.back());
    }
}

const RenderNode* DrawCallContext::get_render_node_by_mesh(SHARE_PTR<Mesh>& mesh) const
{
    auto ptr = mesh_2_node.find(mesh.get());
    RUNTIME_ASSERT(ptr != mesh_2_node.end(), "get_render_node_by_mesh ptr != mesh_2_node.end()");
    return &this->nodes[ptr->second];
}


int DrawCallContext::get_mesh_index(int vert_index, int& mesh_vert_index) const
{
    if (vert_index >= get_mesh_vert_count())
    {
        return -1;
    }
    auto _upper_bound = std::upper_bound(mesh_vert_count.begin() + 1, mesh_vert_count.end(), vert_index);
    if (_upper_bound != mesh_vert_count.end())
    {
        auto _mesh_index = _upper_bound - mesh_vert_count.begin() - 1;
        mesh_vert_index = vert_index - (*(_upper_bound - 1));
        return _mesh_index;
    }
    return -1;
}

int DrawCallContext::get_mesh_index_by_tri_index(int tri_index, int& mesh_tri_index) const
{
    if (tri_index >= get_mesh_tri_count())
    {
        return -1;
    }
    auto _upper_bound = std::upper_bound(mesh_tri_count.begin() + 1, mesh_tri_count.end(), tri_index);
    if (_upper_bound != mesh_tri_count.end())
    {
        auto _mesh_index = _upper_bound - mesh_tri_count.begin() - 1;
        mesh_tri_index = tri_index - (*(_upper_bound - 1));
        return _mesh_index;
    }
    return -1;
}


template void DrawCallContext::get_vert_attribute_value(int vert_index, int attribute_index,
                                                        L_MATH::Vec<float, 1>& result);
template void DrawCallContext::get_vert_attribute_value(int vert_index, int attribute_index,
                                                        L_MATH::Vec<float, 2>& result);
template void DrawCallContext::get_vert_attribute_value(int vert_index, int attribute_index,
                                                        L_MATH::Vec<float, 3>& result);
template void DrawCallContext::get_vert_attribute_value(int vert_index, int attribute_index,
                                                        L_MATH::Vec<float, 4>& result);



template <int N>
void DrawCallContext::get_vert_attribute_value(int vert_index,int attribute_index, L_MATH::Vec<float, N>& result)
{
    auto& gl_vert = gl_verts[vert_index];
    gl_vert.vertex_interpolation.get_attribute_value(attribute_index, result);
}

void DrawCallContext::create_vert_attribute(int vert_index0, int vert_index1, int vert_index2,
    const L_MATH::Vec<float, 3>& alpha, VertexInterpolation& result) const
{
    interpolation_attribute(gl_verts[vert_index0].vertex_interpolation, gl_verts[vert_index1].vertex_interpolation,
                            gl_verts[vert_index2].vertex_interpolation
                            , result, alpha);
    interpolation_out_put(vert_index0, vert_index1, vert_index2, alpha, result);
}

void DrawCallContext::create_vert(int vert_index
                                  , int mesh_tri_index,
                                  SHARE_PTR<Mesh>& mesh,
                                  const Vec3& clip_alpha)
{

    int mesh_vert_index[3];
    mesh->get_triangle_index(mesh_vert_index, mesh_tri_index);
    auto& gl_vert = this->gl_verts[vert_index];
    gl_vert.mesh_vert = false;
    gl_vert.render_node = get_render_node_by_mesh(mesh);
    gl_vert.mesh_vert_index[0] = mesh_vert_index[0];
    gl_vert.mesh_vert_index[1] = mesh_vert_index[1];
    gl_vert.mesh_vert_index[2] = mesh_vert_index[2];
    gl_vert.alpha = clip_alpha;
    this->build_vert_interpolation(vert_index);
    this->vert_shader->run(this, vert_index);
    this->vert_invalid[vert_index] = true;
}

void DrawCallContext::create_vert(int vert_index, int mesh_vert_index, SHARE_PTR<Mesh>& mesh)
{
    auto& gl_vert = this->gl_verts[vert_index];
    gl_vert.mesh_vert = true;
    gl_vert.render_node = get_render_node_by_mesh(mesh);
    gl_vert.mesh_vert_index[0] = mesh_vert_index;
    this->build_vert_interpolation(vert_index);
    this->vert_shader->run(this, vert_index);
    this->vert_invalid[vert_index] = true;
}

TrianglePrimitive* DrawCallContext::create_tri(int tri_index, int mesh_index, int mesh_tri_index)
{
    auto* result = new(reinterpret_cast<TrianglePrimitive*>(this->geometrie_pools[TRI]) + tri_index) TrianglePrimitive();
    if (this->geometries.size() > tri_index)
    {
        this->geometries[tri_index] = result;
    }
    int mesh_vert_index[3];
    meshes[mesh_index]->get_triangle_index(mesh_vert_index, mesh_tri_index);
    this->geometry_invalid[TRI][tri_index] = true;
    int _pre_mesh_count = this->mesh_vert_count[mesh_index];
    result->gl_vert_index[0] = _pre_mesh_count + mesh_vert_index[0];
    result->gl_vert_index[1] = _pre_mesh_count + mesh_vert_index[1];
    result->gl_vert_index[2] = _pre_mesh_count + mesh_vert_index[2];
    result->id = tri_index;
    Vec4& v0 = static_cast<L_MATH::Vec<float, 4>&>(result->v[0]);
    Vec4& v1 = static_cast<L_MATH::Vec<float, 4>&>(result->v[1]);
    Vec4& v2 = static_cast<L_MATH::Vec<float, 4>&>(result->v[2]);

    v0 = this->gl_verts[result->gl_vert_index[0]].gl_position;
    v1 = this->gl_verts[result->gl_vert_index[1]].gl_position;
    v2 = this->gl_verts[result->gl_vert_index[2]].gl_position;

    auto render_node = this->get_render_node(result->gl_vert_index[0]);
    if (geometry_2_node.size()>tri_index)
    {
        this->geometry_2_node[tri_index] = render_node;
    }
    result->ctx = this;
    result->update_param();
    return result;
}


TrianglePrimitive* DrawCallContext::create_tri(int tri_index, int vert_v0, int vert_v1, int vert_v2)
{
    auto* tri = new(reinterpret_cast<TrianglePrimitive*>(this->geometrie_pools[TRI]) + tri_index) TrianglePrimitive();
    if (this->geometries.size() > tri_index)
    {
        this->geometries[tri_index] = tri;
    }
    auto node = this->gl_verts[vert_v0].render_node;

    this->geometry_invalid[TRI][tri_index] = true;
    tri->gl_vert_index[0] = vert_v0;
    tri->gl_vert_index[1] = vert_v1;
    tri->gl_vert_index[2] = vert_v2;
    tri->id = tri_index;
    Vec4& v0 = static_cast<L_MATH::Vec<float, 4>&>(tri->v[0]);
    Vec4& v1 = static_cast<L_MATH::Vec<float, 4>&>(tri->v[1]);
    Vec4& v2 = static_cast<L_MATH::Vec<float, 4>&>(tri->v[2]);

    v0 = this->gl_verts[tri->gl_vert_index[0]].gl_position;
    v1 = this->gl_verts[tri->gl_vert_index[1]].gl_position;
    v2 = this->gl_verts[tri->gl_vert_index[2]].gl_position;

    if (this->geometry_2_node.size() > tri_index)
    {
        this->geometry_2_node[tri_index] = node;
    }
    tri->ctx = this;
    tri->update_param();
    return tri;
}

void DrawCallContext::build_vert_interpolation(int v0) const
{
    auto& gl_vert = gl_verts[v0];
    auto& alpha = gl_vert.alpha;
    auto& mesh_vert_index = gl_verts[v0].mesh_vert_index;
    auto& mesh = *gl_vert.render_node->mesh;
    new(&gl_vert.vertex_interpolation.output.tbn_light_dirs) std::vector<Vec4>(
        this->ctx->light_manager->get_light_count());

    new(&gl_vert.vertex_interpolation.output.view_light_dirs) std::vector<Vec4>(
        this->ctx->light_manager->get_light_count());

    gl_vert.vertex_interpolation.mesh = &mesh;

    if (gl_vert.mesh_vert)
    {
        for (int i = 0; i < mesh.vert_data_length; ++i)
        {
            gl_vert.vertex_interpolation.values[i] = mesh[mesh_vert_index[0]][i];
        }
        mesh.get_attribute_value(mesh_vert_index[0], TANGENT, gl_vert.vertex_interpolation.tangent);
    }else
    {
        for (int i = 0; i < mesh.vert_data_length; ++i)
        {
            gl_vert.vertex_interpolation.values[i] = mesh[mesh_vert_index[0]][i] * alpha[0]
                + mesh[mesh_vert_index[1]][i] * alpha[1]
                + mesh[mesh_vert_index[2]][i] * alpha[2];
        }
        Vec3 tan0, tan1, tan2;
        mesh.get_attribute_value(mesh_vert_index[0], TANGENT, tan0);
        mesh.get_attribute_value(mesh_vert_index[1], TANGENT, tan1);
        mesh.get_attribute_value(mesh_vert_index[2], TANGENT, tan2);

        gl_vert.vertex_interpolation.tangent = tan0 * alpha[0] + tan1 * alpha[1] + tan2 * alpha[2];
    }

}

void DrawCallContext::interpolation_attribute(const VertexInterpolation& v0, const VertexInterpolation& v1,
                                              const VertexInterpolation& v2,
                                              VertexInterpolation& result, const L_MATH::Vec<float, 3>& alpha)
{
    result.mesh = v0.mesh;
    auto& mesh = *v0.mesh;
    for (int i = 0; i < mesh.vert_data_length; ++i)
    {
        result.values[i] = v0.values[i] * alpha[0] + v1.values[i] * alpha[1] + v2.values[i] * alpha[2];
    }
    result.tangent = v0.tangent * alpha[0] + v1.tangent * alpha[1] + v2.tangent * alpha[2];
}

void DrawCallContext::interpolation_out_put(int v0, int v1, int v2,const Vec3& alpha, VertexInterpolation& result) const
{
    auto& gl_vert0 = this->gl_verts[v0];
    auto& gl_vert1 = this->gl_verts[v1];
    auto& gl_vert2 = this->gl_verts[v2];

    auto& vertex_output0 = gl_vert0.vertex_interpolation.output.fix_outputs;
    auto& vertex_output1 = gl_vert1.vertex_interpolation.output.fix_outputs;
    auto& vertex_output2 = gl_vert2.vertex_interpolation.output.fix_outputs;


    for (int i = 0; i < VertOutPutTypeCount; ++i)
    {
        result.output.fix_outputs[i] = vertex_output0[i] * alpha[0] + vertex_output1[i] * alpha[1] +
            vertex_output2[i] * alpha[2];
    }

    if (this->setting.enable_light_interpolation)
    {
        auto& tbn_light_dirs0 = gl_vert0.vertex_interpolation.output.tbn_light_dirs;
        auto& tbn_light_dirs1 = gl_vert1.vertex_interpolation.output.tbn_light_dirs;
        auto& tbn_light_dirs2 = gl_vert2.vertex_interpolation.output.tbn_light_dirs;
        auto& view_light_dirs0 = gl_vert0.vertex_interpolation.output.view_light_dirs;
        auto& view_light_dirs1 = gl_vert1.vertex_interpolation.output.view_light_dirs;
        auto& view_light_dirs2 = gl_vert2.vertex_interpolation.output.view_light_dirs;
        result.output.tbn_light_dirs.resize(tbn_light_dirs0.size());
        result.output.view_light_dirs.resize(view_light_dirs0.size());
        for (int i = 0; i < result.output.tbn_light_dirs.size(); ++i)
        {
            result.output.tbn_light_dirs[i] =
                tbn_light_dirs0[i] * alpha[0] + tbn_light_dirs1[i] * alpha[1] + tbn_light_dirs2[i] *
                alpha[2];

        }
        for (int i = 0; i < result.output.view_light_dirs.size(); ++i)
        {
            result.output.view_light_dirs[i] =
                view_light_dirs0[i] * alpha[0] + view_light_dirs1[i] * alpha[1] + view_light_dirs2[i] *
                alpha[2];
        }
    }
}


bool DrawCallContext::get_clip_near_plane_alpha(const TrianglePrimitive* triangle_primitive,std::vector<Vec3>& clip_vertices_alpha) const
{
    auto v0 = triangle_primitive->gl_vert_index[0];
    auto v1 = triangle_primitive->gl_vert_index[1];
    auto v2 = triangle_primitive->gl_vert_index[2];

    std::vector<Vec3> clip_near_plane_alpha;
    std::vector<Vec3> gl_view_pos = {
        static_cast<Vec3&>(this->gl_verts[v0].vertex_interpolation.output.fix_outputs[VIEW_POS]),
        static_cast<Vec3&>(this->gl_verts[v1].vertex_interpolation.output.fix_outputs[VIEW_POS]),
        static_cast<Vec3&>(this->gl_verts[v2].vertex_interpolation.output.fix_outputs[VIEW_POS]),
    };

    auto near = -this->view_frustum.near;
    if (gl_view_pos[0][2] > near && gl_view_pos[1][2] > near && gl_view_pos[2][2] > near)
    {
        return false;
    }
    if (gl_view_pos[0][2] < near && gl_view_pos[1][2] < near && gl_view_pos[2][2] < near)
    {
        return true;
    }
    std::vector<std::vector<float>> clip_alphas;
    std::vector<Vec3> near_plane = {Vec3{0, 0, 1}};
    std::vector<float> c = {near};
    Sutherland_Hodgman(near_plane, c, gl_view_pos, &clip_alphas);
    clip_vertices_alpha.resize(clip_alphas.size());
    for (int i = 0; i < clip_alphas.size(); ++i)
    {
        auto& clip_alpha = clip_alphas[i];
        clip_vertices_alpha[i][0] = clip_alpha[0];
        clip_vertices_alpha[i][1] = clip_alpha[1];
        clip_vertices_alpha[i][2] = clip_alpha[2];
    }
    return false;
}



template void VertexInterpolation::get_attribute_value(int attribute_index,L_MATH::Vec<float, 4>& result);
template void VertexInterpolation::get_attribute_value(int attribute_index,L_MATH::Vec<float, 3>& result);
template void VertexInterpolation::get_attribute_value(int attribute_index,L_MATH::Vec<float, 2>& result);
template void VertexInterpolation::get_attribute_value(int attribute_index,L_MATH::Vec<float, 1>& result);


template <int N>
void VertexInterpolation::get_attribute_value(int attribute_index, L_MATH::Vec<float, N>& result)
{
    if constexpr (N == 3)
    {
        if (attribute_index==TANGENT)
        {
            result = tangent;
        }
    }
    auto offset = mesh->data_formats[attribute_index].offset;
    if constexpr (N == 3)
    {
        result[0] = values[offset];
        result[1] = values[offset + 1];
        result[2] = values[offset + 2];
    }

    if constexpr (N == 2)
    {
        result[0] = values[offset];
        result[1] = values[offset + 1];
    }

    if constexpr (N == 4)
    {
        result[0] = values[offset];
        result[1] = values[offset + 1];
        result[2] = values[offset + 2];
        result[3] = values[offset + 3];
    }
}



