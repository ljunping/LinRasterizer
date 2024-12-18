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
    int render_order{};
    int mesh{};
    Mat44 model_matrix;
    Camera* camera{};
    int frame_buff_index{};
    int texture;
    int material;
    int frag_shader;
};

inline bool is_same_camera_render(int sort_layer1,int sort_layer2);


inline bool less_compare_render_node(const RenderNode& a, const RenderNode& b);

inline bool equal_compare_render_node(const RenderNode& a, const RenderNode& b);

struct RenderPass
{
    RenderNode pass_node{};
    Context* ctx{};
    bool is_init = false;
    std::vector<std::pair<int,Mat44>> meshes;
    std::vector<TrianglePrimitive*> primitives;
    BVHTree* bvh_tree{};
    TrianglePrimitive* tri_pool;
    bool try_add_render_node(RenderNode& node);

    ~RenderPass();

    void assign_triangle_primitives(int size);

    void build_bvh_tree();
};

class RenderComponent : public Component
{
    INIT_TYPE(RenderComponent, Component)

public:
    int render_order = 0;
    int sort_layer = 1;
    void on_create() override;
    void on_delete() override;

    virtual void collect_render_node(Camera* camera, std::vector<RenderNode>& render_nodes){};
};

class MeshRender : public RenderComponent
{
    INIT_TYPE(MeshRender,RenderComponent)
public:
    int frame_buff_index = 0;
    const bool NEED_UPDATE{true};
    int mesh{};
    int texture;
    int material;
    int frag_shader;
    //如果需要多纹理材质支持则
    // int texture1;
    // int material1;
    // int frag_shader1;
    void collect_render_node(Camera* camera,std::vector<RenderNode>& render_nodes) override;
};

class RenderManager : public ObjectManger<RenderComponent>
{
public:
    void collection_render_node(Camera* camera, std::vector<RenderNode>& render_nodes);

    void calculate_render_pass(Camera* camera, std::vector<RenderPass>& render_passes);
};
#endif //MESHRENDER_H
