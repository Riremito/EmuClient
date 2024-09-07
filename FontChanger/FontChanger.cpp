#include"../Share/Simple/Simple.h"
#include"../Share/Hook/SimpleHook.h"
#include<imm.h>
#pragma comment(lib, "Imm32.lib")

struct ORIGINAL
{
	HANDLE hHeap;
	UINT CodePage;
	const char* lpDefaultChar = "";
	BOOL lpUsedDefaultChar = TRUE;
};

struct LRProfile
{
	UINT CodePage;
	UINT LCID;
	long Bias;
	int HookIME;
	int HookLCID;
};

LRProfile settings = { 0 };
ORIGINAL Original = { 0 };

decltype(MultiByteToWideChar) *_MultiByteToWideChar = NULL;
int WINAPI MultiByteToWideChar_Hook(UINT CodePage, DWORD dwFlags, LPCSTR lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar) {
	CodePage = (CodePage >= CP_UTF7) ? CodePage : settings.CodePage;
	return _MultiByteToWideChar(CodePage, dwFlags, lpMultiByteStr, cbMultiByte, lpWideCharStr, cchWideChar);
}

decltype(WideCharToMultiByte) *_WideCharToMultiByte = NULL;
int WINAPI WideCharToMultiByte_Hook(UINT CodePage, DWORD dwFlags, LPCWSTR lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte, LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar) {
	CodePage = (CodePage >= CP_UTF7) ? CodePage : settings.CodePage;
	return _WideCharToMultiByte(CodePage, dwFlags, lpWideCharStr, cchWideChar, lpMultiByteStr, cbMultiByte, lpDefaultChar, lpUsedDefaultChar);
}

// not a hook
inline LPVOID AllocateZeroedMemory(SIZE_T size/*eax*/) {
	return HeapAlloc(Original.hHeap, HEAP_ZERO_MEMORY, size);
}

inline VOID FreeStringInternal(LPVOID pBuffer/*ecx*/) {
	HeapFree(Original.hHeap, 0, pBuffer);
}

inline LPWSTR MultiByteToWideCharInternal(LPCSTR lstr, UINT CodePage = CP_ACP) {
	int lsize = lstrlenA(lstr)/* size without '\0' */, n = 0;
	int wsize = (lsize + 1) << 1;
	LPWSTR wstr = (LPWSTR)HeapAlloc(Original.hHeap, 0, wsize);
	if (wstr) {
		if (CodePage)
			n = _MultiByteToWideChar(CodePage, 0, lstr, lsize, wstr, wsize);
		else
			n = MultiByteToWideChar(CodePage, 0, lstr, lsize, wstr, wsize);
		wstr[n] = L'\0'; // make tail ! 
	}
	return wstr;
}

inline LPSTR WideCharToMultiByteInternal(LPCWSTR wstr, UINT CodePage = CP_ACP) {
	int wsize = lstrlenW(wstr)/* size without '\0' */, n = 0;
	int lsize = (wsize + 1) << 1;
	LPSTR lstr = (LPSTR)HeapAlloc(Original.hHeap, 0, lsize);
	if (lstr) {
		if (CodePage)
			n = _WideCharToMultiByte(CodePage, 0, wstr, wsize, lstr, lsize, NULL, NULL);
		else
			n = WideCharToMultiByte(CodePage, 0, wstr, wsize, lstr, lsize, NULL, NULL);
		lstr[n] = '\0'; // make tail ! 
	}
	return lstr;
}
// not a hook end

// LR
decltype(CreateWindowExA) *_CreateWindowExA = NULL;
HWND WINAPI CreateWindowExA_Hook(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
	LPCWSTR wstrlpClassName = lpClassName ? MultiByteToWideCharInternal(lpClassName) : NULL;
	LPCWSTR wstrlpWindowName = lpWindowName ? MultiByteToWideCharInternal(lpWindowName) : NULL;
	HWND ret = CreateWindowExW(dwExStyle, wstrlpClassName, wstrlpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
	if (wstrlpClassName) {
		FreeStringInternal((LPVOID)wstrlpClassName);
	}
	if (wstrlpWindowName) {
		FreeStringInternal((LPVOID)wstrlpWindowName);
	}
	return ret;
}

decltype(MessageBoxA) *_MessageBoxA = NULL;
int WINAPI MessageBoxA_Hook(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) {
	LPCWSTR wlpText = MultiByteToWideCharInternal(lpText);
	LPCWSTR wlpCaption = MultiByteToWideCharInternal(lpCaption);
	int ret = MessageBoxW(hWnd, wlpText, wlpCaption, uType);
	if (wlpText) {
		FreeStringInternal((LPVOID)wlpText);
	}
	if (wlpCaption) {
		FreeStringInternal((LPVOID)wlpCaption);
	}
	return ret;
}

decltype(GetACP) *_GetACP = NULL;
UINT WINAPI GetACP_Hook() {
	return settings.CodePage;
}

decltype(GetOEMCP) *_GetOEMCP = NULL;
UINT WINAPI GetOEMCP_Hook() {
	return settings.CodePage;
}

decltype(GetCPInfo) *_GetCPInfo = NULL;
BOOL WINAPI GetCPInfo_Hook(
	UINT       CodePage,
	LPCPINFO  lpCPInfo
)
{
	CodePage = settings.CodePage;
	return _GetCPInfo(CodePage, lpCPInfo);
}

static int CheckWindowStyle(HWND hWnd, DWORD type/*ebx*/) {

	LONG_PTR n = GetWindowLongPtrW(hWnd, GWL_STYLE);
	// window no needs conversion ?? 
	if (n == 0) {
		return (0);
	}
	else if (n == /*0x84C820E4*/(WS_POPUP | WS_CLIPSIBLINGS | WS_BORDER | WS_DLGFRAME | WS_SYSMENU |
		WS_EX_RTLREADING | WS_EX_TOOLWINDOW | WS_EX_MDICHILD | WS_EX_TRANSPARENT | WS_EX_NOPARENTNOTIFY)) {
		return (0);
	}
	else if (!(n & (WS_EX_ACCEPTFILES | WS_EX_TRANSPARENT))) {
		return (0);
	}
	else if (!type && (n & WS_EX_CLIENTEDGE)) {
		return (0);
	}
	else if (n & WS_EX_MDICHILD) {
		return (0);
	}
	// other case : 
	return (-1); // xor ebx, ebx !
}

inline LRESULT CallWindowSendMessage(LPVOID lpProcAddress, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
	DWORD_PTR Param1/*ecx*/, DWORD_PTR Param2/*ecx*/, DWORD_PTR Param3/*ecx*/, int FunctionType)
{
	switch (FunctionType) {
	case 0:
	default:
		return ((LRESULT(WINAPI*)(HWND, UINT, WPARAM, LPARAM))(DWORD_PTR)lpProcAddress)
			(hWnd, uMsg, wParam, lParam);
	case 1:
		return ((LRESULT(WINAPI*)(HWND, UINT, WPARAM, LPARAM, DWORD_PTR, DWORD_PTR))(DWORD_PTR)lpProcAddress)
			(hWnd, uMsg, wParam, lParam, Param1, Param2);
	case 2:
		return ((LRESULT(WINAPI*)(HWND, UINT, WPARAM, LPARAM, DWORD_PTR, DWORD_PTR, DWORD_PTR))(DWORD_PTR)lpProcAddress)
			(hWnd, uMsg, wParam, lParam, Param1, Param2, Param3);
	case 3:
		return ((LRESULT(WINAPI*)(DWORD_PTR, HWND, UINT, WPARAM, LPARAM))(DWORD_PTR)lpProcAddress)
			(Param1, hWnd, uMsg, wParam, lParam);
	}
}

static LRESULT SendUnicodeMessage(LPVOID lpProcAddress, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
	DWORD_PTR Param1/*ecx*/, DWORD_PTR Param2/*ecx*/, DWORD_PTR Param3/*ecx*/, int FunctionType)
{
	LPCWSTR lpUnicodeWindowName = NULL, lpUnicodeClassName = NULL;
	WCHAR CharBuffer[2];
	int type = 0;

	//char classname[256]; GetClassNameA(hWnd, classname, sizeof(classname));
	//if (lstrcmpiA(classname, "TListBox") == 0) {
	//ntprintfA(256, 1, "%s: proc-%p hwnd=%p, msg=%04x, wParam=%d, lParam=%d\n", __FUNCTION__, lpProcAddress, hWnd, uMsg, wParam, lParam);
	//}
	switch (uMsg) {
	case EM_REPLACESEL: // LN320
	case WM_SETTEXT: // LN320
	case WM_SETTINGCHANGE: // LN320
	case WM_DEVMODECHANGE: // LN320
	{
		LPCWSTR lParamW = lParam ? MultiByteToWideCharInternal((LPCSTR)lParam) : NULL;
		//	ntprintfA(1024, 1, "3. A(%s) -> W(%S)", (LPCSTR)lParam, lParamW);
		LRESULT hr = CallWindowSendMessage(lpProcAddress, hWnd, uMsg, wParam, (LPARAM)lParamW, Param1, Param2, Param3, FunctionType);
		// LN301
		if (lParamW) FreeStringInternal((LPVOID)lParamW);
		return hr;
	}	break;
	case WM_IME_CHAR: // LN309
	case WM_CHAR: // LN309
	{
		if ((wchar_t)wParam > 0x7F) { // is multibyte ... 
			// here we exchange the order : 
			//	char t = *((char*)&wParam + 0);
			//	*((char*)&wParam + 0) = *((char*)&wParam + 1);
			//	*((char*)&wParam + 1) = t;
			wParam = ((wParam & 0xFF) << 8) | ((wParam & 0xFF00) >> 8);
			MultiByteToWideChar(CP_ACP, 0, (LPCSTR)&wParam, -1, CharBuffer, 2);
			//	*((wchar_t*)&wParam) = CharBuffer[0];
			wParam = CharBuffer[0];
		}
	}	break;
	case WM_GETTEXTLENGTH: // LN327
	{
		LRESULT len = CallWindowSendMessage(lpProcAddress, hWnd, WM_GETTEXTLENGTH, 0, 0, Param1, Param2, Param3, FunctionType);
		if (len > 0) {
			LPWSTR lParamW = (LPWSTR)AllocateZeroedMemory((len + 1) * sizeof(wchar_t));
			CallWindowSendMessage(lpProcAddress, hWnd, WM_GETTEXT, (len + 1) * sizeof(wchar_t), (LPARAM)lParamW,
				Param1, Param2, Param3, FunctionType);
			len = WideCharToMultiByte(CP_ACP, 0, lParamW, -1, NULL, 0, NULL, NULL) - 1; // required
			// LN793
			if (lParamW) FreeStringInternal(lParamW);
		}
		return len;
	}	break;
	case WM_GETTEXT: // LN310
	{
		if (IsBadWritePtr((LPVOID)lParam, 1)) {
			return (0);
		}
		else {
			// L311
			int len = (int)CallWindowSendMessage(lpProcAddress, hWnd, WM_GETTEXTLENGTH, 0, 0, Param1, Param2, Param3, FunctionType);
			// no needs check len == 0 ?? 
			LPWSTR lParamW = (LPWSTR)AllocateZeroedMemory((len + 1) * sizeof(wchar_t));
			len = (int)CallWindowSendMessage(lpProcAddress, hWnd, uMsg, wParam, (LPARAM)lParamW, Param1, Param2, Param3, FunctionType);
			len = WideCharToMultiByte(CP_ACP, 0, lParamW, -1, (LPSTR)lParam, len, NULL, NULL) - 1;
			if (len > 0) {
				// LN793
				if (lParamW) FreeStringInternal(lParamW);
			}
			else {
				// L316
				if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
					*((LPSTR)lParam + wParam - 1) = '\0';
				}
				else {
					// L317
					*((LPSTR)lParam) = '\0';
				}
			}
			return len;
		}
	}	break;
	case LB_FINDSTRINGEXACT: // LN305
	case LB_ADDSTRING: // LN305
	case LB_INSERTSTRING: // LN305
	case LB_FINDSTRING: // LN305
	case LB_ADDFILE: // LN305
	case LB_SELECTSTRING: // LN305
	case LB_DIR: // LN305
		type = 1;
		//	break;
	case CB_FINDSTRINGEXACT: // LN306
	case CB_ADDSTRING: // LN306
	case CB_INSERTSTRING: // LN306
	case CB_SELECTSTRING: // LN306
	case CB_DIR: // LN306
	case CB_FINDSTRING: // LN306
	{
		int ret = CheckWindowStyle(hWnd, type); // ebx = 0 / 1
		if (ret != -1) {
			LPCWSTR lParamW = lParam ? MultiByteToWideCharInternal((LPCSTR)lParam) : NULL;
			// LN899
			LRESULT hr = CallWindowSendMessage(lpProcAddress, hWnd, uMsg, wParam, (LPARAM)lParamW, Param1, Param2, Param3, FunctionType);
			if (lParamW) FreeStringInternal((LPVOID)lParamW);
			return hr;
		}
	}	break;
	// ----------- common controls end ---------------
	default: // LN301
		break;
	}
	// --------- 
	return CallWindowSendMessage(lpProcAddress, hWnd, uMsg, wParam, lParam, Param1, Param2, Param3, FunctionType);
}

decltype(SendMessageA) *_SendMessageA = NULL;
LRESULT WINAPI SendMessageA_Hook(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	return SendUnicodeMessage((LPVOID)(DWORD_PTR)SendMessageW, hWnd, uMsg, wParam, lParam, 0, 0, 0, 0);
}

decltype(SetWindowTextA) *_SetWindowTextA = NULL;
BOOL WINAPI SetWindowTextA_Hook(
	_In_ HWND hWnd,
	_In_opt_ LPCSTR lpString
)
{
	LPCWSTR wstr = lpString ? MultiByteToWideCharInternal(lpString) : NULL;
	//LONG_PTR originalWndProc = GetWindowLongPtrW(hWnd, GWLP_WNDPROC);
	//SetWindowLongPtrW(hWnd, GWLP_WNDPROC, (LONG_PTR)DefWindowProcW);
	BOOL ret = SetWindowTextW(hWnd, wstr);
	//SetWindowLongPtrW(hWnd, GWLP_WNDPROC, originalWndProc);
	if (wstr) {
		FreeStringInternal((LPVOID)wstr);
	}
	return ret;
}

decltype(GetWindowTextA) *_GetWindowTextA = NULL;
int WINAPI GetWindowTextA_Hook(_In_ HWND hWnd, _Out_writes_(nMaxCount) LPSTR lpString, _In_ int nMaxCount)
{
	int wlen = GetWindowTextLengthW(hWnd) + 1;
	LPWSTR lpStringW = (LPWSTR)AllocateZeroedMemory(wlen * sizeof(wchar_t));
	int wsize = GetWindowTextW(hWnd, lpStringW, wlen);
	int lsize = wsize ? WideCharToMultiByte(CP_ACP, 0, lpStringW, wsize, lpString, nMaxCount, NULL, NULL) : 0;
	FreeStringInternal(lpStringW);
	return lsize;
}

decltype(ImmGetCompositionStringA) *_ImmGetCompositionStringA = NULL;
LONG WINAPI ImmGetCompositionStringA_Hook(
	HIMC hIMC,
	DWORD dwIndex,
	LPSTR lpBuf,
	DWORD  dwBufLen
)
{
	LONG ret = _ImmGetCompositionStringA(hIMC, dwIndex, lpBuf, dwBufLen);
	if (lpBuf)
	{
		LPWSTR wstr = MultiByteToWideCharInternal(lpBuf, Original.CodePage);
		if (wstr)
		{
			int wsize = lstrlenW(wstr), n = 0;
			int lsize = (wsize + 1) << 1;
			n = _WideCharToMultiByte(settings.CodePage, 0, wstr, wsize, lpBuf, lsize, NULL, NULL);
			dwBufLen = lsize;
			lpBuf[n] = '\0';
		}
		FreeStringInternal((LPVOID)wstr);
	}
	return ret;
}

// for English players?
LONG WINAPI HookImmGetCompositionStringA_WM(
	HIMC hIMC,
	DWORD dwIndex,
	LPSTR lpBuf,
	DWORD  dwBufLen
)
{
	LONG wsize = ImmGetCompositionStringW(hIMC, dwIndex, NULL, 0);
	LPWSTR wstr = (LPWSTR)AllocateZeroedMemory(wsize);
	ImmGetCompositionStringW(hIMC, dwIndex, wstr, wsize);
	LONG lsize = (wsize + 1) << 1;
	if (lpBuf)
	{
		lsize = _WideCharToMultiByte(settings.CodePage, 0, wstr, wsize, lpBuf, lsize, Original.lpDefaultChar, &Original.lpUsedDefaultChar);
		lpBuf[lsize] = '\0'; // make tail ! 
	}
	FreeStringInternal(wstr);
	return lsize;
}

decltype(ImmGetCandidateListA) *_ImmGetCandidateListA = NULL;
DWORD WINAPI ImmGetCandidateListA_Hook(
	HIMC            hIMC,
	DWORD           deIndex,
	LPCANDIDATELIST lpCandList,
	DWORD           dwBufLen
)
{
	DWORD ret = _ImmGetCandidateListA(hIMC, deIndex, lpCandList, dwBufLen);
	if (lpCandList)
	{
		for (int i = 0; i < lpCandList->dwCount; i++)
		{
			LPSTR lstr = (LPSTR)lpCandList + lpCandList->dwOffset[i];
			LPWSTR wstr = MultiByteToWideCharInternal(lstr, Original.CodePage);
			if (wstr)
			{
				int wsize = lstrlenW(wstr), n = 0;
				int lsize = (wsize + 1) << 1;
				n = _WideCharToMultiByte(settings.CodePage, 0, wstr, wsize, lstr, lsize, NULL, NULL);
				dwBufLen = lsize;
				lstr[n] = '\0';
				FreeStringInternal(wstr);
			}
		}
	}
	return ret;
}

// ?
DWORD WINAPI HookImmGetCandidateListA_WM(
	HIMC            hIMC,
	DWORD           deIndex,
	LPCANDIDATELIST lpCandList,
	DWORD           dwBufLen
)
{
	DWORD ret = _ImmGetCandidateListA(hIMC, deIndex, lpCandList, dwBufLen);
	if (lpCandList)
	{
		DWORD dwBufLenW = ImmGetCandidateListW(hIMC, deIndex, NULL, NULL);
		LPCANDIDATELIST lpCandListW = (LPCANDIDATELIST)AllocateZeroedMemory(dwBufLenW);
		ImmGetCandidateListW(hIMC, deIndex, lpCandListW, dwBufLenW);
		for (int i = 0; i < lpCandList->dwCount; i++)
		{
			LPSTR lstr = (LPSTR)lpCandList + lpCandList->dwOffset[i];
			LPWSTR wstr = (LPWSTR)lpCandListW + lpCandListW->dwOffset[i];
			if (lstr)
			{
				int lsize = lstrlenA(lstr);
				int wsize = wcslen(wstr);
				_WideCharToMultiByte(settings.CodePage, 0, wstr, wsize, lstr, lsize, NULL, NULL);
				//filelog << lstr << "###" << lsize << "###" << wstr << "###" <<wsize << std::endl;
			}
		}
		FreeStringInternal(lpCandListW);
	}
	return ret;
}

decltype(CreateFontA) *_CreateFontA = NULL;
HFONT WINAPI CreateFontA_Hook(
	_In_ int cHeight,
	_In_ int cWidth,
	_In_ int cEscapement,
	_In_ int cOrientation,
	_In_ int cWeight,
	_In_ DWORD bItalic,
	_In_ DWORD bUnderline,
	_In_ DWORD bStrikeOut,
	_In_ DWORD iCharSet,
	_In_ DWORD iOutPrecision,
	_In_ DWORD iClipPrecision,
	_In_ DWORD iQuality,
	_In_ DWORD iPitchAndFamily,
	_In_opt_ LPCSTR pszFaceName
)
{
	LPWSTR pszFaceNameW = MultiByteToWideCharInternal(pszFaceName);
	HFONT ret = CreateFontW(
		cHeight,
		cWidth,
		cEscapement,
		cOrientation,
		cWeight,
		bItalic,
		bUnderline,
		bStrikeOut,
		iCharSet,
		iOutPrecision,
		iClipPrecision,
		iQuality,
		iPitchAndFamily,
		pszFaceNameW
	);
	FreeStringInternal(pszFaceNameW);
	return ret;
}

decltype(CreateFontIndirectA) *_CreateFontIndirectA = NULL;
HFONT WINAPI CreateFontIndirectA_Hook(
	LOGFONTA* lplf
)
{
	LOGFONTW logfont = { sizeof(LOGFONTW), };
	memcpy(&logfont, lplf, sizeof(LOGFONTW));
	MultiByteToWideChar(settings.CodePage, 0, lplf->lfFaceName, -1, logfont.lfFaceName, LF_FACESIZE);
	return CreateFontIndirectW(&logfont);
}

decltype(CreateFontIndirectExA) *_CreateFontIndirectExA = NULL;
HFONT WINAPI CreateFontIndirectExA_Hook(
	ENUMLOGFONTEXDVA* lplf
)
{
	ENUMLOGFONTEXDVW lplfW = { sizeof(ENUMLOGFONTEXDVW), };
	memcpy(&lplfW, lplf, sizeof(ENUMLOGFONTEXDVW));
	MultiByteToWideChar(settings.CodePage, 0, lplf->elfEnumLogfontEx.elfLogFont.lfFaceName, -1, lplfW.elfEnumLogfontEx.elfLogFont.lfFaceName, LF_FACESIZE);
	return CreateFontIndirectExW(&lplfW);
}

decltype(TextOutA) *_TextOutA = NULL;
BOOL WINAPI TextOutA_Hook(
	HDC    hdc,
	int    x,
	int    y,
	LPSTR lpString,
	int    c
)
{
	LPWSTR wstr = MultiByteToWideCharInternal(lpString);
	if (wstr)
	{
		bool ret = TextOutW(hdc, x, y, wstr, 1);
		FreeStringInternal(wstr);
		return ret;
	}
	return _TextOutA(hdc, x, y, lpString, c);
}

decltype(DrawTextExA) *_DrawTextExA = NULL;
int WINAPI DrawTextExA_Hook(
	_In_ HDC hdc,
	LPSTR lpchText,
	_In_ int cchText,
	_Inout_ LPRECT lprc,
	_In_ UINT format,
	_In_opt_ LPDRAWTEXTPARAMS lpdtp
)
{
	LPWSTR wstr = MultiByteToWideCharInternal(lpchText);
	if (wstr)
	{
		int wsize = lstrlenW(wstr);
		int ret = DrawTextExW(hdc, wstr, wsize, lprc, format, lpdtp);
		FreeStringInternal(wstr);
		return ret;
	}
	return _DrawTextExA(
		hdc,
		lpchText,
		cchText,
		lprc,
		format,
		lpdtp
	);
}

decltype(GetClipboardData) *_GetClipboardData = NULL;
HANDLE WINAPI GetClipboardData_Hook(
	UINT uFormat
)
{
	if (uFormat == CF_TEXT)
	{
		HANDLE hClipMemory = _GetClipboardData(CF_UNICODETEXT);
		HANDLE hGlobalMemory = NULL;
		LPWSTR wstr = (LPWSTR)GlobalLock(hClipMemory);
		if (wstr)
		{
			int wsize = lstrlenW(wstr);
			int lsize = (wsize + 1) << 1;
			hGlobalMemory = GlobalAlloc(GHND, lsize);
			if (hGlobalMemory)
			{
				LPSTR lstr = (LPSTR)GlobalLock(hGlobalMemory);
				if (lstr)
				{
					lsize = _WideCharToMultiByte(settings.CodePage, 0, wstr, wsize, lstr, lsize, NULL, NULL);
					lstr[lsize] = '\0';
				}
				GlobalUnlock(hGlobalMemory);
			}
		}
		GlobalUnlock(hClipMemory);
		if (hGlobalMemory)
			return hGlobalMemory;
	}
	return _GetClipboardData(uFormat);
}

decltype(SetClipboardData) *_SetClipboardData = NULL;
HANDLE WINAPI SetClipboardData_Hook(
	UINT uFormat,
	HANDLE hMem
)
{
	if (uFormat == CF_TEXT)
	{
		HANDLE hGlobalMemory = NULL;
		LPSTR lstr = (LPSTR)GlobalLock(hMem);
		if (lstr)
		{
			int lsize = lstrlenA(lstr);
			int wsize = (lsize + 1) << 1;
			hGlobalMemory = GlobalAlloc(GHND, wsize);
			if (hGlobalMemory)
			{
				LPWSTR wstr = (LPWSTR)GlobalLock(hGlobalMemory);
				if (wstr)
				{
					wsize = _MultiByteToWideChar(settings.CodePage, 0, lstr, lsize, wstr, wsize);
					wstr[wsize] = L'\0';
				}
				GlobalUnlock(hGlobalMemory);
			}
		}
		GlobalUnlock(hMem);
		if (hGlobalMemory)
			return _SetClipboardData(CF_UNICODETEXT, hGlobalMemory);
	}
	return _SetClipboardData(uFormat, hMem);
}

decltype(CharPrevExA) *_CharPrevExA = NULL;
LPSTR WINAPI CharPrevExA_Hook(
	_In_ WORD CodePage,
	_In_ LPCSTR lpStart,
	_In_ LPCSTR lpCurrentChar,
	_In_ DWORD dwFlags
)
{
	CodePage = (CodePage >= CP_UTF7) ? CodePage : settings.CodePage;
	return _CharPrevExA(CodePage, lpStart, lpCurrentChar, dwFlags);
}

decltype(CharNextExA) *_CharNextExA = NULL;
LPSTR WINAPI CharNextExA_Hook(
	_In_ WORD CodePage,
	_In_ LPCSTR lpCurrentChar,
	_In_ DWORD dwFlags
)
{
	CodePage = (CodePage >= CP_UTF7) ? CodePage : settings.CodePage;
	return _CharNextExA(CodePage, lpCurrentChar, dwFlags);
}

decltype(IsDBCSLeadByteEx) *_IsDBCSLeadByteEx = NULL;
BOOL WINAPI IsDBCSLeadByteEx_Hook(
	_In_ UINT  CodePage,
	_In_ BYTE  TestChar
)
{
	CodePage = (CodePage >= CP_UTF7) ? CodePage : settings.CodePage;
	return _IsDBCSLeadByteEx(CodePage, TestChar);
}

decltype(DialogBoxParamA) *_DialogBoxParamA = NULL;
INT_PTR WINAPI DialogBoxParamA_Hook(
	_In_opt_ HINSTANCE hInstance,
	_In_ LPCSTR lpTemplateName,
	_In_opt_ HWND hWndParent,
	_In_opt_ DLGPROC lpDialogFunc,
	_In_ LPARAM dwInitParam
)
{
	LPWSTR lpTemplateNameW = MultiByteToWideCharInternal(lpTemplateName);
	return DialogBoxParamW(
		hInstance,
		lpTemplateNameW ? lpTemplateNameW : (LPCWSTR)lpTemplateName,
		hWndParent,
		lpDialogFunc,
		dwInitParam
	);
}

decltype(CreateDialogIndirectParamA) *_CreateDialogIndirectParamA = NULL;
HWND WINAPI CreateDialogIndirectParamA_Hook(
	_In_opt_ HINSTANCE hInstance,
	_In_ LPCDLGTEMPLATEA lpTemplate,
	_In_opt_ HWND hWndParent,
	_In_opt_ DLGPROC lpDialogFunc,
	_In_ LPARAM dwInitParam
)
{
	return CreateDialogIndirectParamW(hInstance, lpTemplate, hWndParent, lpDialogFunc, dwInitParam);
}

/*
decltype(VerQueryValueA) *_VerQueryValueA = NULL;
BOOL WINAPI VerQueryValueA_Hook(
	LPCVOID pBlock,
	LPCSTR lpSubBlock,
	LPVOID* lplpBuffer,
	PUINT puLen
)
{
	if (lstrlenA(lpSubBlock) > 2 && lpSubBlock[0] == '\\' && lpSubBlock[1] == 'S')
	{
		LPWSTR lpSubBlockW = MultiByteToWideCharInternal(lpSubBlock);
		LPWSTR lpBufferW;
		BOOL ret = VerQueryValueW(pBlock, lpSubBlockW, (LPVOID*)&lpBufferW, puLen);
		LPSTR lpBufferA = WideCharToMultiByteInternal(lpBufferW);
		*lplpBuffer = lpBufferA;
		*puLen = lstrlenA(lpBufferA);
		FreeStringInternal(lpSubBlockW);
		return ret;
	}
	return _VerQueryValueA(pBlock, lpSubBlock, lplpBuffer, puLen);
}
*/

decltype(GetModuleFileNameA) *_GetModuleFileNameA = NULL;
DWORD WINAPI GetModuleFileNameA_Hook(
	HMODULE hModule,
	LPSTR lpFilename,
	DWORD nSize
)
{
	LPWSTR lpFilenameW = (LPWSTR)AllocateZeroedMemory(MAX_PATH);
	DWORD ret = GetModuleFileNameW(hModule, lpFilenameW, nSize);
	_WideCharToMultiByte(settings.CodePage, 0, lpFilenameW, MAX_PATH, lpFilename, MAX_PATH, NULL, NULL);
	FreeStringInternal(lpFilenameW);
	return ret;
}

decltype(LoadLibraryExA) *_LoadLibraryExA = NULL;
HMODULE WINAPI LoadLibraryExA_Hook(
	_In_ LPCSTR lpLibFileName,
	_Reserved_ HANDLE hFile,
	_In_ DWORD dwFlags
)
{
	LPWSTR lpLibFileNameW = MultiByteToWideCharInternal(lpLibFileName);
	HMODULE ret = LoadLibraryExW(lpLibFileNameW, hFile, dwFlags);
	FreeStringInternal(lpLibFileNameW);
	return ret;
}

decltype(RegisterClassA) *_RegisterClassA = NULL;
ATOM WINAPI RegisterClassA_Hook(
	_In_ CONST WNDCLASSA* lpWndClass
)
{
	WNDCLASSW* lpWndClassW = new(WNDCLASSW);
	lpWndClassW->style = lpWndClass->style;
	lpWndClassW->lpfnWndProc = lpWndClass->lpfnWndProc;
	lpWndClassW->cbClsExtra = lpWndClass->cbClsExtra;
	lpWndClassW->cbWndExtra = lpWndClass->cbWndExtra;
	lpWndClassW->hInstance = lpWndClass->hInstance;
	lpWndClassW->hIcon = lpWndClass->hIcon;
	lpWndClassW->hCursor = lpWndClass->hCursor;
	lpWndClassW->hbrBackground = lpWndClass->hbrBackground;
	lpWndClassW->lpszMenuName = MultiByteToWideCharInternal(lpWndClass->lpszMenuName);
	lpWndClassW->lpszClassName = MultiByteToWideCharInternal(lpWndClass->lpszClassName);
	return RegisterClassW(lpWndClassW);
}

decltype(RegisterClassExA) *_RegisterClassExA = NULL;
ATOM WINAPI RegisterClassExA_Hook(
	_In_ CONST WNDCLASSEXA* lpWndClass
)
{
	WNDCLASSEXW* lpWndClassW = new(WNDCLASSEXW);
	lpWndClassW->cbSize = lpWndClass->cbSize;
	lpWndClassW->style = lpWndClass->style;
	lpWndClassW->lpfnWndProc = lpWndClass->lpfnWndProc;
	lpWndClassW->cbClsExtra = lpWndClass->cbClsExtra;
	lpWndClassW->cbWndExtra = lpWndClass->cbWndExtra;
	lpWndClassW->hInstance = lpWndClass->hInstance;
	lpWndClassW->hIcon = lpWndClass->hIcon;
	lpWndClassW->hCursor = lpWndClass->hCursor;
	lpWndClassW->hbrBackground = lpWndClass->hbrBackground;
	lpWndClassW->lpszMenuName = MultiByteToWideCharInternal(lpWndClass->lpszMenuName);
	lpWndClassW->lpszClassName = MultiByteToWideCharInternal(lpWndClass->lpszClassName);
	lpWndClassW->hIconSm = lpWndClass->hIconSm;
	return RegisterClassExW(lpWndClassW);
}

inline LRESULT CallProcAddress(LPVOID lpProcAddress, HWND hWnd, HWND hMDIClient,
	BOOL bMDIClientEnabled, INT uMsg, WPARAM wParam, LPARAM lParam)
{
	typedef LRESULT(WINAPI* fnWNDProcAddress)(HWND, int, WPARAM, LPARAM);
	typedef LRESULT(WINAPI* fnMDIProcAddress)(HWND, HWND, int, WPARAM, LPARAM);
	// MDI or not ??? 
	return (bMDIClientEnabled) ? ((fnMDIProcAddress)(DWORD_PTR)lpProcAddress)(hWnd, hMDIClient, uMsg, wParam, lParam)
		: ((fnWNDProcAddress)(DWORD_PTR)lpProcAddress)(hWnd, uMsg, wParam, lParam);
}

decltype(DefWindowProcA) *_DefWindowProcA = NULL;
LRESULT CALLBACK DefWindowProcA_Hook(
	_In_ HWND hWnd,
	_In_ UINT Msg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
)
{
	if (IsWindowUnicode(hWnd))
		return DefWindowProcW(hWnd, Msg, wParam, lParam);
	else
		return _DefWindowProcA(hWnd, Msg, wParam, lParam);
}

decltype(GetTimeZoneInformation) *_GetTimeZoneInformation = NULL;
DWORD WINAPI GetTimeZoneInformation_Hook(
	_Out_ LPTIME_ZONE_INFORMATION lpTimeZoneInformation
)
{
	DWORD ret = _GetTimeZoneInformation(lpTimeZoneInformation);
	if (ret != TIME_ZONE_ID_INVALID) {
		// Warning Bias becomes negative!!!
		lpTimeZoneInformation->Bias = -settings.Bias;
	}
	return ret;
}

decltype(CreateDirectoryA) *_CreateDirectoryA = NULL;
BOOL WINAPI CreateDirectoryA_Hook(
	_In_ LPCSTR lpPathName,
	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes
)
{
	LPWSTR lpPathNameW = MultiByteToWideCharInternal(lpPathName, Original.CodePage);
	BOOL ret = CreateDirectoryW(lpPathNameW, lpSecurityAttributes);
	if (lpPathNameW)
	{
		FreeStringInternal(lpPathNameW);
	}
	return ret;
}

decltype(CreateFileA) *_CreateFileA = NULL;
HANDLE WINAPI CreateFileA_Hook(
	_In_ LPCSTR lpFileName,
	_In_ DWORD dwDesiredAccess,
	_In_ DWORD dwShareMode,
	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	_In_ DWORD dwCreationDisposition,
	_In_ DWORD dwFlagsAndAttributes,
	_In_opt_ HANDLE hTemplateFile
)
{
	LPWSTR lpFileNameW = MultiByteToWideCharInternal(lpFileName, Original.CodePage);
	HANDLE ret = CreateFileW(
		lpFileNameW,
		dwDesiredAccess,
		dwShareMode,
		lpSecurityAttributes,
		dwCreationDisposition,
		dwFlagsAndAttributes,
		hTemplateFile
	);
	if (lpFileNameW)
	{
		FreeStringInternal(lpFileNameW);
	}
	return ret;
}

decltype(GetLocaleInfoA) *_GetLocaleInfoA = NULL;
int WINAPI GetLocaleInfoA_Hook(
	_In_ LCID Locale,
	_In_ LCTYPE LCType,
	_Out_writes_opt_(cchData) LPSTR lpLCData,
	_In_ int cchData
)
{
	return _GetLocaleInfoA(settings.LCID, LCType, lpLCData, cchData);
}

decltype(GetLocaleInfoW) *_GetLocaleInfoW = NULL;
int WINAPI GetLocaleInfoW_Hook(
	_In_ LCID Locale,
	_In_ LCTYPE LCType,
	_Out_writes_opt_(cchData) LPWSTR lpLCData,
	_In_ int cchData
)
{
	return _GetLocaleInfoW(settings.LCID, LCType, lpLCData, cchData);
}
// LR
/*
decltype(CreateFontIndirectA) *_CreateFontIndirectA = NULL;
HFONT WINAPI CreateFontIndirectA_Hook(LOGFONTA *lplf) {
	if (lplf) {
		SHIFTJIS_CHARSET;
		DEFAULT_CHARSET;
		ANSI_CHARSET;
		JOHAB_CHARSET;
		if (lplf->lfCharSet == DEFAULT_CHARSET) {
			//HANGUL_CHARSET;
			//lplf->lfCharSet = HANGUL_CHARSET;
		}

	}
	return _CreateFontIndirectA(lplf);
}
*/

bool FontHook() {
	SHook(MultiByteToWideChar);
	SHook(WideCharToMultiByte);
	SHook(CreateWindowExA);
	SHook(MessageBoxA);
	SHook(GetACP);
	SHook(GetOEMCP);
	SHook(GetCPInfo);
	SHook(SendMessageA);
	SHook(SetWindowTextA);
	SHook(GetWindowTextA);
	SHook(CreateFontA);
	SHook(CreateFontIndirectA);
	SHook(CreateFontIndirectExA);
	SHook(TextOutA);
	SHook(DrawTextExA);
	SHook(GetClipboardData);
	SHook(SetClipboardData);
	SHook(CharPrevExA);
	SHook(CharNextExA);
	SHook(IsDBCSLeadByteEx);
	SHook(DialogBoxParamA);
	SHook(CreateDialogIndirectParamA);
	//SHook(VerQueryValueA);
	SHook(RegisterClassA);
	SHook(RegisterClassExA);
	SHook(DefWindowProcA);
	SHook(GetLocaleInfoA);
	SHook(GetLocaleInfoW);
	SHook(ImmGetCompositionStringA);
	SHook(ImmGetCandidateListA);
	return true;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
	{
		DisableThreadLibraryCalls(hinstDLL);
		Original.hHeap = GetProcessHeap();
		Original.CodePage = GetACP();
		settings.CodePage = 949;
		settings.LCID = MAKELANGID(LANG_KOREAN, SUBLANG_DEFAULT);
		settings.HookIME = 1;
		settings.HookLCID = 1;
		DEBUG(DWORDtoString(settings.LCID) + L", " + std::to_wstring(settings.CodePage));
		FontHook();
		break;
	}
	case DLL_PROCESS_DETACH:
	{
		break;
	}
	default:
	{
		break;
	}
	}
	return TRUE;
}