#include"../Share/Simple/Simple.h"
#include"../Share/Hook/SimpleHook.h"

void PatchNameSpace() {
	Rosemary rns(L"NameSpace.dll");
	ULONG_PTR uBandAid_1 = rns.Scan(L"6A 01 5B 0F 84 9A 00 00 00 66");
	ULONG_PTR uBandAid_2 = rns.Scan(L"6A 01 50 8D 45 F0 50 8D 45 14 50 C6 45 FC 06 E8");

	SCANRES(uBandAid_1);
	SCANRES(uBandAid_2);
	if (uBandAid_1 && uBandAid_2) {
		rns.Patch(uBandAid_1 + 1, L"02");
		rns.Patch(uBandAid_2 + 1, L"02");
		DEBUG(L"bandaid OK!");
	}
	else {
		DEBUG(L"where should i put bandaid?");
	}
}

decltype(LoadLibraryA) *_LoadLibraryA;
HMODULE WINAPI LoadLibraryA_Hook(LPCSTR lpLibFileName) {
	HMODULE hRet = _LoadLibraryA(lpLibFileName);


	static bool first = true;
	if (first) {
		if (hRet) {
			if (lpLibFileName && (strstr(lpLibFileName, "NAMESPACE.DLL") || strstr(lpLibFileName, "NameSpace.dll"))) {
				PatchNameSpace(); // i have no idea, why LoadLibrary does not work for NameSpace.dll
				first = false;
			}
		}
	}

	return hRet;
}

void BandAid() {
	SHook(LoadLibraryA);
}

void(__thiscall *_CWvsApp__Setup)(void*);
void __fastcall CWvsApp__Setup_Hook(void *ecx) {
	_CWvsApp__Setup(ecx); // it loads some data, and it takes long time
	Rosemary rns(L"NameSpace.dll");
	// disable cache
	rns.Patch(0x5080E922, L"6A 02");
	rns.Patch(0x5080EDC5, L"6A 02");
}

void Hook() {
	// JMS v186.1
	SHookFunction(CWvsApp__Setup, 0x00B5DF75);
}


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
	{
		DisableThreadLibraryCalls(hinstDLL);
		BandAid();
		//Hook();
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

DWORD __stdcall TestFunction() {
	return 0x1337;
}