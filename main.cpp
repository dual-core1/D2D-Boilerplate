#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d2d1.h>

#include "init_dinput.h"

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

    // create factory
    ID2D1Factory* pD2DFactory = NULL;
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);
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
        return -1;

    // initialize direct input
    if (!InitDI(hInst, hWnd))
        return -1;

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
            ExitProcess(0);
        }

        // update state here!

        pRenderTarget->BeginDraw();
        pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));

        // draw here!

        pRenderTarget->EndDraw();
    }

    // shut down direct input
    ShutdownDI();
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
