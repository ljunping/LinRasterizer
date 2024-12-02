#include "DrawUtil.h"
#include "L_math.h"
#include "Rasterizer.h"
#include "WindowHandle.h"
#include "LuaFragShader.h"
using namespace L_MATH;


void test()
{
    Scene* scene = new Scene();

    std::vector<Attributes> meshes;
    // Attributes::load_obj_file("cottage/cottage_obj.obj", meshes);
    // auto texture = TEXTURE_MANAGER.create_texture("cottage/cottage_textures/cottage_diffuse.png", true);
    // for (auto &mesh : meshes)
    // {
    //     mesh.bind_texture(texture);
    // }
    scene->clear();
    auto attributes = generate_sphere(3, 10, 10, L_MATH::translate(Vec3{0, 0, -3.5}));
    auto texture = TEXTURE_MANAGER.create_texture("06489085_2.jpg", true);
    attributes->bind_texture(texture);
    std::vector<TrianglePrimitive> primitives;
    attributes->generate_triangles(primitives);
    scene->add_attributes(attributes);
    // scene->add_origin_triangle(primitives[11]);
    // scene->add_origin_triangle(primitives[13]);
    Camera* camera = new Camera(scene, 0.1f, 1000.0f, 120, 640 * 1.0 / 480, true);
    LuaFragShader *frag_shader = (LuaFragShader *)new TextureFragShader();
    auto *rasterizer = new Rasterizer(scene, camera, frag_shader);
    WindowHandle* window_handle = new WindowHandle(0, 0, 640, 480, rasterizer);
    window_handle->open();
}

void test2()
{
    Scene* scene = new Scene();
    float z=-500;
    float r=200;
    float *vert_buff = new float[15]{
        -r, -r, z, 0, 0,
        r, -r, z, 1, 0,
        0, r, z, 0.5, 1
    };
    auto _Attributes = new Attributes(vert_buff, 15);
    _Attributes->bind_attribute(POS,3, 0, 5);
    _Attributes->bind_attribute (UV, 2, 3, 5);
    std::vector<TrianglePrimitive> primitives;
    _Attributes->generate_triangles(primitives);
    scene->model_matrix = translate(Vec3{0, 0, z}) * rotate(L_MATH::LEFT, 70) * translate(Vec3{0, 0, -z});
    auto texture = TEXTURE_MANAGER.create_texture("black_white_box.png", true);
    _Attributes->bind_texture(texture);
    scene->add_origin_triangles(primitives);
    Camera* camera = new Camera(scene, 0.1f, 1000.0f, 40, 640 * 1.0 / 480, true);
    LuaFragShader *frag_shader = (LuaFragShader *)new TextureFragShader();
    auto *rasterizer = new Rasterizer(scene, camera, frag_shader);
    WindowHandle* window_handle = new WindowHandle(0, 0, 640, 480, rasterizer);
    window_handle->open();
}

int main()
{
    test();
    return 0;
}