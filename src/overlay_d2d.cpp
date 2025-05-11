#include "../include/overlay_d2d.h"

overlay_D2D* overlay_D2D::instance = nullptr;

overlay_D2D::overlay_D2D() : 
    g_hwnd(nullptr), 
    m_pFactory(nullptr), 
    m_pRenderTarget(nullptr), 
    pBrush(nullptr),
    m_width(300),
    m_height(300)
{
    instance = this;
}

overlay_D2D::~overlay_D2D()
{
    DestroyOverlayWindow();
    SafeRelease(&m_pFactory);
    instance = nullptr;
}

overlay_D2D* overlay_D2D::GetInstance()
{
    if (!instance)
    {
        instance = new overlay_D2D();
    }
    return instance;
}

bool overlay_D2D::InitializeDirect2D()
{
    if (!m_pFactory)
    {
        HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pFactory);
        if (FAILED(hr))
        {
            MessageBoxA(NULL, "Failed to create D2D1Factory!", "Error", MB_ICONERROR);
            return false;
        }
    }

    if (g_hwnd && m_pFactory && !m_pRenderTarget)
    {
        RECT rc;
        GetClientRect(g_hwnd, &rc);
        D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

        D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_DEFAULT,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
        );

        HRESULT hr = m_pFactory->CreateHwndRenderTarget(
            rtProps,
            D2D1::HwndRenderTargetProperties(g_hwnd, size),
            &m_pRenderTarget
        );

        if (FAILED(hr))
        {
            MessageBoxA(NULL, "Failed to create HwndRenderTarget!", "Error", MB_ICONERROR);
            SafeRelease(&m_pRenderTarget);
            return false;
        }

        hr = m_pRenderTarget->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::AliceBlue, 0.7f),
            &pBrush
        );
        if (FAILED(hr))
        {
             MessageBoxA(NULL, "Failed to create solid color brush!", "Error", MB_ICONERROR);
             SafeRelease(&pBrush);
             return false;
        }
    }
    return m_pRenderTarget != nullptr;
}

void overlay_D2D::CleanupDirect2D()
{
    SafeRelease(&pBrush);
    SafeRelease(&m_pRenderTarget);
}

void overlay_D2D::DestroyOverlayWindow()
{
    CleanupDirect2D();

    if (g_hwnd && IsWindow(g_hwnd))
    {
        DestroyWindow(g_hwnd);
        g_hwnd = nullptr;
    }
}

LRESULT CALLBACK overlay_D2D::StaticWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    overlay_D2D* pThis = nullptr;

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

bool overlay_D2D::CreateOverlayWindow(HINSTANCE hInstance, int width, int height)
{
    if (g_hwnd != nullptr && IsWindow(g_hwnd))
    {
        ForceTopLeft(m_width, m_height);
        Render();
        return true;
    }

    m_width = width;
    m_height = height;

    const char CLASS_NAME[] = "MyOverlayD2DWindowClass";

    WNDCLASSA wc = {};
    wc.lpfnWndProc = StaticWindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0,0,0));

    // Реєструємо клас вікна, якщо ще не зареєстрований
    WNDCLASSA existing_wc = {};
    if (!GetClassInfoA(hInstance, CLASS_NAME, &existing_wc))
    {
        if (!RegisterClassA(&wc))
        {
            MessageBoxA(NULL, "Failed to register window class!", "Error", MB_ICONERROR);
            return false;
        }
    }


    g_hwnd = CreateWindowExA(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, // WS_EX_NOACTIVATE, щоб не забирати фокус
        CLASS_NAME,
        "D2D Overlay Window",
        WS_POPUP,
        0, 0,
        m_width, m_height,
        NULL,
        NULL,
        hInstance,
        this
    );

    if (g_hwnd == NULL)
    {
        MessageBoxA(NULL, "Failed to create overlay window!", "Error", MB_ICONERROR);
        return false;
    }

    SetLayeredWindowAttributes(g_hwnd, RGB(0,0,0), 0, LWA_COLORKEY);


    if (!InitializeDirect2D()) 
    {
        DestroyOverlayWindow();
        return false;
    }
    
    ForceTopLeft(m_width, m_height);

    ShowWindow(g_hwnd, SW_SHOW);
    UpdateWindow(g_hwnd);

    return true;
}

void overlay_D2D::ForceTopLeft(int width, int height)
{
    if (!g_hwnd) return;
    SetWindowPos(g_hwnd, HWND_TOPMOST, 0, 0, width, height, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE); // Спочатку встановлюємо TOPMOST
    SetWindowPos(g_hwnd, NULL, 0, 0, width, height, SWP_NOACTIVATE | SWP_FRAMECHANGED | SWP_SHOWWINDOW); // Потім розмір і позицію
}


LRESULT CALLBACK overlay_D2D::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        return 0;

    case WM_PAINT:
        Render();
        return 0;

    case WM_SIZE:
        {
            UINT width = LOWORD(lParam);
            UINT height = HIWORD(lParam);
            Resize(width, height);
        }
        return 0;
    
    case WM_DISPLAYCHANGE:
        InvalidateRect(g_hwnd, NULL, FALSE);
        return 0;
    case WM_DESTROY:
        return 0;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

void overlay_D2D::Render()
{
    if (!g_hwnd || !IsWindow(g_hwnd) || !m_pRenderTarget) return;

    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(g_hwnd, &ps);

    m_pRenderTarget->BeginDraw();
    m_pRenderTarget->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f)); 

    D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();

    ID2D1SolidColorBrush* redBrush = nullptr;
    ID2D1SolidColorBrush* greenBrush = nullptr;
    ID2D1SolidColorBrush* blueBrush = nullptr;

    HRESULT hr_red = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red, 1.0f), &redBrush);
    HRESULT hr_green = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green, 1.0f), &greenBrush);
    HRESULT hr_blue = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Blue, 1.0f), &blueBrush);

    if (SUCCEEDED(hr_green))
    {
        D2D1_RECT_F rect = D2D1::RectF(
            rtSize.width * 0.1f, 
            rtSize.height * 0.1f, 
            rtSize.width * 0.4f, 
            rtSize.height * 0.3f
        );
        m_pRenderTarget->FillRectangle(rect, greenBrush);
    }

    if (SUCCEEDED(hr_blue))
    {
        D2D1_ELLIPSE ellipse = D2D1::Ellipse(
            D2D1::Point2F(rtSize.width * 0.7f, rtSize.height * 0.25f),
            rtSize.width * 0.1f, 
            rtSize.height * 0.1f
        );
        m_pRenderTarget->FillEllipse(ellipse, blueBrush);
    }

    if (SUCCEEDED(hr_red))
    {
        m_pRenderTarget->DrawLine(
            D2D1::Point2F(rtSize.width * 0.1f, rtSize.height * 0.5f),
            D2D1::Point2F(rtSize.width * 0.9f, rtSize.height * 0.5f),
            redBrush,
            3.0f
        );
    }

    if (pBrush)
    {
        D2D1_RECT_F roundedRectShape = D2D1::RectF(
            rtSize.width * 0.2f, 
            rtSize.height * 0.6f, 
            rtSize.width * 0.8f, 
            rtSize.height * 0.9f
        );
        D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(roundedRectShape, 20.0f, 20.0f);
        m_pRenderTarget->FillRoundedRectangle(roundedRect, pBrush);
    }

    SafeRelease(&redBrush);
    SafeRelease(&greenBrush);
    SafeRelease(&blueBrush);

    HRESULT hr = m_pRenderTarget->EndDraw();
    if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
    {
        CleanupDirect2D();
    }

    EndPaint(g_hwnd, &ps);
}

void overlay_D2D::Resize(UINT width, UINT height)
{
    if (m_pRenderTarget)
    {
        m_width = width;
        m_height = height;
        
        D2D1_SIZE_U newSize = D2D1::SizeU(width, height);
        m_pRenderTarget->Resize(newSize);
        
        // ForceTopLeft(width, height); 

        InvalidateRect(g_hwnd, NULL, FALSE); 
    }
}