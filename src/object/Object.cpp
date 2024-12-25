//
// Created by Lin on 2024/12/4.
//

#include "Object.h"
#include "Component.h"
#include "Transform.h"
#include "Camera.h"
#include "FragShader.h"
#include "Light.h"
#include "MeshRender.h"

int TypeFactory::type_id = 0;
std::unordered_map<int, Object* (*)()> TypeFactory::type_creator_map;
std::unordered_map<int, int> TypeFactory::parent_map;
std::unordered_map<int, Object*> TypeFactory::objet_inst_map;
int TypeFactory::objet_inst_id = 0;

DEFINE_TYPE(Object)
DEFINE_TYPE(Component)
DEFINE_TYPE(RenderComponent)
DEFINE_TYPE(Transform)
DEFINE_TYPE(FragShader)
DEFINE_TYPE(TextureFragShader)
DEFINE_TYPE(Mesh)
DEFINE_TYPE(MeshRender)
DEFINE_TYPE(Camera)
DEFINE_TYPE(Texture)
DEFINE_TYPE(Material)
DEFINE_TYPE(Resource)
DEFINE_TYPE(MeshProvider)
DEFINE_TYPE(LightFragShader)
DEFINE_TYPE(NormalTextureLightFragShader)
DEFINE_TYPE(Light)




int TypeFactory::get_parent_type(int type_id)
{
    if (parent_map.contains(type_id))
    {
        return parent_map[type_id];
    }
    return 0;
}

bool TypeFactory::subclass(int parent_id, int child_type_id)
{
    while (child_type_id != parent_id && parent_map.contains(child_type_id))
    {
        child_type_id = parent_map[child_type_id];
    }
    return child_type_id == parent_id;
}

Object* TypeFactory::create_type_inst_by_id(int type_id)
{
    const auto it = type_creator_map.find(type_id);
    if (it != type_creator_map.end())
    {
        auto* res = it->second();
        res->instance_id = ++objet_inst_id;
        objet_inst_map[res->instance_id] = res;
        res->on_create();
        return res;
    }
    return nullptr;
}



Object* TypeFactory::get_type_inst_by_inst_id(int inst_id)
{
    auto it = objet_inst_map.find(inst_id);
    return (it != objet_inst_map.end()) ? it->second : nullptr;
}

void TypeFactory::RegisterTypes()
{
    REGISTER_TYPE(Component)
    REGISTER_TYPE(Transform)
    REGISTER_TYPE(FragShader)
    REGISTER_TYPE(TextureFragShader)
    REGISTER_TYPE(Mesh)
    REGISTER_TYPE(RenderComponent)
    REGISTER_TYPE(MeshRender)
    REGISTER_TYPE(Camera)
    REGISTER_TYPE(Texture)
    REGISTER_TYPE(Material)
    REGISTER_TYPE(Resource)
    REGISTER_TYPE(MeshProvider)
    REGISTER_TYPE(LightFragShader)
    REGISTER_TYPE(Light);
    REGISTER_TYPE(NormalTextureLightFragShader)
}



