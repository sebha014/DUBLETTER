#pragma once
#include "window.h"
#include "os.h"

#ifdef WINDOWS

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

class Win32_Window : public Window {
public:
    Win32_Window();
    ~Win32_Window();

    void show_multiple(const std::vector<std::string> &captions, const std::vector<Image> &images, bool pause) override;

private:
    HANDLE thread;
    HANDLE wake_sema;
    HWND window;

    class DIB;

    std::vector<std::string> captions;
    std::vector<DIB *> images;

    bool waiting;

    HBRUSH background;

    void main();
    void create_window();
    LRESULT on_message(UINT message, WPARAM wparam, LPARAM lparam);
    void draw();

    static void draw_image(HDC dc, RECT rect, DIB *bitmap, const std::string &caption);
    static DWORD WINAPI thread_main(void *);
    static LRESULT CALLBACK window_proc(HWND, UINT, WPARAM, LPARAM);
};

#endif
