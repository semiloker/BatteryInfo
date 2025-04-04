#include <windows.h>
#include <initguid.h>
#include <setupapi.h>
#include <batclass.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "setupapi.lib")

DEFINE_GUID(GUID_DEVINTERFACE_BATTERY,
0x72631e54, 0x78a4, 0x11d0, 0xbc, 0xf7, 0x00, 0xaa, 0x00, 0xb7, 0xb3, 0x2a);

int main() {
    HDEVINFO hDevInfo = SetupDiGetClassDevs(&GUID_DEVINTERFACE_BATTERY, NULL, NULL,
                                            DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (hDevInfo == INVALID_HANDLE_VALUE) {
        printf("SetupDiGetClassDevs failed\n");
        return 1;
    }

    SP_DEVICE_INTERFACE_DATA did = { sizeof(SP_DEVICE_INTERFACE_DATA) };
    if (!SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &GUID_DEVINTERFACE_BATTERY, 0, &did)) {
        printf("No battery device found\n");
        SetupDiDestroyDeviceInfoList(hDevInfo);
        return 1;
    }

    DWORD size = 0;
    SetupDiGetDeviceInterfaceDetail(hDevInfo, &did, NULL, 0, &size, NULL);
    PSP_DEVICE_INTERFACE_DETAIL_DATA detail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(size);
    detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

    if (!SetupDiGetDeviceInterfaceDetail(hDevInfo, &did, detail, size, NULL, NULL)) {
        printf("SetupDiGetDeviceInterfaceDetail failed\n");
        free(detail);
        SetupDiDestroyDeviceInfoList(hDevInfo);
        return 1;
    }

    HANDLE hBattery = CreateFile(detail->DevicePath, GENERIC_READ | GENERIC_WRITE,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hBattery == INVALID_HANDLE_VALUE) {
        printf("Failed to open battery device\n");
        free(detail);
        SetupDiDestroyDeviceInfoList(hDevInfo);
        return 1;
    }

    // Get battery tag
    ULONG tag = 0;
    DWORD bytesReturned = 0;
    if (!DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_TAG,
                         NULL, 0,
                         &tag, sizeof(tag),
                         &bytesReturned, NULL) || tag == 0) {
        printf("Failed to get battery tag\n");
        CloseHandle(hBattery);
        free(detail);
        SetupDiDestroyDeviceInfoList(hDevInfo);
        return 1;
    }

    // Get battery info
    BATTERY_QUERY_INFORMATION bqi = {};
    bqi.BatteryTag = tag;
    bqi.InformationLevel = BatteryInformation;

    BATTERY_INFORMATION bi = {};
    if (!DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_INFORMATION,
                         &bqi, sizeof(bqi),
                         &bi, sizeof(bi),
                         &bytesReturned, NULL)) {
        printf("Failed to get battery info\n");
    } else {
        printf("Battery Chemistry: %.4s\n", bi.Chemistry);
        printf("Designed Capacity: %lu mWh\n", bi.DesignedCapacity);
        printf("Full Charged Capacity: %lu mWh\n", bi.FullChargedCapacity);
        printf("Default Alert1: %lu mWh\n", bi.DefaultAlert1);
        printf("Default Alert2: %lu mWh\n", bi.DefaultAlert2);

        // Wear Level
        if (bi.DesignedCapacity > 0) {
            int wear = 100 - (bi.FullChargedCapacity * 100 / bi.DesignedCapacity);
            printf("Wear Level: %d%%\n", wear);
        }
    }

    // Get battery status
    BATTERY_WAIT_STATUS bws = {};
    bws.BatteryTag = tag;

    BATTERY_STATUS bs = {};
    if (!DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_STATUS,
                         &bws, sizeof(bws),
                         &bs, sizeof(bs),
                         &bytesReturned, NULL)) {
        printf("Failed to get battery status\n");
    } else {
        printf("Voltage: %lu mV\n", bs.Voltage);
        printf("Rate: %ld mW (%s)\n", bs.Rate, (bs.PowerState & BATTERY_CHARGING) ? "Charging" :
                                                     (bs.PowerState & BATTERY_DISCHARGING) ? "Discharging" : "Idle");

        printf("Remaining Capacity: %lu mWh\n", bs.Capacity);

        if (bi.FullChargedCapacity > 0) {
            int percent = (bs.Capacity * 100) / bi.FullChargedCapacity;
            printf("Charge Level: %d%%\n", percent);
        }
    }

    CloseHandle(hBattery);
    free(detail);
    SetupDiDestroyDeviceInfoList(hDevInfo);

    return 0;
}
