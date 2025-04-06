#include "../include/init_d2d1_bi.h"

void init_d2d1_bi::InitDirect2D()
{
    if (!pD2DFactory)
        D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);
}

ID2D1HwndRenderTarget* init_d2d1_bi::CreateRenderTarget(HWND hwnd)
{
    RECT rc;
    GetClientRect(hwnd, &rc);

    D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties();
    D2D1_HWND_RENDER_TARGET_PROPERTIES hwndProps =
        D2D1::HwndRenderTargetProperties(hwnd,
            D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top));

    ID2D1HwndRenderTarget* pRenderTarget = nullptr;
    pD2DFactory->CreateHwndRenderTarget(rtProps, hwndProps, &pRenderTarget);
    return pRenderTarget;
}