//
// Created by Lin on 2024/12/26.
//

#ifndef DRAWCALLDATA_H
#define DRAWCALLDATA_H
#include <unordered_map>
#include <vector>
#include "Color.h"
#include "CommonMacro.h"
#include "L_math.h"

struct VertexInterpolation;
class Mesh;
class BVHTree;
class VertShader;
class FragShader;
class Texture;
class Material;
struct TrianglePrimitive;
class Context;
class Camera;

enum VertOutPutType
{
    VIEW_POS,
    VIEW_NORMAL,
    VIEW_VIEW_DIR,
    TBN_VIEW_DIR,
    TBN_TANGENT,
    VertOutPutTypeCount
};

struct RenderNode
{
    bool transparent{};
    int render_order{};
    int mesh{};
    Mat44 model_matrix;
    Camera* camera{};
    int frame_buff_index{};
    int textures[MAX_TEXTURES_COUNT];
    int materials[MAX_MATERIAL_COUNT];
    int frag_shader{};
    int vert_shader{};
};

bool less_compare_render_node(const RenderNode& a, const RenderNode& b);

bool equal_compare_render_node(const RenderNode& a, const RenderNode& b);
class VertexOutput
{
public:
    Vec4 fix_outputs[VertOutPutTypeCount];
    std::vector<Vec4> view_light_dirs;
    std::vector<Vec4> tbn_light_dirs;
};

class DrawCallContext
{
private:
    bool is_render_job_finish(int job_group_id) const;
    int frame_begin_job_id = 0;
    int frame_cur_max_job_id = 0;
    void update_frame_job_id(int job_group_id);
public:
    VertexOutput* outputs;
    RenderNode pass_node{};
    Context* ctx{};
    bool is_init = false;
    std::vector<std::pair<Mesh*,Mat44>> meshes;
    std::vector<TrianglePrimitive*> primitives;
    Material* materials[MAX_MATERIAL_COUNT]{};
    Texture* textures[MAX_TEXTURES_COUNT]{};
    FragShader* frag_shader{};
    VertShader* vert_shader{};
    Color* frame_buff{};
    BVHTree* bvh_tree{};
    Camera* camera{};
    TrianglePrimitive* tri_pool{};
    int gl_position_count = 0;
    Vec4* gl_positions{};
    bool try_add_render_node(RenderNode& node);
    void init(const RenderNode& node);
    ~DrawCallContext();
    void assign_triangle_primitives(int size);
    void build_bvh_tree();
    void get_model_matrix(Mesh* mesh, L_MATH::Mat<float, 4, 4>& m) const;
    int run_frag_shader();
    void run_vert_shader();
    void process_primitives();
    void draw_begin();
    void draw_end();
    int ray_cast_scene(int msaa_index);
    Mesh* get_mesh(int vert_index, int& mesh_index) const;
    int get_muti_mesh_vert_index(const Mesh* mesh, int mesh_index) const;
    int raster_scene(int msaa_index);
    void wait_finish() const;
    void generate_triangle_primitive(TrianglePrimitive& tri);
    template<int N>
    void get_vert_attribute_value(Mesh*, int vert_index, int attribute_index, L_MATH::Vec<float, N>& result);

    void create_vert_attribute(Mesh* mesh, int v0, int v1, int v2,
                               const L_MATH::Vec<float, 3>& alpha, VertexInterpolation& result);

};



//这个类纯数据类，不要继承其他类，不能执行析构函数
struct VertexInterpolation
{
    DrawCallContext* draw_call_context{};
    Mesh* mesh_ptr{};
    float values[AttributeTypeCount * 4]{};
    Vec3 tangent;
    VertexOutput output;
    int v[3]{};
    Vec3 alpha;
    template <int N>
    void get_attribute_value(int attribute_index, L_MATH::Vec<float, N>& result);
    void calculate_values();
};

struct Fragment
{
    TrianglePrimitive* triangle{};
    Vec3 alpha;
    Vec3 frag_coord;
    Vec2 resolution;
    DrawCallContext* draw_call{};
    VertexInterpolation interpolation_data;
    bool build_interpolation_data = false;
};

#endif //DRAWCALLDATA_H
