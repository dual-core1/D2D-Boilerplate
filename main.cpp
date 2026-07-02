#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d2d1.h>

#include "init_dinput.h"
#include "wicimaging.h"
#include "audio_engine.h"

LPCWSTR CLASS_NAME = L"Direct2D Window Class";

// forward declaration of wndproc
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int wWinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPWSTR lpCmdLine, int nCmdShow) {
    // create + register window class
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    RegisterClassExW(&wc);

    // create window
    HWND hWnd = CreateWindow(CLASS_NAME, L"Direct2D Window", WS_OVERLAPPED,
    CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInst, NULL);
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // initialize direct 2d

    // initialize COM
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    if (SUCCEEDED(hr)) {
        // create factory
        ID2D1Factory* pD2DFactory = NULL;
        hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);
        if (FAILED(hr))
            return -1;
	    
        // create render target
        RECT rc;
        GetClientRect(hWnd, &rc);
	    
        ID2D1HwndRenderTarget* pRenderTarget = NULL;
        hr = pD2DFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(
                hWnd,
                D2D1::SizeU(
                    rc.right - rc.left,
                    rc.bottom - rc.top
                )
            ),
            &pRenderTarget
        );
        if (FAILED(hr))
            PostQuitMessage(-1);
	    
        // initialize DirectInput
        if (!InitDI(hInst, hWnd))
            PostQuitMessage(-1);
        
        // initialize WIC
        if (!InitWIC())
            PostQuitMessage(-1);
        
        // initialize XAudio
        if (!InitXAudio())
            PostQuitMessage(-1);

        // testing: load the image
        ID2D1Bitmap* pBedroomBitmap = NULL;
        pBedroomBitmap = CreateD2DBitmapFromFile(pRenderTarget, L"bedroom.png");

        if (pBedroomBitmap == NULL) {
            PostQuitMessage(-1);
        }

        // testing: load the music
        HANDLE hFilein;
        WAVEFORMATEXTENSIBLE wfxFormat = {0};
        XAUDIO2_BUFFER xaBuffer = {0};
        BYTE* pDataBuffer = NULL;

        hFilein = CreateFile(
            L"loop.wav",
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            0,
            NULL
        );

        hr = WavFromFileHandle(hFilein, wfxFormat, xaBuffer, pDataBuffer);

        CloseHandle(hFilein);

        if (FAILED(hr))
            PostQuitMessage(-1);

        // also want it to loop...
        xaBuffer.LoopBegin = 0;
        xaBuffer.LoopLength = 0; // to the end of the buffer
        xaBuffer.LoopCount = XAUDIO2_LOOP_INFINITE;

        // start playing music
        IXAudio2SourceVoice* pSourceVoice{};
        g_pXAudio->CreateSourceVoice(&pSourceVoice, reinterpret_cast<WAVEFORMATEX*>(&wfxFormat));
        pSourceVoice->SubmitSourceBuffer(&xaBuffer);
        pSourceVoice->Start();
	    
        // main loop + issue drawing commands
        MSG msg = {};
	    
        while (msg.message != WM_QUIT) {
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            
            // process input here!
            ReadKeyboardState();
	    
            // exit on escape pressed
            if (keyboardState[DIK_ESCAPE] & 0x80) {
                break;
            }
	    
            // update state here!
	    
            pRenderTarget->BeginDraw();
            pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));
	    
            // draw here!
            D2D1_SIZE_F imgSize = pBedroomBitmap->GetSize();
            D2D1_RECT_F imgRect = D2D1::RectF(16.0f, 44.0f, imgSize.width, imgSize.height);
            pRenderTarget->DrawBitmap(pBedroomBitmap, &imgRect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, NULL);
	    
            pRenderTarget->EndDraw();
        }
        pBedroomBitmap->Release();
        pBedroomBitmap = NULL;
        pSourceVoice->DestroyVoice();
        pSourceVoice = NULL;

        // shut down DirectInput, WIC, and XAudio
        ShutdownDI();
        ShutdownWIC();
        ShutdownXAudio();

        // clean up D2D
        pRenderTarget->Release();
        pRenderTarget = NULL;
        pD2DFactory->Release();
        pD2DFactory = NULL;
	    
        // uninitialize
        CoUninitialize();
    }

    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
