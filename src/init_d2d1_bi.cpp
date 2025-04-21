#include "../include/init_d2d1_bi.h"

void init_d2d1_bi::InitDirect2D()
{
    if (!pD2DFactory)
        D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);
}

ID2D1HwndRenderTarget* init_d2d1_bi::GetOrCreateRenderTarget(HWND hwnd)
{
    if (!pRenderTarget)
    {
        RECT rc;
        GetClientRect(hwnd, &rc);

        D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties();
        D2D1_HWND_RENDER_TARGET_PROPERTIES hwndProps =
            D2D1::HwndRenderTargetProperties(hwnd,
                D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top));

        HRESULT hr = pD2DFactory->CreateHwndRenderTarget(rtProps, hwndProps, &pRenderTarget);
        if (FAILED(hr)) 
        {
            pRenderTarget = nullptr;
        }
    }
    return pRenderTarget;
}

void init_d2d1_bi::ResizeRenderTarget(HWND hwnd) 
{
    if (pRenderTarget) 
    {
        RECT rc;
        GetClientRect(hwnd, &rc);
        D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
        pRenderTarget->Resize(size);
    }
}


void init_d2d1_bi::DiscardRenderTarget()
{
    if (pRenderTarget)
    {
        pRenderTarget->Release();
        pRenderTarget = nullptr;
    }
}