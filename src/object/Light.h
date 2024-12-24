//
// Created by Lin on 2024/12/23.
//

#ifndef LIGHT_H
#define LIGHT_H
#include "Component.h"
#include "L_math.h"
#include "Object.h"


class Light : public Component
{
    INIT_TYPE(Light, Component)

public:
    Vec3 color;
    float intensity{};
    void on_create() override;
    void on_delete() override;
}
;


class LightManager : public ObjectManger<Light>
{

};

#endif //LIGHT_H
