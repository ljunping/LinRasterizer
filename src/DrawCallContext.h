//
// Created by Lin on 2024/12/26.
//

#ifndef DRAWCALLDATA_H
#define DRAWCALLDATA_H
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "Color.h"
#include "CommonMacro.h"
#include "DrawCallSetting.h"
#include "FragShader.h"
#include "Geometry.h"
#include "L_math.h"
#include "Material.h"
#include "Mesh.h"
#include "Texture.h"
#include "VertShader.h"

class Transform;
struct Geometry;
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
    WORLD_NORMAL,
    VIEW_POS,
    VIEW_NORMAL,
    VIEW_VIEW_DIR,
    TBN_VIEW_DIR,
    TBN_TANGENT,
    VertOutPutTypeCount
};

struct RenderNode
{
    bool emit = false;
    bool shadow_caster = false;
    bool transparent{};
    int render_order{};
    SHARE_PTR<Mesh> mesh{};
    const Transform* transform{};
    SHARE_PTR<Geometry> local_geometry;
    Geometry* transform_geometry;
    Mat44 model_matrix;
    Camera* camera{};
    int frame_buff_index{};
    SHARE_PTR<Texture> textures[MAX_TEXTURES_COUNT];
    SHARE_PTR<Material> materials[MAX_MATERIAL_COUNT];
    SHARE_PTR<FragShader> frag_shader{};
    SHARE_PTR<VertShader> vert_shader{};

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
    Object* host;
    std::vector<RenderNode> nodes;
    std::vector<int> emit_render_node;
    std::unordered_map<Mesh*, int> mesh_2_node;
    std::vector<SHARE_PTR<Mesh>> meshes;
    std::unordered_map<Mesh*, int> mesh_2_index;
    std::vector<int> mesh_vert_count = std::vector(1, 0);
    std::vector<int> mesh_tri_count = std::vector(1, 0);
    std::vector<RenderNode*> geometry_2_node;
    std::vector<Geometry*> geometries;
    Geometry* geometrie_pools[GeometryCount]{};
    int geometry_count[GeometryCount]{};
    int w{}, h{};
    Fragment* fragment_map{};
    Camera* camera{};
    float* depth_buff{};
    Color* frame_buff{};
    Context* ctx{};
    DrawCallContextSetting setting;
    VertexOutput* outputs{};
    bool is_set_render_node = false;
    Mat44 proj_matrix;
    Mat44 view_matrix;
    Vec3 view_world_pos;
    SHARE_PTR<Material> materials[MAX_MATERIAL_COUNT]{};
    SHARE_PTR<Texture> textures[MAX_TEXTURES_COUNT]{};
    SHARE_PTR<FragShader> frag_shader{};
    SHARE_PTR<VertShader> vert_shader{};
    BVHTree* bvh_tree{};
    int gl_position_count = 0;
    Vec4* gl_positions{};
    ~DrawCallContext();
    void set_mesh_render_params(const RenderNode& node);
    void add_render_node(RenderNode& node);
    void assign_geometry_primitives(GeometryType type, int count);
    RenderNode* get_render_node(Mesh* mesh);
    std::shared_ptr<Mesh> get_mesh(int vert_index, int& mesh_index) const;
    std::shared_ptr<Mesh> get_mesh_by_tri_index(int tri_index, int& mesh_tri_index) const;
    int get_mesh_vert_index(Mesh* mesh, int mesh_index) ;
    void get_tri_mesh_index (int index, TrianglePrimitive& tri);;
    template <int N>
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
    Material* material{};
    bool build_interpolation_data = false;
};

#endif //DRAWCALLDATA_H
