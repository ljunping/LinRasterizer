//
// Created by Lin on 2024/12/26.
//

#include "DrawCallContext.h"

#include "BVHTree.h"
#include "Geometry.h"
#include "Camera.h"
#include "Context.h"
#include "FragShader.h"
#include "RasterizerJob.h"
#include "Resource.h"
#include "TrianglePrimitive.h"
#include "VertShader.h"
#include "CommonMacro.h"
#include "Light.h"
#include "Transform.h"


void DrawCallContext::generate_triangle_primitive(TrianglePrimitive& tri)
{
    for (int i = 0; i < 3; ++i)
    {
        auto vert_index = this->get_muti_mesh_vert_index(tri.mesh, tri.vert_index[i]);
        Vec4& v = static_cast<L_MATH::Vec<float, 4>&>(tri.v[i]);
        v = this->gl_positions[vert_index];
    }
    tri.update_param();
}

bool DrawCallContext::try_add_render_node(RenderNode& node)
{
    set_render_node(node);
    if (!equal_compare_render_node(node, render_node))
    {
        return false;
    }
    auto resource = Resource::get_resource<Mesh>(node.mesh);
    if (resource == nullptr)
    {
        return false;
    }
    meshes.emplace_back(resource, node.model_matrix);
    return true;
}

void DrawCallContext::set_render_node(const RenderNode& node)
{
    if (!is_set_render_node)
    {
        is_set_render_node = true;
        render_node = node;
        for (int i = 0; i < MAX_MATERIAL_COUNT; ++i)
        {
            this->materials[i] = Resource::get_resource<Material>(node.materials[i]);
        }
        for (int i = 0; i < MAX_TEXTURES_COUNT; ++i)
        {
            this->textures[i] = Resource::get_resource<Texture>(node.textures[i]);
        }
        this->frag_shader = Resource::get_resource<FragShader>(node.frag_shader);
        this->vert_shader = Resource::get_resource<VertShader>(node.vert_shader);
    }
}

DrawCallContext::~DrawCallContext()
{
    free(this->tri_pool);
    free(this->gl_positions);
    free(this->outputs);
    delete bvh_tree;
}

void DrawCallContext::assign_triangle_primitives(int size)
{
    if (primitives.size() != size)
    {
        primitives.resize(size);
        free(this->tri_pool);
        this->tri_pool = static_cast<TrianglePrimitive*>(malloc(size * sizeof(TrianglePrimitive)));
    }
}



Mat44 DrawCallContext::get_model_matrix(Mesh* mesh) const
{
    for (auto _mesh : meshes)
    {
        if (_mesh.first == mesh)
        {
            return _mesh.second;
        }
    }
    return L_MATH::Mat<float, 4, 4>::IDENTITY;
}


Mesh* DrawCallContext::get_mesh(int vert_index,int& mesh_index) const
{
    for (auto mesh : meshes)
    {
        mesh_index = vert_index;
        if (mesh.first->vert_count > vert_index)
        {
            return mesh.first;
        }
        vert_index -= mesh.first->vert_count;
    }
    return nullptr;
}

int DrawCallContext::get_muti_mesh_vert_index(const Mesh* mesh, int mesh_index) const
{
    int vert_index = 0;
    for (auto mesh_ : meshes)
    {
        if (mesh_.first != mesh)
        {
            vert_index += mesh_.first->vert_count;
        }
        else
        {
            vert_index += mesh_index;
            break;
        }
    }
    return vert_index;
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