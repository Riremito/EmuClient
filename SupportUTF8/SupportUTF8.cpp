#include"SupportUTF8.h"
#include<intrin.h>
#pragma intrinsic(_ReturnAddress)

std::wstring AOB_DecodeStr[] = {
	// v164.0 to v186.1
	L"B8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 83 EC 18 53 56 57 89 65 F0 6A 01 33 FF 8B F1 5B",
	// v188.0
	L"55 8B EC 6A FF 68 ?? ?? ?? ?? 64 A1 00 00 00 00 50 83 EC 14 53 56 57 A1 ?? ?? ?? ?? 33 C5 50 8D 45 F4 64 A3 00 00 00 00 89 65 F0 8B F1 89 75 E8 C7 45 EC 00 00 00 00 8B 7D 08 B8 01 00 00 00 89 45 FC C7 07 00 00 00 00 0F B7 56 0C 8B 4E 08 89 45 EC 8B 46 14 2B D0 52 03 C1 50 57 E8",
};

std::wstring AOB_DecodeBuffer[] = {
	// v164.0 to v186.1
	L"B8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 83 EC 14 83 65 FC 00 53 56 8B F1 0F B7 46 0C",
	// v188.0
	L"55 8B EC 6A FF 68 ?? ?? ?? ?? 64 A1 00 00 00 00 50 83 EC 14 53 56 57 A1 ?? ?? ?? ?? 33 C5 50 8D 45 F4 64 A3 00 00 00 00 89 65 F0 8B F1 89 75 E8 0F B7 46 0C 8B 4E 14 8B 56 08 8B 7D 0C 2B C1 03 CA C7 45 FC 00 00 00 00 3B C7 73",
};

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


#pragma pack(push, 1)
typedef struct {
	DWORD unk1; // 0
	DWORD unk2; // 0x02
	BYTE *packet; // unk4bytes + packet
	WORD length1; // data length
	WORD unk5; // unk 2 bytes?
	WORD length2; // packet length
	WORD unk7; // ??
	DWORD decoded; // from 0x04 to decoded
} InPacket;
#pragma pack(pop)

bool SJIStoUTF8(BYTE *input, std::string &output, int input_size = 0) {

	if (input_size == 0) {
		input_size = *(int *)((DWORD)input - 0x04);
	}
	// sjis to utf16
	int new_length = MultiByteToWideChar(932, 0, (char *)input, input_size, 0, 0);
	if (!new_length) {
		return false;
	}

	std::vector<WORD> w(new_length + 1);
	if (!MultiByteToWideChar(932, 0, (char *)input, input_size, (WCHAR *)&w[0], new_length)) {
		return false;
	}

	// utf16 to utf8
	new_length = WideCharToMultiByte(CP_UTF8, 0, (WCHAR *)&w[0], -1, 0, 0, 0, 0);
	if (!new_length) {
		return false;
	}

	std::vector<BYTE> b(new_length + 1);
	if (!WideCharToMultiByte(CP_UTF8, 0, (WCHAR *)&w[0], -1, (char *)&b[0], new_length, 0, 0)) {
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





char** (__thiscall *_DecodeStr)(InPacket *p, char **s);
void(__thiscall *_DecodeBuffer)(InPacket *p, BYTE *b, DWORD len);

char** __fastcall DecodeStr_Hook(InPacket *p, void *edx, char **s) {
	char** ret = _DecodeStr(p, s);

	std::string utf8;
	if (SJIStoUTF8((BYTE *)*s, utf8)) {
		int max_size = *(int *)((DWORD)*s - 0x04);
		memset(*s, 0, max_size);
		memcpy_s(*s, utf8.size(), utf8.c_str(), utf8.size());
	}

	return ret;
}

void __fastcall DecodeBuffer_Hook(InPacket *p, void *edx, BYTE *b, DWORD len) {
	_DecodeBuffer(p, b, len);

	std::string utf8;
	if (SJIStoUTF8(b, utf8, len - 1)) {
		int max_size = len - 1;
		memset(b, 0, len);
		memcpy_s(b, utf8.size(), utf8.c_str(), utf8.size());
	}
}

void test(Rosemary &r) {
	int iWorkingAob = 0; // do not change name
	ULONG_PTR uDecodeStr = 0;
	ULONG_PTR uDecodeBuffer = 0;
	HOOKDEBUG(DecodeStr);
	HOOKDEBUG(DecodeBuffer);
}



typedef void(__fastcall* ZXString_char__Assign_t)(void* pThis, void* edx, const char* s, int n);
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

// 00406C76
WCHAR** (__thiscall *_StrToWStr)(void *ecx, char *s) = 0;
WCHAR** __fastcall StrToWStr_Hook(void *ecx, void *edx, char *s) {

	std::string utf8;
	if (SJIStoUTF8((BYTE *)s, utf8, strlen(s))) {
		//return _StrToWStr(ecx, (char *)utf8.c_str());
	}

	return _StrToWStr(ecx, s);
}


ULONG_PTR uMS_Start = 0;
ULONG_PTR uMS_End = 0;

bool IsCallerMS(void *retAddr) {
	if (uMS_Start <= (ULONG_PTR)retAddr && (ULONG_PTR)retAddr <= uMS_End) {
		return true;
	}
	return false;
}

decltype(MultiByteToWideChar) *_MultiByteToWideChar = 0;
int WINAPI MultiByteToWideChar_Hook(UINT CodePage, DWORD dwFlags, LPCCH lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar) {

	if (IsCallerMS(_ReturnAddress()) && CodePage == CP_ACP) {
		return _MultiByteToWideChar(932, dwFlags, lpMultiByteStr, cbMultiByte, lpWideCharStr, cchWideChar);
	}

	return _MultiByteToWideChar(CodePage, dwFlags, lpMultiByteStr, cbMultiByte, lpWideCharStr, cchWideChar);
}

decltype(WideCharToMultiByte) *_WideCharToMultiByte = 0;
int WINAPI WideCharToMultiByte_Hook(UINT CodePage, DWORD dwFlags, LPCWCH lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte, LPCCH lpDefaultChar, LPBOOL lpUsedDefaultChar) {

	if (IsCallerMS(_ReturnAddress()) && CodePage == CP_ACP) {
		return _WideCharToMultiByte(CP_UTF8, dwFlags, lpWideCharStr, cchWideChar, lpMultiByteStr, cbMultiByte, lpDefaultChar, lpUsedDefaultChar);
	}

	return _WideCharToMultiByte(CodePage, dwFlags, lpWideCharStr, cchWideChar, lpMultiByteStr, cbMultiByte, lpDefaultChar, lpUsedDefaultChar);
}

bool SupportUTF8() {
	Rosemary r;
	std::vector<MEMORY_BASIC_INFORMATION> section;
	if (r.GetSectionList(section) && section.size()) {
		uMS_Start = (ULONG_PTR)section[0].BaseAddress;
		uMS_End = uMS_Start + section[0].RegionSize;
	}


	// PreBB v186.1
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

	ULONG_PTR uGetString = 0x008723EC;
	//SHookFunction(GetString, uGetString);
	//SHookFunction(StrToWStr, 0x00406C76);


	SHook(MultiByteToWideChar);
	SHook(WideCharToMultiByte);

	//test(r);
	return true;
}