//
// Created by Lin on 2024/12/30.
//

#include "GPU.h"

#include "BVHTree.h"
#include "Geometry.h"
#include "DrawCallContext.h"
#include "FragShader.h"
#include "JobSystem.h"
#include "Mesh.h"
#include "RasterizerJob.h"


int GPU::frame_begin_job_id = 0;
int GPU::frame_cur_max_job_id = 0;

void GPU::draw(DrawCallContext* dc)
{
    run_vert_shader(dc);
    process_primitives(dc);
    if (dc->setting.enable_global_path_trace)
    {
        global_ray_trace(dc);
        return;
    }
    for (int j = 0; j < dc->setting.msaa_factor; ++j)
    {
        if (!dc->setting.enable_ray_cast)
        {
            raster_scene(dc, j);
        }
        else
        {
            ray_cast_scene(dc, j);
        }
    }
    if (dc->setting.run_fragment)
    {
        run_frag_shader(dc);
    }
}



GPUCmds::GPUCmds(DrawCallContext& draw_call_context, GPUCommandType cmd_type)
{
    this->cmd_types.push_back(cmd_type);
    this->context = draw_call_context;
}

GPUCmds::GPUCmds(DrawCallContext& draw_call_context, std::initializer_list<GPUCommandType> cmd_types)
{
    this->context = draw_call_context;
    for (auto cmd_type : cmd_types)
    {
        this->cmd_types.push_back(cmd_type);
    }
}

GPUCmds::GPUCmds(DrawCallContext& draw_call_context, int cmd_count,...)
{
    this->context = draw_call_context;
    va_list ap;
    va_start(ap, cmd_count);
    for (int i = 0; i < cmd_count; ++i)
    {
        this->cmd_types.push_back(va_arg(ap, GPUCommandType));
    }
    va_end(ap);
}


void GPU::build_bvh_tree(DrawCallContext* dc)
{
    dc->bvh_tree = new BVHTree();
    dc->bvh_tree->build(dc->geometries);
}

int GPU::run_frag_shader(DrawCallContext* dc)
{
    dc->frag_shader->begin_draw_call(dc);
    auto prepare_fence = JOB_SYSTEM.create_job_group(frame_cur_max_job_id);
    JOB_SYSTEM.alloc_jobs(prepare_fence, 0,
                          0, dc->w * dc->h,
                          dc, interpolation_frag_output_execute, default_complete);

    JOB_SYSTEM.submit_job_group(prepare_fence);
    int pre_fence = prepare_fence;
    if (dc->setting.enable_light_interpolation)
    {
        auto clear_vert_output = JOB_SYSTEM.create_job_group(prepare_fence);
        JOB_SYSTEM.alloc_jobs(clear_vert_output, 0,
                              0, dc->gl_position_count,
                              dc, clear_vert_output_execute, default_complete);

        JOB_SYSTEM.submit_job_group(clear_vert_output);
        pre_fence = clear_vert_output;
    }

    auto job_group = JOB_SYSTEM.create_job_group(pre_fence);
    JOB_SYSTEM.alloc_jobs(job_group, 0,
                          0, dc->w * dc->h,
                          dc, run_frag_shader_execute, run_frag_shader_complete);

    JOB_SYSTEM.submit_job_group(job_group);
    update_frame_job_id(job_group);
    return job_group;
}



void GPU::wait_finish()
{
    JOB_SYSTEM.wait_job_group_finish(frame_cur_max_job_id);
}



bool GPU::is_render_job_finish(int job_group_id)
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


void GPU::update_frame_job_id(int job_group_id)
{
    if (frame_begin_job_id == 0)
    {
        frame_begin_job_id = job_group_id;
        frame_cur_max_job_id = job_group_id;
    }
    frame_cur_max_job_id = std::max(frame_cur_max_job_id, job_group_id);
}



void GPU::begin()
{
    frame_cur_max_job_id = 0;
    frame_begin_job_id = 0;
}

void GPU::end()
{
    wait_finish();
}


void GPU::process_primitives(DrawCallContext* dc)
{
    std::vector<std::tuple<DrawCallContext*,Mat44>> data(dc->meshes.size());
    int mesh_tri_count = 0;
    for (int i = 0; i < dc->meshes.size(); ++i)
    {
        auto& mesh = dc->meshes[i];
        mesh_tri_count += mesh->tri_count();
    }
    int geometry_index = mesh_tri_count;
    std::vector<int> render_nodes[GeometryCount];
    int geometry_count[GeometryCount]{};
    int _geometry_sum = mesh_tri_count;
    for (int i = 0; i < dc->nodes.size(); ++i)
    {
        auto& node = dc->nodes[i];
        if (node.local_geometry)
        {
            render_nodes[node.local_geometry->geometry_type].emplace_back(i);
            geometry_count[node.local_geometry->geometry_type]++;
            _geometry_sum++;
        }
    }
    dc->geometry_2_node.resize(_geometry_sum);
    dc->geometries.resize(_geometry_sum);
    for (int i = 0; i < GeometryCount; ++i)
    {
        int offset = i == TRI ? mesh_tri_count : 0;
        auto _GeometryType = static_cast<GeometryType>(i);
        dc->assign_geometry_primitives(static_cast<GeometryType>(i), geometry_count[i] + offset);
        for (int j = offset; j < geometry_count[i] + offset; ++j)
        {
            auto render_node_indx = render_nodes[i][j - offset];
            auto& render_node = dc->nodes[render_node_indx];
            auto _Geometry = reinterpret_cast<Geometry*>(
                reinterpret_cast<char*>(dc->geometrie_pools[i]) +
                j * geometry_size(_GeometryType));
            build_geometry(_GeometryType, _Geometry);
            render_node.local_geometry->clone(_Geometry);
            _Geometry->id = geometry_index;
            dc->geometry_2_node[_Geometry->id] = &render_node;
            dc->geometries[geometry_index] = _Geometry;
            _Geometry->transform(render_node.model_matrix);
            render_node.transform_geometry = _Geometry;
            geometry_index++;
        }
    }

    auto job_group = JOB_SYSTEM.create_job_group(frame_cur_max_job_id);
    JOB_SYSTEM.alloc_jobs(job_group,
                          0,
                          0,
                          mesh_tri_count,
                          dc,
                          run_process_tri_primitive,
                          default_complete);
    JOB_SYSTEM.submit_job_group(job_group);
    update_frame_job_id(job_group);
    if (dc->setting.build_bvh && dc->bvh_tree == nullptr)
    {
        wait_finish();
        build_bvh_tree(dc);
    }
}


void GPU::run_vert_shader(DrawCallContext* dc)
{
    int vert_count = 0;
    for (int i = 0; i < dc->meshes.size(); ++i)
    {
        auto mesh = dc->meshes[i];
        vert_count += mesh->vert_count;
    }
    if (vert_count != dc->gl_position_count)
    {
        dc->gl_position_count = vert_count;
        free(dc->gl_positions);
        dc->gl_positions = static_cast<L_MATH::Vec<float, 4>*>(malloc(sizeof(Vec4) * vert_count));
        dc->outputs = static_cast<VertexOutput*>(malloc(sizeof(VertexOutput) * vert_count));
    }
    auto job_group = JOB_SYSTEM.create_job_group(frame_cur_max_job_id);
    JOB_SYSTEM.alloc_jobs(job_group,
                          0,
                          0,
                          vert_count,
                          dc,
                          run_vert_shader_execute,
                          default_complete);
    JOB_SYSTEM.submit_job_group(job_group);
    update_frame_job_id(job_group);
}


int GPU::ray_cast_scene(DrawCallContext* dc,int msaa_index)
{
    auto data = new std::tuple<DrawCallContext*, int>(dc, msaa_index);
    auto job_group = JOB_SYSTEM.create_job_group(frame_cur_max_job_id);
    JOB_SYSTEM.alloc_jobs(job_group, 0,
                          0, dc->w * dc->h,
                          data, ray_cast_frag_execute, ray_cast_frag_complete);
    JOB_SYSTEM.submit_job_group(job_group);
    update_frame_job_id(job_group);
    return job_group;
}

int GPU::raster_scene(DrawCallContext* dc,int msaa_index)
{
    auto data = new std::tuple<DrawCallContext*, int>(dc, msaa_index);
    auto tri_count = dc->geometries.size();
    if (tri_count == 0)
    {
        return 0;
    }
    auto job_group = JOB_SYSTEM.create_job_group(frame_cur_max_job_id);
    JOB_SYSTEM.alloc_jobs(job_group, 0,
                          0, dc->geometries.size(),
                          data, rast_tri_execute, rast_tri_complete);
    JOB_SYSTEM.submit_job_group(job_group);
    update_frame_job_id(job_group);
    return job_group;
}

void GPU::global_ray_trace(DrawCallContext* dc)
{
    global_count = 0;
    auto job_group = JOB_SYSTEM.create_job_group(frame_cur_max_job_id);

    JOB_SYSTEM.alloc_jobs(job_group, 0,
                          0, dc->w * dc->h, dc
                          , global_path_ray_cast_execute, default_complete);
    JOB_SYSTEM.submit_job_group(job_group);
    auto mid_filter_job = JOB_SYSTEM.create_job_group(job_group);
    JOB_SYSTEM.alloc_jobs(mid_filter_job, 0,
                          0, dc->w * dc->h, dc
                          , mid_filter_execute, default_complete);
    JOB_SYSTEM.submit_job_group(mid_filter_job);
    update_frame_job_id(mid_filter_job);
}

int GPU::clear_fragment(DrawCallContext* dc)
{
    auto job_group = JOB_SYSTEM.create_job_group(frame_cur_max_job_id);

    JOB_SYSTEM.alloc_jobs(job_group, 0,
                          0, dc->w * dc->h, dc
                          , clear_fragment_execute, default_complete);
    JOB_SYSTEM.submit_job_group(job_group);
    update_frame_job_id(job_group);
    return job_group;
}

void GPU::clear_frame_buff(DrawCallContext* draw_call_context)
{
    auto job_group = JOB_SYSTEM.create_job_group(frame_cur_max_job_id);
    JOB_SYSTEM.alloc_jobs(job_group, 0,
                          0, draw_call_context->w * draw_call_context->h,
                          draw_call_context, clear_frame_buff_execute, default_complete);

    JOB_SYSTEM.submit_job_group(job_group);
    update_frame_job_id(job_group);
}

void GPU::clear_depth(DrawCallContext* draw_call_context)
{
    auto job_group = JOB_SYSTEM.create_job_group(frame_cur_max_job_id);
    JOB_SYSTEM.alloc_jobs(job_group, 0,
                          0, draw_call_context->w * draw_call_context->h,
                          draw_call_context, clear_depth_execute, default_complete);

    JOB_SYSTEM.submit_job_group(job_group);
    update_frame_job_id(job_group);
}



void GPU::run(GPUCmds& cmd)
{
    for (auto cmd_type : cmd.cmd_types)
    {
        if (cmd_type==DRAW)
        {
            draw(&cmd.context);
        }
        else if (cmd_type == CLEAR_FRAME_BUFF)
        {
            clear_frame_buff(&cmd.context);
        }
        else if (cmd_type == CLEAR_DEPTH)
        {
            clear_depth(&cmd.context);
        }
        else if (cmd_type == CLEAR_FRAG)
        {
            clear_fragment(&cmd.context);
        }
    }

}
