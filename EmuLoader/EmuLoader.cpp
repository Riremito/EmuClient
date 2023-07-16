#include"../Share/Simple/Simple.h"
#include"../Share/Hook/SimpleHook.h"
#include<intrin.h>
#pragma intrinsic(_ReturnAddress)


#ifndef _WIN64
#define EXE_NAME L"RunEmu"
#define DLL_NAME L"EmuLoader"
#else
#define EXE_NAME L"RunEmu64"
#define DLL_NAME L"EmuLoader64"
#endif

#define FAST_LOAD L"FastLoad"
#define DELAY_LOAD L"DelayLoad"
#define MUTEX_MAPLE L"WvsClientMtx"

std::vector<std::wstring> vFastLoadDlls;
std::vector<std::wstring> vDelayLoadDlls;

// �������W�J�O�ɓǂݍ���
bool FastLoad() {
	for (size_t i = 0; i < vFastLoadDlls.size(); i++) {
		if (LoadLibraryW(vFastLoadDlls[i].c_str())) {
			DEBUG(L"FastLoad:" + vFastLoadDlls[i]);
		}
	}
	return true;
}

// �������W�J��ɓǂݍ���
bool bAlreadyLoaded = false;
bool DelayLoad() {
	if (bAlreadyLoaded) {
		return false;
	}

	bAlreadyLoaded = true;
	for (size_t i = 0; i < vDelayLoadDlls.size(); i++) {
		if (LoadLibraryW(vDelayLoadDlls[i].c_str())) {
			DEBUG(L"DelayLoad:" + vDelayLoadDlls[i]);
		}
	}

	return true;
}

bool IsMapleMutex(LPCWSTR lpName) {
	if (!lpName) {
		return false;
	}

	if (wcsstr(lpName, MUTEX_MAPLE)) {
		return true;
	}

	return false;
}

bool CloseMutex(HANDLE hMutex) {
	HANDLE hDuplicatedMutex = NULL;
	if (DuplicateHandle(GetCurrentProcess(), hMutex, 0, &hDuplicatedMutex, 0, FALSE, DUPLICATE_CLOSE_SOURCE)) {
		CloseHandle(hDuplicatedMutex);
		DEBUG(L"MuliClient: Enabled");
		return true;
	}
	return false;
}

decltype(CreateMutexExW) *_CreateMutexExW = NULL;
HANDLE WINAPI CreateMutexExW_Hook(LPSECURITY_ATTRIBUTES lpMutexAttributes, LPCWSTR lpName, DWORD dwFlags, DWORD dwDesiredAccess) {
	HANDLE hRet = _CreateMutexExW(lpMutexAttributes, lpName, dwFlags, dwDesiredAccess);

	if (IsMapleMutex(lpName)) {
		CloseMutex(hRet);
		if (!bAlreadyLoaded) {
			DEBUG(L"DelayLoad CreateMutexExW");
			DelayLoad();
		}
	}

	return hRet;
}

decltype(RegCreateKeyExA) *_RegCreateKeyExA = NULL;
LSTATUS APIENTRY RegCreateKeyExA_Hook(HKEY hKey, LPCSTR lpSubKey, DWORD Reserved, LPSTR lpClass, DWORD dwOptions, REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition) {
	if (!bAlreadyLoaded) {
		if (lpSubKey && strstr(lpSubKey, "SOFTWARE\\Wizet\\Maple")) {
			DEBUG(L"DelayLoad RegCreateKeyExA");
			DelayLoad();
		}
	}
	return _RegCreateKeyExA(hKey, lpSubKey, Reserved, lpClass, dwOptions, samDesired, lpSecurityAttributes, phkResult, lpdwDisposition);
}

decltype(GetStartupInfoA) *_GetStartupInfoA = NULL;
auto WINAPI GetStartupInfoA_Hook(LPSTARTUPINFOA lpStartupInfo) {
	if (!bAlreadyLoaded) {
		if (lpStartupInfo && SimpleHook::IsCallerEXE(_ReturnAddress())) {
			DEBUG(L"DelayLoad GetStartupInfoA");
			DelayLoad();
		}
	}
	return _GetStartupInfoA(lpStartupInfo);
}


bool EnableHook() {
	SHook(CreateMutexExW);
	// v334.2
	SHook(RegCreateKeyExA);
	// TMS (cause DNS Resolution on CreateMutexExW before)
	SHook(GetStartupInfoA);
	return true;
}

bool EmuLoader(HMODULE hDll) {
	Config conf(EXE_NAME".ini", hDll);
	std::wstring wLoaderDir;

	if (!GetDir(wLoaderDir, hDll)) {
		return false;
	}

	// FixThemida.dll, �N���ł��Ȃ����̏C��
	// LocalHost.dll, �ڑ���̕ύX
	for (size_t i = 1; i <= 10; i++) {
		std::wstring wDllName;
		conf.Read(FAST_LOAD, L"DLL_" + std::to_wstring(i), wDllName);

		if (wDllName.length()) {
			if (wDllName.find('\\') == std::wstring::npos) {
				vFastLoadDlls.push_back(wLoaderDir + L"\\" + wDllName);
			}
			else {
				vFastLoadDlls.push_back(wDllName);
			}
		}
	}

	// EmuMain.dll, GameGuard, HackShield, XignCode�̍폜��MSCRC�̍폜�܂���Bypass, �E�B���h�E���ȂǃQ�[���N���Ɋւ���C��
	// EmuExtra.dll, �Q�[�����̏����̕ύX
	for (size_t i = 1; i <= 10; i++) {
		std::wstring wDllName;
		conf.Read(DELAY_LOAD, L"DLL_" + std::to_wstring(i), wDllName);

		if (wDllName.length()) {
			if (wDllName.find('\\') == std::wstring::npos) {
				vDelayLoadDlls.push_back(wLoaderDir + L"\\" + wDllName);
			}
			else {
				vDelayLoadDlls.push_back(wDllName);
			}
		}
	}

	EnableHook();
	FastLoad();
	return true;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
	{
		DisableThreadLibraryCalls(hinstDLL);
		EmuLoader(hinstDLL);
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