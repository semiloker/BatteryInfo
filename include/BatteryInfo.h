#ifndef BATTERYINFO_H
#define BATTERYINFO_H

#include <sstream>
#include <windows.h>
#include <initguid.h>
#include <setupapi.h>
#include <batclass.h>
#include <iostream>

#pragma comment(lib, "setupapi.lib")

DEFINE_GUID(GUID_DEVINTERFACE_BATTERY,
0x72631e54, 0x78a4, 0x11d0, 0xbc, 0xf7, 0x00, 0xaa, 0x00, 0xb7, 0xb3, 0x2a);

class BatteryInfo 
{
public:
    BatteryInfo() : hDevInfo(INVALID_HANDLE_VALUE), hBattery(INVALID_HANDLE_VALUE), detail(nullptr), tag(0) {}

    ~BatteryInfo() 
    {
        if (hBattery != INVALID_HANDLE_VALUE)
            CloseHandle(hBattery);
        if (detail)
            free(detail);
        if (hDevInfo != INVALID_HANDLE_VALUE)
            SetupDiDestroyDeviceInfoList(hDevInfo);
    }

    bool Init();
    void PrintBatteryInfo();
    void PrintBatteryStatus();

    std::string GetBatteryInfoText();
    std::string GetBatteryStatusText();  

private:
    HDEVINFO hDevInfo;
    HANDLE hBattery;
    PSP_DEVICE_INTERFACE_DETAIL_DATA detail;
    ULONG tag;
    ULONG fullChargedCapacity = 0;
};

#endif