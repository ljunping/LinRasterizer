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

struct GlVert;
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


# define CLIP_NEAR_VERTEX_MAX_COUNT 2
# define CLIP_NEAR_TRI_MAX_COUNT 2

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
    std::vector<const RenderNode*> geometry_2_node;
    std::vector<Geometry*> geometries;
    Geometry* geometrie_pools[GeometryCount]{};
    int geometry_count[GeometryCount]{};
    bool* geometry_invalid[GeometryCount]{};
    bool* vert_invalid{};
    int w{}, h{};
    Fragment* fragment_map{};
    Frustum view_frustum;
    float* depth_buff{};
    Color* frame_buff{};
    Context* ctx{};
    DrawCallContextSetting setting;
    bool is_set_render_node = false;
    Camera* camera{};
    Mat44 proj_matrix;
    Mat44 view_matrix;
    Mat44 inv_view_matrix;
    Vec3 view_world_pos;
    SHARE_PTR<Material> materials[MAX_MATERIAL_COUNT]{};
    SHARE_PTR<Texture> textures[MAX_TEXTURES_COUNT]{};
    SHARE_PTR<FragShader> frag_shader{};
    SHARE_PTR<VertShader> vert_shader{};
    BVHTree* bvh_tree{};
    int gl_vert_count = 0;
    GlVert* gl_verts{};
    ~DrawCallContext();
    int get_mesh_vert_count() const;
    int get_mesh_tri_count() const;
    void create_vert(int vert_index, int mesh_vert_index, std::shared_ptr<Mesh>& mesh);
    TrianglePrimitive* create_tri(int tri_index, int mesh_index, int mesh_tri_index);
    TrianglePrimitive* create_tri(int tri_index, int v0, int v1, int v2);

    void create_vert(int vert_index
                     , int mesh_tri_index,
                     std::shared_ptr<Mesh>& mesh,
                     const L_MATH::Vec<float, 3>& clip_alpha);

    const RenderNode* get_render_node(int vert_index) const;
    void set_mesh_render_params(const RenderNode& node);
    void add_render_node(RenderNode& node);
    const RenderNode* get_render_node_by_mesh(std::shared_ptr<Mesh>& mesh) const;
    void assign_geometry_primitives(GeometryType type, int count);
    void assign_vert(int vert_count);
    void raycaster_process_geometry(int mesh_tri_count);
    int get_mesh_index(int vert_index, int& mesh_vert_index) const;
    int get_mesh_index_by_tri_index(int tri_index, int& mesh_tri_index) const;
    bool get_clip_near_plane_alpha(const TrianglePrimitive* triangle_primitive,
                                   std::vector<Vec3>& clip_vertices_alpha) const;
    template <int N>
    void get_vert_attribute_value(int vert_index, int attribute_index, L_MATH::Vec<float, N>& result);
    void create_vert_attribute(int vert_index0, int vert_index1, int vert_index2,
                               const L_MATH::Vec<float, 3>& alpha, VertexInterpolation& result) const;
    void build_vert_interpolation(int v0) const;
    static void interpolation_attribute(
        const VertexInterpolation& v0, const VertexInterpolation& v1, const VertexInterpolation& v2,
        VertexInterpolation&
        result, const L_MATH::Vec<float, 3>& alpha);
    void interpolation_out_put(int v0, int v1, int v2, const L_MATH::Vec<float, 3>& alpha, VertexInterpolation&) const;
};



//这个类纯数据类，不要继承其他类，不能执行析构函数
struct VertexInterpolation
{
    float values[AttributeTypeCount * 4]{};
    Vec3 tangent;
    Mesh* mesh{};
    VertexOutput output;
    template <int N>
    void get_attribute_value(int attribute_index, L_MATH::Vec<float, N>& result);
    void interpolation();
};

struct GlVert
{
    const RenderNode* render_node{};
    int mesh_vert_index[3]{};
    Vec3 alpha;
    bool mesh_vert = false;
    Vec4 gl_position;
    VertexInterpolation vertex_interpolation{};
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
