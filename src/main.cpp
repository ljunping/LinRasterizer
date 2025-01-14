#include <span>

#include "L_math.h"
#include "Camera.h"
#include "WindowHandle.h"
#include "FragShader.h"
#include "Light.h"
#include "Material.h"
#include "Mesh.h"
#include "MeshGenerator.h"
#include "MeshRender.h"
#include "RasterizerJob.h"
#include "Transform.h"
#include "Texture.h"
#include "VertShader.h"

using namespace L_MATH;
static int layer = 1;
int next_layer()
{
    return 1 << (layer++);
}

Transform* outer_generate_cube()
{
    auto cube = CREATE_OBJECT_BY_TYPE(Transform);
    Vec3 local_euler_angles_s[6] = {
        {90, 0, 0}, {0, 0, 0}, {-90, 0, 0}, {180, 0, 0}, {0, -90, 0}, {0, 90, 0}
    };
    Vec3 local_pos_s[6] = {
        {0, -0.5, 0},Vec3{0, 0, 0.5},Vec3{0, 0.5, 0},Vec3{0, 0, -0.5},Vec3{-0.5, 0, 0},Vec3{0.5, 0, 0}
    };
    for (int i = 0; i < 6; ++i)
    {
        auto quad_node = create_mesh_provider(generate_quad());
        quad_node->local_euler_angles = local_euler_angles_s[i];
        quad_node->local_pos = local_pos_s[i];
        quad_node->local_scale = {1, 1, 1};
        cube->add_child(quad_node);
    }
    return cube;
}

Transform* inner_generate_cube()
{
    auto cube = CREATE_OBJECT_BY_TYPE(Transform);
    Vec3 local_euler_angles_s[6] = {
        {-90, 0, 0}, {180, 0, 0}, {90, 0, 0}, {0, 0, 0}, {0, 90, 0}, {0, -90, 0}
    };
    Vec3 local_pos_s[6] = {
        {0, -0.5, 0},Vec3{0, 0, 0.5},Vec3{0, 0.5, 0},Vec3{0, 0, -0.5},Vec3{-0.5, 0, 0},Vec3{0.5, 0, 0}
    };
    for (int i = 0; i < 6; ++i)
    {
        auto quad_node = create_mesh_provider(generate_quad());
        quad_node->local_euler_angles = local_euler_angles_s[i];
        quad_node->local_pos = local_pos_s[i];
        quad_node->local_scale = {1, 1, 1};
        cube->add_child(quad_node);
    }
    return cube;
}

SHARE_PTR<Mesh> vert_buff()
{
    //mesh
    float r = 200;
    float *vert_buff = new float[30]{
        -r, 0, 0, 0, 0,
        r, -r, 0, 1, 0,
        r, 0, 0, 0, 1,
        -r, 0, 0, 1, 0,
        r, 0, 0, 1, 1,
        r, r, 0, 0, 1
    };
    SHARE_PTR<float[]> vert_buff_shared_ptr(vert_buff);
    auto _Mesh = Resource::create_unnamed_resource<Mesh>(vert_buff_shared_ptr, 30);
    _Mesh->bind_attribute(POS,3, 0, 5);
    _Mesh->bind_attribute (UV, 2, 3, 5);
    _Mesh->locate_zero();
    _Mesh->calculate_tangents();
    return _Mesh;
}


int main()
{
    // window
    auto* window_handle = new WindowHandle(0, 0, 640, 480);
    window_handle->open();

    TypeFactory::RegisterTypes();

    auto ctx = get_current_ctx();
    auto& setting = ctx->setting;
    setting.enable_ray_cast = true;
    setting.build_bvh = true;
    setting.msaa_factor = 2;
    setting.enable_edge = false;
    setting.background_color = BLACK;
    setting.enable_mipmap = true;
    setting.shadow_distance = 100;
    auto camera_node = CREATE_OBJECT_BY_TYPE(Transform);
    auto camera = camera_node->add_component<Camera>(0.1f, 1000.0f, 60, 640 * 1.0 / 480, true);
    camera->render_layer = 1;

    // auto camera_node2 = CREATE_OBJECT_BY_TYPE(Transform);
    // auto camera_2 = camera_node2->add_component<Camera>(0.1f, 1000.0f, 40, 640 * 1.0 / 480, true);
    // camera_2->render_layer = 2;
    // auto _light0_node = CREATE_OBJECT_BY_TYPE(Transform);
    //
    // auto light0 = _light0_node->add_component<SpotLight>();
    // light0->color = {1, 1, 1};
    // light0->intensity = 30.0f;
    // light0->scene_node->local_pos = {-1, -1, 1};
    // light0->angle = 45;
    // light0->max_distance = 100;



    SHARE_PTR<FragShader> _text_frag_shader0 = Resource::get_or_create_default_resource<TextureFragShader>();

    auto _frag_shader0 = Resource::get_or_create_default_resource<FragShader>();


    auto _lightFragShader0 = Resource::get_or_create_default_resource<LightFragShader>();


    auto _NormalTextureLightFragShader0 = Resource::get_or_create_default_resource<NormalTextureLightFragShader>();


    auto _MaterialBRDFFragShader = Resource::get_or_create_default_resource<MaterialBRDFFragShader>();


    auto _vert_shader0 = Resource::get_or_create_default_resource<VertShader>();


    auto texture0 = Resource::create_unnamed_resource<Texture>("cat-7737618_1280.jpg", true);

    auto texture1 = Resource::create_unnamed_resource<Texture>( "black_white_box.png", true);
    auto texture2 = Resource::create_unnamed_resource<Texture>("colorful.png", true);

    auto normal_texture0 = Resource::create_unnamed_resource<Texture>("normal_img.jpeg", true);


    auto normal_texture1 = Resource::create_unnamed_resource<Texture>("noise.jpeg", true);

    auto _PureColorTexture0 = Resource::create_unnamed_resource<PureColorTexture>(Vec4(1, 0, 0, 1));

    auto _PureColorTexture1 = Resource::create_unnamed_resource<PureColorTexture>(Vec4(0, 1, 0, 1));

    auto _PureColorTexture2 = Resource::create_unnamed_resource<PureColorTexture>(Vec4(0, 0, 1, 1));
    auto _PureColorTexture3 = Resource::create_unnamed_resource<PureColorTexture>(Vec4(1, 1, 1, 1));


    auto _material0 = Resource::create_unnamed_resource<Material>();

    _material0->set_Vec4_uniform(MATERIAL_COLOR1, {1, 0, 0, 1});

    auto _material1 = Resource::create_unnamed_resource<Material>();
    _material1->set_Vec4_uniform(MATERIAL_COLOR1, {0,1,0,0.5});

    auto _material2 = Resource::create_unnamed_resource<Material>();

    _material2->set_Vec4_uniform(MATERIAL_COLOR1, {0, 0, 1, 0.5});

    auto _BliPhongMaterial0 = Resource::create_unnamed_resource<BliPhongMaterial>(200, 0.5, 0.5);


    auto _LambertianMaterial0 = Resource::create_unnamed_resource<LambertianMaterial>();

    _LambertianMaterial0->set_float_uniform(MATERIAL_NORMAL_BUMP_FACTOR, 1.0f);
    _LambertianMaterial0->set_Vec4_uniform(MATERIAL_COLOR1, {1, 0, 0, 1});

    auto _DiffuseLightMaterial0 = Resource::create_unnamed_resource<DiffuseLightMaterial>(1);

    _DiffuseLightMaterial0->set_float_uniform(MATERIAL_NORMAL_BUMP_FACTOR, 1.0f);
    _DiffuseLightMaterial0->set_Vec4_uniform(MATERIAL_COLOR1, {1, 0, 0, 1});

    auto _MetalMaterial0 = Resource::create_unnamed_resource<MetalMaterial>();
    _MetalMaterial0->set_float_uniform(MATERIAL_NORMAL_BUMP_FACTOR, 1.0f);
    _MetalMaterial0->set_Vec4_uniform(MATERIAL_COLOR1, {1, 0, 0, 1});

    auto shadow_ts = CREATE_OBJECT_BY_TYPE(Transform);



    // _pig_mesh_provider->get_component<MeshProvider>()->locate_centroid(camera);
    // auto venti = Resource::create_resource<Mesh>("GenshinImpact_Venti/Avatar_Boy_Bow_Venti.fbx",
    //                                                 "GenshinImpact_Venti/Avatar_Boy_Bow_Venti.fbx");
    // auto venti_node = create_mesh_provider(venti);
    // venti_node->local_scale*=0.001;
    // venti_node->get_component<MeshProvider>()->locate_centroid(camera);
    //sphere
    //
    // auto sun_node = create_mesh_provider(generate_sphere(0.1, 10, 10));
    // sun_node->local_pos = _light1_node->local_pos;

    auto sphere_node = create_mesh_provider(generate_sphere(0.5, 30, 30));
    sphere_node->local_pos = {-0, 0, 0};
    sphere_node->local_euler_angles = {-90, 0, 0};
    sphere_node->local_scale={0.25, 0.25, 0.25};
    sphere_node->set_parent(shadow_ts);

    // auto point = create_mesh_provider(generate_sphere(0.1, 5, 5));
    // point->local_pos = {-0.7, -0.25, -2 + 0.2};
    // point->local_euler_angles = {-90, 0, 0};
    // point->local_scale={0.25, 0.25, 0.25};

    std::vector<SHARE_PTR<Texture>> inner_cube_textures = {
        _PureColorTexture3, _PureColorTexture3, _PureColorTexture3, _PureColorTexture3, _PureColorTexture0,
        _PureColorTexture1
    };

    auto inner_cube = inner_generate_cube();
    for (int i = 0; i < inner_cube->child_count(); ++i)
    {
        auto cube = inner_cube->get_child(i);
        add_mesh_render(cube, _vert_shader0, _frag_shader0, _LambertianMaterial0, inner_cube_textures[i], 1, false);
    }
    auto outter_cube0 = outer_generate_cube();
    auto outter_cube1 = outer_generate_cube();

    for (int i = 0; i < outter_cube1->child_count(); ++i)
    {
        auto cube = outter_cube1->get_child(i);
        add_mesh_render(cube, _vert_shader0, _frag_shader0, _LambertianMaterial0, _PureColorTexture3, 1, false);
    }
    for (int i = 0; i < outter_cube0->child_count(); ++i)
    {
        auto cube = outter_cube0->get_child(i);
        add_mesh_render(cube, _vert_shader0, _frag_shader0, _LambertianMaterial0, _PureColorTexture3, 1, false);
    }

    inner_cube->set_parent(shadow_ts);
    inner_cube->local_scale = {1, 1, 3};
    outter_cube1->set_parent(shadow_ts);
    outter_cube0->set_parent(shadow_ts);
    outter_cube0->local_scale={0.2,0.2,0.2};
    outter_cube0->local_euler_angles = {0, 45, 0};
    outter_cube0->local_pos = {-0.2, -0.4, -1.0};
    outter_cube1->local_scale={0.2,0.2,0.2};
    outter_cube1->local_pos = {0.2, -0.4, -0.8};
    outter_cube1->local_euler_angles = {0, 0, 0};

    auto _light1_node = CREATE_OBJECT_BY_TYPE(Transform);
    auto light1 = _light1_node->add_component<SpotLight>();
    light1->c_light = {10, 10, 10};
    light1->scene_node->local_pos = {0, 0.49, -0.9};
    light1->scene_node->local_euler_angles = {-90, 0, 0};
    light1->max_distance = 100;
    light1->angle = 85;
    light1->radius = 0.1;
    _light1_node->set_parent(shadow_ts);

    auto sphere_geo_node = CREATE_OBJECT_BY_TYPE(Transform);
    sphere_geo_node->local_pos = {-0.3, -0.3, -0.7};
    sphere_geo_node->set_parent(shadow_ts);
    auto sphere = std::make_shared<Sphere>();
    sphere->radius = 0.1;
    add_geometry_render(sphere_geo_node, sphere, _MetalMaterial0, _PureColorTexture0, 1);

    auto _Pig = Resource::create_unnamed_resource<Mesh>("pig/16433_Pig.obj");
    auto _pig_mesh_provider = create_mesh_provider(_Pig);
    _pig_mesh_provider->local_pos = {0.1, -0.3, -1};
    _pig_mesh_provider->local_scale = {0.3, 0.3, 0.3};
    _pig_mesh_provider->local_euler_angles = {90, 90, 180};
    _pig_mesh_provider->set_parent(shadow_ts);
    auto _pig_mesh_provider_mesh_render = add_mesh_render(_pig_mesh_provider, _vert_shader0,
                                                          _MaterialBRDFFragShader, _MetalMaterial0, texture2, 1,
                                                          false);
    _pig_mesh_provider_mesh_render->textures[1] = normal_texture0;
    _pig_mesh_provider_mesh_render->shader_caster = true;
    // auto quad_node = create_mesh_provider(generate_quad());
    // quad_node->local_euler_angles = {-90, 0, -0.};
    // quad_node->local_pos = Vec3{-0.f, -0.5, 0};
    // quad_node->local_scale={1, 1, 1};
    // add_mesh_render(quad_node, _vert_shader0, _frag_shader0, _LambertianMaterial0, _PureColorTexture0, 1, false);
    //
    // quad_node->set_parent(shadow_ts);
    // //
    //
    // auto quad_node1 = create_mesh_provider(generate_quad());
    // quad_node1->local_euler_angles = {-0, 0, -0};
    // quad_node1->local_pos = Vec3{0.f, 00, 0.5};
    // quad_node1->local_scale={1, 1, 1};
    // add_mesh_render(quad_node1, _vert_shader0, _frag_shader0, _LambertianMaterial0, _PureColorTexture1, 1, false);
    //
    // quad_node1->set_parent(shadow_ts);
    //
    shadow_ts->local_pos = {0, -0, -0.2};
    shadow_ts->local_scale = {1, 1, 1};
    shadow_ts->local_euler_angles = {0, 0, 0};
    //

    //
    // auto tri_node = create_mesh_provider(generate_tri());
    // tri_node->local_euler_angles = {-0, 0, 0};
    // tri_node->local_pos = Vec3{0.0f, -1, -2};
    // tri_node->local_scale={1, 1, 1};
    //
    // //vert_buff
    // auto _vert_buff = create_mesh_provider(vert_buff());
    // _vert_buff->local_pos = Vec3{-1, 0, 200};
    // _vert_buff->local_scale = Vec3{0.25, 0.25, 0.25};
    // _vert_buff->local_euler_angles = Vec3{50, 0, 0};

    // // add_mesh_render(quad_node2, vert_shader0,_text_frag_shader0, _material0, texture0, 1, true);

    // auto sphere_render = add_mesh_render(sphere_node, vert_shader0, _MaterialBRDFFragShader, _BliPhongMaterial0, texture2, 1,
    //                                    false);
    // sphere_render->shader_caster = true;
    // sphere_render->textures[1] = normal_texture0->get_resource_id();

    // add_mesh_render(point,vert_shader0, _lightFragShader0, _light_material, texture1, 1, false);


    ctx->setting.enable_global_path_trace = true;
    ctx->setting.global_path_trace_samples = 30;
    ctx->setting.global_ray_trace_thr = 90;
    ctx->setting.global_mid_filter_size = 3;
    ctx->setting.global_radiance_std = 0.01;
    ctx->setting.enable_shadow = false;
    ctx->setting.global_ray_trace_max_depth=10;

    light1->generate_render_node();


    ctx->main_loop();
    return 0;
}