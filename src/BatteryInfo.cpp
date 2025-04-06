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

    return QueryTag() && QueryBatteryInfo() && QueryBatteryStatus() && QueryBatteryRemaining();
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

void batteryinfo_bi::PrintAllWinD2D(ID2D1HwndRenderTarget* pRT, int startX, int startY, int lineHeight)
{
    InitDirectWrite();

    // win size
    D2D1_SIZE_F rtSize = pRT->GetSize();
    FLOAT maxWidth = rtSize.width;

    D2D1_COLOR_F textColor = D2D1::ColorF(0.1f, 0.1f, 0.1f);            // Темно-сірий текст
    D2D1_COLOR_F labelColor = D2D1::ColorF(0.4f, 0.4f, 0.4f);           // Сірий для назв
    D2D1_COLOR_F separatorColor = D2D1::ColorF(0.8f, 0.8f, 0.8f);       // Світло-сірий розділювач
    D2D1_COLOR_F backgroundColor = D2D1::ColorF(0.98f, 0.98f, 0.98f);   // Майже білий фон
    D2D1_COLOR_F headerColor = D2D1::ColorF(0.2f, 0.4f, 0.8f);          // Акцент (синій)

    pRT->Clear(backgroundColor);

    ID2D1SolidColorBrush* pLabelBrush = nullptr;
    ID2D1SolidColorBrush* pValueBrush = nullptr;
    ID2D1SolidColorBrush* pHeaderBrush = nullptr;
    ID2D1SolidColorBrush* pSeparatorBrush = nullptr;

    pRT->CreateSolidColorBrush(labelColor, &pLabelBrush);
    pRT->CreateSolidColorBrush(textColor, &pValueBrush);
    pRT->CreateSolidColorBrush(headerColor, &pHeaderBrush);
    pRT->CreateSolidColorBrush(separatorColor, &pSeparatorBrush);

    std::map<std::wstring, std::vector<std::pair<std::wstring, std::wstring>>> categories = {
        {L"Basic Info", {
            {L"Chemistry", std::wstring(info_static.Chemistry.begin(), info_static.Chemistry.end())},
            {L"Power state", std::wstring(info_1s.PowerState.begin(), info_1s.PowerState.end())},
        }},
        {L"Capacity", {
            {L"Designed capacity", std::wstring(info_static.DesignedCapacity.begin(), info_static.DesignedCapacity.end())},
            {L"Full charged capacity", std::wstring(info_static.FullChargedCapacity.begin(), info_static.FullChargedCapacity.end())},
            {L"Remaining capacity", std::wstring(info_1s.RemainingCapacity.begin(), info_1s.RemainingCapacity.end())},
            {L"Charge level", std::wstring(info_1s.ChargeLevel.begin(), info_1s.ChargeLevel.end())},
            {L"Wear level", std::wstring(info_static.WearLevel.begin(), info_static.WearLevel.end())},
        }},
        {L"Voltage & Rate", {
            {L"Voltage", std::wstring(info_1s.Voltage.begin(), info_1s.Voltage.end())},
            {L"Rate", std::wstring(info_1s.Rate.begin(), info_1s.Rate.end())},
        }},
        {L"Alerts", {
            {L"Default alert 1", std::wstring(info_static.DefaultAlert1.begin(), info_static.DefaultAlert1.end())},
            {L"Default alert 2", std::wstring(info_static.DefaultAlert2.begin(), info_static.DefaultAlert2.end())},
        }},
        {L"Time Remaining", {
            {L"Time to 0%", std::wstring(info_10s.TimeRemaining.begin(), info_10s.TimeRemaining.end())},
            {L"Time to full charge", std::wstring(info_10s.TimeToFullCharge.begin(), info_10s.TimeToFullCharge.end())},
        }}
    };

    int y = startY;

    std::wstring header = L"Battery Status";
    pRT->DrawText(
        header.c_str(),
        static_cast<UINT32>(header.length()),
        pTextFormatHeader,
        D2D1::RectF((FLOAT)startX, (FLOAT)y, maxWidth, (FLOAT)(y + lineHeight + 8)),
        pHeaderBrush
    );
    y += lineHeight + 16;

    for (const auto& category : categories)
    {
        // category
        pRT->DrawText(
            category.first.c_str(),
            static_cast<UINT32>(category.first.length()),
            pTextFormatLabel,
            D2D1::RectF((FLOAT)startX, (FLOAT)y, maxWidth, (FLOAT)(y + lineHeight)),
            pHeaderBrush
        );
        y += lineHeight + 4;

        for (const auto& field : category.second)
        {
            std::wstring line = field.first + L" - " + field.second;

            pRT->DrawText(
                line.c_str(),
                static_cast<UINT32>(line.length()),
                pTextFormatValue,
                D2D1::RectF((FLOAT)startX, (FLOAT)y, maxWidth, (FLOAT)(y + lineHeight)),
                pValueBrush
            );
            y += lineHeight;

            pRT->DrawLine(
                D2D1::Point2F((FLOAT)startX, (FLOAT)(y + 2)),
                D2D1::Point2F(maxWidth, (FLOAT)(y + 2)),
                pSeparatorBrush,
                0.5f
            );
            y += 8;
        }
        y += 12;
    }

    // clear
    pLabelBrush->Release();
    pValueBrush->Release();
    pHeaderBrush->Release();
    pSeparatorBrush->Release();
}