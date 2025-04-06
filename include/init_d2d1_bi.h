#ifndef INIT_D2D1_BI_H
#define INIT_D2D1_BI_H

#include "d2d1.h"

class init_d2d1_bi
{
public:
    ID2D1Factory* pD2DFactory = nullptr;

    void InitDirect2D();

    ID2D1HwndRenderTarget* CreateRenderTarget(HWND hwnd);
};

#endif