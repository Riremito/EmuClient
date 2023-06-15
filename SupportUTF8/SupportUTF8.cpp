#include"SupportUTF8.h"
#include<intrin.h>
#pragma intrinsic(_ReturnAddress)

#define HOOKDEBUG(func) \
{\
	ListScan(r, u##func, AOB_##func, _countof(AOB_##func), iWorkingAob);\
	DEBUG(L""#func" = " + QWORDtoString(u##func) + L", Aob = " + std::to_wstring(iWorkingAob));\
	if (iWorkingAob > -1) {\
		SHookFunction(func, u##func);\
	}\
}

bool ListScan(Rosemary &r, ULONG_PTR &result, std::wstring aob[], size_t count, int &used) {
	result = 0; // scan result
	used = -1; // which aob is used
	for (size_t i = 0; i < count; i++) {
		result = r.Scan(aob[i]);
		if (result) {
			used = (int)i;
			return true;
		}
	}
	return false;
}

// SJIStoUTF8 calls these apis
decltype(MultiByteToWideChar) *_MultiByteToWideChar = 0;
decltype(WideCharToMultiByte) *_WideCharToMultiByte = 0;

bool SJIStoUTF8(BYTE *input, std::string &output, int input_size = 0) {

	if (input_size == 0) {
		input_size = *(int *)((DWORD)input - 0x04);
	}
	// sjis to utf16
	int new_length = _MultiByteToWideChar(932, 0, (char *)input, input_size, 0, 0);
	if (!new_length) {
		return false;
	}

	std::vector<WORD> w(new_length + 1);
	if (!_MultiByteToWideChar(932, 0, (char *)input, input_size, (WCHAR *)&w[0], new_length)) {
		return false;
	}

	// utf16 to utf8
	new_length = _WideCharToMultiByte(CP_UTF8, 0, (WCHAR *)&w[0], -1, 0, 0, 0, 0);
	if (!new_length) {
		return false;
	}

	std::vector<BYTE> b(new_length + 1);
	if (!_WideCharToMultiByte(CP_UTF8, 0, (WCHAR *)&w[0], -1, (char *)&b[0], new_length, 0, 0)) {
		return false;
	}

	if ((input_size + 1) == new_length) {
		return false;
	}

	if ((input_size + 1) != new_length) {
		output = (char *)&b[0];
	}

	return true;
}

typedef void(__fastcall* ZXString_char__Assign_t)(void* pThis, void* edx, const char* s, int n);
// v186.1
auto ZXString_char__Assign = reinterpret_cast<ZXString_char__Assign_t>(0x00419946); // ZXString<char>::Assign(char const *,int)


// 8B 86 ?? ?? ?? ?? 0F BE 00 6A 04
ULONG_PTR (__thiscall *_GetString)(void *ecx, char **v1, void *v2, void *v3);
ULONG_PTR  __fastcall GetString_Hook(void *ecx, void *edx, char **v1, void *v2, void *v3) {
	ULONG_PTR uRet = _GetString(ecx, v1, v2, v3);


	std::string utf8;
	if (SJIStoUTF8((BYTE *)*v1, utf8)) {
		//DEBUG(DatatoString((BYTE *)utf8.c_str(), utf8.size(), true));
		ZXString_char__Assign(v1, edx, (char *)utf8.c_str(), utf8.size());
	}

	return uRet;
}

int WINAPI MultiByteToWideChar_Hook(UINT CodePage, DWORD dwFlags, LPCCH lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar) {
	CodePage = CP_UTF8;
	return _MultiByteToWideChar(CodePage, dwFlags, lpMultiByteStr, cbMultiByte, lpWideCharStr, cchWideChar);
}

int WINAPI WideCharToMultiByte_Hook(UINT CodePage, DWORD dwFlags, LPCWCH lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte, LPCCH lpDefaultChar, LPBOOL lpUsedDefaultChar) {
	CodePage = CP_UTF8;
	return _WideCharToMultiByte(CodePage, dwFlags, lpWideCharStr, cchWideChar, lpMultiByteStr, cbMultiByte, lpDefaultChar, lpUsedDefaultChar);
}

bool SupportUTF8() {
	Rosemary r;
	// PreBB v186.1
	/*
	ULONG_PTR uCodePageCheck = r.Scan(L"74 75 FF 15 ?? ?? ?? ?? 66 3D 11 04 75 0D FF 15 ?? ?? ?? ?? 3D A4 03 00 00 74");

	if (uCodePageCheck) {
		r.Patch(uCodePageCheck, L"EB");
	}
	else {
		// PostBB v188.0
		uCodePageCheck = r.Scan(L"0F 84 84 00 00 00 FF 15 ?? ?? ?? ?? B9 11 04 00 00 66 3B C1 75 0D FF 15");
		if (uCodePageCheck) {
			r.Patch(uCodePageCheck, L"90 E9");
		}
	}

	SCANRES(uCodePageCheck);
	*/

	ULONG_PTR uGetString = 0x008723EC; // v186.1
	SHookFunction(GetString, uGetString);

	
	SHook(MultiByteToWideChar);
	SHook(WideCharToMultiByte);
	return true;
}