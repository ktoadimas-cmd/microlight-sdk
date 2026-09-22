#pragma once
#include "gl.h"
#include <string>
#include <unordered_map>

namespace ml {

class Window {
public:
    Window(int width, int height, const std::string& title);
    ~Window();

    bool PumpMessages();
    void SwapBuffers();

    HWND Handle() const { return hwnd_; }
    int  Width()  const { return width_; }
    int  Height() const { return height_; }
    bool ShouldClose() const { return shouldClose_; }

    // === Input ===
    bool IsKeyDown(int vk) const;
    bool WasKeyPressed(int vk) const;
    bool WasLeftClicked();
    bool WasRightClicked();

    void GetMouseDelta(float& dx, float& dy);
    void SetMouseCaptured(bool on);
    bool IsMouseCaptured() const { return mouseCaptured_; }

    // === Text input ===
    void BeginTextInput(const std::string& initial = "");
    void EndTextInput();
    bool IsTextInputActive() const { return textInputActive_; }
    const std::string& TextInputValue() const { return textInput_; }
    bool TextInputCommitted();     // edge: Enter нажат
    bool TextInputCancelled();     // edge: Esc нажат

private:
    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    LRESULT HandleMessage(HWND, UINT, WPARAM, LPARAM);
    void CenterMouse();

    HWND   hwnd_        = nullptr;
    HDC    hdc_         = nullptr;
    HGLRC  hglrc_       = nullptr;
    int    width_       = 0;
    int    height_      = 0;
    bool   shouldClose_ = false;

    std::unordered_map<int, bool> keys_;
    std::unordered_map<int, bool> keysPressed_;
    float  mouseDX_ = 0, mouseDY_ = 0;
    bool   mouseCaptured_ = false;
    bool   firstMouse_    = true;
    bool   leftClicked_   = false;
    bool   rightClicked_  = false;
    POINT  lastMouse_     = {0,0};

    // text input
    bool        textInputActive_ = false;
    std::string textInput_;
    bool        textInputCommit_ = false;
    bool        textInputCancel_ = false;
};

} // namespace ml
