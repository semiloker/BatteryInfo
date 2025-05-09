#include "../include/overlay_d2d.h"

overlay_D2D* overlay_D2D::instance = nullptr;

overlay_D2D::overlay_D2D() : g_hwnd(nullptr), pBrush(nullptr), m_pRenderTarget(nullptr), m_pFactory(nullptr)
{
    instance = this;
}

overlay_D2D::~overlay_D2D()
{
    DestroyOverlayWindow();
}

void overlay_D2D::DestroyOverlayWindow()
{
    if (g_hwnd && IsWindow(g_hwnd))
    {
        if (pBrush) 
        {
            pBrush->Release();
            pBrush = nullptr;
        }
        DestroyWindow(g_hwnd);
        g_hwnd = nullptr;
    }
}

LRESULT CALLBACK overlay_D2D::StaticWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    overlay_D2D* pThis;

    if (uMsg == WM_NCCREATE)
    {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = static_cast<overlay_D2D*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    }
    else
    {
        pThis = reinterpret_cast<overlay_D2D*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (pThis)
    {
        return pThis->WindowProc(hwnd, uMsg, wParam, lParam);
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void overlay_D2D::CreateOverlayWindow(HINSTANCE hInstance, ID2D1HwndRenderTarget* pRenderTarget, ID2D1Factory* pFactory) 
{
    // Якщо вікно вже створено, просто оновлюємо його
    if (g_hwnd != nullptr && IsWindow(g_hwnd)) 
    {
        ForceTopLeft();
        Render(pRenderTarget, pFactory);
        return;
    }

    const char CLASS_NAME[] = "ov d2d";

    WNDCLASSA wc = {};
    wc.lpfnWndProc = StaticWindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;

    this->m_pRenderTarget = pRenderTarget;
    this->m_pFactory = pFactory;

    if (!GetClassInfoA(hInstance, CLASS_NAME, &wc))
    {
        RegisterClassA(&wc);
    }

    g_hwnd = CreateWindowExA(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
        CLASS_NAME,
        "Transparent Overlay",
        WS_POPUP,
        0, 0,
        300, 300,
        NULL,
        NULL,
        hInstance,
        this
    );

    if (g_hwnd == NULL) 
    {
        MessageBoxA(NULL, "Failed to create window!", "Error", MB_ICONERROR);
        return;
    }

    SetLayeredWindowAttributes(g_hwnd, 0, 255, LWA_ALPHA);

    BOOL compositionEnabled = FALSE;
    HRESULT hr = DwmIsCompositionEnabled(&compositionEnabled);
    if (SUCCEEDED(hr) && compositionEnabled) 
    {
        DWM_BLURBEHIND bb = {0};
        bb.dwFlags = DWM_BB_ENABLE;
        bb.fEnable = TRUE;
        bb.hRgnBlur = NULL;
        
        DwmEnableBlurBehindWindow(g_hwnd, &bb);
    }

    // Встановлюємо вікно завжди зверху і робимо його прозорим для вводу
    SetWindowLong(g_hwnd, GWL_EXSTYLE, 
                 GetWindowLong(g_hwnd, GWL_EXSTYLE) | 
                 WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_NOACTIVATE);

    ForceTopLeft();
    Render(pRenderTarget, pFactory);

    ShowWindow(g_hwnd, SW_SHOW);
    UpdateWindow(g_hwnd);
    
    // ВИДАЛЕНО цикл обробки повідомлень, який блокував головний потік
}

void overlay_D2D::ForceTopLeft()
{
    if (!g_hwnd) return;
    
    int x = 0;
    int y = 0;

    SetWindowPos(g_hwnd, HWND_TOPMOST, x, y, 300, 300, SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

LRESULT CALLBACK overlay_D2D::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
    switch (uMsg) 
    {
    case WM_CREATE:
        return 0;

    case WM_DESTROY:
        return 0;

    case WM_PAINT:
        Render(this->m_pRenderTarget, this->m_pFactory);
        return 0;

    case WM_SIZE:
        Resize(this->m_pRenderTarget);
        ForceTopLeft();
        return 0;
        
    case WM_DISPLAYCHANGE:
        ForceTopLeft();
        return 0;
    case WM_WINDOWPOSCHANGED:
        ForceTopLeft();
        return 0;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

void overlay_D2D::Render(ID2D1HwndRenderTarget* pRenderTarget, ID2D1Factory* pFactory) 
{
    if (!g_hwnd || !IsWindow(g_hwnd)) return;
    
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(g_hwnd, &ps);

    // Якщо RenderTarget ще не створено, створюємо його
    if (!pRenderTarget || !pBrush) {
        RECT rc;
        GetClientRect(g_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
        
        // Властивості для створення RenderTarget
        D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_DEFAULT,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
        );
        
        // Створюємо RenderTarget для вікна, якщо його ще немає
        if (!pRenderTarget && pFactory) {
            HRESULT hr = pFactory->CreateHwndRenderTarget(
                rtProps,
                D2D1::HwndRenderTargetProperties(g_hwnd, size),
                &pRenderTarget
            );
            
            if (SUCCEEDED(hr)) {
                this->m_pRenderTarget = pRenderTarget;
            }
        }

        // Створюємо чорну напівпрозору кисть, якщо її ще немає
        if (pRenderTarget && !pBrush) {
            pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::Black, 0.5f), // Напівпрозорий чорний колір (альфа = 0.5)
                &pBrush
            );
        }
    }

    if (pRenderTarget && pBrush) {
        pRenderTarget->BeginDraw();
        
        // Очищаємо вікно (прозоро)
        pRenderTarget->Clear(D2D1::ColorF(0, 0, 0, 0));
        
        // Розміри вікна
        D2D1_SIZE_F size = pRenderTarget->GetSize();
        
        // Малюємо напівпрозорий чорний квадрат у центрі вікна
        float squareSize = size.width; // Розмір квадрата - половина меншої сторони вікна
        D2D1_RECT_F square = D2D1::RectF(
            (size.width - squareSize) / 2,
            (size.height - squareSize) / 2,
            (size.width + squareSize) / 2,
            (size.height + squareSize) / 2
        );
        
        pRenderTarget->FillRectangle(square, pBrush);
        
        // Завершуємо малювання
        HRESULT hr = pRenderTarget->EndDraw();
        if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET) {
            // Обробка помилок - пересоздать ресурси при необхідності
            if (pBrush) {
                pBrush->Release();
                pBrush = nullptr;
            }
        }
    }

    EndPaint(g_hwnd, &ps);
}

void overlay_D2D::Resize(ID2D1HwndRenderTarget* pRenderTarget) 
{
    if (pRenderTarget && g_hwnd) 
    {
        RECT rc;
        GetClientRect(g_hwnd, &rc);
        
        D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
        
        pRenderTarget->Resize(size);
        
        InvalidateRect(g_hwnd, NULL, FALSE);
    }
}