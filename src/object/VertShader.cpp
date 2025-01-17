//
// Created by Lin on 2024/12/26.
//

#include "VertShader.h"
#include "Camera.h"
#include "DrawCallContext.h"
#include "Light.h"
#include "Mesh.h"
#include "Transform.h"


void VertShader::run(DrawCallContext* context,int vert_index)
{
    Vec3& view_pos = context->view_world_pos;
    auto& gl_vert = context->gl_verts[vert_index];
    auto& vertex_output = gl_vert.vertex_interpolation.output;
    auto render_node = context->get_render_node(vert_index);
    auto& fix_outputs = vertex_output.fix_outputs;
    auto& tbn_light_dirs = vertex_output.tbn_light_dirs;
    auto& view_light_dirs = vertex_output.view_light_dirs;
    const Mat44& model_mat = render_node->model_matrix;
    const Mat44& view_mat = context->view_matrix;
    const Mat44& projection_mat = context->proj_matrix;
    Vec3 pos, normal;
    context->get_vert_attribute_value(vert_index, POS, pos);
    context->get_vert_attribute_value(vert_index, NORMAL, normal);

    L_MATH::pos3_l_dot_mat44(pos, model_mat);
    fix_outputs[WORLD_POS] = (static_cast<L_MATH::Vec<float, 4>&>(pos));
    Vec3 view_dir = view_pos - pos;
    L_MATH::pos3_l_dot_mat33(normal, model_mat);
    L_MATH::pos3_l_dot_mat44(pos, view_mat);
    L_MATH::pos3_l_dot_mat33(normal, view_mat);
    L_MATH::pos3_l_dot_mat33(view_dir, view_mat);
    fix_outputs[VIEW_POS] = (static_cast<L_MATH::Vec<float, 4>&>(pos));
    fix_outputs[VIEW_NORMAL] = (static_cast<L_MATH::Vec<float, 4>&>(normal));
    fix_outputs[VIEW_VIEW_DIR] = (static_cast<L_MATH::Vec<float, 4>&>(view_dir));
    Vec3& gl_pos = static_cast<L_MATH::Vec<float, 3>&>(gl_vert.gl_position);
    gl_pos = pos;
    gl_pos[3] = 1.0f;
    if (!context->setting.enable_ray_cast)
    {
        L_MATH::pos3_l_dot_mat44(gl_pos, projection_mat);
    }
    Vec3 tangent;
    context->get_vert_attribute_value(vert_index, TANGENT, tangent);
    L_MATH::pos3_l_dot_mat33(tangent, model_mat);
    L_MATH::pos3_l_dot_mat33(tangent, view_mat);
    fix_outputs[TBN_TANGENT] = (static_cast<L_MATH::Vec<float, 4>&>(tangent));
    Vec3 bit_tangent = cross(normal, tangent);
    view_dir = {dot(tangent, view_dir), dot(bit_tangent, view_dir), dot(normal, view_dir)};
    fix_outputs[TBN_VIEW_DIR] = (static_cast<L_MATH::Vec<float, 4>&>(view_dir));
    int light_index = 0;

    for (auto light : context->ctx->light_manager->get_objects())
    {
        auto light_pos = light->scene_node->get_global_pos();
        L_MATH::pos3_l_dot_mat44(light_pos, view_mat);
        auto light_dir = light_pos - pos;
        view_light_dirs[light_index] = (static_cast<L_MATH::Vec<float, 4>&>(light_dir));
        light_dir = {dot(tangent, light_dir), dot(bit_tangent, light_dir), dot(normal, light_dir)};
        tbn_light_dirs[light_index] = ((static_cast<L_MATH::Vec<float, 4>&>(light_dir)));
        ++light_index;
    }
}

void LightShadowMapVertShader::run(DrawCallContext* context, int vert_index)
{
    auto& gl_vert = context->gl_verts[vert_index];
    auto& vertex_output = gl_vert.vertex_interpolation.output;
    auto render_node = context->get_render_node(vert_index);
    Mat44 model_mat = render_node->model_matrix;
    const Mat44& view_mat = context->view_matrix;
    const Mat44& projection_mat = context->proj_matrix;
    Vec3 pos;
    context->get_vert_attribute_value(vert_index, POS, pos);
    L_MATH::pos3_l_dot_mat44(pos, model_mat);
    auto fix_outputs = vertex_output.fix_outputs;
    fix_outputs[WORLD_POS] = (static_cast<L_MATH::Vec<float, 4>&>(pos));
    L_MATH::pos3_l_dot_mat44(pos, view_mat);
    fix_outputs[VIEW_POS] = (static_cast<L_MATH::Vec<float, 4>&>(pos));
    Vec3& gl_pos = static_cast<L_MATH::Vec<float, 3>&>(gl_vert.gl_position);
    gl_pos = pos;
    gl_pos[3] = 1.0f;
    if (!context->setting.enable_ray_cast)
    {
        L_MATH::pos3_l_dot_mat44(gl_pos, projection_mat);
    }
}

void GlobalRayTraceVertShader::run(DrawCallContext* context, int vert_index)
{
    auto& gl_vert = context->gl_verts[vert_index];
    auto& vertex_output = gl_vert.vertex_interpolation.output;
    auto render_node = context->get_render_node(vert_index);
    const Mat44& model_mat = render_node->model_matrix;
    Vec3 pos, normal;
    context->get_vert_attribute_value(vert_index, POS, pos);
    context->get_vert_attribute_value(vert_index, NORMAL, normal);
    L_MATH::pos3_l_dot_mat44(pos, model_mat);
    L_MATH::pos3_l_dot_mat33(normal, model_mat);
    Vec3& gl_pos = static_cast<L_MATH::Vec<float, 3>&>(gl_vert.gl_position);
    gl_pos = pos;
    gl_pos[3] = 1.0f;
    auto fix_outputs = vertex_output.fix_outputs;
    fix_outputs[WORLD_NORMAL] = (static_cast<L_MATH::Vec<float, 4>&>(normal));
    fix_outputs[WORLD_POS] = (static_cast<L_MATH::Vec<float, 4>&>(pos));
}
