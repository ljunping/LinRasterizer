#include "DrawUtil.h"
#include "L_math.h"
#include "Rasterizer.h"
#include "WindowHandle.h"
#include "FragShader.h"
using namespace L_MATH;


void test()
{
    Scene* scene = new Scene();

    std::vector<Attributes> meshes;
    Attributes::load_obj_file("pig/16433_Pig.obj", meshes);
    scene->clear();
    auto texture = TEXTURE_MANAGER.create_texture("black_white_box.png", true);
    std::vector<TrianglePrimitive> primitives;
    meshes[0].generate_triangles(primitives);
    meshes[0].bind_texture(texture);
    scene->add_attributes(&meshes[0]);
    scene->model_matrix =   translate(Vec3{0, -0.5, -1}) * rotate(LEFT, 270) * rotate(FORWARD, 90);
    Camera* camera = new Camera(scene, 0.1f, 1000.0f, 40, 640 * 1.0 / 480, true);
    auto _texture_shader = new TextureFragShader();
    _texture_shader->text_attribute_index = 2;
    auto *rasterizer = new Rasterizer(scene, camera, _texture_shader);
    rasterizer->enable_ray_cast=true;
    camera->build_bvh = true;
    WindowHandle* window_handle = new WindowHandle(0, 0, 640, 480, rasterizer);
    window_handle->open();
}

void test2()
{
    Scene* scene = new Scene();
    float z=-300;
    float r=200;
    float *vert_buff = new float[30]{
        -r, 0, z, 0, 0,
        r, -r, z, 1, 0,
        r, 0, z, 0, 1,
        -r, 0, z, 1, 0,
        r, 0, z, 1, 1,
        r, r, z, 0, 1
    };
    auto _Attributes = new Attributes(vert_buff, 30);
    _Attributes->bind_attribute(POS,3, 0, 5);
    _Attributes->bind_attribute (UV, 2, 3, 5);
    std::vector<TrianglePrimitive> primitives;
    _Attributes->generate_triangles(primitives);
    scene->model_matrix = translate(Vec3{0, 0, z}) * rotate(L_MATH::LEFT, 0) * translate(Vec3{0, 0, -z});
    auto texture = TEXTURE_MANAGER.create_texture("black_white_box.png", true);
    _Attributes->bind_texture(texture);
    scene->add_origin_triangles(primitives);
    Camera* camera = new Camera(scene, 0.1f, 1000.0f, 120, 640 * 1.0 / 480, true);
    auto *frag_shader = new FragShader();
    // frag_shader->text_attribute_index = 1;
    auto *rasterizer = new Rasterizer(scene, camera, frag_shader);
    WindowHandle* window_handle = new WindowHandle(0, 0, 640, 480, rasterizer);
    rasterizer->enable_ray_cast=true;
    camera->build_bvh = true;
    window_handle->open();
}

void draw_line_test()
{
    Scene* scene = new Scene();
    Camera* camera = new Camera(scene, 0.1f, 1000.0f, 40, 640 * 1.0 / 480, true);
    FragShader *frag_shader = (FragShader *)new FragShader();
    auto *rasterizer = new Rasterizer(scene, camera, frag_shader);
    auto line = new DrawLineInfo();
    line->x0=199;
    line->y0=233;
    line->x1=599;
    line->y1=246;
    line->color=RED;
    auto line2 = new DrawLineInfo();
    line2->x0=599;
    line2->y0=246;
    line2->x1=199;
    line2->y1=233;
    line2->color=BLUE;
    rasterizer->register_after_scene_render_func(draw_line, line);
    rasterizer->register_after_scene_render_func(draw_line, line2);

    WindowHandle* window_handle = new WindowHandle(0, 0, 640, 480, rasterizer);
    window_handle->open();
}

int main()
{
    test();
    return 0;
}