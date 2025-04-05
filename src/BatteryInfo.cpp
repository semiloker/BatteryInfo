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

void batteryinfo_bi::PrintAllWinD2D(ID2D1HwndRenderTarget* pRT, int startX, int startY, int lineHeight)
{
    InitDirectWrite(); // не забудь викликати

    ID2D1SolidColorBrush* pTextBrush = nullptr;
    pRT->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &pTextBrush);

    ID2D1SolidColorBrush* pHeaderBrush = nullptr;
    pRT->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::SkyBlue), &pHeaderBrush);

    ID2D1SolidColorBrush* pSeparatorBrush = nullptr;
    pRT->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkSlateGray), &pSeparatorBrush);

    std::vector<std::pair<std::wstring, std::wstring>> fields = {
        {L"CHEMISTRY", std::wstring(info.Chemistry.begin(), info.Chemistry.end())},
        {L"DESIGNED CAPACITY", std::wstring(info.DesignedCapacity.begin(), info.DesignedCapacity.end())},
        {L"FULL CHARGED CAPACITY", std::wstring(info.FullChargedCapacity.begin(), info.FullChargedCapacity.end())},
        {L"DEFAULT ALERT1", std::wstring(info.DefaultAlert1.begin(), info.DefaultAlert1.end())},
        {L"DEFAULT ALERT2", std::wstring(info.DefaultAlert2.begin(), info.DefaultAlert2.end())},
        {L"WEAR LEVEL", std::wstring(info.WearLevel.begin(), info.WearLevel.end())},
        {L"VOLTAGE", std::wstring(info.Voltage.begin(), info.Voltage.end())},
        {L"RATE", std::wstring(info.Rate.begin(), info.Rate.end())},
        {L"POWER STATE", std::wstring(info.PowerState.begin(), info.PowerState.end())},
        {L"REMAINING CAPACITY", std::wstring(info.RemainingCapacity.begin(), info.RemainingCapacity.end())},
        {L"CHARGE LEVEL", std::wstring(info.ChargeLevel.begin(), info.ChargeLevel.end())},
    };

    int y = startY;

    std::wstring header = L"=== BATTERY INFORMATION ===";
    pRT->DrawText(
        header.c_str(),
        static_cast<UINT32>(header.length()),
        pTextFormat,
        D2D1::RectF((FLOAT)startX, (FLOAT)y, 800, (FLOAT)(y + lineHeight)),
        pHeaderBrush
    );
    y += lineHeight + 10;

    for (const auto& field : fields)
    {
        std::wstring line = field.first + L": " + field.second;
        pRT->DrawText(
            line.c_str(),
            static_cast<UINT32>(line.length()),
            pTextFormat,
            D2D1::RectF((FLOAT)startX, (FLOAT)y, 800, (FLOAT)(y + lineHeight)),
            pTextBrush
        );

        y += lineHeight;

        // Сепаратор
        pRT->DrawLine(
            D2D1::Point2F((FLOAT)startX, (FLOAT)y),
            D2D1::Point2F(800, (FLOAT)y),
            pSeparatorBrush,
            1.0f
        );
        y += 4;
    }

    pTextBrush->Release();
    pHeaderBrush->Release();
    pSeparatorBrush->Release();
}
