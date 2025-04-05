#ifndef BATTERYINFO_H
#define BATTERYINFO_H

#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <algorithm>
#include <map>
#include <iostream>
#include <vector>
#include <initguid.h>
#include <setupapi.h>
#include <batclass.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

// #pragma comment(lib, "setupapi.lib")
// #pragma comment(lib, "d2d1.lib")
// #pragma comment(lib, "dwrite.lib")

template<typename T>
T clamp(T value, T min, T max)
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

DEFINE_GUID(GUID_DEVINTERFACE_BATTERY,
0x72631e54, 0x78a4, 0x11d0, 0xbc, 0xf7, 0x00, 0xaa, 0x00, 0xb7, 0xb3, 0x2a);

class batteryinfo_bi
{
private:
    HDEVINFO hDevInfo;
    HANDLE hBattery;
    BATTERY_INFORMATION bi{};
    BATTERY_STATUS bs{};
    ULONG tag;

public:
    struct bi_struct 
    {
        std::string Chemistry;
        std::string DesignedCapacity;
        std::string FullChargedCapacity;
        std::string DefaultAlert1;
        std::string DefaultAlert2;
        std::string WearLevel;
        std::string Voltage;
        std::string Rate;
        std::string PowerState;
        std::string RemainingCapacity;
        std::string ChargeLevel;
        std::string TimeRemaining;
        std::string TimeToFullCharge;
    };

    bi_struct info;

    batteryinfo_bi() : hDevInfo(INVALID_HANDLE_VALUE), hBattery(INVALID_HANDLE_VALUE), tag(0) 
    {
    }

    ~batteryinfo_bi() 
    {
        if (hBattery != INVALID_HANDLE_VALUE)
            CloseHandle(hBattery);
        if (hDevInfo != INVALID_HANDLE_VALUE)
            SetupDiDestroyDeviceInfoList(hDevInfo);
    }

    bool Initialize();
    bool QueryTag();
    bool QueryBatteryInfo();
    bool QueryBatteryStatus();
    void UpdateInfo();
    void PrintAllConsole() const;

    ID2D1Factory* pD2DFactory = nullptr;

    IDWriteFactory* pDWriteFactory = nullptr;

    IDWriteTextFormat* pTextFormatHeader = nullptr;
    IDWriteTextFormat* pTextFormatLabel = nullptr;
    IDWriteTextFormat* pTextFormatValue = nullptr;
    
    void InitDirectWrite()
    {
        if (!pDWriteFactory)
        {
            DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&pDWriteFactory));

            pDWriteFactory->CreateTextFormat(
                L"Segoe UI",              
                NULL,
                DWRITE_FONT_WEIGHT_SEMI_BOLD,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                20.0f,                      
                L"en-us",
                &pTextFormatHeader
            );
            
            pDWriteFactory->CreateTextFormat(
                L"Segoe UI",
                NULL,
                DWRITE_FONT_WEIGHT_NORMAL,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                12.0f,                     
                L"en-us",
                &pTextFormatLabel
            );
            
            pDWriteFactory->CreateTextFormat(
                L"Segoe UI",
                NULL,
                DWRITE_FONT_WEIGHT_REGULAR,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                14.0f,                     
                L"en-us",
                &pTextFormatValue
            );
        }
    }

    void InitDirect2D()
    {
        if (!pD2DFactory)
            D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);
    }

    ID2D1HwndRenderTarget* CreateRenderTarget(HWND hwnd)
    {
        RECT rc;
        GetClientRect(hwnd, &rc);

        D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties();
        D2D1_HWND_RENDER_TARGET_PROPERTIES hwndProps =
            D2D1::HwndRenderTargetProperties(hwnd,
                D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top));

        ID2D1HwndRenderTarget* pRenderTarget = nullptr;
        pD2DFactory->CreateHwndRenderTarget(rtProps, hwndProps, &pRenderTarget);
        return pRenderTarget;
    }

    void CleanupDirectWrite()
    {
        if (pTextFormatHeader) { pTextFormatHeader->Release(); pTextFormatHeader = nullptr; }
        if (pTextFormatLabel) { pTextFormatLabel->Release(); pTextFormatLabel = nullptr; }
        if (pTextFormatValue) { pTextFormatValue->Release(); pTextFormatValue = nullptr; }
    
        if (pDWriteFactory) { pDWriteFactory->Release(); pDWriteFactory = nullptr; }
    }

    void PrintAllWinD2D(ID2D1HwndRenderTarget* pRT, int startX = 10, int startY = 10, int lineHeight = 24);
    // void PrintAllWin(HDC hdc, int startX = 10, int startY = 10, int lineHeight = 20);
};

#endif