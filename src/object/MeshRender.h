//
// Created by Lin on 2024/12/4.
//

#ifndef MESHRENDER_H
#define MESHRENDER_H
#include "Component.h"
struct DrawCallContext;
class Camera;
struct RenderNode;
class VertShader;
class Context;


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
    int textures[MAX_TEXTURES_COUNT];
    int materials[MAX_MATERIAL_COUNT];
    int frag_shader{};
    int vert_shader{};
    void collect_render_node(Camera* camera,std::vector<RenderNode>& render_nodes) override;
};

class RenderManager : public ObjectManger<RenderComponent>
{
public:
    void collection_render_node(Camera* camera, std::vector<RenderNode>& render_nodes,bool transparent);

    void calculate_render_pass(Camera* camera, std::vector<DrawCallContext>& render_passes, bool transparent);
};
#endif //MESHRENDER_H
