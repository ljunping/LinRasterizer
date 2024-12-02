
#include "WindowHandle.h"
#include <thread>
#include "Color.h"
#include "debug.h"
#include "DrawUtil.h"
#include "JobSystem.h"


#if WINDOW_X11

#include <iostream>
#include <X11/keysym.h>
#include<X11/Xlib.h>
const int target_fps = 60; // 目标帧率
const int frame_duration = 1000 / target_fps; // 每帧持续时间（毫秒）


WindowHandle::WindowHandle() : display(nullptr), window(0),screen(0), x(0), y(0), w(800), h(600) {}

WindowHandle::WindowHandle(int x, int y, int w, int h, Rasterizer* rasterizer): x(x), y(y), w(w), h(h), rast(rasterizer)
{
}


WindowHandle::~WindowHandle() {
    close();
}

void WindowHandle::on_key_event(int keysym)
{
    static int __ID = 0;
    if (keysym == XK_1) // 如果按下的是1
    {
        rast->enable_ray_cast = !rast->enable_ray_cast;
    }
    if (keysym == XK_2) // 如果按下的是1
    {
        rast->camera->build_bvh = !rast->camera->build_bvh;
    }

    if (keysym == XK_3) // 如果按下的是2
    {
        rast->scene->clear();
        std::vector<TrianglePrimitive> primitives;
        auto attributes = generate_sphere(3, 10, 10, L_MATH::translate(Vec3{0, 0, -3.5}));
        attributes->generate_triangles(primitives);
        rast->scene->add_origin_triangle(primitives[9]);
        __ID++;
    }
    if (keysym == XK_4) // 如果按下的是2
    {
        rast->scene->clear();
        std::vector<TrianglePrimitive> primitives;
        auto attributes = generate_sphere(3, 10, 10, L_MATH::translate(Vec3{0, 0, -3.5}));
        rast->scene->add_attributes(attributes);
        __ID = 0;
    }

    if (keysym == XK_5) // 如果按下的是2
    {
        rast->scene->clear();
        std::vector<TrianglePrimitive> primitives;
        generate_quad(L_MATH::scale(Vec3{2, 2, 2}), primitives);
        rast->scene->add_origin_triangles(primitives);
        __ID = 0;
    }
    if (keysym == XK_6) // 如果按下的是2
    {
        HUB_TRIANGLE_BREAK_COUNT_MAX = HUB_TRIANGLE_BREAK_COUNT_MAX == 0 ? 2 : 0;
    }
    if (keysym == XK_7)
    {
        rast->scene->clear();
        float* vert_buff = new float[18]{
            -3.5, -0.5, -1, 1, 0, 0,
            3.5, -0.5, -1, 0, 1, 0,
            0, 0.5, -1, 0, 0, 1
        };
        auto _Attributes = new Attributes(vert_buff, 18);
        _Attributes->bind_attribute(POS,3, 0, 6);
        _Attributes->bind_attribute(COLOR,3, 3, 6);
        std::vector<TrianglePrimitive> primitives;
        _Attributes->generate_triangles(primitives);
        rast->scene->add_origin_triangles(primitives);
    }
    if(keysym==XK_8)
    {
        float z=-500;
        float r=200;
        float *vert_buff = new float[15]{
            -r, -r, z, 0, 0,
            r, -r, z, 1, 0,
            0, r, z, 0.5, 1
        };
        auto _Attributes = new Attributes(vert_buff, 15);
        _Attributes->bind_attribute(POS,3, 0, 5);
        _Attributes->bind_attribute (UV, 2, 3, 5);
        std::vector<TrianglePrimitive> primitives;
        _Attributes->generate_triangles(primitives);
        rast->scene->add_origin_triangles(primitives);
    }
}
void WindowHandle::event_loop()
{
    XEvent event;
    while (XPending(display))
    {
        XNextEvent(display, &event);
        // 处理事件逻辑
        if (event.type == ConfigureNotify) {
            XConfigureEvent xce = event.xconfigure;
            x = xce.x;
            y = xce.y;
            if (w != xce.width || h != xce.height)
            {
                on_resize(xce.width, xce.height);
            }
        }
        if (event.type == KeyPress)
        {
            // 获取按键事件
            KeySym keysym = XLookupKeysym(&event.xkey, 0); // 获取 keysym，0 表示不使用修饰键

            on_key_event(keysym);
        }
    }
}

void WindowHandle::on_resize(int _w, int _h)
{
    w = _w;
    h = _h;
    if (image_buff != nullptr)
    {
        image_buff->f.destroy_image(image_buff);
    }
    image_buff = XCreateImage(display, DefaultVisual(display, screen), DefaultDepth(display, screen),
                              ZPixmap, 0, (char*)malloc(w * h * 4), w, h, 32, 0);
    std::fill((int*)image_buff->data, (int*)image_buff->data + w * h, BLACK);
    rast->on_window_resize(w, h);
    rast->camera->ratio = w * 1.0 / h;
}

void WindowHandle::game_logic(float t)
{

    // static auto camera_pos = rast->camera->pos;
    // static auto look_dir = rast->camera->look_dir;
    // rast->camera->pos = (camera_pos + Vec3({
    //     0.1f * (float)::sin(2 * PI * t), 0.1f * (float)::cos(2 * PI * t), 0
    // }));
    // rast->camera->look_dir = (Vec3(Vec4(L_MATH::rotate(L_MATH::UP, 10) * Vec4(look_dir, 1))));

}

void WindowHandle::render_scene()
{
    rast->draw_begin();
    rast->clear_color(reinterpret_cast<Color*>(image_buff->data), background_color);
    rast->raster_scene();
    rast->render_fragment(reinterpret_cast<Color*>(image_buff->data));
    rast->draw_end();
    rast->draw_after_scene(reinterpret_cast<Color*>(image_buff->data));
    this->draw_frame_buff();
}


void WindowHandle::open() {
    display = XOpenDisplay(NULL);
    if (display == NULL) {
        std::cerr << "无法打开显示" << std::endl;
        return;
    }

    screen = DefaultScreen(display);
    window = XCreateSimpleWindow(display, RootWindow(display, screen), x, y, w, h, 1,
                                 BlackPixel(display, screen), WhitePixel(display, screen));
    XSelectInput(display, window, ExposureMask | KeyPressMask | StructureNotifyMask);
    XMapWindow(display, window);
    on_resize(w, h);
    auto start_time = std::chrono::high_resolution_clock::now();
    float t = 0;
    while (true)
    {
        TIME_RUN_BEGIN()

        event_loop();
        game_logic(t);
        render_scene();

        TIME_RUN_END()
        t = std::chrono::duration_cast<std::chrono::milliseconds>(frame_end - start_time).count() * 1.0f / 1000.f;

        PRINT_RUN_TIME(MAIN_LOOP)
        // 计算渲染时间并休眠以限制帧率
        if (elapsed_time < frame_duration)
        {
            // std::this_thread::sleep_for(std::chrono::milliseconds(frame_duration - elapsed_time));
        }
    }
}

void WindowHandle::close() {
    if (display)
    {
        XDestroyWindow(display, window);
        XCloseDisplay(display);
        XCloseDisplay(display);
        display = nullptr;
    }
}

void WindowHandle::resize(int width, int height) const
{
    XResizeWindow(display, window, width, height);
}

void WindowHandle::move(int x, int y) const
{
    XMoveWindow(display, window, x, y);
}

void WindowHandle::show() const
{
    XMapWindow(display, window);
}

void WindowHandle::hide() const
{
    XUnmapWindow(display, window);
}

void WindowHandle::set_pixel(int x, int y, Color color) const
{
    XSetForeground(display, DefaultGC(display, screen), color);
    XDrawPoint(display, window, DefaultGC(display, screen), x, y);
}

void WindowHandle::draw_frame_buff() const
{
    XPutImage(display, window, DefaultGC(display, screen), image_buff, 0, 0, 0, 0, w, h);
}

void WindowHandle::clear(Color color) const
{
    if (image_buff)
    {
        std::fill((int*)image_buff->data, (int*)image_buff->data + w * h, color);
    }
    // XSetWindowBackground(display, window, color);
    // XClearWindow(display, window);
}
#endif




