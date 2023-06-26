#include"EmuMain.h"
#include"AobList.h"
#include"RemoveCRC.h"
#include"CRCBypass.h"

// this was called hsless in JMS/EMS
// this code fully removes HackShield from Client, so you can remove HShield folder
// HSUpdate.exe won't be executed, EHSvc.dll won't be loaded
void RemoveHackShield(Rosemary &r) {
	AOBPatch(HackShield_Init, L"31 C0 C2 04 00");
	AOBPatch(EHSvc_Loader_1, L"31 C0 C2 10 03");
	AOBPatch(EHSvc_Loader_2, L"31 C0 C2 18 00");
	// heatbeat is not needed to run client for private server, but if you send heartbeaat packet to client, client will get crash
	AOBPatch(HeartBeat, L"31 C0 C2 04 00");
	AOBPatch(MKD25tray, L"31 C0 C3");
	AOBPatch(Autoup, L"31 C0 C3");
	AOBPatch(ASPLunchr, L"31 C0 C3");
	AOBPatch(HSUpdate, L"31 C0 C3");
}

// i do not know what this code does
// but it disables HackShield
// if you get crash with this code, you should check MSCRC
// there are some CRCs that checks only certain function memory, CSecurityClient__IsInstantiated is scaned by the CRC
bool RemoveHS_TaiwanVer(Rosemary &r) {
	ULONG_PTR uCSecurityClient__IsInstantiated = r.Scan(AOB_EasyRemoveHS[0]);

	SCANRES(uCSecurityClient__IsInstantiated);
	if (!uCSecurityClient__IsInstantiated) {
		return false;
	}

	r.Patch(uCSecurityClient__IsInstantiated, L"31 C0 C3");
	AOBPatch(StartKeyCrypt, L"31 C0 C3");
	AOBPatch(StartKeyCrypt, L"31 C0 C3");
	AOBPatch(StopKeyCrypt, L"31 C0 C3");
	return true;
}

// Window Mode is needed to run old client with recent displays
// or change display settings 59.9 fps to 60.1 fps, 60.1 fps may allow to use full screen mode
void FixClient(Rosemary &r) {
	// JMS v186 or before
	AOBPatch_ADD(WindowMode_PreBB, 0x03, L"00 00 00 00");
	// JMS v187 only, later version of bigbang client has window mode option
	AOBPatch(WindowMode_PostBB, L"31 C0 C3");
	AOBPatch(Launcher, L"B8 01 00 00 00 C3");
	AOBPatch(Ad, L"B8 01 00 00 00 C3");
}

// Hardware BreakPoint Detection
// DR_Check calls NtGetContextThread of drxxx.tmp DLL, so you can disable it by editing DLL memory
void Disable_AntiDebug(Rosemary &r) {
	AOBPatch(DR_Check, L"31 C0 C3");
}

void EmuMain() {
	Rosemary r;

	if (!RemoveCRC(r)) {
		CRCBypass(r);
	}

	//r.Patch(0x00454AEA, L"31 C0 C3"); // hide DLL v97
	//r.Patch(0x004C1930, L"31 C0 C3"); // hide DLL v102
	//r.Patch(0x00AB83A0, L"83 C4 14"); // MSCRC function
	//r.JMP(0x00AB83A0 + 3, 0x00AB9C8F); // MSCRC function

	//RemoveHS_TaiwanVer(r);
	RemoveHackShield(r);

	Disable_AntiDebug(r);
	FixClient(r);
}