#include"../Share/Simple/Simple.h"
#include"../Share/Hook/SimpleHook.h"

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
	return true;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	if(fdwReason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(hinstDLL);
		LoadConfig(hinstDLL);
		FontHook();
	}
	return TRUE;
}