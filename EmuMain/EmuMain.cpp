#include"EmuMain.h"
#include"AobList.h"
#include"RemoveCRC.h"
#include"CRCBypass.h"


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

#define PATCHDEBUG_ADD(func, add, patch) \
{\
	ULONG_PTR u##func = 0;\
	AobListScan(r, u##func, AOB_##func, _countof(AOB_##func), iWorkingAob);\
	DEBUG(L""#func" = " + QWORDtoString(u##func) + L", Aob = " + std::to_wstring(iWorkingAob));\
	if(u##func) {\
		r.Patch(u##func + add, patch);\
	}\
}

void RemoveHackShield(Rosemary &r) {
	int iWorkingAob = 0; // do not change name
	PATCHDEBUG(HackShield_Init, L"31 C0 C2 04 00");
	PATCHDEBUG(EHSvc_Loader_1, L"31 C0 C2 10 03");
	PATCHDEBUG(EHSvc_Loader_2, L"31 C0 C2 18 00");
	PATCHDEBUG(HeartBeat, L"31 C0 C2 04 00");
	PATCHDEBUG(MKD25tray, L"31 C0 C3");
	PATCHDEBUG(Autoup, L"31 C0 C3");
	PATCHDEBUG(ASPLunchr, L"31 C0 C3");
	PATCHDEBUG(HSUpdate, L"31 C0 C3");
}

bool RemoveHS_TaiwanVer(Rosemary &r) {
	int iWorkingAob = 0;
	ULONG_PTR uCSecurityClient__IsInstantiated = r.Scan(AOB_EasyRemoveHS[0]);

	SCANRES(uCSecurityClient__IsInstantiated);
	if (!uCSecurityClient__IsInstantiated) {
		return false;
	}

	r.Patch(uCSecurityClient__IsInstantiated, L"31 C0 C3");
	PATCHDEBUG(StartKeyCrypt, L"31 C0 C3");
	PATCHDEBUG(StartKeyCrypt, L"31 C0 C3");
	PATCHDEBUG(StopKeyCrypt, L"31 C0 C3");
	return true;
}

void FixClient(Rosemary &r) {
	int iWorkingAob = 0; // do not change name

	PATCHDEBUG_ADD(WindowMode_PreBB, 0x03, L"00 00 00 00");
	PATCHDEBUG(WindowMode_PostBB, L"31 C0 C3");
	PATCHDEBUG(Launcher, L"B8 01 00 00 00 C3");
	PATCHDEBUG(Ad, L"B8 01 00 00 00 C3");
}

// Hardware BreakPoint Detection
void Disable_AntiDebug(Rosemary &r) {
	int iWorkingAob = 0;

	PATCHDEBUG(DR_Check, L"31 C0 C3");
}

void EmuMain() {
	Rosemary r;

	if (!RemoveCRC(r)) {
		CRCBypass(r);
	}

	RemoveHS_TaiwanVer(r);
	RemoveHackShield(r);


	Disable_AntiDebug(r);
	FixClient(r);
}