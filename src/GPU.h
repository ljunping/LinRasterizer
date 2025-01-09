//
// Created by Lin on 2024/12/30.
//

#ifndef GPU_H
#define GPU_H
#include "DrawCallContext.h"
class DrawCallContext;
class Context;

enum GPUCommandType
{
    CLEAR_DEPTH,
    CLEAR_FRAG,
    CLEAR_FRAME_BUFF,
    DRAW,
};
struct GPUCmds
{
    std::vector<GPUCommandType> cmd_types;
    DrawCallContext context;
public:
    GPUCmds(DrawCallContext& draw_call_context,GPUCommandType cmd_type);
    GPUCmds(DrawCallContext& draw_call_context,  std::initializer_list<GPUCommandType>  cmd_types);
    GPUCmds(DrawCallContext& draw_call_context, int cmd_count, ...);
};

class GPU
{
    static void build_bvh_tree(DrawCallContext* dc);
    static int run_frag_shader(DrawCallContext* dc);
    static void run_vert_shader(DrawCallContext*);
    static void process_primitives(DrawCallContext*);
    static int ray_cast_scene(DrawCallContext* dc, int msaa_index);
    static int raster_scene(DrawCallContext* context, int msaa_index);
    static void clear_frame_buff(DrawCallContext* draw_call_context);
    static void clear_depth(DrawCallContext* draw_call_context);
    static void global_ray_trace(DrawCallContext* dc);
    static void draw(DrawCallContext* dc);
    static int clear_fragment(DrawCallContext* dc);
    static bool is_render_job_finish(int job_group_id);
    static int frame_begin_job_id;
    static int frame_cur_max_job_id;
    static void update_frame_job_id(int job_group_id);
public:
    static void begin();
    static void end();
    static void run(GPUCmds& cmd);
    static void wait_finish() ;
};



#endif //GPU_H
