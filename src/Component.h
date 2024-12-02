//
// Created by Lin on 2024/11/16.
//

#ifndef COMPONENT_H
#define COMPONENT_H

#define INIT_COMPONENT(component_name)\
public:\
    static int TYPE_ID;\
    static Component* create_##component_name()\
    {\
        return new component_name();\
    }

#define REGISTER_COMPONENT(component_name)\
int component_name::TYPE_ID=COMPONENT_FACTORY.register_component(component_name::create_##component_name);


#include <string>
#include<unordered_map>

#include "Attribute.h"
#include "FragShader.h"
#include "LuaVertShader.h"
class Component;

class ComponentFactory
{
    int type_id = 0;
    std::unordered_map<int, Component* (*)()> component_creator_map;
    public:
    int register_component(Component* (*func)());
    Component* create_component(int component_type_id);
    void destroy_component(Component* component);
};
inline ComponentFactory COMPONENT_FACTORY;


class Component {
    public:
    virtual ~Component() = default;
    virtual void on_create(){};
    virtual void on_destroy(){};
    virtual void update(float time){};
};

class Mesh
{
    Attributes* attributes;
    public:
    Mesh(Attributes* attributes): attributes(attributes){};
    virtual ~Mesh()
    {
        delete attributes;
    }
};

class MeshRender : public Component
{
    INIT_COMPONENT(MeshRender)
    Mesh* mesh;
    FragShader* lua_frag_shader;
    LuaVertShader* lua_vert_shader;
    ~MeshRender() override
    {
        delete mesh;
        delete lua_frag_shader;
        delete lua_vert_shader;
    }
};

#endif //COMPONENT_H
