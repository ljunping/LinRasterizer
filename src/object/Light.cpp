//
// Created by Lin on 2024/12/23.
//

#include "Light.h"

#include "Context.h"

void Light::on_create()
{
    Component::on_create();
    get_current_ctx()->light_manager->on_create_obj(this);
}

void Light::on_delete()
{
    get_current_ctx()->light_manager->on_delete_obj(this);
    Component::on_delete();
}

int LightManager::get_light_count()
{
    return this->get_objects().valid_size();
}
