#ifndef BATTERYINFO_H
#define BATTERYINFO_H

#include <windows.h>
#include <iostream>
#include <vector>
#include <initguid.h>
#include <setupapi.h>
#include <batclass.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#pragma comment(lib, "setupapi.lib")

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
    void PrintAllWin(HDC hdc, int startX = 10, int startY = 10, int lineHeight = 20);
};

#endif