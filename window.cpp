#include "window.h"
#include <stdexcept>
#include <iostream>

namespace ml {

static const char* kClassName = "MicrolightWindow";

typedef HGLRC (WINAPI *PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC, HGLRC, const int*);
typedef BOOL  (WINAPI *PFNWGLSWAPINTERVALEXTPROC)(int);

#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB  0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001

LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    Window* self = nullptr;
    if (msg == WM_NCCREATE) {
        CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lp);
        self = reinterpret_cast<Window*>(cs->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        self->hwnd_ = hwnd;
    } else {
        self = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }
    if (self) return self->HandleMessage(hwnd, msg, wp, lp);
    return DefWindowProc(hwnd, msg, wp, lp);
}

LRESULT Window::HandleMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_CLOSE:
            shouldClose_ = true;
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE: {
            width_  = LOWORD(lp);
            height_ = HIWORD(lp);
            if (hglrc_) glViewport(0, 0, width_, height_);
            return 0;
        }
        case WM_KEYDOWN: {
            if (!(lp & (1 << 30))) keysPressed_[wp] = true;
            keys_[wp] = true;
            if (textInputActive_) {
                if (wp == VK_RETURN) { textInputCommit_ = true; }
                else if (wp == VK_ESCAPE) { textInputCancel_ = true; }
                else if (wp == VK_BACK) {
                    if (!textInput_.empty()) textInput_.pop_back();
                }
            }
            return 0;
        }
        case WM_KEYUP:
            keys_[wp] = false;
            return 0;
        case WM_CHAR: {
            if (textInputActive_) {
                // 8=backspace, 27=esc, 13=enter — обрабатываются в WM_KEYDOWN
                if (wp >= 32 && wp < 127) {
                    textInput_ += (char)wp;
                }
            }
            return 0;
        }
        case WM_LBUTTONDOWN:
            leftClicked_ = true;
            return 0;
        case WM_RBUTTONDOWN:
            rightClicked_ = true;
            return 0;
        case WM_MOUSEMOVE:
            if (mouseCaptured_) {
                POINT p; GetCursorPos(&p);
                if (!firstMouse_) {
                    mouseDX_ += (float)(p.x - lastMouse_.x);
                    mouseDY_ += (float)(p.y - lastMouse_.y);
                }
                firstMouse_ = false;
                CenterMouse();
            }
            return 0;
        case WM_KILLFOCUS:
            SetMouseCaptured(false);
            keys_.clear();
            return 0;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

Window::Window(int width, int height, const std::string& title)
    : width_(width), height_(height)
{
    HINSTANCE hInst = GetModuleHandle(nullptr);

    WNDCLASSA wc = {};
    wc.style         = CS_OWNDC;
    wc.lpfnWndProc   = Window::WndProc;
    wc.hInstance     = hInst;
    wc.lpszClassName = kClassName;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);

    if (!RegisterClassA(&wc)) throw std::runtime_error("RegisterClass failed");

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);

    hwnd_ = CreateWindowExA(
        0, kClassName, title.c_str(),
        WS_POPUP | WS_VISIBLE,
        0, 0,
        screenW, screenH,
        nullptr, nullptr, hInst, this
    );

    width_  = screenW;
    height_ = screenH;
    if (!hwnd_) throw std::runtime_error("CreateWindow failed");

    hdc_ = GetDC(hwnd_);

    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize      = sizeof(pfd);
    pfd.nVersion   = 1;
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;

    int pf = ChoosePixelFormat(hdc_, &pfd);
    if (!pf) throw std::runtime_error("ChoosePixelFormat failed");
    if (!SetPixelFormat(hdc_, pf, &pfd)) throw std::runtime_error("SetPixelFormat failed");

    HGLRC tmpCtx = wglCreateContext(hdc_);
    if (!tmpCtx) throw std::runtime_error("wglCreateContext (temp) failed");
    wglMakeCurrent(hdc_, tmpCtx);

    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB =
        (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
    if (!wglCreateContextAttribsARB) {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(tmpCtx);
        throw std::runtime_error("wglCreateContextAttribsARB not available");
    }

    const int attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };

    hglrc_ = wglCreateContextAttribsARB(hdc_, nullptr, attribs);
    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(tmpCtx);

    if (!hglrc_) throw std::runtime_error("wglCreateContextAttribsARB failed");
    if (!wglMakeCurrent(hdc_, hglrc_)) throw std::runtime_error("wglMakeCurrent failed");

    PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT =
        (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
    if (wglSwapIntervalEXT) wglSwapIntervalEXT(1);
}

Window::~Window() {
    if (hglrc_) {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(hglrc_);
    }
    if (hdc_ && hwnd_) ReleaseDC(hwnd_, hdc_);
    if (hwnd_) DestroyWindow(hwnd_);
}

bool Window::PumpMessages() {
    MSG msg;
    while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) shouldClose_ = true;
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return !shouldClose_;
}

void Window::SwapBuffers() {
    ::SwapBuffers(hdc_);
    keysPressed_.clear();
}

bool Window::IsKeyDown(int vk) const {
    auto it = keys_.find(vk);
    return it != keys_.end() && it->second;
}

bool Window::WasKeyPressed(int vk) const {
    auto it = keysPressed_.find(vk);
    return it != keysPressed_.end() && it->second;
}

bool Window::WasLeftClicked() {
    bool v = leftClicked_;
    leftClicked_ = false;
    return v;
}

bool Window::WasRightClicked() {
    bool v = rightClicked_;
    rightClicked_ = false;
    return v;
}

void Window::GetMouseDelta(float& dx, float& dy) {
    dx = mouseDX_;
    dy = mouseDY_;
    mouseDX_ = mouseDY_ = 0;
}

void Window::SetMouseCaptured(bool on) {
    if (on == mouseCaptured_) return;
    mouseCaptured_ = on;
    if (on) {
        ShowCursor(FALSE);
        SetCapture(hwnd_);
        firstMouse_ = true;
        GetCursorPos(&lastMouse_);
        CenterMouse();
    } else {
        ShowCursor(TRUE);
        ReleaseCapture();
    }
}

void Window::CenterMouse() {
    RECT r;
    GetClientRect(hwnd_, &r);
    POINT c;
    c.x = (r.right  - r.left) / 2;
    c.y = (r.bottom - r.top ) / 2;
    ClientToScreen(hwnd_, &c);
    SetCursorPos(c.x, c.y);
    lastMouse_ = c;
}

// === Text input ===
void Window::BeginTextInput(const std::string& initial) {
    textInputActive_ = true;
    textInput_ = initial;
    textInputCommit_ = false;
    textInputCancel_ = false;
    SetMouseCaptured(false);
}

void Window::EndTextInput() {
    textInputActive_ = false;
    textInputCommit_ = false;
    textInputCancel_ = false;
}

bool Window::TextInputCommitted() {
    bool v = textInputCommit_;
    textInputCommit_ = false;
    return v;
}

bool Window::TextInputCancelled() {
    bool v = textInputCancel_;
    textInputCancel_ = false;
    return v;
}

} // namespace ml
