//
// Created by Lin on 2024/11/16.
//

#include "Component.h"

#include "Context.h"

void Component::on_create()
{
    get_current_ctx()->component_update_manager->on_create_obj(this);
}

void Component::on_delete()
{
    get_current_ctx()->component_update_manager->on_delete_obj(this);
}

void ComponentUpdateManager::on_create_obj(Component* obj)
{
    if (obj->need_update())
    {
        this->objects.push_back(obj);
    }
}

void ComponentUpdateManager::on_delete_obj(Component* obj)
{
    if (obj->need_update())
    {
        this->objects.easy_remove(obj);
    }
}

void ComponentUpdateManager::update(float time)
{
    for (auto object : objects)
    {
        object->update(time);
    }
}

