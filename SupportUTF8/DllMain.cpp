#include"SupportUTF8.h"

// �����삵�܂���

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
	{
		DisableThreadLibraryCalls(hinstDLL);
		SupportUTF8();
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