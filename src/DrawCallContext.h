//
// Created by Lin on 2024/12/26.
//

#ifndef DRAWCALLDATA_H
#define DRAWCALLDATA_H
#include <unordered_map>
#include <vector>
#include "Color.h"
#include "CommonMacro.h"
#include "DrawCallSetting.h"
#include "L_math.h"

struct Fragment;
class Light;
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
    WORLD_POS,
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
    friend class GPU;
public:
    int w{}, h{};
    Fragment* fragment_map{};
    Camera* camera{};
    float* depth_buff{};
    Color* frame_buff{};
    Context* ctx{};
    DrawCallContextSetting setting;
    VertexOutput* outputs{};
    RenderNode render_node{};
    bool is_set_render_node = false;
    std::vector<std::pair<Mesh*,Mat44>> meshes;
    Mat44 proj_matrix;
    Mat44 view_matrix;
    Vec3 view_world_pos;
    std::vector<TrianglePrimitive*> primitives;
    Material* materials[MAX_MATERIAL_COUNT]{};
    Texture* textures[MAX_TEXTURES_COUNT]{};
    FragShader* frag_shader{};
    VertShader* vert_shader{};
    BVHTree* bvh_tree{};
    TrianglePrimitive* tri_pool{};
    int gl_position_count = 0;
    Vec4* gl_positions{};
    bool try_add_render_node(RenderNode& node);
    void set_render_node(const RenderNode& node);
    ~DrawCallContext();
    void assign_triangle_primitives(int size);
    L_MATH::Mat<float, 4, 4> get_model_matrix(Mesh* mesh) const;
    Mesh* get_mesh(int vert_index, int& mesh_index) const;
    int get_muti_mesh_vert_index(const Mesh* mesh, int mesh_index) const;
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
