#include "../include/BatteryInfo.h"

bool BatteryInfo::Init()
{
    hDevInfo = SetupDiGetClassDevs(&GUID_DEVINTERFACE_BATTERY, NULL, NULL,
        DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (hDevInfo == INVALID_HANDLE_VALUE) {
    std::cerr << "SetupDiGetClassDevs failed\n";
    return false;
    }

    SP_DEVICE_INTERFACE_DATA did = { sizeof(SP_DEVICE_INTERFACE_DATA) };
    if (!SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &GUID_DEVINTERFACE_BATTERY, 0, &did)) {
    std::cerr << "No battery device found\n";
    return false;
    }

    DWORD size = 0;
    SetupDiGetDeviceInterfaceDetail(hDevInfo, &did, NULL, 0, &size, NULL);
    detail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(size);
    if (!detail) {
    std::cerr << "Memory allocation failed\n";
    return false;
    }
    detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

    if (!SetupDiGetDeviceInterfaceDetail(hDevInfo, &did, detail, size, NULL, NULL)) {
    std::cerr << "SetupDiGetDeviceInterfaceDetail failed\n";
    return false;
    }

    hBattery = CreateFile(detail->DevicePath, GENERIC_READ | GENERIC_WRITE,
    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hBattery == INVALID_HANDLE_VALUE) {
    std::cerr << "Failed to open battery device\n";
    return false;
    }

    DWORD bytesReturned = 0;
    if (!DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_TAG,
    NULL, 0,
    &tag, sizeof(tag),
    &bytesReturned, NULL) || tag == 0) {
    std::cerr << "Failed to get battery tag\n";
    return false;
    }

    return true;
}

void BatteryInfo::PrintBatteryInfo() 
{
    BATTERY_QUERY_INFORMATION bqi = {};
    bqi.BatteryTag = tag;
    bqi.InformationLevel = BatteryInformation;

    BATTERY_INFORMATION bi = {};
    DWORD bytesReturned = 0;
    if (!DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_INFORMATION,
                         &bqi, sizeof(bqi),
                         &bi, sizeof(bi),
                         &bytesReturned, NULL)) {
        std::cerr << "Failed to get battery info\n";
        return;
    }

    std::cout << "Battery Chemistry: " << std::string((char*)bi.Chemistry, 4) << "\n";
    std::cout << "Designed Capacity: " << bi.DesignedCapacity << " mWh\n";
    std::cout << "Full Charged Capacity: " << bi.FullChargedCapacity << " mWh\n";
    std::cout << "Default Alert1: " << bi.DefaultAlert1 << " mWh\n";
    std::cout << "Default Alert2: " << bi.DefaultAlert2 << " mWh\n";

    if (bi.DesignedCapacity > 0) {
        int wear = 100 - (bi.FullChargedCapacity * 100 / bi.DesignedCapacity);
        std::cout << "Wear Level: " << wear << "%\n";
    }

    this->fullChargedCapacity = bi.FullChargedCapacity;
}

void BatteryInfo::PrintBatteryStatus() 
{
    BATTERY_WAIT_STATUS bws = {};
    bws.BatteryTag = tag;

    BATTERY_STATUS bs = {};
    DWORD bytesReturned = 0;

    if (!DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_STATUS,
                         &bws, sizeof(bws),
                         &bs, sizeof(bs),
                         &bytesReturned, NULL)) {
        std::cerr << "Failed to get battery status\n";
        return;
    }

    std::cout << "Voltage: " << bs.Voltage << " mV\n";
    std::cout << "Rate: " << bs.Rate << " mW ("
              << ((bs.PowerState & BATTERY_CHARGING) ? "Charging" :
                 (bs.PowerState & BATTERY_DISCHARGING) ? "Discharging" : "Idle") << ")\n";
    std::cout << "Remaining Capacity: " << bs.Capacity << " mWh\n";

    if (fullChargedCapacity > 0) {
        int percent = (bs.Capacity * 100) / fullChargedCapacity;
        std::cout << "Charge Level: " << percent << "%\n";
    }
}

std::string BatteryInfo::GetBatteryInfoText() {
    std::ostringstream oss;
    BATTERY_QUERY_INFORMATION bqi = { tag, BatteryInformation };
    BATTERY_INFORMATION bi = {};
    DWORD br = 0;

    if (DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_INFORMATION,
                        &bqi, sizeof(bqi),
                        &bi, sizeof(bi), &br, NULL)) {

        oss << "🔋 Battery Info\n";
        oss << "------------------------------\n";
        oss << "🔹 Chemistry: " << std::string((char*)bi.Chemistry, 4) << "\n";
        oss << "🔹 Designed Capacity: " << bi.DesignedCapacity << " mWh\n";
        oss << "🔹 Full Charged Capacity: " << bi.FullChargedCapacity << " mWh\n";
        oss << "🔹 Alert1: " << bi.DefaultAlert1 << " mWh\n";
        oss << "🔹 Alert2: " << bi.DefaultAlert2 << " mWh\n";
        if (bi.DesignedCapacity > 0) {
            int wear = 100 - (bi.FullChargedCapacity * 100 / bi.DesignedCapacity);
            oss << "🔹 Wear Level: " << wear << "%\n";
        }
        oss << "------------------------------\n";
        fullChargedCapacity = bi.FullChargedCapacity;
    } else {
        oss << "❌ Failed to read battery info.\n";
        oss << "------------------------------\n";
    }
    return oss.str();
}

std::string BatteryInfo::GetBatteryStatusText() {
    std::ostringstream oss;
    BATTERY_WAIT_STATUS bws = { tag };
    BATTERY_STATUS bs = {};
    DWORD br = 0;

    if (DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_STATUS,
                        &bws, sizeof(bws),
                        &bs, sizeof(bs), &br, NULL)) {

        oss << "⚡ Battery Status\n";
        oss << "------------------------------\n";
        oss << "🔸 Voltage: " << bs.Voltage << " mV\n";
        oss << "🔸 Rate: " << bs.Rate << " mW ("
            << ((bs.PowerState & BATTERY_CHARGING) ? "Charging" :
                (bs.PowerState & BATTERY_DISCHARGING) ? "Discharging" : "Idle") << ")\n";
        oss << "🔸 Remaining Capacity: " << bs.Capacity << " mWh\n";
        if (fullChargedCapacity > 0) {
            int percent = (bs.Capacity * 100) / fullChargedCapacity;
            oss << "🔸 Charge Level: " << percent << "%\n";
        }
        oss << "------------------------------\n";
    } else {
        oss << "❌ Failed to read battery status.\n";
        oss << "------------------------------\n";
    }
    return oss.str();    
}    