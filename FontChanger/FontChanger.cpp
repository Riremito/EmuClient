#include"../Share/Simple/Simple.h"
#include"../Share/Hook/SimpleHook.h"
#pragma comment(lib, "Imm32.lib")
#include <intrin.h>
#pragma intrinsic(_ReturnAddress)

UINT gCodePage = 949;
UINT getCodePage() {
	return gCodePage;
}

void setCodePage(UINT codepage) {
	gCodePage = codepage;
}

decltype(MultiByteToWideChar) *_MultiByteToWideChar = NULL;
int WINAPI MultiByteToWideChar_Hook(UINT CodePage, DWORD dwFlags, LPCSTR lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar) {
	CodePage = (CodePage >= CP_UTF7) ? CodePage : getCodePage();
	return _MultiByteToWideChar(CodePage, dwFlags, lpMultiByteStr, cbMultiByte, lpWideCharStr, cchWideChar);
}

decltype(WideCharToMultiByte) *_WideCharToMultiByte = NULL;
int WINAPI WideCharToMultiByte_Hook(UINT CodePage, DWORD dwFlags, LPCWSTR lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte, LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar) {
	CodePage = (CodePage >= CP_UTF7) ? CodePage : getCodePage();
	return _WideCharToMultiByte(CodePage, dwFlags, lpWideCharStr, cchWideChar, lpMultiByteStr, cbMultiByte, lpDefaultChar, lpUsedDefaultChar);
}

decltype(GetACP) *_GetACP = NULL;
UINT WINAPI GetACP_Hook() {
	return getCodePage();
}

decltype(CreateFontIndirectA) *_CreateFontIndirectA = NULL;
HFONT WINAPI CreateFontIndirectA_Hook(LOGFONTA* lplf) {
	LOGFONTW logfont = { sizeof(LOGFONTW), };
	memcpy(&logfont, lplf, sizeof(LOGFONTW));
	MultiByteToWideChar(getCodePage(), 0, lplf->lfFaceName, -1, logfont.lfFaceName, LF_FACESIZE);
	return CreateFontIndirectW(&logfont);
}

decltype(CharNextA) *_CharNextA = NULL;
LPSTR WINAPI CharNextA_Hook(LPCSTR lpsz) {
	return CharNextExA(gCodePage, lpsz, 0);
}

#define DLL_NAME L"FontChanger"
bool LoadConfig(HINSTANCE hinstDLL) {
	Config conf(DLL_NAME".ini", hinstDLL);

	std::wstring wCodePage;
	if (conf.Read(DLL_NAME, L"CodePage", wCodePage)) {
		int codepage = std::stoi(wCodePage);
		if (codepage) {
			setCodePage(codepage);
			DEBUG(L"setCodePage : " + std::to_wstring(codepage));
			return true;
		}
	}

	return false;
}

bool FontHook() {
	SHook(MultiByteToWideChar);
	SHook(WideCharToMultiByte);
	SHook(GetACP);
	SHook(CreateFontIndirectA);
	SHook(CharNextA);
	return true;
}

// Enable utf8 paste for JMS
decltype(GetClipboardData) *_GetClipboardData = NULL;
HANDLE WINAPI GetClipboardData_Hook(UINT uFormat) {
	// CF_TEXT -> CF_OEMTEXT
	return _GetClipboardData(CF_OEMTEXT);
}

// Enable IME for MSEA
decltype(ImmAssociateContext) *_ImmAssociateContext = NULL;
HIMC WINAPI ImmAssociateContext_Hook(HWND hw, HIMC hi) {
	if (SimpleHook::IsCallerEXE(_ReturnAddress())) {
		return 0;
	}

	return _ImmAssociateContext(hw, hi);
}

bool ClipBoardHook() {
	SHook(GetClipboardData);
	SHook(ImmAssociateContext);
	return true;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	if(fdwReason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(hinstDLL);
		LoadConfig(hinstDLL);
		FontHook();
		ClipBoardHook();
	}
	return TRUE;
}