#include "wicimaging.h"

IWICImagingFactory* g_pWicImagingFactory = NULL;

bool InitWIC() {
    // create the wic imaging factory
    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&g_pWicImagingFactory)
    );

    // true on success, obviously
    if (SUCCEEDED(hr))
        return true;
    else
        return false;
}

void ShutdownWIC() {
    if (g_pWicImagingFactory) {
        g_pWicImagingFactory->Release();
        g_pWicImagingFactory = NULL;
    }
}

// Creates D2D bitmap from specified file.
// Returns:
// (SUCCESS) Pointer to an ID2D1Bitmap.
// (FAILURE) NULL.
ID2D1Bitmap* CreateD2DBitmapFromFile(ID2D1HwndRenderTarget* pRT, LPCWSTR filename) {
    HRESULT hr = S_OK;

    // converter and bitmap
    IWICFormatConverter* pConvertedBitmap = NULL;
    ID2D1Bitmap* pBitmap = NULL; // to be returned by the function

    // create a decoder
    IWICBitmapDecoder* pDecoder = NULL;
    IWICBitmapFrameDecode* pFrame = NULL;

    hr = g_pWicImagingFactory->CreateDecoderFromFilename(
        filename,
        NULL,
        GENERIC_READ,
        WICDecodeMetadataCacheOnDemand, // cache metadata when needed
        &pDecoder
    );
    if (FAILED(hr)) goto cleanup;
    
    // get first frame of image from the decoder    
    hr = pDecoder->GetFrame(0, &pFrame);
    if (FAILED(hr)) goto cleanup;

    // convert to 32 bits-per-pixel premultiplied BGRA
    hr = g_pWicImagingFactory->CreateFormatConverter(&pConvertedBitmap);
    if (FAILED(hr)) goto cleanup;

    hr = pConvertedBitmap->Initialize(
        pFrame,                        // bitmap to convert
        GUID_WICPixelFormat32bppPBGRA, // destination format
        WICBitmapDitherTypeNone,       // no dither
        NULL,                          // palette
        0.f,                           // alpha threshold
        WICBitmapPaletteTypeCustom     // palette translation type
    );
    if (FAILED(hr)) goto cleanup;

    // create our bitmap...
    hr = pRT->CreateBitmapFromWicBitmap(pConvertedBitmap, NULL, &pBitmap);
    if (FAILED(hr)) goto cleanup;

cleanup:
    if (pConvertedBitmap) pConvertedBitmap->Release();
    if (pFrame) pFrame->Release();
    if (pDecoder) pDecoder->Release();

    return pBitmap;
}
