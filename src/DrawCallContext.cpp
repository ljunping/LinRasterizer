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
#include "VertShader.h"



DrawCallContext::~DrawCallContext()
{
    for (int i = 0; i < GeometryCount; ++i)
    {
        free(this->geometrie_pools[i]);
    }
    free(this->gl_positions);
    free(this->outputs);
    delete bvh_tree;
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

RenderNode* DrawCallContext::get_render_node(Mesh* mesh)
{
    auto ptr = mesh_2_node.find(mesh);
    RUNTIME_ASSERT(ptr != mesh_2_node.end(),"get_render_node ptr != mesh_2_node.end()");
    return &this->nodes[ptr->second];
}


std::shared_ptr<Mesh> DrawCallContext::get_mesh(int vert_index, int& mesh_vert_index) const
{
    auto _upper_bound = std::upper_bound(mesh_vert_count.begin() + 1, mesh_vert_count.end(), vert_index);
    if (_upper_bound != mesh_vert_count.end())
    {
        auto _mesh_index = _upper_bound - mesh_vert_count.begin() - 1;
        mesh_vert_index = vert_index - (*(_upper_bound - 1));
        return meshes[_mesh_index];
    }
    return nullptr;
}

std::shared_ptr<Mesh> DrawCallContext::get_mesh_by_tri_index(int tri_index, int& mesh_tri_index) const
{
    auto _upper_bound = std::upper_bound(mesh_tri_count.begin() + 1, mesh_tri_count.end(), tri_index);
    if (_upper_bound != mesh_tri_count.end())
    {
        auto _mesh_index = _upper_bound - mesh_tri_count.begin() - 1;
        mesh_tri_index = tri_index - (*(_upper_bound - 1));
        RUNTIME_ASSERT(_mesh_index<meshes.size(), "Mesh index is out of range");
        return meshes[_mesh_index];
    }
    return nullptr;
}

int DrawCallContext::get_mesh_vert_index( Mesh* mesh, int mesh_index)
{
    auto ptr = this->mesh_2_index.find(mesh);
    RUNTIME_ASSERT(ptr!=this->mesh_2_index.end(),"get_mesh_vert_index ptr != mesh_2_index.end()");
    RUNTIME_ASSERT(ptr->second<this->mesh_vert_count.size(), "get_mesh_vert_index ptr->second < mesh_vert_count.size()");
    return this->mesh_vert_count[ptr->second] + mesh_index;
}

void DrawCallContext::get_tri_mesh_index(int tri_count, TrianglePrimitive& tri)
{
    int mesh_tri_index = 0;
    auto mesh = get_mesh_by_tri_index(tri_count, mesh_tri_index);
    mesh->generate_triangle_index(tri, mesh_tri_index);
}


template void DrawCallContext::get_vert_attribute_value(Mesh*,int vert_index, int attribute_index,
                                                        L_MATH::Vec<float, 1>& result);
template void DrawCallContext::get_vert_attribute_value(Mesh*,int vert_index, int attribute_index,
                                                        L_MATH::Vec<float, 2>& result);
template void DrawCallContext::get_vert_attribute_value(Mesh*,int vert_index, int attribute_index,
                                                        L_MATH::Vec<float, 3>& result);
template void DrawCallContext::get_vert_attribute_value(Mesh*,int vert_index, int attribute_index,
                                                        L_MATH::Vec<float, 4>& result);


template <int N>
void DrawCallContext::get_vert_attribute_value(Mesh* mesh,int vert_index,int attribute_index, L_MATH::Vec<float, N>& result)
{
    if (mesh != nullptr)
    {
        mesh->get_attribute_value(vert_index, attribute_index, result);
    }
}

void DrawCallContext::create_vert_attribute(Mesh* mesh, int v0, int v1, int v2, const L_MATH::Vec<float, 3>& alpha,
                                            VertexInterpolation& result)
{
    result.v[0] = v0;
    result.v[1] = v1;
    result.v[2] = v2;
    result.alpha = alpha;
    result.draw_call_context = this;
    result.mesh_ptr = mesh;
    result.calculate_values();
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
    auto offset = mesh_ptr->data_formats[attribute_index].offset;
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

void VertexInterpolation::calculate_values()
{
    auto& mesh = *mesh_ptr;
    for (int i = 0; i < mesh.vert_data_length; ++i)
    {
        values[i] = mesh[v[0]][i] * alpha[0] + mesh[v[1]][i] * alpha[1] + mesh[v[2]][i] *
            alpha[2];
    }
    Vec3 tan0, tan1, tan2;
    mesh.get_attribute_value(v[0], TANGENT, tan0);
    mesh.get_attribute_value(v[1], TANGENT, tan1);
    mesh.get_attribute_value(v[2], TANGENT, tan2);
    this->tangent = tan0 * alpha[0] + tan1 * alpha[1] + tan2 * alpha[2];
    auto& vertex_output0 = draw_call_context->outputs[v[0]].fix_outputs;
    auto& vertex_output1 = draw_call_context->outputs[v[1]].fix_outputs;
    auto& vertex_output2 = draw_call_context->outputs[v[2]].fix_outputs;

    for (int i = 0; i < VertOutPutTypeCount; ++i)
    {
        output.fix_outputs[i] = vertex_output0[i] * alpha[0] + vertex_output1[i] * alpha[1] + vertex_output2[i] * alpha[2];
    }
    if (this->draw_call_context->setting.enable_light_interpolation)
    {

        auto& tbn_light_dirs0 = draw_call_context->outputs[v[0]].tbn_light_dirs;
        auto& tbn_light_dirs1 = draw_call_context->outputs[v[1]].tbn_light_dirs;
        auto& tbn_light_dirs2 = draw_call_context->outputs[v[2]].tbn_light_dirs;

        auto& view_light_dirs0 = draw_call_context->outputs[v[0]].view_light_dirs;
        auto& view_light_dirs1 = draw_call_context->outputs[v[1]].view_light_dirs;
        auto& view_light_dirs2 = draw_call_context->outputs[v[2]].view_light_dirs;
        output.tbn_light_dirs.resize(tbn_light_dirs0.size());
        output.view_light_dirs.resize(view_light_dirs0.size());


        for (int i = 0; i < output.tbn_light_dirs.size(); ++i)
        {
            output.tbn_light_dirs[i]=
                tbn_light_dirs0[i] * alpha[0] + tbn_light_dirs1[i] * alpha[1] + tbn_light_dirs2[i] * alpha[2];
        }
        for (int i = 0; i < output.view_light_dirs.size(); ++i)
        {
            output.view_light_dirs[i] =
                view_light_dirs0[i] * alpha[0] + view_light_dirs1[i] * alpha[1] + view_light_dirs2[i] * alpha[2];

        }
    }

}