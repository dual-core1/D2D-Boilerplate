#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

extern LPDIRECTINPUT8 g_pDirectInput;
extern LPDIRECTINPUTDEVICE8 g_pKeyboardDevice;
extern BYTE keyboardState[256];

bool InitDI(HINSTANCE hInst, HWND hWnd);
void ReadKeyboardState();
void ShutdownDI();
