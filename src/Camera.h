//
// Created by Lin on 2024/11/15.
//

#ifndef CAMERA_H
#define CAMERA_H
#include "L_math.h"
#include "Scene.h"
#include "TrianglePrimitive.h"



class Camera
{
    bool is_proj;
public:
    Scene* scene;
    Mat44 view_mat;
    Mat44 projection_mat;
    std::mutex proj_triangles_mutex;
    std::vector<TrianglePrimitive> proj_triangles;
    void update_view_mat();
    void update_projection_mat();
    Vec3 pos;
    Mat44 thread_mvp;
    Vec3 look_dir = L_MATH::FORWARD;
    float near, far, fov, ratio;
    Camera(Scene* scene,float near,float far,float fov,float ratio,bool isProj);
    std::vector<TrianglePrimitive>& update_proj_triangle_list();
};



#endif //CAMERA_H
