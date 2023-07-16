#include"SupportUTF8.h"
#pragma comment(lib, "Imm32.lib")
#include <intrin.h>
#pragma intrinsic(_ReturnAddress)

#define LANGUAGE_JMS 1041
#define CODEPAGE_JMS 932

// Convert shift-jis to utf8 for JMS
decltype(MultiByteToWideChar) *_MultiByteToWideChar = 0;
int WINAPI MultiByteToWideChar_Hook(UINT CodePage, DWORD dwFlags, LPCCH lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar) {
	// CP_ACP -> CP_UTF8
	return _MultiByteToWideChar(CP_UTF8, dwFlags, lpMultiByteStr, cbMultiByte, lpWideCharStr, cchWideChar);
}

// Convert shift-jis to utf8 for JMS
decltype(WideCharToMultiByte) *_WideCharToMultiByte = 0;
int WINAPI WideCharToMultiByte_Hook(UINT CodePage, DWORD dwFlags, LPCWCH lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte, LPCCH lpDefaultChar, LPBOOL lpUsedDefaultChar) {
	// CP_ACP -> CP_UTF8
	return _WideCharToMultiByte(CP_UTF8, dwFlags, lpWideCharStr, cchWideChar, lpMultiByteStr, cbMultiByte, lpDefaultChar, lpUsedDefaultChar);
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
	if(SimpleHook::IsCallerEXE(_ReturnAddress())) {
		return 0;
	}

	return _ImmAssociateContext(hw, hi);
}

// Bypasss OS Language Checks
// JMS v186.1 Pre-BB
// 74 75 FF 15 ?? ?? ?? ?? 66 3D 11 04 75 0D FF 15 ?? ?? ?? ?? 3D A4 03 00 00 74
// JMS v188.0 Post-BB
// 0F 84 84 00 00 00 FF 15 ?? ?? ?? ?? B9 11 04 00 00 66 3B C1 75 0D FF 15

decltype(GetSystemDefaultLangID) *_GetSystemDefaultLangID = NULL;
LANGID WINAPI GetSystemDefaultLangID_Hook() {
	if (SimpleHook::IsCallerEXE(_ReturnAddress())) {
		return LANGUAGE_JMS;
	}

	return _GetSystemDefaultLangID();
}

decltype(GetACP) *_GetACP = NULL;
UINT WINAPI GetACP_Hook() {
	if (SimpleHook::IsCallerEXE(_ReturnAddress())) {
		return CODEPAGE_JMS;
	}

	return _GetACP();
}

bool SupportUTF8() {
	// for JMS, TWMS
	SHook(GetACP);
	SHook(GetSystemDefaultLangID);
	// for JMS, TWMS
	SHook(MultiByteToWideChar);
	SHook(WideCharToMultiByte);
	SHook(GetClipboardData);
	// for MSEA
	SHook(ImmAssociateContext);
	return true;
}