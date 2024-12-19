#include "L_math.h"
#include "Camera.h"
#include "WindowHandle.h"
#include "FragShader.h"
#include "Material.h"
#include "MeshRender.h"
#include "Transform.h"
#include "Texture.h"
using namespace L_MATH;
static int layer;
int next_layer()
{
    return 1 << (layer++);
}
Transform* simple_tri()
{
    //mesh
    float z = -200;
    float r = 200;
    float *vert_buff = new float[30]{
        -r, 0, 0, 0, 0,
        r, -r, 0, 1, 0,
        r, 0, 0, 0, 1,
        -r, 0, 0, 1, 0,
        r, 0, 0, 1, 1,
        r, r, 0, 0, 1
    };
    auto parent = CREATE_OBJECT_BY_TYPE(Transform);
    SHARE_PTR<float[]> vert_buff_shared_ptr(vert_buff);
    auto _Mesh = Resource::get_or_create_resource<Mesh>("simple_tri",vert_buff_shared_ptr, 30);
    _Mesh->bind_attribute(POS,3, 0, 5);
    _Mesh->bind_attribute (UV, 2, 3, 5);
    auto camera = parent->add_component<Camera>(0.1f, 1000.0f, 120, 640 * 1.0 / 480, true);
    camera->render_layer = next_layer();
    parent->local_pos = Vec3{-1, 0, z};
    parent->local_scale = Vec3{1, 1, 1};
    parent->local_euler_angles = Vec3{50, 0, 0};
    auto texture = Resource::get_or_create_resource<Texture>("black_white_box.png", "black_white_box.png", true);
    auto frag_shader = Resource::get_or_create_resource<FragShader>("FragShader.frag");
    auto material = Resource::get_or_create_resource<Material>("default_material");
    auto meshrender = parent->add_component<MeshRender>();
    meshrender->frag_shader = frag_shader->get_resource_id();
    meshrender->frame_buff_index = 0;
    meshrender->mesh = _Mesh->get_resource_id();
    meshrender->sort_layer = camera->render_layer;
    meshrender->material = material->get_resource_id();
    meshrender->texture = texture->get_resource_id();
    return parent;
}

Transform* create_obj_model_node(const char* file_name)
{
    TypeFactory::RegisterTypes();

    // window
    auto parent = CREATE_OBJECT_BY_TYPE(Transform);
    auto camera = parent->add_component<Camera>(0.1f, 1000.0f, 40, 640 * 1.0 / 480, true);
    camera->render_layer = next_layer();
    parent->local_euler_angles = Vec3{60, 0, 0};
    auto texture = Resource::get_or_create_resource<Texture>("black_white_box.png","black_white_box.png", true);
    auto material = Resource::get_or_create_resource<Material>("default.material");
    auto frag_shader = Resource::get_or_create_resource<TextureFragShader>("TextureFragShader.frag");
    //mesh
    auto mesh = Resource::get_or_create_resource<Mesh>(file_name,file_name);
    auto node = CREATE_OBJECT_BY_TYPE(Transform);
    parent->add_child(node);
    auto model_mat = translate(Vec3{0, -0.5, -1}) * rotate(LEFT, 270) * rotate(FORWARD, 90);
    node->set_local_to_global_mat(model_mat);
    auto meshrender = node->add_component<MeshRender>();
    meshrender->sort_layer = camera->render_layer;
    meshrender->frag_shader = (frag_shader)->get_resource_id();
    meshrender->frame_buff_index = 0;
    meshrender->texture = texture->get_resource_id();
    meshrender->material = material->get_resource_id();
    meshrender->mesh = mesh->get_resource_id();
    return parent;
}


int main()
{
    TypeFactory::RegisterTypes();
    // window
    auto* window_handle = new WindowHandle(0, 0, 640, 480);
    window_handle->open();
    auto ctx = get_current_ctx();
    ctx->enable_ray_cast = true;
    ctx->build_bvh = true;
    ctx->msaa_factor = 2;
    ctx->enable_edge = true;
    ctx->root->add_child(create_obj_model_node("pig/16433_Pig.obj"));
    // ctx->root->add_child(simple_tri());
    ctx->main_loop();
    return 0;
}