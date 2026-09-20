#include "win32_window.h"
#include <cmath>

#ifdef WINDOWS
#pragma comment (lib, "user32.lib")
#pragma comment (lib, "gdi32.lib")

static const UINT msg_Show = WM_USER + 0x1;
static const HDC no_bitmap = reinterpret_cast<HDC>(INVALID_HANDLE_VALUE);

struct Show_Data {
    const std::vector<std::string> *captions;
    const std::vector<Image> *images;
};

static int to_byte(float f) {
    int b = static_cast<int>(f * 255.0f);
    if (b < 0)
        b = 0;
    if (b > 255)
        b = 255;
    return b;
}

class Win32_Window::DIB {
public:
    DIB(const Image &src) {
        info.bmiHeader.biSize = sizeof(info.bmiHeader);
        info.bmiHeader.biWidth = LONG(src.width());
        info.bmiHeader.biHeight = LONG(src.height());
        info.bmiHeader.biPlanes = 1;
        info.bmiHeader.biBitCount = 32;
        info.bmiHeader.biCompression = BI_RGB;
        info.bmiHeader.biSizeImage = 0;

        data = new DWORD[src.width() * src.height()];
        for (size_t y = 0; y < src.height(); y++) {
            for (size_t x = 0; x < src.width(); x++) {
                Pixel px = src.pixel(x, y);
                data[x + (src.height() - y - 1)*src.width()]
                    = to_byte(px.r) << 16 | to_byte(px.g) << 8 | to_byte(px.b);
            }
        }
    }

    ~DIB() {
        delete []data;
    }

    BITMAPINFO info;
    DWORD *data;
};


#include <map>

std::map<HWND, Win32_Window *> open_windows;

Win32_Window::Win32_Window() {
    waiting = false;
    wake_sema = CreateSemaphore(NULL, 0, 1, NULL);
    thread = CreateThread(NULL, 0, &thread_main, this, 0, NULL);

    WaitForSingleObject(wake_sema, INFINITE);
}

Win32_Window::~Win32_Window() {
    PostMessage(window, WM_DESTROY, 0, 0);

    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
    CloseHandle(wake_sema);
}

DWORD Win32_Window::thread_main(void *window) {
    Win32_Window *me = reinterpret_cast<Win32_Window *>(window);
    me->main();
    return 0;
}

void Win32_Window::main() {
    create_window();
    ReleaseSemaphore(wake_sema, 1, NULL);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    for (size_t i = 0; i < images.size(); i++)
        delete images[i];

    DeleteObject(background);
}

void Win32_Window::create_window() {
    WNDCLASS wc = {};
    wc.lpfnWndProc = &Win32_Window::window_proc;
    wc.lpszClassName = "Images";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    ATOM cls = RegisterClass(&wc);

    background = CreateSolidBrush(RGB(0xFF, 0xFF, 0xFF));

    RECT pos = { 0, 0, 1000, 500 };
    AdjustWindowRect(&pos, WS_OVERLAPPEDWINDOW, FALSE);
    window = CreateWindowEx(0, (LPCSTR)cls, "Images", WS_OVERLAPPEDWINDOW,
                            CW_USEDEFAULT, CW_USEDEFAULT,
                            pos.right - pos.left, pos.bottom - pos.top,
                            NULL, NULL, GetModuleHandle(NULL), NULL);
    open_windows[window] = this;
    ShowWindow(window, SW_SHOWDEFAULT);
}

LRESULT Win32_Window::window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    std::map<HWND, Win32_Window *>::iterator found = open_windows.find(hwnd);
    if (found != open_windows.end())
        return found->second->on_message(message, wparam, lparam);
    else
        return DefWindowProc(hwnd, message, wparam, lparam);
}

LRESULT Win32_Window::on_message(UINT message, WPARAM wparam, LPARAM lparam) {
    switch (message) {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_PAINT:
        draw();
        break;
    case WM_SIZE:
        InvalidateRect(window, NULL, FALSE);
        break;
    case WM_LBUTTONUP:
        if (waiting) {
            waiting = false;
            ReleaseSemaphore(wake_sema, 1, NULL);
        }
        break;
    case msg_Show: {
        Show_Data *info = reinterpret_cast<Show_Data *>(lparam);

        for (size_t i = 0; i < images.size(); i++)
            delete images[i];
        images.clear();
        for (size_t i = 0; i < info->images->size(); i++)
            images.push_back(new DIB(info->images->at(i)));

        captions = *info->captions;
        while (captions.size() < images.size())
            captions.push_back("");

        waiting = wparam == 1;
        InvalidateRect(window, NULL, FALSE);
        break;
    }
    }

    return DefWindowProc(window, message, wparam, lparam);
}

void Win32_Window::draw_image(HDC dc, RECT rect, DIB *bitmap, const std::string &caption) {
    rect.top += 8;
    rect.top += DrawText(dc, (LPCSTR)caption.c_str(), -1, &rect, DT_TOP | DT_CENTER);
    rect.top += 8;

    int bm_width = bitmap->info.bmiHeader.biWidth;
    int bm_height = bitmap->info.bmiHeader.biHeight;

    float scale = std::min(
        static_cast<float>(rect.right - rect.left) / bm_width,
        static_cast<float>(rect.bottom - rect.top) / bm_height);

    if (scale > 1.0f)
        scale = 1.0f;

    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    int center_x = rect.left + width / 2;
    int center_y = rect.top + height / 2;

    SetStretchBltMode(dc, STRETCH_HALFTONE);
    SetBrushOrgEx(dc, 0, 0, NULL);
    StretchDIBits(
        dc,
        center_x - static_cast<int>(bm_width * scale / 2),
        center_y - static_cast<int>(bm_height * scale / 2),
        static_cast<int>(bm_width * scale),
        static_cast<int>(bm_height * scale),
        0,
        0,
        bm_width,
        bm_height,
        bitmap->data,
        &bitmap->info,
        DIB_RGB_COLORS,
        SRCCOPY);
}

void Win32_Window::draw() {
    PAINTSTRUCT info;
    HDC dc = BeginPaint(window, &info);

    RECT client;
    GetClientRect(window, &client);
    client.left += 8;
    client.right -= 8;
    client.bottom -= 8;

    FillRect(dc, &info.rcPaint, background);

    size_t width = static_cast<size_t>(std::ceil(std::sqrt(static_cast<double>(images.size()))));
    if (width <= 0)
        width = 1;
    size_t height = (images.size() + width - 1) / width;

    int client_width = client.right - client.left;
    int client_height = client.bottom - client.top;

    size_t i = 0;
    for (size_t y = 0; y < height; y++) {
        size_t line_width = width;
        if (images.size() - i < line_width)
            line_width = images.size() - i;

        for (size_t x = 0; x < line_width; x++) {
            RECT r;
            r.left = LONG(client.left + client_width * x / line_width);
            r.right = LONG(r.left + client_width / line_width);
            r.top = LONG(client.top + client_height * y / height);
            r.bottom = LONG(r.top + client_height / height);

            draw_image(dc, r, images[i], captions[i]);
            i++;
        }
    }

    EndPaint(window, &info);
}

void Win32_Window::show_multiple(const std::vector<std::string> &captions, const std::vector<Image> &images, bool pause) {
    if (disable_pause)
        pause = false;
    Show_Data data = { &captions, &images };
    SendMessage(window, msg_Show, pause ? 1 : 0, (LPARAM)&data);
    if (pause)
        WaitForSingleObject(wake_sema, INFINITE);
}

#endif
