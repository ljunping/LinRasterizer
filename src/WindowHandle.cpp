
#include "WindowHandle.h"
#include <thread>
#include "Color.h"
#include "Context.h"
#include "debug.h"
#include "DrawUtil.h"
#include "JobSystem.h"


#if WINDOW_X11

#include <iostream>
#include <X11/keysym.h>
#include<X11/Xlib.h>



WindowHandle::WindowHandle(int x, int y, int w, int h): x(x), y(y), w(w), h(h)
{
    for (int i = 0; i < MAX_FRAME_BUFFERS; ++i)
    {
        frame_buffs[i] = nullptr;
    }
}

WindowHandle::~WindowHandle() {
    close();
}

void WindowHandle::on_key_event(int keysym)
{
    get_current_ctx()->on_key_event(keysym);
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
    if (default_frame_buff != nullptr)
    {
        default_frame_buff->f.destroy_image(default_frame_buff);
    }
    default_frame_buff = XCreateImage(display, DefaultVisual(display, screen), DefaultDepth(display, screen),
                              ZPixmap, 0, (char*)malloc(w * h * 4), w, h, 32, 0);
    std::fill((int*)default_frame_buff->data, (int*)default_frame_buff->data + w * h, BLACK);
    frame_buffs[0] = default_frame_buff;
    for (int i = 1; i < MAX_FRAME_BUFFERS; ++i)
    {
        if (frame_buffs[i] != nullptr)
        {
            frame_buffs[i]->f.destroy_image(frame_buffs[i]);
        }
        frame_buffs[i] = XCreateImage(display, DefaultVisual(display, screen), DefaultDepth(display, screen),
                                  ZPixmap, 0, (char*)malloc(w * h * 4), w, h, 32, 0);
        std::fill((int*)frame_buffs[i]->data, (int*)frame_buffs[i]->data + w * h, BLACK);
    }
    get_current_ctx()->on_window_resize(w, h);
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
    auto* ctx = new Context(this);
    set_current_ctx(ctx);
    ctx->init();
    on_resize(w, h);
}

void WindowHandle::close() {
    if (display)
    {
        for (auto frame_buff : frame_buffs)
        {
            frame_buff->f.destroy_image(frame_buff);
        } 
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
    XPutImage(display, window, DefaultGC(display, screen), default_frame_buff, 0, 0, 0, 0, w, h);
}


void WindowHandle::clear(Color color) const
{
    if (default_frame_buff != nullptr)
    {
        std::fill((int*)default_frame_buff->data, (int*)default_frame_buff->data + w * h, color);
    }
    // XSetWindowBackground(display, window, color);
    // XClearWindow(display, window);
}

Color* WindowHandle::get_frame_buff(int frame_buff_id)
{
    return (Color*)frame_buffs[frame_buff_id]->data;
}


#endif




