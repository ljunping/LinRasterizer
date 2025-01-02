//
// Created by Lin on 2024/12/4.
//

#ifndef MESHRENDER_H
#define MESHRENDER_H
#include "Component.h"
class Texture;
class Material;
class FragShader;
class Mesh;
struct DrawCallContext;
class Camera;
struct RenderNode;
class VertShader;
class Context;


class RenderNodeComponent : public Component
{
    INIT_TYPE(RenderNodeComponent, Component)
public:
    virtual Mesh* get_mesh(){return nullptr;};
    bool transparent{};
    int render_order = 0;
    int render_layer = 1;
    int frame_buff_index = 0;
    int textures[MAX_TEXTURES_COUNT]{};
    int materials[MAX_MATERIAL_COUNT]{};
    int frag_shader{};
    int vert_shader{};
    bool shader_caster = false;
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
    Mesh* get_mesh() const;
};

class MeshRender : public RenderNodeComponent
{
    INIT_TYPE(MeshRender, RenderNodeComponent)
    bool NEED_UPDATE{true};
public:
    Mesh* get_mesh() override;
    void collect_render_node(Camera* camera, std::vector<RenderNode>& render_nodes) override;
};

class RenderNodeManager : public ObjectManger<RenderNodeComponent>
{
public:
    void collection_render_node(Camera* camera, std::vector<RenderNode>& render_nodes,bool transparent);
};
#endif //MESHRENDER_H


MeshRender* add_mesh_render(Transform* node,VertShader* vert_shader, FragShader* frag_shader, Material* material,
                     Texture* texture,
                     int render_layer, bool transparent);


Transform* create_mesh_provider(Mesh* mesh);
