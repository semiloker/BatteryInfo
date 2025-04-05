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

    return QueryTag() && QueryBatteryInfo() && QueryBatteryStatus();
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
    info.Chemistry = std::string((char*)bi.Chemistry, 4);
    info.DesignedCapacity = std::to_string(bi.DesignedCapacity) + " mWh";
    info.FullChargedCapacity = std::to_string(bi.FullChargedCapacity) + " mWh";
    info.DefaultAlert1 = std::to_string(bi.DefaultAlert1) + " mWh";
    info.DefaultAlert2 = std::to_string(bi.DefaultAlert2) + " mWh";

    if (bi.DesignedCapacity > 0) 
    {
        int wear = 100 - (bi.FullChargedCapacity * 100 / bi.DesignedCapacity);
        info.WearLevel = std::to_string(wear) + "%";
    } 
    else 
    {
        info.WearLevel = "Unknown";
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

        // writing to struct
    info.Voltage = std::to_string(bs.Voltage) + " mV";
    info.Rate = std::to_string(bs.Rate) + " mW";
    info.PowerState =
        (bs.PowerState & BATTERY_CHARGING) ? "Charging" :
        (bs.PowerState & BATTERY_DISCHARGING) ? "Discharging" : "Idle";

    info.RemainingCapacity = std::to_string(bs.Capacity) + " mWh";

    if (bi.FullChargedCapacity > 0) 
    {
        int percent = (bs.Capacity * 100) / bi.FullChargedCapacity;
        info.ChargeLevel = std::to_string(percent) + "%";
    } 
    else 
    {
        info.ChargeLevel = "Unknown";
    }

    return true;
}

void batteryinfo_bi::PrintAllConsole() const 
{
    std::cout << "Chemistry: " << info.Chemistry << "\n"
              << "Designed Capacity: " << info.DesignedCapacity << "\n"
              << "Full Charged Capacity: " << info.FullChargedCapacity << "\n"
              << "Default Alert1: " << info.DefaultAlert1 << "\n"
              << "Default Alert2: " << info.DefaultAlert2 << "\n"
              << "Wear Level: " << info.WearLevel << "\n"
              << "Voltage: " << info.Voltage << "\n"
              << "Rate: " << info.Rate << "\n"
              << "Power State: " << info.PowerState << "\n"
              << "Remaining Capacity: " << info.RemainingCapacity << "\n"
              << "Charge Level: " << info.ChargeLevel << "\n";
}

void batteryinfo_bi::PrintAllWin(HDC hdc, int startX, int startY, int lineHeight)
{
    OutputDebugStringA(("DEBUG Chemistry = " + info.Chemistry + "\n").c_str());

    std::vector<std::string> lines = 
    {
        "Chemistry: " + info.Chemistry,
        "Designed Capacity: " + info.DesignedCapacity,
        "Full Charged Capacity: " + info.FullChargedCapacity,
        "Default Alert1: " + info.DefaultAlert1,
        "Default Alert2: " + info.DefaultAlert2,
        "Wear Level: " + info.WearLevel,
        "Voltage: " + info.Voltage,
        "Rate: " + info.Rate,
        "Power State: " + info.PowerState,
        "Remaining Capacity: " + info.RemainingCapacity,
        "Charge Level: " + info.ChargeLevel
    };

    int y = startY;
    for (const auto& line : lines) 
    {
        TextOutA(hdc, startX, y, line.c_str(), static_cast<int>(line.length()));
        y += lineHeight;
    }
}