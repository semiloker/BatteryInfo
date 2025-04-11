#ifndef BATTERYINFO_H
#define BATTERYINFO_H

#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <pdh.h>
#include <algorithm>
#include <map>
#include <iomanip>
#include <iostream>
#include <sstream>
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
    struct bi_struct_static 
    {
        std::string Chemistry;
        std::string DesignedCapacity;
        std::string FullChargedCapacity;
        std::string DefaultAlert1;
        std::string DefaultAlert2;
        std::string WearLevel;
    };

    struct bi_struct_dynamic_1s
    {
        std::string Voltage;
        std::string Rate;
        std::string PowerState;
        std::string RemainingCapacity;
        std::string ChargeLevel;
    };

    struct bi_struct_dynamic_10s
    {
        std::string TimeRemaining;
        std::string TimeToFullCharge;
    };

    bi_struct_static info_static;
    bi_struct_dynamic_1s info_1s;
    bi_struct_dynamic_10s info_10s;

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
    bool QueryBatteryRemaining();

    // FILETIME prevIdleTime = {}, prevKernelTime = {}, prevUserTime = {};

    // bool QueryRamInfo();
    // bool QueryCpuInfo();

    void PrintAllConsole() const;
};

#endif