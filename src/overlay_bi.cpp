#include "../include/overlay_bi.h"

overlay_bi* overlay_bi::instance = nullptr;

overlay_bi::overlay_bi(HWND g_hwnd, HFONT g_hFont, RECT g_textRectPos, std::string g_text) 
    : g_hwnd(g_hwnd), g_hFont(g_hFont), g_textRectPos(g_textRectPos), g_text(g_text) 
{
    instance = this;
}

overlay_bi::~overlay_bi()
{
    if (g_hwnd && IsWindow(g_hwnd))
    {
        DestroyWindow(g_hwnd);
    }
    g_hwnd = nullptr;

    if (g_hFont)
    {
        DeleteObject(g_hFont);
        g_hFont = nullptr;
    }
}

void overlay_bi::DestroyOverlayWindow()
{
    if (g_hwnd && IsWindow(g_hwnd))
    {
        DestroyWindow(g_hwnd);
        g_hwnd = nullptr;
    }

    if (g_hFont)
    {
        DeleteObject(g_hFont);
        g_hFont = nullptr;
    }
}


LRESULT CALLBACK overlay_bi::StaticWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (instance) 
    {
        return instance->WindowProc(hwnd, uMsg, wParam, lParam);
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void overlay_bi::CreateOverlayWindow(HINSTANCE hInstance, HWND parentHwnd) 
{
    if (g_hwnd != nullptr && IsWindow(g_hwnd)) 
    {
        return;
    }

    const char CLASS_NAME[] = "TransparentOverlayClass";

    WNDCLASSA wc = {};
    wc.lpfnWndProc = StaticWindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
<<<<<<< HEAD
    wc.hbrBackground = NULL;
    
    RegisterClassA(&wc);
=======
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

    if (!GetClassInfoA(hInstance, CLASS_NAME, &wc))
    {
        RegisterClassA(&wc);
    }
>>>>>>> ba73bc728e55c7daa95bb3eb40bb0a7aef35e01d

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    g_hwnd = CreateWindowExA(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW ,
        CLASS_NAME,
        "Transparent Overlay",
        WS_POPUP,
        0, 0,
        screenWidth, screenHeight,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (g_hwnd == NULL) 
    {
        MessageBoxA(NULL, "Failed to create window!", "Error", MB_ICONERROR);
        return;
    }

    SetLayeredWindowAttributes(g_hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

    g_hFont = CreateFontA(
        14,                         // Font height
        0,                          // Character width
        0,                          // Escape angle
        0,                          // Orientation angle
        FW_BOLD,                    // Font weight
        FALSE,                      // Italic
        FALSE,                      // Underline
        FALSE,                      // Strikeout
        DEFAULT_CHARSET,            // Character set
        OUT_OUTLINE_PRECIS,         // Output precision
        CLIP_DEFAULT_PRECIS,        // Clipping precision
        CLEARTYPE_QUALITY,          // Quality
        DEFAULT_PITCH | FF_SWISS,   // Pitch and family
        "Segoe UI"                     // Font name
    );

    BOOL enable = TRUE;
    DwmSetWindowAttribute(g_hwnd, DWMWA_TRANSITIONS_FORCEDISABLED, &enable, sizeof(enable));
    
    BOOL disableMaximize = TRUE;
    DwmSetWindowAttribute(g_hwnd, DWMWA_DISALLOW_PEEK, &disableMaximize, sizeof(disableMaximize));
    
    SetWindowPos(g_hwnd, HWND_TOPMOST, 0, 0, screenWidth, screenHeight, 
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    
    SetWindowLong(g_hwnd, GWL_EXSTYLE, 
                 GetWindowLong(g_hwnd, GWL_EXSTYLE) | 
                 WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_NOACTIVATE);

    ShowWindow(g_hwnd, SW_SHOW);
    UpdateWindow(g_hwnd);
    
    RenderText(g_hwnd);
    
    ForceTopMost();
}

void overlay_bi::ForceTopMost()
{
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    
    SetWindowPos(g_hwnd, HWND_TOPMOST, 0, 0, screenWidth, screenHeight, 
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

void overlay_bi::UpdatePosition()
{
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    
    RECT currentRect;
    GetWindowRect(g_hwnd, &currentRect);
    
    if (currentRect.right - currentRect.left != screenWidth || 
        currentRect.bottom - currentRect.top != screenHeight) 
        {
        SetWindowPos(g_hwnd, HWND_TOPMOST, 0, 0, screenWidth, screenHeight, 
                     SWP_NOACTIVATE);
    } 
    else 
    {
        SetWindowPos(g_hwnd, HWND_TOPMOST, 0, 0, 0, 0, 
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
}

LRESULT CALLBACK overlay_bi::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) 
    {
    case WM_CREATE:
        return 0;
        
    case WM_PAINT:
        RenderText(hwnd);
        return 0;

    case WM_DESTROY:
        // PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

void overlay_bi::RenderText(HWND hwnd) 
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    
    HDC memDC = CreateCompatibleDC(hdc);
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, clientRect.right, clientRect.bottom);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);
    
    HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(memDC, &clientRect, blackBrush);
    DeleteObject(blackBrush);

    SetBkMode(memDC, TRANSPARENT);
    
    HFONT oldFont = (HFONT)SelectObject(memDC, g_hFont);
    
    SetTextColor(memDC, RGB(0, 0, 0));
    for (int i = -1; i <= 1; i++) 
    {
        for (int j = -1; j <= 1; j++) 
        {
            if (i != 0 || j != 0)
            {
                RECT shadowRect = g_textRectPos;
                OffsetRect(&shadowRect, i, j);
                ::DrawTextA(memDC, g_text.c_str(), -1, &shadowRect, DT_LEFT | DT_TOP | DT_WORDBREAK);
            }
        }
    }
    
    SetTextColor(memDC, RGB(255, 255, 255));
    ::DrawTextA(memDC, g_text.c_str(), -1, &g_textRectPos, DT_LEFT | DT_TOP | DT_WORDBREAK);
    
    BitBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, memDC, 0, 0, SRCCOPY);
    
    SelectObject(memDC, oldFont);
    SelectObject(memDC, oldBitmap);
    DeleteObject(memBitmap);
    DeleteDC(memDC);
    
    EndPaint(hwnd, &ps);
}

void overlay_bi::UpdateText(const std::string& newText)
{
    g_text = newText;

<<<<<<< HEAD
    HDC hdc = GetDC(g_hwnd);
    RenderText(g_hwnd);
    ReleaseDC(g_hwnd, hdc);
}
=======
    InvalidateRect(g_hwnd, NULL, TRUE);
    UpdateWindow(g_hwnd);
}
>>>>>>> ba73bc728e55c7daa95bb3eb40bb0a7aef35e01d
