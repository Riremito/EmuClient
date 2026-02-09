#include"../Share/Simple/Simple.h"
#include"../Share/Hook/SimpleHook.h"
#include<intrin.h>
#pragma intrinsic(_ReturnAddress)

float gScale = 2.0;
#define DLL_NAME L"Resizer"
bool LoadConfig(HINSTANCE hinstDLL) {
	Config conf(DLL_NAME".ini", hinstDLL);

	std::wstring wScale;
	if (conf.Read(DLL_NAME, L"scale", wScale)) {
		float scale = std::stof(wScale);
		if (scale != 0) {
			gScale = scale;
			DEBUG(L"Resizer : scale = " + std::to_wstring(gScale));
			return true;
		}
	}

	return false;
}

decltype(SetWindowPos) *_SetWindowPos = NULL;
// for calculating window frame size.
bool Resize(HWND hWnd, int cx, int cy) {
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

	if (!_SetWindowPos) {
		DEBUG(L"Resizer : _SetWindowPos is NULL.");
		return false;
	}

	if (!_SetWindowPos(hWnd, HWND_TOP, NULL, NULL, (cx + (window_rect.right - window_rect.left) - (client_rect.right - client_rect.left)), (cy + (window_rect.bottom - window_rect.top) - (client_rect.bottom - client_rect.top)), SWP_NOMOVE)) {
		return false;
	}

	return true;
}

bool AutoResize(HWND hWnd) {
	RECT client_rect;
	memset(&client_rect, 0, sizeof(client_rect));
	if (!GetClientRect(hWnd, &client_rect)) {
		return false;
	}
	int width = client_rect.right - client_rect.left;
	int height = client_rect.bottom - client_rect.top;
	std::wstring text = L"Resizer : AutoResize. " + std::to_wstring(width) + L"x" + std::to_wstring(height);
	DEBUG(text.c_str());
	Resize(hWnd, width * gScale, height * gScale);
	return true;
}

HWND gWnd = NULL;
// get HWND by this hook or use GetWindowThreadProcessId to get it.
decltype(CreateWindowExA) *_CreateWindowExA = NULL;
HWND WINAPI CreateWindowExA_Hook(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
	if (!gWnd) {
	if (lpClassName && strcmp("MapleStoryClass", lpClassName) == 0) {
		std::wstring text = L"Resizer : found main window.";
		DEBUG(text.c_str());
		// adding WS_THICKFRAME allows you manually resizing window.
		gWnd = _CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle/* | WS_THICKFRAME*/, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
		return gWnd;
	}
	}
	return _CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

// you can change client window size when ShowWindow is called. before client calls this, you cannot change window size.
decltype(ShowWindow) *_ShowWindow = NULL;
BOOL WINAPI ShowWindow_Hook(HWND hWnd, int nCmdShow) {
	static bool already_hooked = false;
	if (!already_hooked) {
		if (gWnd && hWnd == gWnd) {
			already_hooked = true;
			DEBUG(L"Resizer : detected window creation.");
			AutoResize(gWnd);
		}
	}
	return _ShowWindow(hWnd, nCmdShow);
}

// hook resolution changes by Gr2D.
BOOL WINAPI SetWindowPos_Hook(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags) {
	if (gWnd) {
		if (gWnd == hWnd) {
			if (uFlags == (SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOMOVE)) {
				BOOL ret = _SetWindowPos(hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
				DEBUG(L"Resizer : detected SetWindowPos.");
				AutoResize(gWnd);
				return ret;
			}
		}
	}
	return _SetWindowPos(hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
}

void Resizer() {
	DEBUG(L"Resizer... hooking");
	SHook(CreateWindowExA);
	SHook(ShowWindow);
	SHook(SetWindowPos);
	DEBUG(L"Resizer... hook ok");
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
