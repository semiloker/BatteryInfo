#ifndef MAIN_H
#define MAIN_H

#include <minwindef.h>
#include <windows.h>
#include <shellapi.h>
#include <string>

#include "../include/BatteryInfo.h"
#include "../include/init_d2d1_bi.h"
#include "../include/init_dwrite_bi.h"
#include "../include/draw_batteryinfo_bi.h"

class win_bi
{
public:
    win_bi(HINSTANCE hInstance);
    ~win_bi() = default;
    
    bool Register();
    bool Create(int nCmdShow);

    void AddTrayIcon();
    void UpdateTrayTooltip();
    void RemoveTrayIcon();

    WPARAM RunMessageLoop();

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    void OnGetMinMaxInfo(LPARAM lParam);
    
    void OnCommand(WPARAM wParam);
    void OnCreate(HWND hwnd);
    void OnResize(WPARAM wParam);
    void OnPaint(HWND hwnd);
    void OnKeyDown(WPARAM wParam);
    void OnKeyUp(WPARAM wParam);
    void OnMouseMove(WPARAM wParam, LPARAM lParam);
    void OnLeftButtonDown(WPARAM wParam, LPARAM lParam);
    void OnRightButtonDown(WPARAM wParam, LPARAM lParam);
    void OnTimer(WPARAM wParam);
    void OnSetFocus(HWND hwnd); 
    void OnKillFocus(HWND hwnd);
    void OnSysCommand(WPARAM wParam, LPARAM lParam); 
    void OnChar(WPARAM wParam);
    void OnDestroy();

    static const char szClassName[];
    
    NOTIFYICONDATA nid;
    HINSTANCE hInstance;
    HWND hwnd;

    bool isMinimized = false;

    batteryinfo_bi* bi_bi;
    init_d2d1_bi* initd2d1_bi;
    init_dwrite_bi* initdwrite_bi;
    draw_batteryinfo_bi* draw_bibi_bi;
};

#endif