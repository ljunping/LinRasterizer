//
// Created by Lin on 2024/11/14.
//

#ifndef WINDOW_H
#define WINDOW_H


#ifdef WINDOW_X11
#include <X11/Xlib.h>
#endif
#include "Color.h"
#include "L_math.h"
#include "Rasterizer.h"

class WindowHandle
{

#ifdef WINDOW_X11
    Display* display;
    ::Window window;
    XImage* image_buff;
    int screen;
#endif
#ifdef WINDOW_COCOA
    void* window;
#endif
public:
    int x,y,w,h;
    Rasterizer* rast;
    Color background_color = BLACK;
    WindowHandle();
    WindowHandle(int x, int y, int w, int h, Rasterizer* rasterizer);
    ~WindowHandle();
    void on_key_event(int key);
    void event_loop();
    void on_resize(int _w, int _h);
    void game_logic(float t);
    void render_scene();
    void open();
    void close();
    void resize(int x, int y) const;
    void move(int x, int y) const;
    void show() const;
    void hide() const;
    void set_pixel(int x, int y, Color color) const;
    void draw_frame_buff() const;
    void clear(Color color) const;
};




#endif //WINDOW_H
