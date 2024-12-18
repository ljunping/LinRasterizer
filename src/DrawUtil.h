//
// Created by Lin on 2024/11/23.
//

#ifndef DRAWUTIL_H
#define DRAWUTIL_H
#include "BresenhamLine.h"
#include "Mesh.h"
#include "Color.h"
inline Mesh* generate_sphere(float radius, int stacks, int slices, const Mat44& mat)
{
    float* vert_buff = new float[5 * (stacks + 1) * (slices + 1)];
    // Generate vertices
    for (int stack = 0; stack <= stacks; ++stack)
    {
        float phi = M_PI * stack / stacks; // Latitude angle (0 to π)
        for (int slice = 0; slice <= slices; ++slice)
        {
            float theta = 2 * M_PI * slice / slices; // Longitude angle (0 to 2π)
            float x = radius * sin(phi) * cos(theta);
            float y = radius * sin(phi) * sin(theta);
            float z = radius * cos(phi);
            auto var = (Vec4)(mat * Vec4{x, y, z, 1});
            auto var1 = var / var[3];
            vert_buff[5 * (stack * (slices + 1) + slice)] = var1[0];
            vert_buff[5 * (stack * (slices + 1) + slice) + 1] = var1[1];
            vert_buff[5 * (stack * (slices + 1) + slice) + 2] = var1[2];
            vert_buff[5 * (stack * (slices + 1) + slice) + 3] = stack*1.0 / stacks;
            vert_buff[5 * (stack * (slices + 1) + slice) + 4] = slice*1.0 / slices;
        }
    }
    SHARE_PTR<float[]> share_vert_buff(vert_buff);
    auto _Attributes = CREATE_OBJECT_BY_TYPE(Mesh, share_vert_buff, 5 * (stacks + 1) * (slices + 1));
    _Attributes->bind_attribute(POS,3, 0, 5);
    _Attributes->bind_attribute(COLOR,2, 3, 5);
    std::vector<int> eb0;
    // Generate triangles
    for (int stack = 0; stack < stacks; ++stack)
    {
        for (int slice = 0; slice < slices; ++slice)
        {
            // Compute indices for the four vertices of the current quad
            int i1 = stack * (slices + 1) + slice; // Top-left
            int i2 = i1 + 1; // Top-right
            int i3 = i1 + (slices + 1); // Bottom-left
            int i4 = i3 + 1; // Bottom-right
            // Two triangles per quad
            eb0.emplace_back(i1);
            eb0.emplace_back(i3);
            eb0.emplace_back(i2);

            eb0.emplace_back(i3);
            eb0.emplace_back(i4);
            eb0.emplace_back(i2);
        }
    }
    _Attributes->ebo = std::move(eb0);
    return _Attributes;
}

inline  void generate_quad(const Mat44& mat, std::vector<TrianglePrimitive>& result)
{
    float* vert_buff = new float[4 * 6];
    Vec3 vert[4] = {
        Vec3{-0.5, -0.5, -1},
        Vec3{0.5, -0.5, -1},
        Vec3{0.5, 0.5, -1},
        Vec3{-0.5, 0.5, -1},
    };
    for (int i = 0; i < 4; ++i)
    {
        auto v = Vec4(mat * Vec4(vert[i], 1));
        v /= v[3];
        vert_buff[6 * i] = v[0];
        vert_buff[6 * i + 1] = v[1];
        vert_buff[6 * i + 2] = v[2];
        vert_buff[6 * i + 3] = 1;
        vert_buff[6 * i + 4] = 1;
        vert_buff[6 * i + 5] = 1;
    }
    SHARE_PTR<float[]> share_vert_buff(vert_buff);

    auto _Attributes = CREATE_OBJECT_BY_TYPE(Mesh, share_vert_buff, 24);
    _Attributes->bind_attribute(POS,3, 0, 6);
    _Attributes->bind_attribute(COLOR,3, 3, 6);
    std::vector<int> ebo = {0, 1, 2, 2, 3, 0};
    _Attributes->generate_triangles(ebo, result);
}
struct DrawLineInfo
{
    int x0, y0, x1, y1;
    Color color;
};

inline void draw_line(Context* ctx, Color* buff, void* data)
{
    auto w = ctx->window_handle->w;
    auto h = ctx->window_handle->h;
    auto line = (DrawLineInfo*)data;
    auto bresenham_line = BresenhamLine(line->x0, line->y0, line->x1, line->y1);
    int x, y,mx,my;
    x = line->x0;
    y = line->y0;
    buff[y * w + x] = line->color;
    while (bresenham_line.has_next())
    {
        bresenham_line.next_point(x, y, mx, my);
        buff[y * w + x] = line->color;
    }
}

#endif //DRAWUTIL_H
