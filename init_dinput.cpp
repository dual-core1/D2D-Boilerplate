#include "init_dinput.h"

LPDIRECTINPUT8 g_pDirectInput;
LPDIRECTINPUTDEVICE8 g_pKeyboardDevice;
BYTE keyboardState[256];

bool InitDI(HINSTANCE hInst, HWND hWnd) {
    HRESULT hr;

    // create direct input object
    hr = DirectInput8Create(hInst, DIRECTINPUT_VERSION,
    IID_IDirectInput8, (void**)&g_pDirectInput, NULL);

    if (FAILED(hr))
        return false;
    
    // create keyboard device
    hr = g_pDirectInput->CreateDevice(GUID_SysKeyboard, &g_pKeyboardDevice, NULL);

    if (FAILED(hr))
        return false;
    
    // set keyboard's data format
    hr = g_pKeyboardDevice->SetDataFormat(&c_dfDIKeyboard);
    if (FAILED(hr))
        return false;

    // set cooperative level
    hr = g_pKeyboardDevice->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
    if (FAILED(hr))
        return false;
    
    // acquire. we're done
    g_pKeyboardDevice->Acquire();
    return true;
}

// note to self: high-order bit sets to 1 (0x80) when a key is down
void ReadKeyboardState() {
    if (!g_pKeyboardDevice) return;

    HRESULT hr = g_pKeyboardDevice->GetDeviceState(sizeof(keyboardState), (LPVOID)&keyboardState);

    // if device lost/unacquired:
    // reacquire, and try again
    if (FAILED(hr)) {
        if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED) {
            g_pKeyboardDevice->Acquire();    
        }
        return;
    }
}

void ShutdownDI() {
    if (g_pKeyboardDevice) {
        g_pKeyboardDevice->Unacquire();
        g_pKeyboardDevice->Release();
        g_pKeyboardDevice = NULL;
    }
    if (g_pDirectInput) {
        g_pDirectInput->Release();
        g_pDirectInput = NULL;
    }
}
