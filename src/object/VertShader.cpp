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
    Vec3 view_pos = context->view_world_pos;
    auto fix_outputs = context->outputs[vert_index].fix_outputs;
    auto& tbn_light_dirs = context->outputs[vert_index].tbn_light_dirs;
    auto& view_light_dirs = context->outputs[vert_index].view_light_dirs;

    new(&context->outputs[vert_index].tbn_light_dirs) std::vector<Vec4>(context->ctx->light_manager->get_light_count());
    new(&context->outputs[vert_index].view_light_dirs) std::vector<Vec4>(context->ctx->light_manager->get_light_count());

    int mesh_index = 0;
    auto mesh = context->get_mesh(vert_index, mesh_index);
    Mat44 model_mat = context->get_model_matrix(mesh);
    const Mat44& view_mat = context->view_matrix;
    const Mat44& projection_mat = context->proj_matrix;
    Vec3 pos, normal;
    mesh->get_attribute_value(mesh_index, POS, pos);
    mesh->get_attribute_value(mesh_index, NORMAL, normal);

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

    Vec3& gl_pos = static_cast<L_MATH::Vec<float, 3>&>(context->gl_positions[vert_index]);
    gl_pos = pos;
    gl_pos[3] = 1.0f;
    L_MATH::pos3_l_dot_mat44(gl_pos, projection_mat);
    Vec3 tangent;
    mesh->get_attribute_value(mesh_index, TANGENT, tangent);
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
    int mesh_index = 0;
    auto mesh = context->get_mesh(vert_index, mesh_index);
    Mat44 model_mat = context->get_model_matrix(mesh);
    const Mat44& view_mat = context->view_matrix;
    const Mat44& projection_mat = context->proj_matrix;
    Vec3 pos;
    mesh->get_attribute_value(mesh_index, POS, pos);
    L_MATH::pos3_l_dot_mat44(pos, model_mat);
    L_MATH::pos3_l_dot_mat44(pos, view_mat);
    Vec3& gl_pos = static_cast<L_MATH::Vec<float, 3>&>(context->gl_positions[vert_index]);
    gl_pos = pos;
    gl_pos[3] = 1.0f;
    L_MATH::pos3_l_dot_mat44(gl_pos, projection_mat);
}
