//
// Created by Lin on 2024/11/15.
//

#include "Camera.h"

#include "JobSystem.h"
#include "RasterizerJob.h"
#include "TrianglePrimitive.h"

void Camera::update_view_mat()
{
    view_mat = L_MATH::look_at(look_dir, L_MATH::UP);
}

void Camera::update_projection_mat()
{
    projection_mat = ( is_proj
                         ? L_MATH::project(near, far, fov, ratio)
                         : L_MATH::ortho(near, far, fov, ratio));

    projection_mat = L_MATH::translate(pos * -1) * projection_mat;
}

Camera::Camera(Scene* scene, float near, float far, float fov, float ratio,bool isproj): scene(scene), near(near), far(far),
                                                                                         fov(fov), ratio(ratio),is_proj(isproj)
{
    look_dir = L_MATH::FORWARD;
    pos = Vec3::ZERO;
    update_view_mat();
    update_projection_mat();
}


std::vector<TrianglePrimitive>& Camera::update_proj_triangle_list()
{
    update_view_mat();
    update_projection_mat();
    auto& model_triangles = scene->get_model_triangle_list();
    auto mvp = projection_mat * view_mat * scene->model_matrix;
    if (model_triangles.size() != proj_triangles.size())
    {
        proj_triangles.resize(model_triangles.size());
    }
    this->thread_mvp = mvp;
    if (proj_triangles.size() > THREAD_MVP_SIZE)
    {
        auto job_group = JOB_SYSTEM.create_job_group(0);
        JOB_SYSTEM.alloc_jobs(job_group,
                              0,
                              0,
                              model_triangles.size(),
                              this,
                              execute_mvp_break_huge_triangle,
                              complete_mvp_break_huge_triangle);
        JOB_SYSTEM.submit_job_group(job_group);
        JOB_SYSTEM.wait_job_group_finish(job_group);
    }
    else
    {
        for (int i = 0; i < model_triangles.size(); i++)
        {
            if (model_triangles[i].is_remove())
            {
                proj_triangles[i].discard = true;
                continue;
            }
            proj_triangles[i] = *(model_triangles[i]).data;
            proj_triangles[i].update(mvp);
            if (proj_triangles[i].area() > HUGE_TRIANGLE_THR && HUB_TRIANGLE_BREAK_COUNT_MAX > 0)
            {
                int break_count = HUB_TRIANGLE_BREAK_COUNT_MAX;
                std::vector<TrianglePrimitive> result;
                //不要直接传proj_triangles，会发生扩容导致引用失效
                break_huge_triangle(proj_triangles[i], HUGE_TRIANGLE_THR, result, break_count);
                proj_triangles.insert(proj_triangles.end(), result.begin(), result.end());
            }
        }
    }


    if (build_bvh)
    {
        if (!bvh_tree)
        {
            proj_triangles_ptrs.clear();
            for (auto& proj_triangle : this->proj_triangles)
            {
                if (!proj_triangle.discard)
                {
                    proj_triangles_ptrs.emplace_back(&proj_triangle);
                }
            }
            bvh_tree = new BVHTree(proj_triangles_ptrs);
        }
        // else
        // {
        //     bvh_tree->clear();
        //     bvh_tree->build(primitives_ptrs);
        // }
    }
    return proj_triangles;
}





