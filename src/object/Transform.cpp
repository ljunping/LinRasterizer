//
// Created by Lin on 2024/12/4.
//

#include "Transform.h"

#include "debug.h"

void Transform::on_create()
{
    dirty_flag = 0;
    local_scale = Vec3::ONE;
    local_pos = Vec3::ZERO;
    parent = nullptr;
    get_current_ctx()->transform_manager->on_create_obj(this);
}

void Transform::on_delete()
{
    get_current_ctx()->transform_manager->on_delete_obj(this);
}

Transform::~Transform()
{
    for (auto child : children)
    {
        DESTROY_OBJECT(child);
    }
}


L_MATH::Mat<float, 4, 4> Transform::get_local_to_global_mat()
{
    Mat44 result = L_MATH::Mat<float, 4, 4>::IDENTITY;
    Transform* transform = this;
    while (transform != nullptr)
    {
        result = transform->get_local_to_parent_mat() * result;
        transform = transform->parent;
    }
    return result;
}

L_MATH::Mat<float, 4, 4> Transform::get_local_to_parent_mat() const
{
    return L_MATH::compose_trs(local_pos, local_euler_angles, local_scale);
}

void Transform::set_local_to_global_mat(const L_MATH::Mat<float, 4, 4>& mat)
{
    auto local_to_parent_mat = get_local_to_parent_mat();
    Mat44 inv_local_to_parent_mat;
    auto local_to_global_mat = get_local_to_global_mat();
    L_MATH::invert_trs_mat(local_to_global_mat, inv_local_to_parent_mat);
    set_local_to_parent_mat(local_to_parent_mat * inv_local_to_parent_mat * mat);
}


void Transform::set_local_to_parent_mat(const L_MATH::Mat<float, 4, 4>& mat)
{
    L_MATH::decompose_trs(mat, local_pos, local_euler_angles, local_scale);
    local_euler_angles *= 180 / PI;
}

void Transform::add_child(Transform* node)
{
    children.push_back(node);
    node->parent = this;
}

void Transform::remove_child(Transform* node)
{
    auto transform = std::find(children.begin(), children.end(), node);
    children.erase(transform);
}

Transform* Transform::get_child(int index) const
{
    return children[index];
}

int Transform::child_count() const
{
    return children.size();
}

void Transform::set_parent(Transform* node)
{
    this->parent = node;
}

L_MATH::Vec<float, 3> Transform::get_global_pos()
{
    Vec3 result;
    Transform* transform = this;
    while (transform != nullptr)
    {
        result += transform->local_pos;
        transform = transform->parent;
    }
    return result;
}

Transform* Transform::get_parent()
{
    return parent;
}



Component* Transform::add_component(int component_type)
{
    RUNTIME_ASSERT(TypeFactory::subclass(component_type,Component::get_type_id()),
                   "component_type must be subclass from Component");
    auto type_inst = (Component*)CREATE_OBJECT(component_type);
    type_inst->scene_node = this;
    components.push_back(type_inst);;
    return type_inst;
}



void Transform::remove_component(Component* c)
{
    auto it = std::find(components.begin(), components.end(), c);
    DESTROY_OBJECT((*it));
    components.erase(it);
}

void Transform::remove_component(int component_type)
{

    for (auto it = components.begin(); it != components.end(); ++it)
    {
        if (IS_SUBCLASS(component_type,(*it)->inst_get_type_id()))
        {
            DESTROY_OBJECT((*it));
            components.erase(it);
            return;
        }
    }
}


Component* Transform::get_component(int component_type) const
{
    for (auto component : components)
    {
        if (IS_SUBCLASS(component_type,component->inst_get_type_id()))
        {
            return component;
        }
    }
    return nullptr;
}