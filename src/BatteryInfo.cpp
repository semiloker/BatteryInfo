#include "../include/BatteryInfo.h"

bool batteryinfo_bi::Initialize()
{
    hDevInfo = SetupDiGetClassDevs(&GUID_DEVINTERFACE_BATTERY, NULL, NULL,
        DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if (hDevInfo == INVALID_HANDLE_VALUE) 
        return false;

    SP_DEVICE_INTERFACE_DATA did = 
    { 
        sizeof(SP_DEVICE_INTERFACE_DATA) 
    };

    if (!SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &GUID_DEVINTERFACE_BATTERY, 0, &did)) 
        return false;

    DWORD size = 0;
    SetupDiGetDeviceInterfaceDetail(hDevInfo, &did, NULL, 0, &size, NULL);
    
    PSP_DEVICE_INTERFACE_DETAIL_DATA detail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(size);
    if (!detail) 
        return false;

    detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
    
    if (!SetupDiGetDeviceInterfaceDetail(hDevInfo, &did, detail, size, NULL, NULL)) 
    {
        free(detail);
        return false;
    }

    hBattery = CreateFile(detail->DevicePath, GENERIC_READ | GENERIC_WRITE,
    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    free(detail);

    if (hBattery == INVALID_HANDLE_VALUE) 
        return false;

    return QueryTag() && QueryBatteryInfo() && QueryBatteryStatus() && QueryBatteryRemaining() /*&& QueryRamInfo() && QueryCpuInfo()*/;
}

bool batteryinfo_bi::QueryTag() 
{
    DWORD bytesReturned = 0;
    if (!DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_TAG,
                         NULL, 0, &tag, sizeof(tag), &bytesReturned, NULL) || tag == 0)
        return false;
    return true;
}

bool batteryinfo_bi::QueryBatteryInfo() 
{
    BATTERY_QUERY_INFORMATION bqi = {};
    bqi.BatteryTag = tag;
    bqi.InformationLevel = BatteryInformation;

    DWORD bytesReturned = 0;
    if (!DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_INFORMATION,
                         &bqi, sizeof(bqi), &bi, sizeof(bi), &bytesReturned, NULL))
        return false;

    // writing to struct
    info_static.Chemistry = std::string((char*)bi.Chemistry, 4);
    info_static.DesignedCapacity = std::to_string(bi.DesignedCapacity) + " mWh (" + 
                            std::to_string(bi.DesignedCapacity / 1000.0) + " mW)";
    info_static.FullChargedCapacity = std::to_string(bi.FullChargedCapacity) + " mWh (" + 
                               std::to_string(bi.FullChargedCapacity / 1000.0) + " mW)";
    info_static.DefaultAlert1 = std::to_string(bi.DefaultAlert1) + " mWh (" + 
                         std::to_string(bi.DefaultAlert1 / 1000.0) + " mW)";
    info_static.DefaultAlert2 = std::to_string(bi.DefaultAlert2) + " mWh (" + 
                         std::to_string(bi.DefaultAlert2 / 1000.0) + " mW)";

    if (bi.DesignedCapacity > 0) 
    {
        int wear = 100 - (bi.FullChargedCapacity * 100 / bi.DesignedCapacity);
        info_static.WearLevel = std::to_string(wear) + "%";
    } 
    else 
    {
        info_static.WearLevel = "Unknown";
    }

    return true;
}

bool batteryinfo_bi::QueryBatteryStatus() 
{
    BATTERY_WAIT_STATUS bws = {};
    bws.BatteryTag = tag;

    DWORD bytesReturned = 0;
    if (!DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_STATUS,
                         &bws, sizeof(bws), &bs, sizeof(bs), &bytesReturned, NULL))
        return false;

    double voltage = bs.Voltage / 1000.0;
    std::ostringstream voltageStream;
    voltageStream << bs.Voltage << " mV (" << std::fixed << std::setprecision(3) << voltage << " V)";
    info_1s.Voltage = voltageStream.str();

    double rate = bs.Rate / 1000.0; 
    std::ostringstream rateStream;
    rateStream << bs.Rate << " mW (" << std::fixed << std::setprecision(3) << rate << " W)";
    info_1s.Rate = rateStream.str();

    info_1s.PowerState =
        (bs.PowerState & BATTERY_CHARGING) ? "Charging" :
        (bs.PowerState & BATTERY_DISCHARGING) ? "Discharging" : "Idle";

    std::ostringstream capacityStream;
    capacityStream << bs.Capacity << " mWh (" << std::fixed << std::setprecision(3) << bs.Capacity / 1000.0 << " Wh)";
    info_1s.RemainingCapacity = capacityStream.str();

    if (bi.FullChargedCapacity > 0)
    {
        double percent = (bs.Capacity * 100.0) / bi.FullChargedCapacity;
        info_1s.ChargeLevel = std::to_string(percent) + "%";
    }

    return true;
}

bool batteryinfo_bi::QueryBatteryRemaining()
{
    if (bi.FullChargedCapacity > 0) 
    {    
        if ((bs.PowerState & BATTERY_DISCHARGING) && bs.Rate != 0)
        {
            int rate_mW = abs(bs.Rate); // +
            if (rate_mW > 0) 
            {
                int remainingMinutes = (bs.Capacity * 60) / rate_mW;
                int hours = remainingMinutes / 60;
                int minutes = remainingMinutes % 60;
                info_10s.TimeRemaining = std::to_string(hours) + "h. " + std::to_string(minutes) + "m. (" +
                    std::to_string(remainingMinutes) + " min, based on Capacity / Rate)";
            }
            else 
            {
                info_10s.TimeRemaining = "Calculating...";
            }
        }
        else
        {
            info_10s.TimeRemaining = "Not discharging";
        }
    
        if ((bs.PowerState & BATTERY_CHARGING) && bs.Rate != 0)
        {
            int rate_mW = abs(bs.Rate); // +
            int remainingCapacity = bi.FullChargedCapacity - bs.Capacity;
            if (rate_mW > 0)
            {
                int timeToFullMinutes = (remainingCapacity * 60) / rate_mW;
                int fullChargeHours = timeToFullMinutes / 60;
                int fullChargeMinutes = timeToFullMinutes % 60;
    
                info_10s.TimeToFullCharge = std::to_string(fullChargeHours) + "h. " +
                    std::to_string(fullChargeMinutes) + "m. (" +
                    std::to_string(timeToFullMinutes) + " min, based on (FullChargedCapacity - Capacity) / Rate)";
            }
            else 
            {
                info_10s.TimeToFullCharge = "Calculating...";
            }
        }
        else
        {
            info_10s.TimeToFullCharge = "Not Charging";
        }
    }

    return true;
}

// bool batteryinfo_bi::QueryRamInfo() 
// {
//     MEMORYSTATUSEX statex = { sizeof(statex) };

//     if (GlobalMemoryStatusEx(&statex)) {
//         DWORDLONG total = statex.ullTotalPhys;
//         DWORDLONG avail = statex.ullAvailPhys;
//         DWORDLONG used = total - avail;
//         DWORD usagePercent = statex.dwMemoryLoad;

//         ramInfo.TotalPhys = std::to_string(total / (1024 * 1024)) + " MB";
//         ramInfo.AvailPhys = std::to_string(avail / (1024 * 1024)) + " MB";
//         ramInfo.UsedPercent = std::to_string(usagePercent) + "%";
//         return true;
//     }
//     return false;
// }

// bool batteryinfo_bi::QueryCpuInfo() 
// {
//     FILETIME idleTime, kernelTime, userTime;
//     if (!GetSystemTimes(&idleTime, &kernelTime, &userTime))
//         return false;

//     ULARGE_INTEGER idle, kernel, user, prevIdle, prevKernel, prevUser;
//     idle.LowPart = idleTime.dwLowDateTime;
//     idle.HighPart = idleTime.dwHighDateTime;

//     kernel.LowPart = kernelTime.dwLowDateTime;
//     kernel.HighPart = kernelTime.dwHighDateTime;

//     user.LowPart = userTime.dwLowDateTime;
//     user.HighPart = userTime.dwHighDateTime;

//     prevIdle.LowPart = prevIdleTime.dwLowDateTime;
//     prevIdle.HighPart = prevIdleTime.dwHighDateTime;

//     prevKernel.LowPart = prevKernelTime.dwLowDateTime;
//     prevKernel.HighPart = prevKernelTime.dwHighDateTime;

//     prevUser.LowPart = prevUserTime.dwLowDateTime;
//     prevUser.HighPart = prevUserTime.dwHighDateTime;

//     ULONGLONG sysTime = (kernel.QuadPart - prevKernel.QuadPart) + (user.QuadPart - prevUser.QuadPart);
//     ULONGLONG idleDiff = idle.QuadPart - prevIdle.QuadPart;

//     if (sysTime == 0)
//         return false;

//     int cpuUsage = static_cast<int>(100.0 * (sysTime - idleDiff) / sysTime);
//     cpuInfo.UsagePercent = std::to_string(cpuUsage) + "%";

//     // збереження нових значень
//     prevIdleTime = idleTime;
//     prevKernelTime = kernelTime;
//     prevUserTime = userTime;

//     return true;
// }

void batteryinfo_bi::PrintAllConsole() const 
{
    // std::cout << "Chemistry: " << info.Chemistry << "\n"
    //           << "Designed Capacity: " << info.DesignedCapacity << "\n"
    //           << "Full Charged Capacity: " << info.FullChargedCapacity << "\n"
    //           << "Default Alert1: " << info.DefaultAlert1 << "\n"
    //           << "Default Alert2: " << info.DefaultAlert2 << "\n"
    //           << "Wear Level: " << info.WearLevel << "\n"
    //           << "Voltage: " << info.Voltage << "\n"
    //           << "Rate: " << info.Rate << "\n"
    //           << "Power State: " << info.PowerState << "\n"
    //           << "Remaining Capacity: " << info.RemainingCapacity << "\n"
    //           << "Charge Level: " << info.ChargeLevel << "\n";
}