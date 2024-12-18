//
// Created by Lin on 2024/12/17.
//

#include "Resource.h"

std::unordered_map<std::string, int> Resource:: resource_name2_id;
std::unordered_map<int, std::string> Resource::resource_id2_name;

void Resource::destroy_resource(const char* name)
{
    std::string _name(name);
    auto resource_id = resource_name2_id[name];
    DESTROY_OBJECT(GET_OBJECT(resource_id));
    resource_id2_name.erase(resource_id);
    resource_name2_id.erase(name);
}

void Resource::destroy_resource(int resource_id)
{
    DESTROY_OBJECT(GET_OBJECT(resource_id));
    auto name = resource_id2_name[resource_id];
    resource_id2_name.erase(resource_id);
    resource_name2_id.erase(name);
}

int Resource::get_resource_id()
{
    return this->get_instance_id();
}
