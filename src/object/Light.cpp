//
// Created by Lin on 2024/12/23.
//

#include "Light.h"

#include "Camera.h"
#include "Context.h"
#include "GPU.h"
#include "ImageUtil.h"
#include "Mesh.h"
#include "MeshGenerator.h"
#include "MeshRender.h"
#include "Transform.h"
#include "VertShader.h"


float Light::calculate_shadow(Camera* camera, const L_MATH::Vec<float, 3>& world_pos)
{
    return 0.0f;
}

void Light::on_camera_remove(Camera*)
{
}

void Light::on_create()
{
    Component::on_create();
    get_current_ctx()->light_manager->on_create_obj(this);
}

void Light::on_delete()
{
    get_current_ctx()->light_manager->on_delete_obj(this);
    Component::on_delete();
}

bool Light::intersect(const L_MATH::Vec<float, 3>& pos, const L_MATH::Vec<float, 3>& dir)
{
    return false;
}

Vec3 Light::calculate_radiance(L_MATH::Vec<float, 3> pos)
{
    return c_light;
}

void Light::generate_render_node()
{
}


bool SpotLight::need_update() const
{
    return true;
}

static float _time = 0;
void SpotLight::update(float delta_time)
{
    if (pause_update)
    {
        return;
    }
    static int id = 0;
    _time += delta_time;
    if (!save_images)
    {
        for (auto& light_draw_info : light_draw_infos)
        {
            auto& light_draw_call_info = light_draw_info.second;
            Image image{
                reinterpret_cast<unsigned char*>(light_draw_call_info.shadow_map),
                light_draw_call_info.shadow_map_width,
                light_draw_call_info.shadow_map_height,
                4
            };
            save_images = true;
            ImageUtil::save_image(("shadow_map" + std::to_string(this->get_instance_id()) + ".jpg").c_str(), image);
        }
    }
    return;
    Vec3 look_at = {0, 0, -2};
    this->scene_node->local_pos[0] = 2*cos(_time * PI / 4) * 1;
    this->scene_node->local_pos[2] = -2*(sin(_time * PI / 4) * 1) - 2;
    auto look_dir = (look_at - this->scene_node->local_pos).normalize();
    auto rotate = L_MATH::rotate(L_MATH::FORWARD * -1, look_dir);
    Vec3 r, t, s;
    L_MATH::decompose_trs(rotate, t, r, s);
    this->scene_node->local_euler_angles = r * 180 / PI;
    global_mat = this->scene_node->get_local_to_global_mat();
}

Vec3 SpotLight::calculate_radiance(L_MATH::Vec<float, 3> pos)
{
    Vec3& light_pos = static_cast<L_MATH::Vec<float, 3>&>(global_mat[3]);
    Vec3& light_dir = static_cast<L_MATH::Vec<float, 3>&>(global_mat[2]);
    auto dir = pos - light_pos;
    auto sqrt_magnitude = dir.sqrt_magnitude();
    dir /= sqrt_magnitude;
    if (sqrt_magnitude > max_distance)
    {
        return Vec3::ZERO;
    }
    if (-L_MATH::dot(light_dir, dir) < cos_angle)
    {
        return Vec3::ZERO;
    }
    return c_light / (sqrt_magnitude * sqrt_magnitude);
}

bool SpotLight::intersect(const L_MATH::Vec<float, 3>& pos, const L_MATH::Vec<float, 3>& dir)
{
    Vec3& light_pos = static_cast<L_MATH::Vec<float, 3>&>(global_mat[3]);
    auto len = (pos - light_pos).sqrt_magnitude();
    auto light_dir = (light_pos - pos) / len;
    auto cos_v = dot(light_dir, dir);
    auto sin_v = radius / len;
    return cos_v * cos_v > (1 - sin_v * sin_v);
}

SpotLight::~SpotLight()
{
    for (auto& light_draw_info : light_draw_infos)
    {
        free(light_draw_info.second.fragment_map);
        free(light_draw_info.second.depth_buff);
        free(light_draw_info.second.shadow_map);
    }
}

void SpotLight::generate_render_node()
{
    if (_generate_render_node)
    {
        return;
    }
    _generate_render_node = true;
    if (get_current_ctx()->setting.enable_global_path_trace)
    {
        auto sphere = std::make_shared<Quad>();
        sphere->p0 = {radius, -radius, 0};
        sphere->p1 = {-radius, -radius, 0};
        sphere->p2 = {radius, radius, 0};
        c_light[3] = 1;
        global_ray_trace_vert_shader = Resource::get_or_create_default_resource<GlobalRayTraceVertShader>();
        diffuse_light_material = Resource::get_or_create_default_resource<DiffuseLightMaterial>(1);
        pure_texture = Resource::create_unnamed_resource<PureColorTexture>(static_cast<Vec4&>(c_light));
        auto geometry_render = add_geometry_render(this->scene_node, sphere, diffuse_light_material, pure_texture, 1);
        geometry_render->emit = true;
    }
    else
    {
        mesh = generate_sphere(0.1, 10, 10);
        auto mesh_provider = this->scene_node->add_component<MeshProvider>();
        mesh_provider->mesh = (mesh);
        c_light[3] = 1;
        frag_shader = Resource::get_or_create_default_resource<FragShader>();
        pure_texture = Resource::create_unnamed_resource<PureColorTexture>(static_cast<Vec4&>(c_light));
        vert_shader = Resource::get_or_create_default_resource<VertShader>();
        material = Resource::create_unnamed_resource<Material>();
        material->set_Vec4_uniform(MATERIAL_COLOR1, static_cast<L_MATH::Vec<float, 4>&>(c_light));
        add_mesh_render(this->scene_node, vert_shader, frag_shader, material, pure_texture, 1, false);
    }
}


float SpotLight::calculate_shadow(Camera* camera, const L_MATH::Vec<float, 3>& world_pos)
{
    auto pair = light_draw_infos.find(camera);
    if (pair != light_draw_infos.end())
    {
        auto& light_draw_call_info = pair->second;
        auto proj_pos = L_MATH::pos3_dot_mat44(world_pos, light_draw_call_info.pv);
        Vec2& uv = static_cast<L_MATH::Vec<float, 2>&>(proj_pos);
        uv /= 2;
        uv += 0.5;
        int uv_w = std::lround(uv[0] * (light_draw_call_info.shadow_map_width - 1));
        int uv_h = std::lround(uv[1] * (light_draw_call_info.shadow_map_height - 1));
        if (uv_w >= light_draw_call_info.shadow_map_width || uv_w < 0)
        {
            return 0.0f;
        }
        if (uv_h >= light_draw_call_info.shadow_map_height || uv_h < 0)
        {
            return 0.0f;
        }
        auto index = uv_h * light_draw_call_info.shadow_map_width + uv_w;
        auto fragment = light_draw_call_info.fragment_map[index];
        auto depth = light_draw_call_info.depth_buff[index];
        if (fragment.triangle == nullptr)
        {
            return 0.0f;
        }
        auto bias =  this->const_bias;
        if (depth < proj_pos[2] - bias)
        {
            return 1.0f;
        }
        return 0.0f;
    }
    return 0.0f;
}

void SpotLight::on_camera_remove(Camera* camera)
{
    auto pair = light_draw_infos.find(camera);
    if(pair!=light_draw_infos.end())
    {
        free(pair->second.fragment_map);
        free(pair->second.depth_buff);
        free(pair->second.shadow_map);
    }
}


void SpotLight::update_light_shadow_params(Camera* camera, DrawCallContext& call_context)
{
    auto ctx = get_current_ctx();
    if (!ctx->setting.enable_shadow)
    {
        return;
    }
    int w, h;
    ctx->get_screen_size(w, h);
    auto& light_draw_info = light_draw_infos[camera];
    light_draw_info.light = this;
    if (light_draw_info.shadow_map_height != h || light_draw_info.shadow_map_width != w ||
        light_draw_info.shadow_map == nullptr)
    {
        light_draw_info.shadow_map_height = h;
        light_draw_info.shadow_map_width = w;
        if (light_draw_info.shadow_map != nullptr)
        {
            free(light_draw_info.fragment_map);
            free(light_draw_info.depth_buff);
            free(light_draw_info.shadow_map);
        }
        light_draw_info.shadow_map = static_cast<Color*>(malloc(
            sizeof(Color) * w * h));
        light_draw_info.fragment_map = static_cast<Fragment*>(malloc(
            sizeof(Fragment) * w * h));
        light_draw_info.depth_buff = static_cast<float*>(malloc(
            sizeof(float) * w * h));
    }
    auto shadow_view_frustum = camera->get_shadow_view_frustum(ctx);
    light_draw_info.view = L_MATH::invert_trs_mat(this->scene_node->get_local_to_global_mat());
    float near = -INFINITY, far = INFINITY, max_cs_w_fov = 0, max_cs_h_fov = 0;
    float min_near = 0.001;
    Frustum frustum(min_near, max_distance, angle * 2, 1);
    std::vector<RenderNode> render_nodes, select_render_nodes;
    ctx->render_node_manager->collection_render_node(camera, render_nodes);
    float tan_angle = tan(L_MATH::deg2rad(this->angle));
    for (auto& render_node : render_nodes)
    {
        if (!render_node.shadow_caster)
        {
            continue;
        }
        auto& mesh = render_node.mesh;
        if (mesh==nullptr)
        {
            continue;
        }
        auto vm = light_draw_info.view * render_node.model_matrix;
        auto box = mesh->get_box();
        if (!frustum.box_in_frustum(box, vm))
        {
            continue;
        }
        float _near = -INFINITY, _far = INFINITY, _max_cs_w_fov = 0, _max_cs_h_fov = 0;
        auto box_center = mesh->get_box().center();
        L_MATH::pos3_l_dot_mat44(box_center, vm);
        auto box_local_vm = L_MATH::rotate(box_center, L_MATH::FORWARD * -1) * vm;
        auto _camera_vm = camera->get_view_mat() * render_node.model_matrix * L_MATH::inverse(box_local_vm);
        auto vecs = mesh->get_box().get_vertices();
        for (auto& world_pos : vecs)
        {
            L_MATH::pos3_l_dot_mat44(world_pos, box_local_vm);
            if (world_pos[2] > 0)
            {
                continue;
            }
            auto cs_w = abs(world_pos[0] / world_pos[2]);
            auto cs_h = abs(world_pos[1] / world_pos[2]);
            _max_cs_w_fov = std::max(_max_cs_w_fov, cs_w);
            _max_cs_h_fov = std::max(_max_cs_h_fov, cs_h);
            _near = std::max(_near, world_pos[2]);
        }
        float fov = 2 * std::atan(_max_cs_h_fov) * 180 / M_PI;
        float aspect = _max_cs_w_fov / _max_cs_h_fov;
        Frustum _frustum(-_near, -max_distance, fov, aspect);
        if (!shadow_view_frustum.
            frustum_in_frustum(_frustum, _camera_vm))
        {
            continue;
        }
        select_render_nodes.emplace_back(render_node);
    }
    if (select_render_nodes.empty())
    {
        return;
    }
    for (auto& render_node : select_render_nodes)
    {
        call_context.add_render_node(render_node);
        auto& mesh = render_node.mesh;
        auto& model_mat = render_node.model_matrix;
        auto vm = light_draw_info.view * model_mat;
        for (auto& world_pos : mesh->get_box().get_vertices())
        {
            L_MATH::pos3_l_dot_mat44(world_pos, vm);
            if (world_pos[2] > 0)
            {
                continue;
            }
            auto cs_w = abs(world_pos[0] / world_pos[2]);
            auto cs_h = abs(world_pos[1] / world_pos[2]);
            max_cs_w_fov = std::max(max_cs_w_fov, cs_w);
            max_cs_h_fov = std::max(max_cs_h_fov, cs_h);
            near = std::max(near, world_pos[2]);
            far = std::min(far, world_pos[2]);

            max_cs_w_fov = std::min(max_cs_w_fov, tan_angle);
            max_cs_h_fov = std::min(max_cs_h_fov, tan_angle);
            near = std::min(near, -min_near);
            far = std::max(far, -max_distance);
        }
    }
    near *= 0.9;
    far *= 1.1;
    far = std::max(far, -max_distance);
    float fov = 2 * std::atan(max_cs_h_fov) * 180 / M_PI;
    float aspect = max_cs_w_fov / max_cs_h_fov;
    light_draw_info.frustum = Frustum(-near, -far, fov, aspect);
    light_draw_info.proj = light_draw_info.frustum.get_proj_mat();
    light_draw_info.pv = light_draw_info.proj * light_draw_info.view;
}


void SpotLight::collect_draw_call_cmds(Camera* camera,std::vector<GPUCmds>& d_cmds)
{
    auto ctx = get_current_ctx();
    if (!ctx->setting.enable_shadow)
    {
        return;
    }
    light_shadow_frag_shader = Resource::get_or_create_default_resource<LightShadowMapFragShader>();
    light_shadow_map_vert_shader = Resource::get_or_create_default_resource<LightShadowMapVertShader>();
    DrawCallContext draw_call_context{};
    update_light_shadow_params(camera, draw_call_context);

    global_mat = this->scene_node->get_local_to_global_mat();
    cos_angle = cos(L_MATH::deg2rad(angle));

    if (draw_call_context.meshes.empty())
    {
        return;
    }
    auto& light_draw_info = light_draw_infos[camera];
    draw_call_context.setting = ctx->setting;
    draw_call_context.setting.msaa_factor = 1;
    draw_call_context.setting.run_fragment=true;
    draw_call_context.frame_buff = light_draw_info.shadow_map;
    draw_call_context.fragment_map = light_draw_info.fragment_map;
    draw_call_context.depth_buff = light_draw_info.depth_buff;
    draw_call_context.frag_shader = light_shadow_frag_shader;
    draw_call_context.vert_shader = light_shadow_map_vert_shader;
    draw_call_context.view_matrix = light_draw_info.view;
    draw_call_context.proj_matrix = light_draw_info.proj;
    draw_call_context.setting.background_color = BLACK;
    draw_call_context.setting.enable_light_interpolation = false;
    draw_call_context.view_world_pos = this->scene_node->get_global_pos();
    draw_call_context.host = this;
    draw_call_context.setting.enable_global_path_trace = false;
    d_cmds.emplace_back(draw_call_context,4,CLEAR_DEPTH,CLEAR_FRAG,CLEAR_FRAME_BUFF,DRAW);
}

void PointLight::collect_draw_call_cmds(Camera* camera,std::vector<GPUCmds>& d_cmds)
{

}


void DirectionalLight::collect_draw_call_cmds(Camera* camera,std::vector<GPUCmds>& d_cmds)
{
    Light::collect_draw_call_cmds(d_cmds);
}

int LightManager::get_light_count()
{
    return this->get_objects().valid_size();
}


