#ifndef OVERLAY_D2D_H
#define OVERLAY_D2D_H

#include <windows.h>
#include <d2d1.h>
#include <dwmapi.h>

class overlay_D2D
{
public:

    overlay_D2D();
    ~overlay_D2D();

    static overlay_D2D* GetInstance();
    void CreateOverlayWindow(HINSTANCE hInstance, ID2D1HwndRenderTarget* pRenderTarget, ID2D1Factory* pFactory);
    void DestroyOverlayWindow();
    void Render(ID2D1HwndRenderTarget* pRenderTarget, ID2D1Factory* pFactory);
    void Resize(ID2D1HwndRenderTarget* pRenderTarget);
    void ForceTopLeft();

    static overlay_D2D* instance;
    HWND g_hwnd;
private:
    ID2D1SolidColorBrush* pBrush;

    ID2D1HwndRenderTarget* m_pRenderTarget = nullptr;
    ID2D1Factory* m_pFactory = nullptr;


    static LRESULT CALLBACK StaticWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OVERLAY_D2D_H