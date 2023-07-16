#include"StringPool.h"

// Client deafult code page
#define CLIENT_CODEPAGE 932 // JMS
// SJIStoUTF8 calls these apis
decltype(MultiByteToWideChar) *_MultiByteToWideChar = 0;
decltype(WideCharToMultiByte) *_WideCharToMultiByte = 0;

bool SJIStoUTF8(BYTE *input, std::string &output, int input_size = 0) {

	if (input_size == 0) {
		input_size = *(int *)((DWORD)input - 0x04);
	}
	// sjis to utf16
	int new_length = _MultiByteToWideChar(CLIENT_CODEPAGE, 0, (char *)input, input_size, 0, 0);
	if (!new_length) {
		return false;
	}

	std::vector<WORD> w(new_length + 1);
	if (!_MultiByteToWideChar(CLIENT_CODEPAGE, 0, (char *)input, input_size, (WCHAR *)&w[0], new_length)) {
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

void(__thiscall *ZXString_char__Assign)(void *, char*, int) = NULL;
ULONG_PTR(__thiscall *_StringPool__GetString)(void *, char **, int, void *);
ULONG_PTR  __fastcall StringPool__GetString_Hook(void *ecx, void *edx, char **v1, int index, void *v3) {
	ULONG_PTR uRet = _StringPool__GetString(ecx, v1, index, v3);

	std::string utf8;
	if (SJIStoUTF8((BYTE *)*v1, utf8)) {
		/*
		for (auto &strtable : StringPoolTable) {
			if (utf8.compare(strtable[0]) == 0) {
				if (strtable[1].length()) {
					utf8 = strtable[1];
				}
				break;
			}
		}
		*/
		ZXString_char__Assign(v1, (char *)utf8.c_str(), utf8.size());
	}

	return uRet;
}

bool StringPoolHook() {
	Rosemary r;
	// Aob
	// StringPool__GetString
	// 8B 86 ?? ?? ?? ?? 0F BE 00 6A 04
	// ZXString_char__Assign
	// ??
	// v186.1
	//ZXString_char__Assign = (decltype(ZXString_char__Assign))0x00419946;
	//SHookFunction(StringPool__GetString, 0x008723EC);
	// v207 test
	//ZXString_char__Assign = (decltype(ZXString_char__Assign))0x0042FC10;
	//SHookFunction(StringPool__GetString, 0x0087CF40 + *(signed long int *)(0x0087CF40 + 0x01) + 0x05); X
	//SHookFunction(StringPool__GetString, 0x0087CF40);
	return true;
}