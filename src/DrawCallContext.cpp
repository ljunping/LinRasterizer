//
// Created by Lin on 2024/12/26.
//

#include "DrawCallContext.h"
#include "Box.h"
#include "Camera.h"
#include "Context.h"
#include "FragShader.h"
#include "RasterizerJob.h"
#include "Resource.h"
#include "TrianglePrimitive.h"
#include "VertShader.h"
#include "CommonMacro.h"

bool DrawCallContext::try_add_render_node(RenderNode& node)
{
    init(node);
    if (!equal_compare_render_node(node, pass_node))
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

void DrawCallContext::init(const RenderNode& node)
{
    if (!is_init)
    {
        is_init = true;
        pass_node = node;
        this->ctx = get_current_ctx();
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
        this->frame_buff = ctx->get_frame_buffer(node.frame_buff_index);
        this->camera = pass_node.camera;
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

void DrawCallContext::build_bvh_tree()
{
    this->bvh_tree = new BVHTree();
    this->bvh_tree->build(this->primitives);
}


void DrawCallContext::get_model_matrix(Mesh* mesh, L_MATH::Mat<float, 4, 4>& m) const
{
    for (auto _mesh : meshes)
    {
        if (_mesh.first == mesh)
        {
            m = _mesh.second;
            return;
        }
    }
}



void DrawCallContext::run_vert_shader()
{
    int vert_count = 0;
    for (int i = 0; i < meshes.size(); ++i)
    {
        auto mesh = meshes[i].first;
        vert_count += mesh->vert_count;
    }
    if (vert_count != gl_position_count)
    {
        gl_position_count = vert_count;
        free(this->gl_positions);
        gl_positions = static_cast<L_MATH::Vec<float, 4>*>(malloc(sizeof(Vec4) * vert_count));
        outputs = static_cast<VertexOutput*>(malloc(sizeof(VertexOutput) * vert_count));
    }
    auto job_group = JOB_SYSTEM.create_job_group(frame_cur_max_job_id);
    JOB_SYSTEM.alloc_jobs(job_group,
                          0,
                          0,
                          vert_count,
                          this,
                          run_vert_shader_execute,
                          default_complete);
    JOB_SYSTEM.submit_job_group(job_group);
    update_frame_job_id(job_group);
}


int DrawCallContext::ray_cast_scene(int msaa_index)
{
    int w, h;
    auto data = new std::tuple<DrawCallContext*, int>(this, msaa_index);
    ctx->get_screen_size(w, h);
    auto job_group = JOB_SYSTEM.create_job_group(frame_cur_max_job_id);
    JOB_SYSTEM.alloc_jobs(job_group, 0,
                          0, w * h,
                          data, ray_cast_frag_execute, ray_cast_frag_complete);
    JOB_SYSTEM.submit_job_group(job_group);
    update_frame_job_id(job_group);
    return job_group;
}

Mesh* DrawCallContext::get_mesh(int vert_index,int& mesh_index) const
{
    for (auto mesh : meshes)
    {
        mesh_index = vert_index;
        if (mesh.first->vert_count >= vert_index)
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
        }
    }
    return vert_index;
}

int DrawCallContext::run_frag_shader()
{
    this->frag_shader->begin_draw_call(this);
    int w, h;
    ctx->get_screen_size(w, h);
    auto prepare_fence = JOB_SYSTEM.create_job_group(frame_cur_max_job_id);
    JOB_SYSTEM.alloc_jobs(prepare_fence, 0,
                          0, w * h,
                          this, interpolation_frag_output_execute, default_complete);

    JOB_SYSTEM.submit_job_group(prepare_fence);

    auto clear_vert_output = JOB_SYSTEM.create_job_group(prepare_fence);
    JOB_SYSTEM.alloc_jobs(clear_vert_output, 0,
                          0, this->gl_position_count,
                          this, clear_vert_output_execute, default_complete);

    JOB_SYSTEM.submit_job_group(clear_vert_output);


    auto job_group = JOB_SYSTEM.create_job_group(clear_vert_output);
    JOB_SYSTEM.alloc_jobs(job_group, 0,
                          0, w * h,
                          this, run_frag_shader_execute, run_frag_shader_complete);

    JOB_SYSTEM.submit_job_group(job_group);
    update_frame_job_id(job_group);
    return job_group;
}


int DrawCallContext::raster_scene(int msaa_index)
{
    auto data = new std::tuple<DrawCallContext*, int>(this, msaa_index);
    auto tri_count = this->primitives.size();
    if (tri_count == 0)
    {
        return 0;
    }
    auto job_group = JOB_SYSTEM.create_job_group(frame_cur_max_job_id);
    JOB_SYSTEM.alloc_jobs(job_group, 0,
                          0, this->primitives.size(),
                          data, rast_tri_execute, rast_tri_complete);
    JOB_SYSTEM.submit_job_group(job_group);
    update_frame_job_id(job_group);
    return job_group;
}

void DrawCallContext::wait_finish() const
{
    JOB_SYSTEM.wait_job_group_finish(frame_cur_max_job_id);
}

void DrawCallContext::generate_triangle_primitive(TrianglePrimitive& tri)
{
    for (int i = 0; i < 3; ++i)
    {
        auto vert_index = get_muti_mesh_vert_index(tri.mesh, tri.vert_index[i]);
        Vec4& v = static_cast<L_MATH::Vec<float, 4>&>(tri.v[i]);
        v = gl_positions[vert_index];
    }
    tri.update_param();
}



bool DrawCallContext::is_render_job_finish(int job_group_id) const
{
    if (job_group_id < frame_begin_job_id)
    {
        return true;
    }
    if (job_group_id > frame_cur_max_job_id)
    {
        return false;
    }
    return JOB_SYSTEM.is_job_group_finish(job_group_id);
}


void DrawCallContext::update_frame_job_id(int job_group_id)
{
    if (frame_begin_job_id == 0)
    {
        frame_begin_job_id = job_group_id;
        frame_cur_max_job_id = job_group_id;
    }
    frame_cur_max_job_id = std::max(frame_cur_max_job_id, job_group_id);
}

void DrawCallContext::draw_begin()
{
    frame_cur_max_job_id = 0;
    frame_begin_job_id = 0;
}

void DrawCallContext::draw_end()
{
    wait_finish();
}

void DrawCallContext::process_primitives()
{
    std::vector<std::tuple<DrawCallContext*,Mat44>> data(meshes.size());

    int pri_size = 0;
    for (int i = 0; i < meshes.size(); ++i)
    {
        auto& mesh =  meshes[i].first;
        pri_size += mesh->tri_count();
    }
    this->assign_triangle_primitives(pri_size);
    auto job_group = JOB_SYSTEM.create_job_group(frame_cur_max_job_id);
    JOB_SYSTEM.alloc_jobs(job_group,
                          0,
                          0,
                          pri_size,
                          this,
                          run_process_primitive,
                          default_complete);
    JOB_SYSTEM.submit_job_group(job_group);
    update_frame_job_id(job_group);
    if (this->ctx->setting.build_bvh)
    {
        wait_finish();
        this->build_bvh_tree();
    }
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

    auto& tbn_light_dirs0 = draw_call_context->outputs[v[0]].tbn_light_dirs;
    auto& tbn_light_dirs1 = draw_call_context->outputs[v[1]].tbn_light_dirs;
    auto& tbn_light_dirs2 = draw_call_context->outputs[v[2]].tbn_light_dirs;

    auto& view_light_dirs0 = draw_call_context->outputs[v[0]].view_light_dirs;
    auto& view_light_dirs1 = draw_call_context->outputs[v[1]].view_light_dirs;
    auto& view_light_dirs2 = draw_call_context->outputs[v[2]].view_light_dirs;

    output.tbn_light_dirs.resize(tbn_light_dirs0.size());
    output.view_light_dirs.resize(view_light_dirs0.size());

    for (int i = 0; i < VertOutPutTypeCount; ++i)
    {
        output.fix_outputs[i] = vertex_output0[i] * alpha[0] + vertex_output1[i] * alpha[1] + vertex_output2[i] * alpha[2];
    }
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