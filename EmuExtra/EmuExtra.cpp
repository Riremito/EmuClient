#include"EmuExtra.h"
#include"AobList.h"

int conf_MapleVersion = 0;

void MemoryPatch() {
	Rosemary r;

	//DEBUG(L"MemoryPatch ver " +  std::to_wstring(conf_MapleVersion));
	// /+message is enabled
	// vold
	AOBPatch(GMCommand, L"B8 01 00 00 00 90");
	AOBPatch(MapCommand, L"90 90 90 90 90 90 90 90");
	// old-new
	AOBPatch(GMChat, L"B8 01 00 00 00");
	// new bigbang
	AOBPatch(GMCommand_v194_Lv1, L"B8 01 00 00 00");
	AOBPatch(GMCommand_v194_Lv2, L"B8 01 00 00 00");
	AOBPatch(GMCommandLocal_v194, L"B8 01 00 00 00");


	AOBPatch(MapDropLimit, L"B8 00 00 00 00 90 90 90 90 90 90");
	AOBPatch(PointItemDropLimit, L"EB 2D 90 90 90 90");
	AOBPatch(PointItemMultipleDrop, L"B8 00 00 00 00");
}

void MemoryPatchThread() {
	MemoryPatch();
}


#define EXE_NAME L"RunEmu"
#define DLL_NAME L"EmuExtra"

void EmuExtra(HMODULE hDll) {
	/*
	Config conf(DLL_NAME".ini", hDll);

	std::wstring wMapleVersion;
	if (conf.Read(DLL_NAME, L"MapleVersion", wMapleVersion)) {
		conf_MapleVersion = std::stoi(wMapleVersion);
	}
	*/
	HANDLE hThread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)MemoryPatch, NULL, NULL, NULL);
	if (hThread) {
		CloseHandle(hThread);
	}
}