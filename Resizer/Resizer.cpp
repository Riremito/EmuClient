#include"../Share/Simple/Simple.h"
#include"../Share/Hook/SimpleHook.h"
#include<intrin.h>
#pragma intrinsic(_ReturnAddress)

enum ResizerMode {
	RM_NONE,
	RM_SCALE,
	RM_FIXED,
} ;

ResizerMode gResizerMode = RM_NONE;
float gScale = 1.0;
int gWidth = 800;
int gHeight = 600;
HWND gWnd = NULL;

// for calculating window frame size.
bool CalculateWindowSize(HWND hWnd, int &width, int &height) {
	if (!hWnd) {
		return false;
	}

	RECT window_rect;
	memset(&window_rect, 0, sizeof(window_rect));
	if (!GetWindowRect(hWnd, &window_rect)) {
		return false;
	}

	RECT client_rect;
	memset(&client_rect, 0, sizeof(client_rect));
	if (!GetClientRect(hWnd, &client_rect)) {
		return false;
	}

	width = client_rect.right - client_rect.left;
	height = client_rect.bottom - client_rect.top;
	switch (gResizerMode) {
	case RM_SCALE: {
		width *= gScale;
		height *= gScale;
		break;
	}
	case RM_FIXED: {
		width = gWidth;
		height = gHeight;
		break;
	}
	default: {
		return false;
	}
	}

	width += (window_rect.right - window_rect.left) - (client_rect.right - client_rect.left);
	height += (window_rect.bottom - window_rect.top) - (client_rect.bottom - client_rect.top);
	return true;
}

// get HWND by this hook or use GetWindowThreadProcessId to get it.
decltype(CreateWindowExA) *_CreateWindowExA = NULL;
HWND WINAPI CreateWindowExA_Hook(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
	if (!gWnd) {
		if (lpClassName && strcmp("MapleStoryClass", lpClassName) == 0) {
			DEBUG(L"Resizer : detected target window creation.");
			gWnd = _CreateWindowExA(0, lpClassName, lpWindowName, (WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX)), X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
			return gWnd;
		}
	}
	return _CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

// hook resolution changes by Gr2D.
decltype(SetWindowPos) *_SetWindowPos = NULL;
BOOL WINAPI SetWindowPos_Hook(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags) {
	if (gWnd && gWnd == hWnd) {
		if (uFlags == (SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOMOVE)) {
			BOOL ret = _SetWindowPos(hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
			DEBUG(L"Resizer : detected resizing.");
			int width = 0;
			int height = 0;
			if (CalculateWindowSize(hWnd, width, height)) {
				_SetWindowPos(hWnd, HWND_TOP, NULL, NULL, width, height, SWP_NOMOVE);
			}
			return ret;
		}
	}
	return _SetWindowPos(hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
}

// you can change client window size when ShowWindow is called. before client calls this, you cannot change window size.
decltype(ShowWindow) *_ShowWindow = NULL;
BOOL WINAPI ShowWindow_Hook(HWND hWnd, int nCmdShow) {
	static bool already_hooked = false;
	if (!already_hooked && gWnd && hWnd == gWnd) {
		already_hooked = true;
		DEBUG(L"Resizer : detected target window initialize.");
		int width = 0;
		int height = 0;
		if (CalculateWindowSize(hWnd, width, height)) {
			_SetWindowPos(hWnd, HWND_TOP, NULL, NULL, width, height, SWP_NOMOVE);
		}
	}
	return _ShowWindow(hWnd, nCmdShow);
}

void Resizer() {
	DEBUG(L"Resizer : hook start.");
	SHook(CreateWindowExA);
	SHook(SetWindowPos);
	SHook(ShowWindow);
	DEBUG(L"Resizer : hook finished.");
}

#define DLL_NAME L"Resizer"
bool LoadConfig(HINSTANCE hinstDLL) {
	Config conf(DLL_NAME".ini", hinstDLL);

	std::wstring wScale, wWidth, wHeight;
	float scale = 0.0;
	int width = 0;
	int height = 0;

	if (conf.Read(DLL_NAME, L"scale", wScale)) {
		scale = std::stof(wScale);
	}
	if (conf.Read(DLL_NAME, L"width", wWidth)) {
		width = std::stoi(wWidth);
	}
	if (conf.Read(DLL_NAME, L"heigth", wHeight)) {
		height = std::stoi(wHeight);
	}
	// scale, for small display & high resolution display.
	if (0.0 < scale && scale != 1.0) {
		gResizerMode = RM_SCALE;
		gScale = scale;
		DEBUG(L"Resizer : Scale Mode = " + std::to_wstring(scale));
		return true;
	}
	// fixed resolution, for login screen resolution fixes or full screen.
	if (0 < width && 0 < height) {
		gResizerMode = RM_FIXED;
		gWidth = width;
		gHeight = height;
		DEBUG(L"Resizer : Fixed Mode = " + std::to_wstring(width) + L"x" + std::to_wstring(height));
		return true;
	}

	gResizerMode = RM_NONE;
	return false;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	if (fdwReason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(hinstDLL);
		LoadConfig(hinstDLL);
		Resizer();
	}
	return TRUE;
}

DWORD __stdcall TestFunction() {
	return 0x1337;
}
