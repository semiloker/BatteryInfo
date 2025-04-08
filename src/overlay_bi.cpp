#include "../include/overlay_bi.h"

overlay_bi* overlay_bi::instance = nullptr;

overlay_bi::overlay_bi(HWND g_hwnd, HFONT g_hFont, RECT g_textRectPos, std::string g_text) 
    : g_hwnd(g_hwnd), g_hFont(g_hFont), g_textRectPos(g_textRectPos), g_text(g_text) 
{
    instance = this;
}

overlay_bi::~overlay_bi() 
{
    if (g_hFont)
        DeleteObject(g_hFont);
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
    const char CLASS_NAME[] = "TransparentOverlayClass";
    
    WNDCLASSA wc = {};
    wc.lpfnWndProc = StaticWindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    
    RegisterClassA(&wc);

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    g_hwnd = CreateWindowExA(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
        CLASS_NAME,
        "Transparent Overlay",
        WS_POPUP,
        0, 0,
        screenWidth, screenHeight,
        parentHwnd,
        NULL,
        hInstance,
        NULL
    );

    if (g_hwnd == NULL) 
    {
        MessageBoxA(NULL, "Failed to create window!", "Error", MB_ICONERROR);
        return;
    }

    SetLayeredWindowAttributes(g_hwnd, RGB(0, 0, 0), 1, LWA_COLORKEY);

    g_hFont = CreateFontA(
        24,                         // Font height
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
        "Arial"                     // Font name
    );

    BOOL enable = TRUE;
    DwmSetWindowAttribute(g_hwnd, DWMWA_TRANSITIONS_FORCEDISABLED, &enable, sizeof(enable));

    SetWindowLong(g_hwnd, GWL_EXSTYLE, GetWindowLong(g_hwnd, GWL_EXSTYLE) | WS_EX_LAYERED | WS_EX_TRANSPARENT);

    ShowWindow(g_hwnd, SW_SHOW);
    UpdateWindow(g_hwnd);
    
    RenderText(g_hwnd);
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
        PostQuitMessage(0);
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
                ::DrawTextA(memDC, g_text.c_str(), -1, &shadowRect, DT_LEFT | DT_TOP | DT_SINGLELINE);
            }
        }
    }
    
    SetTextColor(memDC, RGB(255, 255, 255));
    ::DrawTextA(memDC, g_text.c_str(), -1, &g_textRectPos, DT_LEFT | DT_TOP | DT_SINGLELINE);
    
    BitBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, memDC, 0, 0, SRCCOPY);
    
    SelectObject(memDC, oldFont);
    SelectObject(memDC, oldBitmap);
    DeleteObject(memBitmap);
    DeleteDC(memDC);
    
    EndPaint(hwnd, &ps);
}

void overlay_bi::UpdateText(const std::string& newText)
{
    g_text = newText;  // Оновлюємо текст

    // Перерисовуємо вікно, щоб показати оновлений текст
    InvalidateRect(g_hwnd, NULL, TRUE);  // Вказуємо, щоб вікно перерисувалось
    UpdateWindow(g_hwnd);                // Оновлюємо вікно
}
