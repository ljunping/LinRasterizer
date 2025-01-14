//
// Created by Lin on 2024/12/17.
//

#include "Resource.h"

std::unordered_map<std::string, int> Resource:: resource_name2_id;
std::unordered_map<int, std::string> Resource::resource_id2_name;


void Resource::destroy_resource(Resource* resource)
{
    auto resource_id = resource->get_resource_id();
    auto _ptr = resource_id2_name.find(resource_id);
    if (_ptr != resource_id2_name.end())
    {
        resource_name2_id.erase(_ptr->second);
        resource_id2_name.erase(_ptr);
    }
    DESTROY_OBJECT(resource);
}

int Resource::get_resource_id() const
{
    return this->get_instance_id();
}
