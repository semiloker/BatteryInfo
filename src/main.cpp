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
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    bi_bi = new batteryinfo_bi();
    initd2d1_bi = new init_d2d1_bi();
    initdwrite_bi = new init_dwrite_bi();
    draw_bibi_bi = new draw_batteryinfo_bi();
    ru_bi = new resource_usage_bi();
    ov_d2d = new overlay_D2D();
    ov_bi = new overlay_bi(NULL, NULL, {initdwrite_bi->overlay_pos_x, initdwrite_bi->overlay_pos_y, screenWidth, screenHeight}, "nullptr");

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
        case WM_SIZE:             pThis->OnResize(wParam); break;
        case WM_PAINT:            pThis->OnPaint(hwnd); break;
        case WM_KEYDOWN:          pThis->OnKeyDown(wParam); break;
        case WM_KEYUP:            pThis->OnKeyUp(wParam); break;
        case WM_MOUSEMOVE:        pThis->OnMouseMove(wParam, lParam); break;
        case WM_MOUSEWHEEL:       pThis->OnMouseWheel(wParam, lParam); break;
        case WM_LBUTTONDOWN:      pThis->OnLeftButtonDown(wParam, lParam); break;
        case WM_RBUTTONDOWN:      pThis->OnRightButtonDown(wParam, lParam); break;
        case WM_TIMER:            pThis->OnTimer(wParam); break;
        case WM_SETFOCUS:         pThis->OnSetFocus(hwnd); break;
        case WM_KILLFOCUS:        pThis->OnKillFocus(hwnd); break;
        case WM_SYSCOMMAND:       pThis->OnSysCommand(wParam, lParam); break;
        case WM_CHAR:             pThis->OnChar(wParam); break;
        case WM_CLOSE:            pThis->OnClose(); break;
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
        case WM_ERASEBKGND:       return 1;
        case WM_DESTROY:          pThis->OnDestroy(); break;
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

    PAINTSTRUCT ps;
    BeginPaint(hwnd, &ps);

    initd2d1_bi->InitDirect2D();
    ID2D1HwndRenderTarget* pRenderTarget = initd2d1_bi->GetOrCreateRenderTarget(hwnd);
    if (pRenderTarget)
    {
        pRenderTarget->BeginDraw();
        // pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));
        
        draw_bibi_bi->initBrush(pRenderTarget);
        // bi_bi->PrintAllWinD2D(pRenderTarget, 20, 30);
        initdwrite_bi->InitGraph();
        draw_bibi_bi->clearBackground(pRenderTarget);

        if (draw_bibi_bi->selectedTab == draw_batteryinfo_bi::BATTERY_INFO)
        {
            draw_bibi_bi->drawHeaderBatteryInfoD2D(pRenderTarget, bi_bi, initdwrite_bi, 20, 30, 24);
        }
        else if (draw_bibi_bi->selectedTab == draw_batteryinfo_bi::SETTINGS)
        {
            draw_bibi_bi->drawHeaderSettingsD2D(pRenderTarget, initdwrite_bi, ov_bi, ru_bi, bi_bi);
        }

        draw_bibi_bi->drawHeaders(pRenderTarget, initdwrite_bi);
        
        if (ov_bi->show_on_screen_display == true)
        {
            ov_d2d->CreateOverlayWindow(hInstance, 300, 300);

            if (!ov_bi->g_hwnd || !IsWindow(ov_bi->g_hwnd))
            {
                ov_bi->CreateOverlayWindow(hInstance, hwnd);
            }
        }
        if (ov_bi->show_on_screen_display == false)
        {
            ov_d2d->DestroyOverlayWindow();
            ov_bi->DestroyOverlayWindow();
        }
        
        HRESULT hr = pRenderTarget->EndDraw();
        if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
        {
            initd2d1_bi->DiscardRenderTarget();
        }
        // pRenderTarget->EndDraw();
        // pRenderTarget->Release();
    }
    EndPaint(hwnd, &ps);
    // ValidateRect(hwnd, nullptr);
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
        SendMessage(hwnd, WM_NULL, 0, 0);

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
                PostQuitMessage(0);
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
    initd2d1_bi->ResizeRenderTarget(hwnd);
    InvalidateRect(hwnd, nullptr, FALSE);

    if (wParam == SIZE_MINIMIZED)
    {
        if (ru_bi && ru_bi->minimize_To_Tray)
        {
            ShowWindow(hwnd, SW_HIDE);
            AddTrayIcon();
        }
    }
}

void win_bi::UpdateOverlayText()
{
    if (ov_bi) 
    {
        ru_bi->updateAll();

        std::string newText;
        
        // CPU Info
        if (ru_bi->cpuInfo.show_cpuName)
            newText += "CPU Name: " + ru_bi->cpuInfo.cpuName + "\n";
            
        if (ru_bi->cpuInfo.show_architecture)
            newText += "Architecture: " + ru_bi->cpuInfo.architecture + "\n";
            
        if (ru_bi->cpuInfo.show_UsagePercent)
            newText += "\nCPU Usage: " + ru_bi->cpuInfo.UsagePercent + "\n";
            
        if (ru_bi->cpuInfo.show_CoreUsagePercents)
        {
            for (size_t i = 0; i < ru_bi->cpuInfo.CoreUsagePercents.size(); ++i)
            {
                newText += "Core " + std::to_string(i + 1) + ": " + 
                          ru_bi->cpuInfo.CoreUsagePercents[i] + "\n";
            }
        }

        // RAM Info
        if (ru_bi->ramInfo.show_dwMemoryLoad)
            newText += "\nMemory Load: " + ru_bi->ramInfo.dwMemoryLoad + "\n";

        if (ru_bi->ramInfo.show_ullTotalPhys)
            newText += "Total RAM: " + ru_bi->ramInfo.ullTotalPhys + "\n";

        if (ru_bi->ramInfo.show_ullAvailPhys)
            newText += "Available RAM: " + ru_bi->ramInfo.ullAvailPhys + "\n";

        if (ru_bi->ramInfo.show_ullTotalPageFile)
            newText += "Total Pagefile: " + ru_bi->ramInfo.ullTotalPageFile + "\n";

        if (ru_bi->ramInfo.show_ullAvailPageFile)
            newText += "Available Pagefile: " + ru_bi->ramInfo.ullAvailPageFile + "\n";

        if (ru_bi->ramInfo.show_ullTotalVirtual)
            newText += "Total Virtual: " + ru_bi->ramInfo.ullTotalVirtual + "\n";

        if (ru_bi->ramInfo.show_ullAvailVirtual)
            newText += "Available Virtual: " + ru_bi->ramInfo.ullAvailVirtual + "\n";

        if (ru_bi->ramInfo.show_ullAvailExtendedVirtual)
            newText += "Extended Virtual: " + ru_bi->ramInfo.ullAvailExtendedVirtual + "\n";

        if (bi_bi->info_1s.Voltage_)
            newText += "\nVoltage: " + bi_bi->info_1s.Voltage + "\n";
        if (bi_bi->info_1s.Rate_)
            newText += "Rate: " + bi_bi->info_1s.Rate + "\n";
        if (bi_bi->info_1s.PowerState_)
            newText += "Power State: " + bi_bi->info_1s.PowerState + "\n";
        if (bi_bi->info_1s.RemainingCapacity_)
            newText += "Remaining Capacity: " + bi_bi->info_1s.RemainingCapacity + "\n";
        if (bi_bi->info_1s.ChargeLevel_)
            newText += "Charge Level: " + bi_bi->info_1s.ChargeLevel + "\n";
        if (bi_bi->info_10s.TimeRemaining_)
            newText += "Time Remaining: " + bi_bi->info_10s.TimeRemaining + "\n";

        // Disk Info
        for (const auto& disk : ru_bi->disksInfo)
        {
            if (disk.show_diskLetter)
                newText += "\nDrive: " + disk.diskLetter + "\n";
                
            if (disk.show_totalSpace)
                newText += "Total Space: " + disk.totalSpace + "\n";
                
            if (disk.show_freeSpace)
                newText += "Free Space: " + disk.freeSpace + "\n";
                
            if (disk.show_usedSpace)
                newText += "Used Space: " + disk.usedSpace + "\n";
                
            if (disk.show_usagePercent)
                newText += "Usage: " + disk.usagePercent + "\n";
        }

        // Network Info
        // for (const auto& net : ru_bi->networkInfo)
        // {
        //     if (net.show_interfaceName)
        //         newText += "\nInterface: " + net.interfaceName + "\n";
                
        //     if (net.show_downloadSpeed)
        //         newText += "Download: " + net.downloadSpeed + "\n";
                
        //     if (net.show_uploadSpeed)
        //         newText += "Upload: " + net.uploadSpeed + "\n";
        // }

        ov_bi->UpdateText(newText);
        ov_bi->UpdatePosition();

        InvalidateRect(ov_bi->g_hwnd, NULL, TRUE);
    }
}

void win_bi::OnKeyDown(WPARAM wParam)
{
    if (wParam == VK_ESCAPE && ru_bi->exit_on_key_esc == true)
        SendMessage(hwnd, WM_CLOSE, 0, 0);
    // else if (wParam == 'N')
    //     ov_bi->CreateOverlayWindow(hInstance, hwnd);
}

void win_bi::OnKeyUp(WPARAM wParam)
{

}

void win_bi::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
    GetCursorPos(&pt);
    ScreenToClient(hwnd, &pt);
}

void win_bi::OnMouseWheel(WPARAM wParam, LPARAM lParam)
{
    short delta = GET_WHEEL_DELTA_WPARAM(wParam);
    
    if (draw_bibi_bi->selectedTab == draw_batteryinfo_bi::SETTINGS)
    {
        draw_bibi_bi->scrollOffsetY -= delta * 0.2f;

        if (draw_bibi_bi->scrollOffsetY < 0) 
            draw_bibi_bi->scrollOffsetY = 0;
        if (draw_bibi_bi->scrollOffsetY > draw_bibi_bi->contentHeight - draw_bibi_bi->viewHeight)
            draw_bibi_bi->scrollOffsetY = draw_bibi_bi->contentHeight - draw_bibi_bi->viewHeight;

        InvalidateRect(hwnd, NULL, TRUE);
    }
}

void win_bi::OnLeftButtonDown(WPARAM wParam, LPARAM lParam)
{
    if (draw_bibi_bi->isCursorInBatteryStatus(pt)) 
    {
        draw_bibi_bi->selectedTab = draw_batteryinfo_bi::BATTERY_INFO;
        InvalidateRect(hwnd, nullptr, TRUE);
    }
    if (draw_bibi_bi->isCursorInSettings(pt)) 
    {
        draw_bibi_bi->selectedTab = draw_batteryinfo_bi::SETTINGS;
        InvalidateRect(hwnd, nullptr, TRUE);
    }

    if (draw_bibi_bi->selectedTab == draw_batteryinfo_bi::SETTINGS && 
        draw_bibi_bi->handleSwitchClick(pt)) 
    {
        InvalidateRect(hwnd, nullptr, TRUE);
    }
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

            if (ov_bi->show_on_screen_display == true)
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

void win_bi::OnClose()
{
    OutputDebugString("WM_CLOSE received\n");
    if (ru_bi && ru_bi->minimize_To_Tray)
    {
        ShowWindow(hwnd, SW_HIDE);
        AddTrayIcon();
    }
    else
        DestroyWindow(hwnd);
}

void win_bi::OnDestroy() 
{
    ru_bi->cleanup();
    draw_bibi_bi->~draw_batteryinfo_bi();
    ov_bi->~overlay_bi();
    ov_d2d->~overlay_D2D();
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

    if (mainWindow.ru_bi && mainWindow.ru_bi->start_With_Windows) 
    {
        mainWindow.ru_bi->enableStartWithWindows();
    }

    return mainWindow.RunMessageLoop();
}