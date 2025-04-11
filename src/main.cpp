#include "../include/main.h"
#include <string>

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
    // wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    // wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIcon = (HICON)LoadImageA(NULL, "sign-of-battery-icon-vector.ico", IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
    wc.hIconSm = (HICON)LoadImageA(NULL, "sign-of-battery-icon-vector.ico", IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
    if (!wc.hIcon) MessageBoxA(NULL, "Failed to load icon!", "Error", MB_ICONERROR);
    
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = szClassName;

    return RegisterClassEx(&wc) != 0;
}

bool win_bi::Create(int nCmdShow) 
{
    bi_bi = new batteryinfo_bi();
    initd2d1_bi = new init_d2d1_bi();
    initdwrite_bi = new init_dwrite_bi();
    draw_bibi_bi = new draw_batteryinfo_bi();
    ru_bi = new resource_usage_bi();
    ov_bi = new overlay_bi(NULL, NULL, {20, 20, 400, 800}, "Test");
    
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int windowWidth = 450;
    int windowHeight = 750;
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
            pThis->OnResize(wParam); 

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
        case WM_MOUSEWHEEL: break;
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
            switch (lParam) 
            {
                case WM_LBUTTONDBLCLK:
                    ShowWindow(hwnd, SW_RESTORE);
                    SetForegroundWindow(hwnd);
                    pThis->RemoveTrayIcon();
                    break;
            
                case WM_RBUTTONUP:
                    pThis->ShowTrayMenu();
                    break;
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
    // PAINTSTRUCT ps;
    // HDC hdc = BeginPaint(hwnd, &ps);
    // bi_bi->PrintAllWin(hdc);
    // EndPaint(hwnd, &ps);

    initd2d1_bi->InitDirect2D();
    ID2D1HwndRenderTarget* pRenderTarget = initd2d1_bi->CreateRenderTarget(hwnd);
    if (pRenderTarget)
    {
        pRenderTarget->BeginDraw();
        pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));

        // bi_bi->PrintAllWinD2D(pRenderTarget, 20, 30);
        initdwrite_bi->InitGraph();
        draw_bibi_bi->drawHeaderBatteryInfoD2D(pRenderTarget, bi_bi, initdwrite_bi, 20, 30, 24);

        pRenderTarget->EndDraw();
        pRenderTarget->Release();
    }
    ValidateRect(hwnd, nullptr);
}

void win_bi::AddTrayIcon() 
{
    ZeroMemory(&nid, sizeof(NOTIFYICONDATA));
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_USER + 1;
    nid.hIcon = (HICON)LoadImageA(NULL, "sign-of-battery-icon-vector.ico", IMAGE_ICON, 32, 32, LR_LOADFROMFILE);

    std::string tooltip = 
        "Power State: " + bi_bi->info_1s.PowerState + "\n" +
        "Charge: " + bi_bi->info_1s.ChargeLevel + "\n" +
        "Voltage: " + bi_bi->info_1s.Voltage;

    strncpy_s(nid.szTip, tooltip.c_str(), sizeof(nid.szTip) - 1);

    Shell_NotifyIcon(NIM_ADD, &nid);

    UpdateTrayTooltip();
}

void win_bi::UpdateTrayTooltip() 
{
    std::string tooltip = 
        "Power State: " + bi_bi->info_1s.PowerState + "\n" +
        "Charge: " + bi_bi->info_1s.ChargeLevel + "\n" +
        "Voltage: " + bi_bi->info_1s.Voltage;

    strncpy_s(nid.szTip, tooltip.c_str(), sizeof(nid.szTip) - 1);
    nid.uFlags = NIF_TIP;
    Shell_NotifyIcon(NIM_MODIFY, &nid);
}

void win_bi::RemoveTrayIcon() 
{
    Shell_NotifyIcon(NIM_DELETE, &nid);
}

void win_bi::ShowTrayMenu()
{
    POINT pt;
    GetCursorPos(&pt);

    HMENU hMenu = CreatePopupMenu();
    if (hMenu) 
    {
        InsertMenu(hMenu, -1, MF_BYPOSITION | MF_STRING, 1, "Open");
        InsertMenu(hMenu, -1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
        InsertMenu(hMenu, -1, MF_BYPOSITION | MF_STRING, 2, "Exit");

        SetForegroundWindow(hwnd);
        int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, NULL);
        DestroyMenu(hMenu);

        switch (cmd) 
        {
            case 1:
                ShowWindow(hwnd, SW_RESTORE);
                SetForegroundWindow(hwnd);
                RemoveTrayIcon();
                break;
            case 2:
                PostMessage(hwnd, WM_CLOSE, 0, 0);
                break;
        }
    }
}

void win_bi::OnCreate(HWND hwnd) 
{
    SetTimer(hwnd, 1, 1000, NULL);
    SetTimer(hwnd, 2, 10000, NULL);

    bool success = bi_bi->Initialize() && ru_bi->updateRam();

    if (!success)
        MessageBoxA(NULL, "Battery initialization failed!", "Error", MB_ICONERROR);

    UpdateOverlayText();
}

void win_bi::OnCommand(WPARAM wParam)
{

}

void win_bi::OnResize(WPARAM wParam)
{

}

void win_bi::UpdateOverlayText()
{
    if (ov_bi) 
    {
        ru_bi->updateRam();
        ru_bi->updateCpu();

        std::string newText = 
        "Power State: " + bi_bi->info_1s.RemainingCapacity + "\n" +
        "Charge: " + bi_bi->info_1s.ChargeLevel + "\n" +
        "Voltage: " + bi_bi->info_1s.Rate + "\n" +
        "ullAvailPhys: " + ru_bi->ramInfo.ullAvailPhys + "\n" +
        "CPU: " + ru_bi->cpuInfo.UsagePercent;
        // "cpuInfo.UsagePercent: " + bi_bi->cpuInfo.UsagePercent + "\n" +
        // "ramInfo.UsedPercent: " + bi_bi->ramInfo.UsedPercent + "\n" +
        // "ramInfo.AvailPhys: " + bi_bi->ramInfo.AvailPhys;

        for (int i = 0; i < ru_bi->cpuInfo.CoreUsagePercents.size(); ++i)
        {
            newText += "\nCore(" + std::to_string(i + 1) + "): " + ru_bi->cpuInfo.CoreUsagePercents[i];
        }

        ov_bi->UpdateText(newText);
        ov_bi->UpdatePosition();

        InvalidateRect(ov_bi->g_hwnd, NULL, TRUE);
    }
}

void win_bi::OnKeyDown(WPARAM wParam)
{
    if (wParam == VK_ESCAPE)
        SendMessage(hwnd, WM_CLOSE, 0, 0);
    else if (wParam == 'N')
        ov_bi->CreateOverlayWindow(hInstance, hwnd);
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
    if (!bi_bi) return;

    switch (wParam)
    {
        case 1:
            bi_bi->QueryBatteryInfo();
            bi_bi->QueryBatteryStatus();
            // bi_bi->QueryCpuInfo();
            // bi_bi->QueryRamInfo();
            UpdateTrayTooltip();
            UpdateOverlayText();
            InvalidateRect(hwnd, NULL, true);
            break;

        case 2:
            bi_bi->QueryBatteryRemaining();
            InvalidateRect(hwnd, NULL, true);
            break;
    }
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
    mmi->ptMinTrackSize.y = 750;
}

void win_bi::OnDestroy() 
{
    ru_bi->cleanup();
    initdwrite_bi->CleanupDirectWrite();
    KillTimer(hwnd, 1);
    KillTimer(hwnd, 2);
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