//
// Created by Lin on 2024/12/4.
//

#ifndef MESHRENDER_H
#define MESHRENDER_H
#include "Camera.h"
#include "Component.h"
#include "Material.h"
#include "Transform.h"
class Context;

struct RenderNode
{
    bool transparent{};
    int render_order{};
    int mesh{};
    Mat44 model_matrix;
    Camera* camera{};
    int frame_buff_index{};
    int texture;
    int material;
    int frag_shader;
};


inline bool less_compare_render_node(const RenderNode& a, const RenderNode& b);

inline bool equal_compare_render_node(const RenderNode& a, const RenderNode& b);

struct DrawCallData
{
    RenderNode pass_node{};
    Context* ctx{};
    bool is_init = false;
    std::vector<std::pair<int,Mat44>> meshes;
    std::vector<TrianglePrimitive*> primitives;
    Material* material0{};
    Texture* texture0{};
    FragShader* frag_shader0{};
    Color* frame_buff{};
    BVHTree* bvh_tree{};
    TrianglePrimitive* tri_pool{};
    bool try_add_render_node(RenderNode& node);
    void init(const RenderNode& node);
    ~DrawCallData();
    void assign_triangle_primitives(int size);
    void build_bvh_tree();
    void draw_call_begin();
    void draw_call_end();
    void get_model_matrix(const Mesh* mesh,Mat44& m) const;
    void get_view_matrix(Mat44& m) const;
    void get_proj_matrix(Mat44& m) const;
};


class RenderComponent : public Component
{
    INIT_TYPE(RenderComponent, Component)
public:
    bool transparent{};
    int render_order = 0;
    int render_layer = 1;
    void on_create() override;
    void on_delete() override;

    virtual void collect_render_node(Camera* camera, std::vector<RenderNode>& render_nodes){};
};


class MeshProvider : public Component
{
    INIT_TYPE(MeshProvider, Component)
public:
    int mesh_id{};
    void locate_centroid(Camera* camera) const;
};

class MeshRender : public RenderComponent
{
    INIT_TYPE(MeshRender,RenderComponent)
public:
    int frame_buff_index = 0;
    const bool NEED_UPDATE{true};
    int texture{};
    int material{};
    int frag_shader{};
    //如果需要多纹理材质支持则
    // int texture1;
    // int material1;
    // int frag_shader1;
    void collect_render_node(Camera* camera,std::vector<RenderNode>& render_nodes) override;
};

class RenderManager : public ObjectManger<RenderComponent>
{
public:
    void collection_render_node(Camera* camera, std::vector<RenderNode>& render_nodes,bool transparent);

    void calculate_render_pass(Camera* camera, std::vector<DrawCallData>& render_passes, bool transparent);
};
#endif //MESHRENDER_H
