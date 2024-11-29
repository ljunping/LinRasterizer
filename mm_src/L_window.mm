#ifdef WINDOW_COCOA
#include <Cocoa/Cocoa.h>
#include "L_window.h"

@interface WindowDelegate : NSObject <NSWindowDelegate>
{
    Window* owner;
}
- (instancetype)initWithOwner:(Window*)owner;
@end


// 默认构造函数
Window::Window() : x(100), y(100), w(800), h(600) {
}

@implementation WindowDelegate

- (instancetype)initWithOwner:(Window*)owner {
    self = [super init];
    if (self) {
        self->owner = owner;
    }
    return self;
}

- (BOOL)windowShouldClose:(id)sender {
    if (owner) {
        owner->close();
    }
    return YES;
}

@end
// 析构函数
Window::~Window() {
    close();
}



// 打开窗口
void Window::open() {
    NSRect frame = NSMakeRect(x, y, w, h);
    window = [[NSWindow alloc] initWithContentRect:frame
                                         styleMask:(NSWindowStyleMaskTitled |
                                                    NSWindowStyleMaskClosable |
                                                    NSWindowStyleMaskResizable)
                                           backing:NSBackingStoreBuffered
                                             defer:NO];
    [window setTitle:@"CLion Window"];
    [window makeKeyAndOrderFront:nil];
    [window setTitle:@"CLion Window"];
    WindowDelegate *delegate = [[WindowDelegate alloc] initWithOwner:this];
    [window setDelegate:delegate];
    [window makeKeyAndOrderFront:nil];

}

void Window::close() {
    if (window) {
        [window close];
        window = nil;
        [NSApp terminate:nil];  // 终止应用程序
    }
}


// 调整窗口大小
void Window::resize(int width, int height) {
    if (window) {
        NSRect frame = [window frame];
        frame.size = NSMakeSize(width, height);
        [window setFrame:frame display:YES animate:YES];
    }
}

// 移动窗口
void Window::move(int x, int y) {
    if (window) {
        NSRect frame = [window frame];
        frame.origin = NSMakePoint(x, y);
        [window setFrameOrigin:frame.origin];
    }
}

// 显示窗口
void Window::show() {
    [window orderFront:nil];
}

// 隐藏窗口
void Window::hide() {
    [window orderOut:nil];
}

// 设置像素颜色（需要自定义绘制逻辑）
void Window::setPixel(int x, int y, const Vec4& color) {
    // 使用 NSView 的 draw 方法或自定义绘制实现
    // Cocoa 不支持直接操作像素级的绘制，通常需自定义 NSView
}

// 清除窗口内容并填充颜色
void Window::clear(const Vec4& color) {
    if (window) {
        NSColor *nsColor = [NSColor colorWithRed:color[0] green:color[1] blue:color[2] alpha:color[3]];
        [window setBackgroundColor:nsColor];
    }
}

// 显示标题栏
void Window::showTitle() {
    [window setStyleMask:([window styleMask] | NSWindowStyleMaskTitled)];
}

// 隐藏标题栏
void Window::hideTitle() {
    [window setStyleMask:([window styleMask] & ~NSWindowStyleMaskTitled)];
}

void LoopWindow(int x, int y, int width, int height) {
  @autoreleasepool {
        Window win(x,y,width,height);
        win.open();
        win.show();
        [NSApp run];  // 启动事件循环
    }
}
#endif