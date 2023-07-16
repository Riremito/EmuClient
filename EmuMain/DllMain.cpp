#include"EmuMain.h"

MSRegion gConfig_Region = MS_JMS;
int gConfig_Version = 0;
bool gDisable_MemoryDump = false;

MSRegion GetMSRegion() {
	return gConfig_Region;
}

bool SetMSRegion(std::wstring wRegion) {
	// default is JMS
	if (wRegion.compare(L"TWMS") == 0) {
		gConfig_Region = MS_TWMS;
		return true;
	}

	if (wRegion.compare(L"MSEA") == 0) {
		gConfig_Region = MS_MSEA;
		return true;
	}

	return false;
}

std::wstring GetMSRegionString() {

	switch (GetMSRegion()) {
	case MS_JMS:
	{
		return L"JMS";
	}
	case MS_TWMS:
	{
		return L"TWMS";
	}
	case MS_MSEA:
	{
		return L"MSEA";
	}
	default:
	{
		break;
	}
	}

	return L"Unknown";
}

int GetMSVersion() {
	return gConfig_Version;
}

void SetMSVersion(int version) {
	gConfig_Version = version;
}

bool GetMSDisableMemoryDump() {
	return gDisable_MemoryDump;
}

void SetMSDisableMemoryDump(bool flag) {
	gDisable_MemoryDump = flag;
}


#ifndef _WIN64
#define DLL_NAME L"EmuMain"
#else
#define DLL_NAME L"EmuMain64"
#endif

#define INI_FILE_NAME DLL_NAME".ini"

bool LoadConfig(HMODULE hDll) {
	Config conf(INI_FILE_NAME, hDll);
	std::wstring wRegion, wVersion, wMemoryDump;

	// Region
	if (conf.Read(DLL_NAME, L"Region", wRegion)) {
		SetMSRegion(wRegion);
		DEBUG(L"Region=" + GetMSRegionString());
	}
	// Version
	if (conf.Read(DLL_NAME, L"Version", wVersion)) {
		int ver = _wtoi(wVersion.c_str());
		SetMSVersion(ver);
		DEBUG(L"Version=" + std::to_wstring(GetMSVersion()));
	}
	// MSCRC Option
	if (conf.Read(DLL_NAME, L"DisableMemoryDump", wMemoryDump)) {
		int val = _wtoi(wMemoryDump.c_str());
		SetMSDisableMemoryDump(val);
		DEBUG(L"VeDisableMemoryDump=" + std::to_wstring(GetMSDisableMemoryDump()));
	}
	return true;
}


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
	{
		DisableThreadLibraryCalls(hinstDLL);
		LoadConfig(hinstDLL);
		EmuMain();
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