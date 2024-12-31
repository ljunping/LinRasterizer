//
// Created by Lin on 2024/12/23.
//

#include "Light.h"

#include "Camera.h"
#include "Context.h"
#include "MeshRender.h"
#include "Transform.h"



float Light::calculate_shadow(L_MATH::Vec<float, 3>& world_pos)
{
    return 0.0f;
}

void Light::on_create()
{
    get_current_ctx()->light_manager->on_create_obj(this);
}

void Light::on_delete()
{
    get_current_ctx()->light_manager->on_delete_obj(this);
}

void SpotLight::collect_draw_call_cmds(std::vector<GPUCmds>& d_cmds)
{
    Light::collect_draw_call_cmds(d_cmds);
}

void PointLight::collect_draw_call_cmds(std::vector<GPUCmds>& d_cmds)
{
    Light::collect_draw_call_cmds(d_cmds);
}

// L_MATH::Mat<float, 4, 4> SpotLight::calculate_spot_light_projection_matrix(Context* ctx,Camera* camera) const
// {
    // auto light_pos = this->scene_node->get_global_pos();
    // Vec3 lookAt_pos;
    // float fov = 0.0;
    // auto view_mat = camera->get_view_mat();
    // auto shadow_clip = [&view_mat](Mesh* mesh,Mat44& model)
    // {
    //     auto mesh_centroid = mesh->get_mesh_centroid();
    //     L_MATH::pos3_l_dot_mat44(mesh_centroid, model);
    //     L_MATH::pos3_l_dot_mat33(mesh_centroid, view_mat);
    //     view_mat[2] >
    // };
    // for (auto render_component : ctx->render_node_manager->get_objects())
    // {
    //     auto mesh = render_component->get_mesh();
    //     if (mesh)
    //     {
    //     }
    //     auto& model_mat = render_component.second;
    //     auto center = mesh.first->get_box().center();
    //     L_MATH::pos3_l_dot_mat44(center, model_mat);
    //     lookAt_pos += center
    // }
    // for (auto mesh : meshes)
    // {
    //     ;
    // }
    // lookAt_pos /= meshes.size();
    // auto light_view_mat = L_MATH::rotate(L_MATH::FORWARD, light_pos - lookAt_pos).transpose();
    //
    // float mw = 0, mh = 0, near, far;
    // for (auto mesh : meshes)
    // {
    //     auto box = mesh.first->get_box();
    //     auto center = mesh.first->get_box().center();
    //     auto xyz = box.max - box.center();
    //     for (int i = 0; i < 8; ++i)
    //     {
    //         int _x = 2 * (i & 1) - 1;
    //         int _y = 2 * ((i >> 1) & 1) - 1;
    //         int _z = 2 * ((i >> 2) & 1) - 1;
    //         Vec3 pos = center + Vec3{xyz[0] * _x, xyz[1] * _y, xyz[2] * _z};
    //         L_MATH::pos3_l_dot_mat44(pos, light_view_mat);
    //         if (abs(pos[0]) > mw)
    //         {
    //             mw = abs(pos[0]);
    //         }
    //         if (abs(pos[1]) > mh)
    //         {
    //             mh = abs(pos[1]);
    //         }
    //     }
    //     mw *= 1.2;
    //     mh *= 1.2;
    // }
// }

void DirectionalLight::collect_draw_call_cmds(std::vector<GPUCmds>& d_cmds)
{
    Light::collect_draw_call_cmds(d_cmds);
}

int LightManager::get_light_count()
{
    return this->get_objects().valid_size();
}


