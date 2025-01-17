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
                              0, dc->gl_vert_count,
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

void GPU::prepare_ctx(DrawCallContext* dc)
{
    int vert_count = dc->get_mesh_vert_count();
    int mesh_tri_count = dc->get_mesh_tri_count();
    if (dc->setting.enable_ray_cast || dc->setting.enable_global_path_trace)
    {
        dc->raycaster_process_geometry(mesh_tri_count);
    }
    else
    {
        for (const auto& mesh : dc->meshes)
        {
            vert_count += mesh->tri_count() * CLIP_NEAR_VERTEX_MAX_COUNT;
        }
        mesh_tri_count *= CLIP_NEAR_TRI_MAX_COUNT;
        dc->geometry_2_node.resize(mesh_tri_count);
        dc->geometries.resize(mesh_tri_count);
        dc->assign_geometry_primitives(TRI, mesh_tri_count);
    }
    dc->assign_vert(vert_count);
    auto job_group = JOB_SYSTEM.create_job_group(frame_cur_max_job_id);
    JOB_SYSTEM.alloc_jobs(job_group, 0,
                          0, std::max(vert_count, mesh_tri_count),
                          dc, prepare_ctx_execute, default_complete);
    JOB_SYSTEM.submit_job_group(job_group);
    update_frame_job_id(job_group);
}


void GPU::process_primitives(DrawCallContext* dc)
{
    int mesh_tri_count = dc->get_mesh_tri_count();
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
    if (dc->setting.enable_global_path_trace || (dc->setting.build_bvh && dc->bvh_tree == nullptr))
    {
        wait_finish();
        build_bvh_tree(dc);
    }
}


void GPU::run_vert_shader(DrawCallContext* dc)
{
    int vert_count = dc->get_mesh_vert_count();
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
    auto tri_count = dc->geometry_count[TRI];
    if (tri_count == 0)
    {
        return 0;
    }
    auto job_group = JOB_SYSTEM.create_job_group(frame_cur_max_job_id);
    JOB_SYSTEM.alloc_jobs(job_group, 0,
                          0, tri_count,
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
    prepare_ctx(&cmd.context);
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
