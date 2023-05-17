#include"EmuExtra.h"
#include"AobList.h"

bool AobListScan(Rosemary &r, ULONG_PTR &result, std::wstring aob[], size_t count, int &used) {
	result = 0; // scan result
	used = -1; // which aob is used
	for (size_t i = 0; i < count; i++) {
		result = r.Scan(aob[i]);
		if (result) {
			used = (int)i;
			return true;
		}
	}
	return false;
}

#define PATCHDEBUG(func, patch) \
{\
	ULONG_PTR u##func = 0;\
	AobListScan(r, u##func, AOB_##func, _countof(AOB_##func), iWorkingAob);\
	DEBUG(L""#func" = " + QWORDtoString(u##func) + L", Aob = " + std::to_wstring(iWorkingAob));\
	if(u##func) {\
		r.Patch(u##func, patch);\
	}\
}

int conf_MapleVersion = 0;

void MemoryPatch() {
	Rosemary r;
	int iWorkingAob = 0; // do not change name

	//DEBUG(L"MemoryPatch ver " +  std::to_wstring(conf_MapleVersion));
	PATCHDEBUG(GMChat, L"B8 01 00 00 00");
	PATCHDEBUG(MapDropLimit, L"B8 00 00 00 00 90 90 90 90 90 90");
	PATCHDEBUG(PointItemDropLimit, L"EB 2D 90 90 90 90");
	PATCHDEBUG(PointItemMultipleDrop, L"B8 00 00 00 00");
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
	MemoryPatch();
}