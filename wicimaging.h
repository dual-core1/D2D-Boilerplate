#include <wincodec.h>
#include <wincodecsdk.h>
#include <d2d1.h>

extern IWICImagingFactory* g_pWicImagingFactory;

bool InitWIC();
void ShutdownWIC();
ID2D1Bitmap* CreateD2DBitmapFromFile(ID2D1HwndRenderTarget* pRT, LPCWSTR filename);
