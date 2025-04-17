#include "../include/draw_batteryinfo_bi.h"

bool draw_batteryinfo_bi::initBrush(ID2D1HwndRenderTarget* pRT)
{
    pRT->CreateSolidColorBrush(labelColor, &pLabelBrush);
    pRT->CreateSolidColorBrush(textColor, &pValueBrush);
    pRT->CreateSolidColorBrush(headerColor, &pHeaderBrush);
    pRT->CreateSolidColorBrush(separatorColor, &pSeparatorBrush);
    
    return true;
}

bool draw_batteryinfo_bi::clearBackground(ID2D1HwndRenderTarget* pRT)
{
    pRT->Clear(backgroundColor);
    return true;
}

bool draw_batteryinfo_bi::isCursorInBatteryStatus(POINT cursorPos)
{
    return (cursorPos.x >= rectBatteryStatus.left && cursorPos.x <= rectBatteryStatus.right &&
            cursorPos.y >= rectBatteryStatus.top && cursorPos.y <= rectBatteryStatus.bottom);
}

bool draw_batteryinfo_bi::isCursorInSettings(POINT cursorPos)
{
    return (cursorPos.x >= rectSettings.left && cursorPos.x <= rectSettings.right &&
            cursorPos.y >= rectSettings.top && cursorPos.y <= rectSettings.bottom);
}

void draw_batteryinfo_bi::drawHeaders(ID2D1HwndRenderTarget* pRT, init_dwrite_bi* initdwrite_bi, int startX, int startY, int lineHeight)
{
    std::wstring header_battery_status = L"Battery Status";
    std::wstring header_settings = L"Settings";

    float currentX = (float)startX;
    float currentY = (float)startY;

    int headerIndex = 0;

    pRT->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);

    auto drawHeaderWithBox = [&](const std::wstring& text) {
        IDWriteTextLayout* pTextLayout = nullptr;
        HRESULT hr = initdwrite_bi->pDWriteFactory->CreateTextLayout(
            text.c_str(),
            static_cast<UINT32>(text.length()),
            initdwrite_bi->pTextFormatHeader,
            1000.0f,
            100.0f,
            &pTextLayout
        );

        if (SUCCEEDED(hr) && pTextLayout)
        {
            DWRITE_TEXT_METRICS metrics;
            if (SUCCEEDED(pTextLayout->GetMetrics(&metrics)))
            {
                float width = metrics.width;
                float height = metrics.height;

                D2D1_RECT_F rect = D2D1::RectF(currentX, currentY, currentX + width, currentY + height);

                if (headerIndex == 0)
                    rectBatteryStatus = rect;
                else if (headerIndex == 1)
                    rectSettings = rect;

                pRT->DrawText(
                    text.c_str(),
                    static_cast<UINT32>(text.length()),
                    initdwrite_bi->pTextFormatHeader,
                    rect,
                    pHeaderBrush
                );

                pRT->DrawRectangle(rect, pHeaderBrush);

                currentX += width + 30.0f;
                headerIndex++;
            }
            pTextLayout->Release();
        }
    };

    drawHeaderWithBox(header_battery_status);
    drawHeaderWithBox(header_settings);
}

void draw_batteryinfo_bi::drawHeaderBatteryInfoD2D(ID2D1HwndRenderTarget* pRT, batteryinfo_bi* bi_bi, init_dwrite_bi* initdwrite_bi, int startX, int startY, int lineHeight)
{
    D2D1_SIZE_F rtSize = pRT->GetSize();
    maxWidth = rtSize.width;

    // pRT->Clear(backgroundColor);

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

void draw_batteryinfo_bi::drawHeaderSettingsD2D(ID2D1HwndRenderTarget* pRT, init_dwrite_bi* initdwrite_bi)
{
    std::wstring settingsTitle = L"Settings Page (stub)";
    pRT->DrawText(
        settingsTitle.c_str(),
        (UINT32)settingsTitle.length(),
        initdwrite_bi->pTextFormatValue,
        D2D1::RectF(20, 80, maxWidth, 120),
        pValueBrush
    );
}