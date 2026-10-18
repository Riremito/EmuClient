#include"../Share/Simple/Simple.h"
#include"../Share/Hook/SimpleHook.h"

void JoySticFix() {
	Rosemary r;
	// CInputSystem::Update
	ULONG_PTR uCInputSystem__Update = r.Scan(L"8B 44 24 04 2B 81 ?? ?? ?? ?? 3D 88 13 00 00 7C 05 E8 ?? ?? ?? ?? C2 04 00");
	SCANRES(uCInputSystem__Update);
	if (uCInputSystem__Update) {
		r.Patch(uCInputSystem__Update, L"31 C0 C2 04 00");
	}
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	if (fdwReason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(hinstDLL);
		JoySticFix();
	}
	return TRUE;
}