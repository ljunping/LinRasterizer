//
// Created by Lin on 2024/11/14.
//

#ifndef WINDOW_H
#define WINDOW_H


#ifdef WINDOW_X11
#include <X11/Xlib.h>
#endif
#include "Color.h"
#include "Context.h"
#include "L_math.h"

#define MAX_FRAME_BUFFERS 2
class WindowHandle
{
#ifdef WINDOW_X11
    Display* display;
    ::Window window;
    int screen;
#endif
#ifdef WINDOW_COCOA
    void* window;
#endif

public:
#ifdef WINDOW_X11
    XImage* frame_buffs[MAX_FRAME_BUFFERS];
    XImage* default_frame_buff{};
#endif
    int x, y, w, h;
    Color background_color = BLACK;
    WindowHandle(int x, int y, int w, int h);
    ~WindowHandle();
    void event_loop();
    void on_resize(int _w, int _h);
    void on_key_event(int key);
    void open();
    void close();
    void resize(int x, int y) const;
    void move(int x, int y) const;
    void show() const;
    void hide() const;
    void set_pixel(int x, int y, Color color) const;
    void draw_frame_buff() const;
    void clear(Color color) const;
    Color* get_frame_buff(int frame_buff_id);
};




#endif //WINDOW_H
