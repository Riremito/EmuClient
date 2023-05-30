#include"../Share/Simple/Simple.h"
#include"../Share/Hook/SimpleHook.h"

void MemoryPatch() {
	Rosemary r;

	// MapleStory AcGuardian Bypass
	r.Patch(0x005D147D, L"EB");
	r.Patch(0x005D15C5, L"90 90 90 90 90");
	r.Patch(0x004FC0D0, L"C3");
	r.Patch(0x005D37FA, L"90 90 90 90 90");
	r.Patch(0x005D3801, L"90 90 90 90 90");
	r.Patch(0x005D4664, L"EB");
	r.Patch(0x005D5511, L"90 90 90 90 90");
	// Japanese OS CP Check Bypass
	r.Patch(0x005D15A1, L"90 90");
	r.Patch(0x005D15AE, L"90 90");
	// Enable Multi-Client
	r.Patch(0x005D14CA, L"EB");
	// Enable Window Mode
	r.Patch(0x005D47AE + 7, L"00 00 00 00");
	// Modifying Client Resolution
	//r.Patch(0x005D487C + 1, L"00 04 00 00"); // 1024
	//r.Patch(0x005D486D + 1, L"00 03 00 00"); // 768
	// Enable Droppable NX
	r.Patch(0x0048613B, L"90 90 90 90 90 90");
	r.Patch(0x00486147, L"90 90 90 90 90 90");
	// Re-Enable Admin Actions
	r.Patch(0x004860AF, L"EB");
	r.Patch(0x0052A0BC, L"EB");
	r.Patch(0x005E0681, L"EB");
	r.Patch(0x00497604, L"EB");
	// Enable Cash Shop
	r.Patch(0x00463900, L"C2 04 00"); //idk
	// Fix Pets
	// Fix Messengers

}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
	{
		DisableThreadLibraryCalls(hinstDLL);
		MemoryPatch();
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