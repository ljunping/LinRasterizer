//
// Created by Lin on 2024/12/4.
//

#ifndef MESHRENDER_H
#define MESHRENDER_H
#include "Component.h"
#include "FragShader.h"
struct Geometry;
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
    virtual std::shared_ptr<Mesh> get_mesh(){return nullptr;};
    int render_order = 0;
    int render_layer = 1;
    int frame_buff_index = 0;
    SHARE_PTR<Texture> textures[MAX_TEXTURES_COUNT]{};
    SHARE_PTR<Material> materials[MAX_MATERIAL_COUNT]{};
    SHARE_PTR<FragShader> frag_shader{};
    SHARE_PTR<VertShader> vert_shader{};
    bool transparent{};
    bool shader_caster = false;
    void on_create() override;
    void on_delete() override;
    ~RenderNodeComponent() override;
    virtual void collect_render_node(Camera* camera, std::vector<RenderNode>& render_nodes){};
};


class MeshProvider : public Component
{
    INIT_TYPE(MeshProvider, Component)
public:
    SHARE_PTR<Mesh> mesh;
    ~MeshProvider() override;
    void locate_centroid(Camera* camera) const;
};

class MeshRender : public RenderNodeComponent
{
    INIT_TYPE(MeshRender, RenderNodeComponent)
    bool NEED_UPDATE{true};
public:
    std::shared_ptr<Mesh> get_mesh() override;
    void collect_render_node(Camera* camera, std::vector<RenderNode>& render_nodes) override;
};

class GeometryRender : public RenderNodeComponent
{
    INIT_TYPE(GeometryRender, RenderNodeComponent)
public:
    bool emit = false;
    SHARE_PTR<Geometry> geometry;
    void collect_render_node(Camera* camera, std::vector<RenderNode>& render_nodes) override;
};

class RenderNodeManager : public ObjectManger<RenderNodeComponent>
{
public:
    void collection_render_node(Camera* camera, std::vector<RenderNode>& render_nodes);
};


#endif //MESHRENDER_H


MeshRender* add_mesh_render(Transform* node, const SHARE_PTR<VertShader>& vert_shader, const SHARE_PTR<FragShader>& frag_shader,
                            const SHARE_PTR<Material>& material,
                            const SHARE_PTR<Texture>& texture, int render_layer, bool transparent);


Transform* create_mesh_provider(const SHARE_PTR<Mesh>& mesh);


GeometryRender* add_geometry_render(Transform* node, const SHARE_PTR<Geometry>& geo,
                                    const SHARE_PTR<Material>& material,
                                    const SHARE_PTR<Texture>& texture, int render_layer);


