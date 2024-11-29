//
// Created by Lin on 2024/11/16.
//

#include "Component.h"
REGISTER_COMPONENT(MeshRender)
int ComponentFactory::register_component(Component*(* func)())
{
    component_creator_map[type_id++] = func;
    return type_id;
}

Component* ComponentFactory::create_component(int component_type_id)
{
    if (component_creator_map.contains(component_type_id))
    {
        return component_creator_map[component_type_id]();
    }
    return nullptr;
}

void ComponentFactory::destroy_component(Component* component)
{
    delete component;
}
