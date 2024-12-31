//
// Created by Lin on 2024/12/4.
//

#ifndef TRANSFORM_H
#define TRANSFORM_H
#include <atomic>
#include "Component.h"
#include "Context.h"

enum DirtyFlag
{
    local = 1 << 0,
    parent = 1 << 1
};


class Transform : public Object
{
    INIT_TYPE(Transform, Object)
    Transform* parent{};
    std::vector<Component*> components;
    std::vector<Transform*> children;
public:
    void on_create() override;
    void on_delete() override;
    ~Transform() override;
    Vec3 local_euler_angles{};
    Vec3 local_pos{};
    Vec3 local_scale{};
    void add_child(Transform* node);
    void remove_child(Transform* node);
    Transform* get_child(int index) const;
    int child_count() const;
    void set_parent(Transform* node);
    Vec3 get_global_pos();
    Mat44 get_local_to_global_mat();
    Mat44 get_local_to_parent_mat() const;
    void set_local_to_global_mat(const L_MATH::Mat<float, 4, 4>& mat);
    void set_local_to_parent_mat(const L_MATH::Mat<float, 4, 4>& mat);
    Transform* get_parent();
    Component* add_component(int component_type);
    void remove_component(Component* c);
    void remove_component(int component_type);
    template <typename T>
    void remove_component();
    Component* get_component(int component_type) const;
    template <class T, class ... ARGS>
    T* add_component(ARGS&&... args);
    template <typename T>
    T* get_component();
    template <typename T>
    void get_components(int component_type, std::vector<T*>&);
    template <class T>
    void get_components_in_children(std::vector<T*>& result);
    template <typename T>
    T* get_component_in_parent();
};

template <typename T>
void Transform::remove_component()
{
    this->remove_component(T::get_type_id());
}

template <class T,class... ARGS>
T* Transform::add_component(ARGS&&... args)
{
    auto type_inst = CREATE_OBJECT_BY_TYPE(T,std::forward<ARGS>(args)...);
    type_inst->scene_node = this;
    components.push_back((Component*)type_inst);;
    return type_inst;
}

template <typename T>
T* Transform::get_component()
{
    return (T*)get_component(T::get_type_id());
}

template <typename T>
void Transform::get_components(int component_type, std::vector<T*>& result)
{
    for (auto component : components)
    {
        if (IS_SUBCLASS(component_type, component->inst_get_type_id()))
        {
            result.emplace_back((T*)component);
        }
    }
}

template <typename T>
void Transform::get_components_in_children(std::vector<T*>& result)
{
    this->get_components(T::get_type_id(), result);
    for (auto child : this->children)
    {
        child->get_components_in_children<T>(result);
    }
}



template <typename T>
T* Transform::get_component_in_parent()
{
    auto p = this;
    T* res = nullptr;
    while (p != nullptr && res == nullptr)
    {
        res = p->get_component<T>();
        p = p->parent;
    }
    return res;
}

class TransformManager : public ObjectManger<Transform>
{


};



#endif //TRANSFORM_H
