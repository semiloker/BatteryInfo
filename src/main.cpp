#include "../include/main.h"

const char win_bi::szClassName[] = "BatteryInfo";

win_bi::win_bi(HINSTANCE hInstance) : hInstance(hInstance), hwnd(NULL) {}

bool win_bi::Register() 
{
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = szClassName;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    return RegisterClassEx(&wc) != 0;
}

bool win_bi::Create(int nCmdShow) 
{
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int windowWidth = 500;
    int windowHeight = 400;
    int x = (screenWidth - windowWidth) / 2;
    int y = (screenHeight - windowHeight) / 2;

    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE, szClassName, szClassName, WS_OVERLAPPEDWINDOW,
        x, y, windowWidth, windowHeight,
        NULL, NULL, hInstance, this);

    if (!hwnd) 
    {
        MessageBox(NULL, "Could not create a window!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    return true;
}

WPARAM win_bi::RunMessageLoop() 
{
    MSG Msg;
    while (GetMessage(&Msg, NULL, 0, 0) > 0) 
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    return Msg.wParam;
}

LRESULT CALLBACK win_bi::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) 
{
    win_bi* pThis = nullptr;
    
    if (msg == WM_NCCREATE) 
    {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        pThis = (win_bi*)pCreate->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
    } 
    else 
    {
        pThis = (win_bi*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    if (!pThis) 
    {
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    switch (msg) 
    {
        case WM_COMMAND:    pThis->OnCommand(wParam); break;
        case WM_GETMINMAXINFO: pThis->OnGetMinMaxInfo(lParam); break;
        case WM_CREATE:     pThis->OnCreate(hwnd); break;
        case WM_SIZE:       pThis->OnResize(); break;
        case WM_PAINT: 
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rect;
            GetClientRect(hwnd, &rect);
            DrawTextA(hdc, pThis->displayText.c_str(), -1, &rect, DT_LEFT | DT_TOP | DT_WORDBREAK);
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_TIMER:
            pThis->UpdateBatteryText();
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        case WM_CLOSE:      DestroyWindow(hwnd); break;
        case WM_DESTROY:    pThis->OnDestroy(); break;
        default:            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void win_bi::OnCreate(HWND hwnd) 
{
    if (battery.Init())
    {
        UpdateBatteryText();
        SetTimer(hwnd, 1, 5000, NULL); // оновлення кожні 5 сек
    } 
    else 
    {
        displayText = "Battery initialization failed.";
    }
}
void win_bi::OnCommand(WPARAM wParam)
{

}

void win_bi::OnResize()
{

}

void win_bi::OnKeydown(WPARAM wParam)
{

}

void win_bi::OnGetMinMaxInfo(LPARAM lParam) 
{
    MINMAXINFO* mmi = (MINMAXINFO*)lParam;
    mmi->ptMinTrackSize.x = 350;
    mmi->ptMinTrackSize.y = 350;
}

void win_bi::UpdateBatteryText() 
{
    std::ostringstream oss;
    oss << battery.GetBatteryInfoText() << "\n";
    oss << battery.GetBatteryStatusText();
    displayText = oss.str();
}

void win_bi::OnDestroy() 
{
    KillTimer(hwnd, 1);

    PostQuitMessage(0);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) 
{
    win_bi mainWindow(hInstance);
    if (!mainWindow.Register() || !mainWindow.Create(nCmdShow)) 
    {
        return 0;
    }
    return mainWindow.RunMessageLoop();
}