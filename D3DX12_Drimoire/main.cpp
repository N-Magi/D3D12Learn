#include <Windows.h>
#ifdef _DEBUG
	#include <iostream>
#endif // _DEBUG
//#include <wrl/client.h>
//#include "dxlib.h"
#include "vertex.h"

//#pragma comment(lib,"d3d12.lib")
//#pragma comment(lib,"dxgi.lib")
//#include <Wsdclient.h>

using namespace std;
//using namespace Microsoft::WRL;


LRESULT WinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void DebugOutputFormatString(const char* format, ...)
{
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	//printf(format, valist);
	va_end(valist);
#endif // _DEBUG
}

#ifdef _DEBUG
int main()
{
#else // _DEBUG
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#endif // _DEBUG
	WNDCLASSEX wClass = { };
	wClass.cbSize = sizeof(WNDCLASSEX);
	wClass.lpfnWndProc = (WNDPROC)WinProc;
	wClass.lpszClassName = L"DX12_GRIMOIRE";
	wClass.hInstance = GetModuleHandle(nullptr);
	RegisterClassEx(&wClass);

	RECT wRect = { 0,0,640,480 };
	vertexApp dxlib;
	AdjustWindowRect(&wRect, WS_OVERLAPPEDWINDOW, false);
	HWND hWnd = CreateWindow(
		wClass.lpszClassName,
		L"DX12_GRIMOIRE",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wRect.right - wRect.left,
		wRect.bottom - wRect.top,
		nullptr,
		nullptr,
		wClass.hInstance,
		&dxlib
	);
	ShowWindow(hWnd, SW_SHOW);

	MSG msg = {};

	
	dxlib.Init(hWnd);

	while (true)
	{
		dxlib.Render();

		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT)
		{
			break;
		}

	}
	UnregisterClass(wClass.lpszClassName, wClass.hInstance);

	DebugOutputFormatString("Show window Test");
	
	return 0;
}

LRESULT WinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}



