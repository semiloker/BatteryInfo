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

    pRT->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkGray), &pScrollBarBrush);

    pRT->CreateSolidColorBrush(D2D1::ColorF(0.98f, 0.98f, 0.98f), &pBackgroundBrush);

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

                // pRT->DrawRectangle(rect, pHeaderBrush);

                currentX += width + 30.0f;
                headerIndex++;

                pTextBrush->Release();
            }
            pTextLayout->Release();
        }
    };

    D2D1_RECT_F box_header = D2D1::RectF(
        0.0f, 0.0f, maxWidth, 60.0f);

    pRT->FillRectangle(&box_header, pBackgroundBrush);
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
                          {L"Cycle count", std::wstring(bi_bi->info_static.CycleCount.begin(), bi_bi->info_static.CycleCount.end())},
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
    // const float switchX = x + 260;
    float switchX = maxWidth - 70.0f;
    
    float textHeight = 24.0f;
    float switchY = y + (textHeight - switchHeight) / 2.0f;

    D2D1_ROUNDED_RECT switchRect = {
        D2D1::RectF(switchX, switchY, switchX + switchWidth, switchY + switchHeight),
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
        D2D1::Point2F(knobX + knobSize / 2, switchY + switchHeight / 2),
        knobSize / 2,
        knobSize / 2
    };
    
    pRT->FillEllipse(knob, pSwitchKnobBrush);
    
    if (toggleState) 
    {
        ID2D1SolidColorBrush* pHighlightBrush;
        
        pRT->CreateSolidColorBrush(D2D1::ColorF(0.3f, 0.7f, 0.4f), &pHighlightBrush);
        
        D2D1_ROUNDED_RECT highlightRect = {
            D2D1::RectF(switchX + 1, switchY + 1, switchX + switchWidth - 1, switchY + switchHeight - 1),
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

void draw_batteryinfo_bi::drawHeaderSettingsD2D(ID2D1HwndRenderTarget* pRT, init_dwrite_bi* initdwrite_bi, overlay_bi* ov_bi, resource_usage_bi* ru_bi, batteryinfo_bi* bi_bi)
{
    D2D1_SIZE_F rtSize = pRT->GetSize();
    maxWidth = rtSize.width;
    viewHeight = rtSize.height;

    switchRects.clear();

    float y = 66.0f - scrollOffsetY;

    // Overlay group
    std::wstring displayGroup = L"Overlay";
    pRT->DrawText(displayGroup.c_str(), (UINT32)displayGroup.length(),
        initdwrite_bi->pTextFormatValue,
        D2D1::RectF(20, y, maxWidth, y + 20), pValueBrush);
    y += 30;

    drawToggleSwitch(pRT, initdwrite_bi, 40, y, ov_bi->show_on_screen_display, L"Show On-Screen Display");
    y += 40;

    y += 10;
    pRT->DrawLine(D2D1::Point2F(20, y), D2D1::Point2F(maxWidth - 20, y), pSeparatorBrush);
    y += 20;

    // CPU
    std::wstring cpuGroup = L"CPU";
    pRT->DrawText(cpuGroup.c_str(), (UINT32)cpuGroup.length(),
        initdwrite_bi->pTextFormatValue,
        D2D1::RectF(40, y, maxWidth, y + 20), pValueBrush);
    y += 30;

    drawToggleSwitch(pRT, initdwrite_bi, 80, y, ru_bi->cpuInfo.show_UsagePercent, L"CPU Usage Percent");
    y += 40;
    drawToggleSwitch(pRT, initdwrite_bi, 80, y, ru_bi->cpuInfo.show_CoreUsagePercents, L"CPU Core Usage Percents");
    y += 40;
    drawToggleSwitch(pRT, initdwrite_bi, 80, y, ru_bi->cpuInfo.show_cpuName, L"CPU Name");
    y += 40;
    drawToggleSwitch(pRT, initdwrite_bi, 80, y, ru_bi->cpuInfo.show_architecture, L"CPU Architecture");
    y += 60;

    pRT->DrawLine(D2D1::Point2F(20, y), D2D1::Point2F(maxWidth - 20, y), pSeparatorBrush);
    y += 20;

    // RAM
    std::wstring batteryGroup = L"Battery";
    pRT->DrawText(batteryGroup.c_str(), (UINT32)batteryGroup.length(),
        initdwrite_bi->pTextFormatValue,
        D2D1::RectF(40, y, maxWidth, y + 20), pValueBrush);
    y += 30;

    drawToggleSwitch(pRT, initdwrite_bi, 80, y, bi_bi->info_1s.Voltage_, L"Battery Voltager");
    y += 40;
    drawToggleSwitch(pRT, initdwrite_bi, 80, y, bi_bi->info_1s.Rate_, L"Battery Rate");
    y += 40;
    drawToggleSwitch(pRT, initdwrite_bi, 80, y, bi_bi->info_1s.PowerState_, L"Battery Power State");
    y += 40;
    drawToggleSwitch(pRT, initdwrite_bi, 80, y, bi_bi->info_1s.RemainingCapacity_, L"Battery Remaining Capacity");
    y += 40;
    drawToggleSwitch(pRT, initdwrite_bi, 80, y, bi_bi->info_1s.ChargeLevel_, L"Battery Charge Level");
    y += 40;
    drawToggleSwitch(pRT, initdwrite_bi, 80, y, bi_bi->info_10s.TimeRemaining_, L"Battery Remaining Time");
    y += 60;

    pRT->DrawLine(D2D1::Point2F(20, y), D2D1::Point2F(maxWidth - 20, y), pSeparatorBrush);
    y += 20;

    std::wstring ramGroup = L"Ram";
    pRT->DrawText(ramGroup.c_str(), (UINT32)ramGroup.length(),
        initdwrite_bi->pTextFormatValue,
        D2D1::RectF(40, y, maxWidth, y + 20), pValueBrush);
    y += 30;

    drawToggleSwitch(pRT, initdwrite_bi, 80, y, ru_bi->ramInfo.show_dwMemoryLoad, L"Memory Load");
    y += 40;
    drawToggleSwitch(pRT, initdwrite_bi, 80, y, ru_bi->ramInfo.show_ullTotalPhys, L"Total Physical RAM");
    y += 40;
    drawToggleSwitch(pRT, initdwrite_bi, 80, y, ru_bi->ramInfo.show_ullAvailPhys, L"Available Physical RAM");
    y += 40;
    drawToggleSwitch(pRT, initdwrite_bi, 80, y, ru_bi->ramInfo.show_ullTotalPageFile, L"Total Page File");
    y += 40;
    drawToggleSwitch(pRT, initdwrite_bi, 80, y, ru_bi->ramInfo.show_ullAvailPageFile, L"Available Page File");
    y += 40;
    drawToggleSwitch(pRT, initdwrite_bi, 80, y, ru_bi->ramInfo.show_ullTotalVirtual, L"Total Virtual Memory");
    y += 40;
    drawToggleSwitch(pRT, initdwrite_bi, 80, y, ru_bi->ramInfo.show_ullAvailVirtual, L"Available Virtual Memory");
    y += 40;
    drawToggleSwitch(pRT, initdwrite_bi, 80, y, ru_bi->ramInfo.show_ullAvailExtendedVirtual, L"Extended Virtual Memory");
    y += 60;

    pRT->DrawLine(D2D1::Point2F(20, y), D2D1::Point2F(maxWidth - 20, y), pSeparatorBrush);
    y += 20;

    // Disks
    // std::wstring diskGroup = L"Disk(s)";
    // pRT->DrawText(diskGroup.c_str(), (UINT32)diskGroup.length(),
    //     initdwrite_bi->pTextFormatValue,
    //     D2D1::RectF(40, y, maxWidth, y + 20), pValueBrush);
    // y += 30;

    // int diskIndex = 1;
    // for (auto& disk : ru_bi->disksInfo)
    // {
    //     std::wstring label = L"Disk " + std::to_wstring(diskIndex++);
    //     pRT->DrawText(label.c_str(), (UINT32)label.length(),
    //         initdwrite_bi->pTextFormatLabel,
    //         D2D1::RectF(60, y, maxWidth, y + 20), pValueBrush);
    //     y += 30;

    //     drawToggleSwitch(pRT, initdwrite_bi, 100, y, disk.show_diskLetter, L"Disk Letter");
    //     y += 40;
    //     drawToggleSwitch(pRT, initdwrite_bi, 100, y, disk.show_totalSpace, L"Total Space");
    //     y += 40;
    //     drawToggleSwitch(pRT, initdwrite_bi, 100, y, disk.show_freeSpace, L"Free Space");
    //     y += 40;
    //     drawToggleSwitch(pRT, initdwrite_bi, 100, y, disk.show_usedSpace, L"Used Space");
    //     y += 40;
    //     drawToggleSwitch(pRT, initdwrite_bi, 100, y, disk.show_usagePercent, L"Usage Percent");
    //     y += 50;
    // }

    // pRT->DrawLine(D2D1::Point2F(20, y), D2D1::Point2F(maxWidth - 20, y), pSeparatorBrush);
    // y += 20;

    // Network 
    // std::wstring netGroup = L"Network";
    // pRT->DrawText(netGroup.c_str(), (UINT32)netGroup.length(),
    //     initdwrite_bi->pTextFormatValue,
    //     D2D1::RectF(40, y, maxWidth, y + 20), pValueBrush);
    // y += 30;

    // int netIndex = 1;
    // for (auto& net : ru_bi->networkInfo)
    // {
    //     std::wstring label = L"Interface " + std::to_wstring(netIndex++);
    //     pRT->DrawText(label.c_str(), (UINT32)label.length(),
    //         initdwrite_bi->pTextFormatLabel,
    //         D2D1::RectF(60, y, maxWidth, y + 20), pValueBrush);
    //     y += 30;

    //     drawToggleSwitch(pRT, initdwrite_bi, 100, y, net.show_interfaceName, L"Interface Name");
    //     y += 40;
    //     drawToggleSwitch(pRT, initdwrite_bi, 100, y, net.show_downloadSpeed, L"Download Speed");
    //     y += 40;
    //     drawToggleSwitch(pRT, initdwrite_bi, 100, y, net.show_uploadSpeed, L"Upload Speed");
    //     y += 50;
    // }

    // Behavior
    std::wstring behaviorGroup = L"Behavior";
    pRT->DrawText(behaviorGroup.c_str(), (UINT32)behaviorGroup.length(),
        initdwrite_bi->pTextFormatValue,
        D2D1::RectF(20, y, maxWidth, y + 20), pValueBrush);
    y += 30;

    drawToggleSwitch(pRT, initdwrite_bi, 40, y, ru_bi->start_With_Windows, L"Start with Windows");
    y += 40;
    drawToggleSwitch(pRT, initdwrite_bi, 40, y, ru_bi->minimize_To_Tray, L"Minimize to tray");
    y += 40;
    drawToggleSwitch(pRT, initdwrite_bi, 40, y, ru_bi->exit_on_key_esc, L"Exit on key 'ESC'");
    y += 60;

    contentHeight = y + scrollOffsetY;

    float topOffset = 60.0f;

    if (contentHeight > rtSize.height - topOffset)
    {
        float visibleHeight = rtSize.height - topOffset;
        float barHeight = ((visibleHeight / contentHeight) * visibleHeight) + topOffset;
        float barY = topOffset + (scrollOffsetY / contentHeight) * visibleHeight;

        D2D1_RECT_F scrollbarRect = D2D1::RectF(
            rtSize.width - 5,
            barY,
            rtSize.width - 1,
            barY + barHeight
        );

        pRT->FillRectangle(&scrollbarRect, pScrollBarBrush);
    }
}