#include "../include/main.h"

const char win_bi::szClassName[] = "BatteryInfo";

win_bi::win_bi(HINSTANCE hInstance) : hInstance(hInstance), hwnd(NULL) {}

bool win_bi::Register() 
{
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = win_bi::WndProc;
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
    bi_bi = new batteryinfo_bi();

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
        case WM_COMMAND:          pThis->OnCommand(wParam); break;
        case WM_GETMINMAXINFO:    pThis->OnGetMinMaxInfo(lParam); break;
        case WM_CREATE:           pThis->OnCreate(hwnd); break;
        case WM_SIZE:             
            pThis->OnResize(); 

            if (wParam == SIZE_MINIMIZED)
            {
                ShowWindow(hwnd, SW_HIDE);
                pThis->AddTrayIcon();
            }
        break;
        case WM_PAINT:            pThis->OnPaint(hwnd); break;
        case WM_KEYDOWN:          pThis->OnKeyDown(wParam); break;
        case WM_KEYUP:            pThis->OnKeyUp(wParam); break;
        case WM_MOUSEMOVE:        pThis->OnMouseMove(wParam, lParam); break;
        case WM_LBUTTONDOWN:      pThis->OnLeftButtonDown(wParam, lParam); break;
        case WM_RBUTTONDOWN:      pThis->OnRightButtonDown(wParam, lParam); break;
        case WM_TIMER:            pThis->OnTimer(wParam); break;
        case WM_SETFOCUS:         pThis->OnSetFocus(hwnd); break;
        case WM_KILLFOCUS:        pThis->OnKillFocus(hwnd); break;
        case WM_SYSCOMMAND:       pThis->OnSysCommand(wParam, lParam); break;
        case WM_CHAR:             pThis->OnChar(wParam); break;
        case WM_CLOSE:          
            OutputDebugString("WM_CLOSE received\n");
            DestroyWindow(hwnd); 
        break;
        case WM_USER + 1:
            if (lParam == WM_LBUTTONDBLCLK) 
            {
                ShowWindow(hwnd, SW_RESTORE);
                SetForegroundWindow(hwnd);
                pThis->RemoveTrayIcon();
            }
        break;
        case WM_DESTROY:
            OutputDebugString("WM_DESTROY received\n");
            pThis->OnDestroy(); 
        break;
        default:                  return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void win_bi::OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    bi_bi->PrintAllWin(hdc);
    EndPaint(hwnd, &ps);
}

void win_bi::AddTrayIcon() 
{
    ZeroMemory(&nid, sizeof(NOTIFYICONDATA));
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_USER + 1;
    nid.hIcon = LoadIcon(NULL, IDI_INFORMATION);

    std::string tooltip = 
        "Power State: " + bi_bi->info.PowerState + "\n" +
        "Charge: " + bi_bi->info.ChargeLevel + "\n" +
        "Voltage: " + bi_bi->info.Voltage;

    strncpy_s(nid.szTip, tooltip.c_str(), sizeof(nid.szTip) - 1);

    Shell_NotifyIcon(NIM_ADD, &nid);
}

void win_bi::RemoveTrayIcon() 
{
    Shell_NotifyIcon(NIM_DELETE, &nid);
}

void win_bi::OnCreate(HWND hwnd) 
{
    
}
void win_bi::OnCommand(WPARAM wParam)
{

}

void win_bi::OnResize()
{

}

void win_bi::OnKeyDown(WPARAM wParam)
{
    if (wParam == VK_ESCAPE)
        SendMessage(hwnd, WM_CLOSE, 0, 0);
}

void win_bi::OnKeyUp(WPARAM wParam)
{

}

void win_bi::OnMouseMove(WPARAM wParam, LPARAM lParam)
{

}

void win_bi::OnLeftButtonDown(WPARAM wParam, LPARAM lParam)
{

}

void win_bi::OnRightButtonDown(WPARAM wParam, LPARAM lParam)
{

}

void win_bi::OnTimer(WPARAM wParam)
{

}

void win_bi::OnSetFocus(HWND hwnd)
{

}

void win_bi::OnKillFocus(HWND hwnd)
{

}

void win_bi::OnSysCommand(WPARAM wParam, LPARAM lParam)
{
    DefWindowProc(hwnd, WM_SYSCOMMAND, wParam, lParam);
}

void win_bi::OnChar(WPARAM wParam)
{
    
}

void win_bi::OnGetMinMaxInfo(LPARAM lParam) 
{
    MINMAXINFO* mmi = (MINMAXINFO*)lParam;
    mmi->ptMinTrackSize.x = 350;
    mmi->ptMinTrackSize.y = 350;
}

void win_bi::OnDestroy() 
{
    RemoveTrayIcon();
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