#include "../include/draw_batteryinfo_bi.h"

bool draw_batteryinfo_bi::initBrush(ID2D1HwndRenderTarget *pRT)
{
    pRT->CreateSolidColorBrush(labelColor, &pLabelBrush);
    pRT->CreateSolidColorBrush(textColor, &pValueBrush);
    pRT->CreateSolidColorBrush(headerColor, &pHeaderBrush);
    pRT->CreateSolidColorBrush(separatorColor, &pSeparatorBrush);

    pRT->CreateSolidColorBrush(D2D1::ColorF(0.2f, 0.6f, 0.3f), &pSwitchOnBrush);    // Зелений для ON
    pRT->CreateSolidColorBrush(D2D1::ColorF(0.7f, 0.7f, 0.7f), &pSwitchOffBrush);   // Сірий для OFF
    pRT->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f), &pSwitchKnobBrush);  // Білий для кнопки

    return true;
}

bool draw_batteryinfo_bi::clearBackground(ID2D1HwndRenderTarget *pRT)
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

void draw_batteryinfo_bi::drawHeaders(ID2D1HwndRenderTarget *pRT, init_dwrite_bi *initdwrite_bi, int startX, int startY, int lineHeight)
{
    std::wstring header_battery_status = L"Battery Status";
    std::wstring header_settings = L"Settings";

    float currentX = (float)startX;
    float currentY = (float)startY;

    int headerIndex = 0;

    pRT->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);

    auto drawHeaderWithBox = [&](const std::wstring &text, bool isSelected)
    {
        IDWriteTextLayout *pTextLayout = nullptr;
        HRESULT hr = initdwrite_bi->pDWriteFactory->CreateTextLayout(
            text.c_str(),
            static_cast<UINT32>(text.length()),
            initdwrite_bi->pTextFormatHeader,
            1000.0f,
            100.0f,
            &pTextLayout);

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

                D2D1_COLOR_F currentTextColor = (isSelected) ? D2D1::ColorF(0.2f, 0.4f, 0.8f) : D2D1::ColorF(0.5f, 0.5f, 0.5f);

                ID2D1SolidColorBrush *pTextBrush;
                pRT->CreateSolidColorBrush(currentTextColor, &pTextBrush);

                pRT->DrawText(
                    text.c_str(),
                    static_cast<UINT32>(text.length()),
                    initdwrite_bi->pTextFormatHeader,
                    rect,
                    pTextBrush);

                pRT->DrawRectangle(rect, pHeaderBrush);

                currentX += width + 30.0f;
                headerIndex++;

                pTextBrush->Release();
            }
            pTextLayout->Release();
        }
    };

    drawHeaderWithBox(header_battery_status, selectedTab == BATTERY_INFO);
    drawHeaderWithBox(header_settings, selectedTab == SETTINGS);
}

void draw_batteryinfo_bi::drawHeaderBatteryInfoD2D(ID2D1HwndRenderTarget *pRT, batteryinfo_bi *bi_bi, init_dwrite_bi *initdwrite_bi, int startX, int startY, int lineHeight)
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
                            }}};

    int y = startY;

    y += lineHeight + 16;

    for (const auto &category : categories)
    {
        // category
        pRT->DrawText(
            category.first.c_str(),
            static_cast<UINT32>(category.first.length()),
            initdwrite_bi->pTextFormatLabel,
            D2D1::RectF((FLOAT)startX, (FLOAT)y, maxWidth, (FLOAT)(y + lineHeight)),
            pHeaderBrush);
        y += lineHeight + 4;

        for (const auto &field : category.second)
        {
            std::wstring line = field.first + L" - " + field.second;

            pRT->DrawText(
                line.c_str(),
                static_cast<UINT32>(line.length()),
                initdwrite_bi->pTextFormatValue,
                D2D1::RectF((FLOAT)startX, (FLOAT)y, maxWidth, (FLOAT)(y + lineHeight)),
                pValueBrush);
            y += lineHeight;

            pRT->DrawLine(
                D2D1::Point2F((FLOAT)startX, (FLOAT)(y + 2)),
                D2D1::Point2F(maxWidth - startX, (FLOAT)(y + 2)),
                pSeparatorBrush,
                0.5f);
            y += 8;
        }
        y += 12;
    }
}

void draw_batteryinfo_bi::drawToggleSwitch(ID2D1HwndRenderTarget* pRT, init_dwrite_bi* initdwrite_bi, 
                                         float x, float y, bool& toggleState, const std::wstring& labelText)
{
    D2D1_RECT_F textRect = D2D1::RectF(x, y, x + 250, y + 24);
    pRT->DrawText(
        labelText.c_str(),
        (UINT32)labelText.length(),
        initdwrite_bi->pTextFormatLabel,
        textRect,
        pLabelBrush
    );
    
    const float switchWidth = 48.0f;
    const float switchHeight = 24.0f;
    const float knobSize = switchHeight - 6.0f;
    const float switchX = x + 260;
    
    D2D1_ROUNDED_RECT switchRect = {
        D2D1::RectF(switchX, y + 2, switchX + switchWidth, y + switchHeight + 2),
        switchHeight / 2,
        switchHeight / 2 
    };
    
    SwitchRect hitRect = 
    {
        switchRect.rect,
        &toggleState
    };
    switchRects.push_back(hitRect);
    
    pRT->FillRoundedRectangle(switchRect, toggleState ? pSwitchOnBrush : pSwitchOffBrush);
    
    float knobX = toggleState ? 
        (switchX + switchWidth - knobSize - 3.0f) : (switchX + 3.0f);
    
    D2D1_ELLIPSE knob = {
        D2D1::Point2F(knobX + knobSize/2, y + 2 + switchHeight/2),
        knobSize/2,
        knobSize/2
    };
    
    pRT->FillEllipse(knob, pSwitchKnobBrush);
    
    if (toggleState) 
    {
        ID2D1SolidColorBrush* pHighlightBrush;
        pRT->CreateSolidColorBrush(D2D1::ColorF(0.3f, 0.7f, 0.4f), &pHighlightBrush);
        D2D1_ROUNDED_RECT highlightRect = {
            D2D1::RectF(switchX + 1, y + 3, switchX + switchWidth - 1, y + switchHeight + 1),
            (switchHeight - 2) / 2,
            (switchHeight - 2) / 2
        };
        pRT->DrawRoundedRectangle(highlightRect, pHighlightBrush, 1.0f);
        pHighlightBrush->Release();
    }
}

bool draw_batteryinfo_bi::handleSwitchClick(POINT cursorPos)
{
    for (size_t i = 0; i < switchRects.size(); i++) {
        if (cursorPos.x >= switchRects[i].rect.left && 
            cursorPos.x <= switchRects[i].rect.right &&
            cursorPos.y >= switchRects[i].rect.top && 
            cursorPos.y <= switchRects[i].rect.bottom) {
            
            *(switchRects[i].pState) = !*(switchRects[i].pState);
            return true;
        }
    }
    return false;
}

void draw_batteryinfo_bi::drawHeaderSettingsD2D(ID2D1HwndRenderTarget* pRT, init_dwrite_bi* initdwrite_bi, overlay_bi* ov_bi)
{
    D2D1_SIZE_F rtSize = pRT->GetSize();
    maxWidth = rtSize.width;
    
    switchRects.clear();
    
    std::wstring settingsTitle = L"Settings";
    pRT->DrawText(
        settingsTitle.c_str(),
        (UINT32)settingsTitle.length(),
        initdwrite_bi->pTextFormatHeader,
        D2D1::RectF(20, 80, maxWidth, 120),
        pHeaderBrush);
    
    static bool showBatteryPercentage = true;
    static bool enableNotifications = false;
    static bool lowBatteryAlert = true;
    static bool start_With_Windows = false;
    static bool minimize_To_Tray = true;
    static bool darkTheme = false;

    std::wstring displayGroup = L"Overlay";
    pRT->DrawText(
        displayGroup.c_str(),
        (UINT32)displayGroup.length(),
        initdwrite_bi->pTextFormatValue,
        D2D1::RectF(20, 140, maxWidth, 160),
        pValueBrush);
    
    drawToggleSwitch(pRT, initdwrite_bi, 40, 170, ov_bi->show_on_screen_display, L"Show On-Screen Display");
    drawToggleSwitch(pRT, initdwrite_bi, 40, 210, darkTheme, L"test");

    pRT->DrawLine(
        D2D1::Point2F(20.0f, 250.0f),
        D2D1::Point2F(maxWidth - 20.0f, 250.0f),
        pSeparatorBrush,
        1.0f);
    
    std::wstring notificationGroup = L"test";
    pRT->DrawText(
        notificationGroup.c_str(),
        (UINT32)notificationGroup.length(),
        initdwrite_bi->pTextFormatValue,
        D2D1::RectF(20, 265, maxWidth, 285),
        pValueBrush);
    
    drawToggleSwitch(pRT, initdwrite_bi, 40, 295, enableNotifications, L"test");
    drawToggleSwitch(pRT, initdwrite_bi, 40, 335, lowBatteryAlert, L"test");
    
    pRT->DrawLine(
        D2D1::Point2F(20.0f, 375.0f),
        D2D1::Point2F(maxWidth - 20.0f, 375.0f),
        pSeparatorBrush,
        1.0f);
    
    std::wstring behaviorGroup = L"test";
    pRT->DrawText(
        behaviorGroup.c_str(),
        (UINT32)behaviorGroup.length(),
        initdwrite_bi->pTextFormatValue,
        D2D1::RectF(20, 390, maxWidth, 410),
        pValueBrush);
    
    drawToggleSwitch(pRT, initdwrite_bi, 40, 420, start_With_Windows, L"Start with Windows");
    drawToggleSwitch(pRT, initdwrite_bi, 40, 460, minimize_To_Tray, L"Minimize to tray");
}