#ifndef OVERLAY_BI_H
#define OVERLAY_BI_H

#include <Windows.h>
#include <dwmapi.h>
#include <string>

class overlay_bi
{
public:
    HWND g_hwnd;
    HFONT g_hFont;
    RECT g_textRectPos;
    std::string g_text;
    static overlay_bi* instance;

    int overlay_pos_x = 20;
    int overlay_pos_y = 20;

    bool show_on_screen_display = false;

    overlay_bi(HWND g_hwnd, HFONT g_hFont, RECT g_textRectPos, std::string g_text);
    
    ~overlay_bi();

    void CreateOverlayWindow(HINSTANCE hInstance, HWND parentHwnd = NULL);
    void DestroyOverlayWindow();
    void ForceTopMost();
    void RenderText(HWND hwnd);
    void UpdateText(const std::string& newText);
    void UpdatePosition();
        
    static LRESULT CALLBACK StaticWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);    
    LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif