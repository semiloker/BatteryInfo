#ifndef MAIN_H
#define MAIN_H

#include <windows.h>
#include <string>

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

    void OnGetMinMaxInfo(LPARAM lParam);
    
    void OnCommand(WPARAM wParam);
    void OnCreate(HWND hwnd);
    void OnResize();
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

    HINSTANCE hInstance;
    HWND hwnd;
};

#endif