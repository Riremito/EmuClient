#include"../Share/Hook/SimpleHook.h"
#pragma comment(lib, "Winmm.lib")

DWORD start_time = 0;
DWORD (WINAPI *_timeGetTime)() = NULL;
DWORD WINAPI timeGetTime_Hook() {
	return _timeGetTime() - start_time;
}

bool TimeFix() {
	start_time = timeGetTime();
	SHook(timeGetTime);
	return true;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	if (fdwReason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(hinstDLL);
		TimeFix();
	}
	return TRUE;
}