#ifndef DRAW_BATTERYINFO_H
#define DRAW_BATTERYINFO_H 

#include <d2d1.h>

#include "BatteryInfo.h"
#include "init_dwrite_bi.h"
#include "overlay_bi.h"
#include "resource_usage_bi.h"

class draw_batteryinfo_bi
{
public:
    draw_batteryinfo_bi()
    {
        textColor = D2D1::ColorF(0.1f, 0.1f, 0.1f);            // Темно-сірий текст
        labelColor = D2D1::ColorF(0.4f, 0.4f, 0.4f);           // Сірий для назв
        separatorColor = D2D1::ColorF(0.8f, 0.8f, 0.8f);       // Світло-сірий розділювач
        backgroundColor = D2D1::ColorF(0.98f, 0.98f, 0.98f);   // Майже білий фон
        headerColor = D2D1::ColorF(0.2f, 0.4f, 0.8f);          // Акцент (синій)
       
        pLabelBrush = nullptr;
        pValueBrush = nullptr;
        pHeaderBrush = nullptr;
        pSeparatorBrush = nullptr;

        pSwitchOnBrush = nullptr;
        pSwitchOffBrush = nullptr;
        pSwitchKnobBrush = nullptr;

        pScrollBarBrush = nullptr;

        pBackgroundBrush = nullptr;
    }

    ~draw_batteryinfo_bi()
    {
        pLabelBrush->Release();
        pValueBrush->Release();
        pHeaderBrush->Release();
        pSeparatorBrush->Release();    

        if (pSwitchOnBrush) pSwitchOnBrush->Release();
        if (pSwitchOffBrush) pSwitchOffBrush->Release();
        if (pSwitchKnobBrush) pSwitchKnobBrush->Release();
          
        if (pScrollBarBrush) pScrollBarBrush->Release();
        
        if (pBackgroundBrush) pBackgroundBrush->Release();
    }

    enum selected_option
    {
        BATTERY_INFO, 
        SETTINGS,
    };

    selected_option selectedTab = BATTERY_INFO;

    void drawHeaderBatteryInfoD2D(ID2D1HwndRenderTarget* pRT, batteryinfo_bi* bi_bi, init_dwrite_bi* initdwrite_bi, int startX, int startY, int lineHeight);
    void drawHeaderSettingsD2D(ID2D1HwndRenderTarget* pRT, init_dwrite_bi* initdwrite_bi, overlay_bi* ov_bi, resource_usage_bi* ru_bi, batteryinfo_bi* bi_bi);
    void drawHeaders(ID2D1HwndRenderTarget* pRT, init_dwrite_bi* initdwrite_bi, int startX = 20, int startY = 20, int lineHeight = 24);

    bool initBrush(ID2D1HwndRenderTarget* pRT);
    bool clearBackground(ID2D1HwndRenderTarget* pRT);

    bool isCursorInBatteryStatus(POINT cursorPos);
    bool isCursorInSettings(POINT cursorPos);

    void drawToggleSwitch(ID2D1HwndRenderTarget* pRT, init_dwrite_bi* initdwrite_bi, 
                      float x, float y, bool& toggleState, const std::wstring& labelText);
    bool handleSwitchClick(POINT cursorPos);

    float scrollOffsetY = 0.0f;
    float contentHeight = 0.0f;
    float viewHeight = 0.0f;

    D2D1_RECT_F rectBatteryStatus{};
    D2D1_RECT_F rectSettings{};
private:
    D2D1_COLOR_F textColor; 
    D2D1_COLOR_F labelColor;
    D2D1_COLOR_F separatorColor;
    D2D1_COLOR_F backgroundColor;
    D2D1_COLOR_F headerColor;
    
    ID2D1SolidColorBrush* pLabelBrush;
    ID2D1SolidColorBrush* pValueBrush;
    ID2D1SolidColorBrush* pHeaderBrush;
    
    ID2D1SolidColorBrush* pSeparatorBrush;
    
    ID2D1SolidColorBrush* pSwitchOnBrush;
    ID2D1SolidColorBrush* pSwitchOffBrush;
    ID2D1SolidColorBrush* pSwitchKnobBrush;

    ID2D1SolidColorBrush* pScrollBarBrush;

    ID2D1SolidColorBrush* pBackgroundBrush;

    struct SwitchRect 
    {
        D2D1_RECT_F rect;
        bool* pState;
    };
    
    std::vector<SwitchRect> switchRects;
    
    FLOAT maxWidth = 0;
};

#endif