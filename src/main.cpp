#include <span>

#include "L_math.h"
#include "Camera.h"
#include "WindowHandle.h"
#include "FragShader.h"
#include "Light.h"
#include "Material.h"
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

MeshRender* add_mesh_render(Transform* node,VertShader* vert_shader, FragShader* frag_shader, Material* material,
                     Texture* texture,
                     int render_layer, bool transparent)
{
    auto _vert_shader = COPY_OBJECT(vert_shader);
    auto _frag_shader = COPY_OBJECT(frag_shader);
    auto meshrender = node->add_component<MeshRender>();
    meshrender->frag_shader = _frag_shader->get_resource_id();
    meshrender->frame_buff_index = 0;
    meshrender->transparent = transparent;
    meshrender->render_layer = render_layer;
    meshrender->materials[0] = material->get_resource_id();
    meshrender->textures[0] = texture->get_resource_id();
    meshrender->vert_shader = _vert_shader->get_resource_id();
    return meshrender;
}

Mesh* vert_buff()
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
    auto _Mesh = Resource::get_or_create_resource<Mesh>("simple_tri",vert_buff_shared_ptr, 30);
    _Mesh->bind_attribute(POS,3, 0, 5);
    _Mesh->bind_attribute (UV, 2, 3, 5);
    return _Mesh;
}

Transform* create_mesh_provider(Mesh* mesh)
{
    //mesh
    auto node = CREATE_OBJECT_BY_TYPE(Transform);
    auto mesh_provider = node->add_component<MeshProvider>();
    mesh_provider->mesh_id = mesh->get_resource_id();
    return node;
}

int main()
{
    TypeFactory::RegisterTypes();
    // window
    auto* window_handle = new WindowHandle(0, 0, 640, 480);
    window_handle->open();
    auto ctx = get_current_ctx();
    auto& setting = ctx->setting;
    setting.enable_ray_cast = false;
    setting.build_bvh = false;
    setting.msaa_factor = 1;
    setting.enable_edge = false;
    setting.background_color = BLACK;
    setting.enable_mipmap = true;
    auto camera_node = CREATE_OBJECT_BY_TYPE(Transform);
    auto camera = camera_node->add_component<Camera>(0.1f, 1000.0f, 40, 640 * 1.0 / 480, true);
    camera->render_layer = 1;

    auto camera_node2 = CREATE_OBJECT_BY_TYPE(Transform);
    auto camera_2 = camera_node2->add_component<Camera>(0.1f, 1000.0f, 40, 640 * 1.0 / 480, true);
    camera_2->render_layer = 2;
    auto _light0_node = CREATE_OBJECT_BY_TYPE(Transform);

    auto light0 = _light0_node->add_component<Light>();
    light0->color = {1, 1, 1};
    light0->intensity = 20.0f;
    light0->scene_node->local_pos = {-1,-1,1};

    auto _light1_node = CREATE_OBJECT_BY_TYPE(Transform);

    auto light1 = _light1_node->add_component<Light>();
    light1->color = {1, 1, 1};
    light1->intensity = 10.0f;
    light1->scene_node->local_pos = {1, 1, 1};

    auto _text_frag_shader0 = Resource::get_or_create_resource<TextureFragShader>("TextureFragShader_Sphere.frag");
    auto _frag_shader0 = Resource::get_or_create_resource<FragShader>("FragShader_Sphere.frag");
    auto _lightFragShader0 = Resource::get_or_create_resource<LightFragShader>("LightFragShader.frag");

    auto _NormalTextureLightFragShader0 = Resource::get_or_create_resource<NormalTextureLightFragShader>(
        "NormalTextureLightFragShader.frag");


    auto vert_shader0 = Resource::get_or_create_resource<VertShader>("VertShader_Sphere.vert");
    auto texture0 = Resource::create_resource<Texture>("cat-7737618_1280.jpg", "cat-7737618_1280.jpg", true);
    texture0->linear = true;
    auto texture1 = Resource::create_resource<Texture>("black_white_box.png", "black_white_box.png", true);
    texture1->linear = true;
    auto texture2 = Resource::create_resource<Texture>("colorful.png", "colorful.png", true);
    texture2->linear = true;
    auto normal_texture0 = Resource::create_resource<Texture>("normal_img.jpeg", "normal_img.jpeg", true);
    normal_texture0->linear = true;
    auto normal_texture1 = Resource::create_resource<Texture>("noise.jpeg", "noise.jpeg", true);
    normal_texture1->linear = true;
    auto _material0 = Resource::create_resource<Material>("default.mat");
    _material0->set_Vec4_uniform(MATERIAL_COLOR1, {1, 0, 0, 0.5});

    auto _material1 = Resource::create_resource<Material>("default1.mat");
    _material1->set_Vec4_uniform(MATERIAL_COLOR1, {0,1,0,0.5});

    auto _material2 = Resource::create_resource<Material>("default2.mat");
    _material2->set_Vec4_uniform(MATERIAL_COLOR1, {0, 0, 1, 0.5});

    auto _light_material = Resource::create_resource<Material>("light0.mat");
    _light_material->set_Vec4_uniform(MATERIAL_COLOR1, {1, 0, 0, 0.5});
    _light_material->set_Vec3_uniform(MATERIAL_LIGHT_KD, {1, 1, 1});
    _light_material->set_Vec3_uniform(MATERIAL_LIGHT_KS, {0.7, 0.7, 0.7});
    _light_material->set_float_uniform(MATERIAL_NORMAL_BUMP_FACTOR, 1.0f);
    // _pig
    auto _Pig = Resource::get_or_create_resource<Mesh>("pig/16433_Pig.obj", "pig/16433_Pig.obj");
    auto _pig_mesh_provider = create_mesh_provider(_Pig);
    // _pig_mesh_provider->set_local_to_global_mat(
    //     translate({
    //         0, -0.5, -1
    //     })  *
    //     rotate(LEFT, 270) * rotate(FORWARD, 90));
    _pig_mesh_provider->local_scale={1.5,1.5,1.5};
    _pig_mesh_provider->local_pos = {0, 0, -3};
    _pig_mesh_provider->local_euler_angles = {90, 90, 180};
    // _pig_mesh_provider->get_component<MeshProvider>()->locate_centroid(camera);
    // auto venti = Resource::create_resource<Mesh>("GenshinImpact_Venti/Avatar_Boy_Bow_Venti.fbx",
    //                                                 "GenshinImpact_Venti/Avatar_Boy_Bow_Venti.fbx");
    // auto venti_node = create_mesh_provider(venti);
    // venti_node->local_scale*=0.001;
    // venti_node->get_component<MeshProvider>()->locate_centroid(camera);
    //sphere
    auto shadow_ts = CREATE_OBJECT_BY_TYPE(Transform);
    auto sphere_node = create_mesh_provider(generate_sphere(0.5, 30, 30));
    sphere_node->local_pos = {0, 0, 0};
    sphere_node->local_scale={1, 1, 1};
    sphere_node->set_parent(shadow_ts);
    //quad
    auto quad_node = create_mesh_provider(generate_quad());
    quad_node->local_euler_angles = {-60, 0, 0};
    quad_node->local_pos = Vec3{0, -0.5, 0};
    quad_node->local_scale={3, 3, 3};
    quad_node->set_parent(shadow_ts);

    shadow_ts->local_pos = {0, 0, -2};
    shadow_ts->local_scale = {0.5, 0.5, 0.5};
    auto quad_node2 = create_mesh_provider(generate_quad());
    quad_node2->local_euler_angles = {-0, 0, 0};
    quad_node2->local_pos = Vec3{0.f, 00, -1};
    quad_node2->local_scale={1, 1, 1};

    auto tri_node = create_mesh_provider(generate_tri());
    tri_node->local_euler_angles = {-0, 0, 0};
    tri_node->local_pos = Vec3{0.0f, -1, -2};
    tri_node->local_scale={1, 1, 1};

    //vert_buff
    auto _vert_buff = create_mesh_provider(vert_buff());
    _vert_buff->local_pos = Vec3{-1, 0, 200};
    _vert_buff->local_scale = Vec3{0.25, 0.25, 0.25};
    _vert_buff->local_euler_angles = Vec3{50, 0, 0};

    // add_mesh_render(quad_node2, vert_shader0,_text_frag_shader0, _material0, texture0, 1, true);
    // add_mesh_render(sphere_node, vert_shader0,_lightFragShader0, _light_material, texture0, 1, false);
    add_mesh_render(quad_node,vert_shader0, _lightFragShader0, _light_material, texture1, 1, false);
    // add_mesh_render(tri_node,vert_shader0, _lightFragShader0, _light_material, texture0, 1, true);
    //
    // auto _pig_mesh_provider_mesh_render = add_mesh_render(_pig_mesh_provider, vert_shader0,
    //                                                       _NormalTextureLightFragShader0, _light_material, texture2, 1,
    //                                                       false);
    //
    // _pig_mesh_provider_mesh_render->textures[1] = normal_texture0->get_resource_id();
    ctx->main_loop();
    return 0;
}