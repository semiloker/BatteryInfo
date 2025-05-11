#ifndef OVERLAY_D2D_H
#define OVERLAY_D2D_H

#include <windows.h>
#include <d2d1.h>
#include <dwmapi.h>
#include <algorithm>

template<class Interface>
inline void SafeRelease(Interface **ppInterfaceToRelease)
{
    if (*ppInterfaceToRelease != nullptr)
    {
        (*ppInterfaceToRelease)->Release();
        (*ppInterfaceToRelease) = nullptr;
    }
}

class overlay_D2D
{
public:
    overlay_D2D();
    ~overlay_D2D();

    static overlay_D2D* GetInstance();
    bool CreateOverlayWindow(HINSTANCE hInstance, int width, int height);
    void DestroyOverlayWindow();
    void Render();
    void Resize(UINT width, UINT height);

    HWND GetWindowHandle() const { return g_hwnd; }

private:
    void ForceTopLeft(int width, int height);
    bool InitializeDirect2D();
    void CleanupDirect2D();

    static LRESULT CALLBACK StaticWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    static overlay_D2D* instance;
    HWND g_hwnd;

    ID2D1Factory* m_pFactory;
    ID2D1HwndRenderTarget* m_pRenderTarget;
    ID2D1SolidColorBrush* pBrush;

    int m_width;
    int m_height;
};

#endif // OVERLAY_D2D_H