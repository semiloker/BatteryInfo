#include "../include/draw_batteryinfo_bi.h"

bool draw_batteryinfo_bi::initBrush(ID2D1HwndRenderTarget* pRT)
{
    pRT->CreateSolidColorBrush(labelColor, &pLabelBrush);
    pRT->CreateSolidColorBrush(textColor, &pValueBrush);
    pRT->CreateSolidColorBrush(headerColor, &pHeaderBrush);
    pRT->CreateSolidColorBrush(separatorColor, &pSeparatorBrush);
    
    return true;
}

void draw_batteryinfo_bi::drawHeaderBatteryInfoD2D(ID2D1HwndRenderTarget* pRT, batteryinfo_bi* bi_bi, init_dwrite_bi* initdwrite_bi, int startX, int startY, int lineHeight)
{
    D2D1_SIZE_F rtSize = pRT->GetSize();
    maxWidth = rtSize.width;

    pRT->Clear(backgroundColor);

    std::map<std::wstring, std::vector<std::pair<std::wstring, std::wstring>>> categories = {
        {L"Basic Info", {
            {L"Chemistry", std::wstring(bi_bi->info_static.Chemistry.begin(), bi_bi->info_static.Chemistry.end())},
            {L"Power state", std::wstring(bi_bi->info_1s.PowerState.begin(), bi_bi->info_1s.PowerState.end())},
        }},
        {L"Capacity", {
            {L"Designed capacity", std::wstring(bi_bi->info_static.DesignedCapacity.begin(), bi_bi->info_static.DesignedCapacity.end())},
            {L"Full charged capacity", std::wstring(bi_bi->info_static.FullChargedCapacity.begin(), bi_bi->info_static.FullChargedCapacity.end())},
            {L"Remaining capacity", std::wstring(bi_bi->info_1s.RemainingCapacity.begin(), bi_bi->info_1s.RemainingCapacity.end())},
            {L"Charge level", std::wstring(bi_bi->info_1s.ChargeLevel.begin(), bi_bi->info_1s.ChargeLevel.end())},
            {L"Wear level", std::wstring(bi_bi->info_static.WearLevel.begin(), bi_bi->info_static.WearLevel.end())},
        }},
        {L"Voltage & Rate", {
            {L"Voltage", std::wstring(bi_bi->info_1s.Voltage.begin(), bi_bi->info_1s.Voltage.end())},
            {L"Rate", std::wstring(bi_bi->info_1s.Rate.begin(), bi_bi->info_1s.Rate.end())},
        }},
        {L"Alerts", {
            {L"Default alert 1", std::wstring(bi_bi->info_static.DefaultAlert1.begin(), bi_bi->info_static.DefaultAlert1.end())},
            {L"Default alert 2", std::wstring(bi_bi->info_static.DefaultAlert2.begin(), bi_bi->info_static.DefaultAlert2.end())},
        }},
        {L"Time Remaining", {
            {L"Time to 0%", std::wstring(bi_bi->info_10s.TimeRemaining.begin(), bi_bi->info_10s.TimeRemaining.end())},
            {L"Time to full charge", std::wstring(bi_bi->info_10s.TimeToFullCharge.begin(), bi_bi->info_10s.TimeToFullCharge.end())},
        }}
    };

    int y = startY;

    std::wstring header = L"Battery Status";
    pRT->DrawText(
        header.c_str(),
        static_cast<UINT32>(header.length()),
        initdwrite_bi->pTextFormatHeader,
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
            initdwrite_bi->pTextFormatLabel,
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
                initdwrite_bi->pTextFormatValue,
                D2D1::RectF((FLOAT)startX, (FLOAT)y, maxWidth, (FLOAT)(y + lineHeight)),
                pValueBrush
            );
            y += lineHeight;

            pRT->DrawLine(
                D2D1::Point2F((FLOAT)startX, (FLOAT)(y + 2)),
                D2D1::Point2F(maxWidth - startX, (FLOAT)(y + 2)),
                pSeparatorBrush,
                0.5f
            );
            y += 8;
        }
        y += 12;
    }
}

void drawHeaderSettingsD2D(ID2D1HwndRenderTarget* pRT, init_dwrite_bi* initdwrite_bi)
{

}