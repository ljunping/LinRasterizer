//
// Created by Lin on 2024/12/4.
//

#include "Object.h"
#include "Component.h"
#include "Transform.h"
#include "Camera.h"
#include "FragShader.h"
#include "Light.h"
#include "Mesh.h"
#include "MeshRender.h"
#include "VertShader.h"

int TypeFactory::type_id = 0;
std::unordered_map<int, Object* (*)()> TypeFactory::type_creator_map;
std::unordered_map<int, int> TypeFactory::parent_map;
std::unordered_map<int, Object*> TypeFactory::objet_inst_map;
int TypeFactory::objet_inst_id = 0;

DEFINE_TYPE(Object)
DEFINE_TYPE(Component)
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
DEFINE_TYPE(SpotLight)
DEFINE_TYPE(DirectionalLight)
DEFINE_TYPE(PointLight)
DEFINE_TYPE(VertShader)
DEFINE_TYPE(DrawCallNodeComponent)
DEFINE_TYPE(RenderNodeComponent)
DEFINE_TYPE(LightShadowMapVertShader)
DEFINE_TYPE(LightShadowMapFragShader)
DEFINE_TYPE(MaterialBRDFFragShader)
DEFINE_TYPE(BliPhongMaterial)
DEFINE_TYPE(LambertianMaterial)
DEFINE_TYPE(GlobalRayTraceVertShader)

int Object::register_type()
{
    return 0;
}

int Object::get_type_id()
{return type_id;}

int Object::get_instance_id() const
{return instance_id;}

int Object::inst_get_type_id()
{return type_id;}

Object* Object::create_Object()
{
    return new Object();
}

void Object::on_create()
{}

void Object::on_delete()
{}

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
    const auto it = objet_inst_map.find(inst_id);
    return (it != objet_inst_map.end()) ? it->second : nullptr;
}

void TypeFactory::RegisterTypes()
{
    REGISTER_TYPE(Component)
    REGISTER_TYPE(Transform)
    REGISTER_TYPE(FragShader)
    REGISTER_TYPE(TextureFragShader)
    REGISTER_TYPE(Mesh)
    REGISTER_TYPE(RenderNodeComponent)
    REGISTER_TYPE(MeshRender)
    REGISTER_TYPE(Camera)
    REGISTER_TYPE(Texture)
    REGISTER_TYPE(Material)
    REGISTER_TYPE(Resource)
    REGISTER_TYPE(MeshProvider)
    REGISTER_TYPE(LightFragShader)
    REGISTER_TYPE(Light);
    REGISTER_TYPE(SpotLight)
    REGISTER_TYPE(PointLight)
    REGISTER_TYPE(DirectionalLight)
    REGISTER_TYPE(NormalTextureLightFragShader)
    REGISTER_TYPE(VertShader)
    REGISTER_TYPE(DrawCallNodeComponent)
    REGISTER_TYPE(LightShadowMapFragShader)
    REGISTER_TYPE(LightShadowMapVertShader)
    REGISTER_TYPE(MaterialBRDFFragShader)
    REGISTER_TYPE(BliPhongMaterial)
    REGISTER_TYPE(LambertianMaterial)
    REGISTER_TYPE(GlobalRayTraceVertShader)
}



