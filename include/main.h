#ifndef MAIN_H
#define MAIN_H

#include <windows.h>
#include <string>
#include "../include/BatteryInfo.h"

class win_bi
{
public:
    win_bi(HINSTANCE hInstance);
    ~win_bi() = default;
    bool Register();
    bool Create(int nCmdShow);
    WPARAM RunMessageLoop();

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    void OnCommand(WPARAM wParam);
    void OnGetMinMaxInfo(LPARAM lParam);
    void OnCreate(HWND hwnd);
    void OnResize();
    void OnKeydown(WPARAM wParam);
    void OnDestroy();

    static const char szClassName[];

    std::string displayText;
    BatteryInfo battery;

    void UpdateBatteryText();

    HINSTANCE hInstance;
    HWND hwnd;
};

#endif