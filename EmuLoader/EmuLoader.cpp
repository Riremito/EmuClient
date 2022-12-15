#include"../Share/Simple/Simple.h"
#include"../Share/Hook/SimpleHook.h"
#include"MultiClient.h"

bool GetDir2(std::wstring &wDir, HMODULE hDll) {
	WCHAR wcDir[MAX_PATH] = { 0 };

	if (!GetModuleFileNameW(hDll, wcDir, _countof(wcDir))) {
		return false;
	}

	std::wstring dir = wcDir;
	size_t pos = dir.rfind(L"\\");

	if (pos == std::wstring::npos) {
		return false;
	}

	dir = dir.substr(0, pos + 1);
	wDir = dir;
	return true;
}

#ifndef _WIN64
#define DLL_NAME L"EmuLoader"
#else
#define DLL_NAME L"EmuLoader64"
#endif

#define FAST_LOAD L"FastLoad"
#define DELAY_LOAD L"DelayLoad"

std::vector<std::wstring> vFastLoadDlls;
std::vector<std::wstring> vDelayLoadDlls;

// �������W�J�O�ɓǂݍ���
bool FastLoad() {
	for (size_t i = 0; i < vFastLoadDlls.size(); i++) {
		LoadLibraryW(vFastLoadDlls[i].c_str());
	}
	return true;
}

// �������W�J��ɓǂݍ���
bool DelayLoad() {
	for (size_t i = 0; i < vDelayLoadDlls.size(); i++) {
		LoadLibraryW(vDelayLoadDlls[i].c_str());
	}
	return true;
}

bool EmuLoader(HMODULE hDll) {
	Config conf(DLL_NAME".ini", hDll);
	std::wstring wLoaderDir;

	if (!GetDir2(wLoaderDir, hDll)) {
		return false;
	}

	// FixThemida.dll, �N���ł��Ȃ����̏C��
	// LocalHost.dll, �ڑ���̕ύX
	for (size_t i = 1; i <= 10; i++) {
		std::wstring wDllName;
		conf.Read(FAST_LOAD, L"DLL_" + std::to_wstring(i), wDllName);

		if (wDllName.length()) {
			vFastLoadDlls.push_back(wLoaderDir + L"\\" + wDllName);
		}
	}

	// EmuMain.dll, GameGuard, HackShield, XignCode�̍폜��MSCRC�̍폜�܂���Bypass, �E�B���h�E���ȂǃQ�[���N���Ɋւ���C��
	// EmuExtra.dll, �Q�[�����̏����̕ύX
	for (size_t i = 1; i <= 10; i++) {
		std::wstring wDllName;
		conf.Read(DELAY_LOAD, L"DLL_" + std::to_wstring(i), wDllName);

		if (wDllName.length()) {
			vDelayLoadDlls.push_back(wLoaderDir + L"\\" + wDllName);
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