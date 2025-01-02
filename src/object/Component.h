//
// Created by Lin on 2024/11/16.
//

#ifndef COMPONENT_H
#define COMPONENT_H
#include "Object.h"

class Transform;

class Component : public Object
{
    INIT_TYPE(Component, Object)
public:
    Transform* scene_node{};
    ~Component() override = default;
    void on_create() override;
    void on_delete() override;
    virtual bool need_update() const
    {
        return false;
    };

    virtual void update(float delta_time)
    {
    };
};

class ComponentUpdateManager : public ObjectManger<Component>
{
public:
    void on_create_obj(Component* obj) override;
    void on_delete_obj(Component* obj) override;
    void update(float time);
}
;





#endif //COMPONENT_H
