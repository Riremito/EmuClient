#include"../Share/Simple/Simple.h"

#ifndef _WIN64
#define EXE_NAME L"RunEmu"
#define DLL_NAME L"EmuLoader"
#else
#define EXE_NAME L"RunEmu64"
#define DLL_NAME L"EmuLoader64"
#endif

bool RunEmu(std::wstring wDroppedTarget) {
	Config conf(EXE_NAME".ini");
	std::wstring wTarget, wLoaderDLL, wMainDLL;
	bool check = true;

	if (wDroppedTarget.length()) {
		conf.Update(EXE_NAME, L"TargetEXE", wDroppedTarget);

		std::wstring wDeafultLoaderDLL = DLL_NAME".dll";
		conf.Update(EXE_NAME, L"LoaderDLL", wDeafultLoaderDLL);
	}

	check &= conf.Read(EXE_NAME, L"TargetEXE", wTarget);
	check &= conf.Read(EXE_NAME, L"LoaderDLL", wLoaderDLL);

	if (!check) {
		return false;
	}

	std::wstring dir;
	if (!GetDir(dir)) {
		return false;
	}

	Injector injector(wTarget, dir + wLoaderDLL);
	return injector.Run();
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {

	if (__argc < 2) {
		// create default ini file
		if (!RunEmu(L"")) {
			MessageBoxW(NULL, L"Please check target exe file path", EXE_NAME, MB_OK);
		}
	}
	else {
		// read config from ini file
		if (!RunEmu(__wargv[1])) {
			MessageBoxW(NULL, L"Please drop target exe file", EXE_NAME, MB_OK);
		}
	}

	return 0;
}