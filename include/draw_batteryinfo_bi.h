#ifndef DRAW_BATTERYINFO_H
#define DRAW_BATTERYINFO_H 

#include <d2d1.h>

#include "BatteryInfo.h"
#include "init_dwrite_bi.h"

class draw_batteryinfo_bi
{
public:
    void drawHeaderBatteryInfoD2D(ID2D1HwndRenderTarget* pRT, batteryinfo_bi* bi_bi, init_dwrite_bi* initdwrite_bi, int startX, int startY, int lineHeight);
};

#endif