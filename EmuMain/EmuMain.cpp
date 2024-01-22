//#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#include<winsock2.h>
//#pragma comment(lib, "ws2_32.lib")
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
bool RemoveAntiCheat_EasyVer(Rosemary &r) {
	ULONG_PTR Call = r.Scan(L"E8 ?? ?? ?? ?? 85 C0 74 0A E8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 83 ?? ?? 00 0F");//HS
	ULONG_PTR uCSecurityClient__IsInstantiated = Call + 0x05 + *(signed long int*)(Call + 0x01);
	if (!Call) {
		Call = r.Scan(L"83 ?? ?? ?? ?? ?? 00 0F 95 C0 C3 A1 ?? ?? ?? ?? C3 8B 01 8B");//XIGNCODE
		uCSecurityClient__IsInstantiated = Call + 0x05 + *(signed long int*)(Call + 0x01);
		if (!Call) return false;
	}

/*	ULONG_PTR uCSecurityClient__IsInstantiated = r.Scan(AOB_EasyRemoveHS[0]);

	if (!uCSecurityClient__IsInstantiated) {
		uCSecurityClient__IsInstantiated = r.Scan(AOB_EasyRemoveHS[1]);

		if (!uCSecurityClient__IsInstantiated) return false;
	}*/

	SCANRES(uCSecurityClient__IsInstantiated);
	r.Patch(uCSecurityClient__IsInstantiated, L"31 C0 C3");

	//AOBPatch(EasyRemoveHS, L"31 C0 C3");

	ULONG_PTR uKeyCryptCall = r.Scan(L"E8 ?? ?? ?? ?? EB 05 E8 ?? ?? ?? ?? 66");
	ULONG_PTR uStartKeyCrypt = uKeyCryptCall + 0x05 + *(signed long int*)(uKeyCryptCall + 0x01);
	ULONG_PTR uStopKeyCrypt = uKeyCryptCall + 0x0C + *(signed long int*)(uKeyCryptCall + 0x08);
	r.Patch(uStartKeyCrypt, L"31 C0 C3");
	r.Patch(uStopKeyCrypt, L"31 C0 C3");

	//AOBPatch(StartKeyCrypt, L"31 C0 C3");
	//AOBPatch(StopKeyCrypt, L"31 C0 C3");

	//TMS157.2 HS things (when enter character select page)
	ULONG_PTR uHS_things = r.Scan(L"83 ?? ?? ?? ?? ?? 00 74 45 83 ?? ?? 00 A1 ?? ?? ?? ?? FF ?? ?? ?? ?? ?? 8D ?? EC 68 ?? ?? ?? ?? 50 C6 ?? ?? 02 E8");
	r.JMP(uHS_things, uHS_things + 0x4E); //r.JMP(0x60ED4F, 0x60ED9D);

	return true;
}

// Window Mode is needed to run old client with recent displays
// or change display settings 59.9 fps to 60.1 fps, 60.1 fps may allow to use full screen mode
void FixClient(Rosemary &r) {
	// JMS v186 or before
	AOBPatch_ADD(WindowMode_PreBB, 0x03, L"00 00 00 00");
	AOBPatch(Launcher, L"B8 01 00 00 00 C3");
	AOBPatch(Ad, L"B8 01 00 00 00 C3");
	// v194.0 has System Settings causes crash without using this code
	AOBPatch(MapleNetwork, L"31 C0 C2 08 00");
}

// Hardware BreakPoint Detection
// DR_Check calls NtGetContextThread of drxxx.tmp DLL, so you can disable it by editing DLL memory
void Disable_AntiDebug(Rosemary &r) {
	AOBPatch(DR_Check, L"31 C0 C3");
}

/*
decltype(gethostbyname) *_gethostbyname = NULL;
struct hostent* PASCAL gethostbyname_Hook(const char * name) {
	// TWMS fix
	if (name && strcmp(name, "tw.login.maplestory.gamania.com")) {
		return _gethostbyname("127.0.0.1");
	}

	return _gethostbyname(name);
}
*/

void EmuMain() {
	Rosemary r;

	// unpack and unvirtualized checks, if your client is unvirtualized, you should remove mscrc yourself
	if (!GetDEVM()) {
		// mscrc remove for packed client, it does not work if client is already unvirtualized
		if (!RemoveCRC(r)) {
			// you can disable memory dump method by ini
			if (!GetMSDisableMemoryDump()) {
				// regular mscrc bypass method
				CRCBypass(r);
			}
		}
	}

	switch (GetMSRegion()) {
	case MS_JMS: {
		RemoveHackShield(r);
		Disable_AntiDebug(r);
		FixClient(r);

		switch (GetMSVersion()) {
		case 186: {
			//r.JMP(0x0086826A, 0x0086874D);
			break;
		}
		case 187:
		{
			// JMS v187 only, later version of bigbang client has window mode option (it will cause TMS crash)
			AOBPatch(WindowMode_PostBB, L"31 C0 C3");
			break;
		}
		default:
		{
			break;
		}
		}

		break;
	}
	case MS_TWMS: {
		switch (GetMSVersion()) {
		case 122:
		{
			RemoveCRC(r);
			ULONG_PTR uGamania = r.StringPatch("tw.login.maplestory.gamania.com", "127.0.0.1");
			SCANRES(uGamania);
			// Remove HackShield Easy Ver
			r.Patch(0x00564878, L"31 C0 C3");
			r.Patch(0x0081FB58, L"31 C0 C3"); // 007E67D4
			r.Patch(0x0081FE3D, L"31 C0 C3"); // 007E67D4
			// Window Mode
			r.Patch(0x007E395D + 3, L"00 00 00 00");

			// modify tw.login.maplestory.gamania.com, or hook inet_addr
			//r.Patch(0x004771D4, L"31 C0 C2 08 00"); // not work...
			//SHook(gethostbyname);

			// HackShield_Init is vmed? not found
			// MSCRC 007DE758

			/*
			// EHSvc_Loader_1 -> ret 0310 C2 10 03 55 8B EC
			r.Patch(0x00826E16, L"31 C0 C2 10 03");
			// EHSvc_Loader_2 -> EB 08 EB AB EB A9 EB A7 EB A5 5F 8B E5 5D C2 18 00
			r.Patch(0x00825F42, L"31 C0 C2 18 00");
			// MKD25tray + ASPLunchr
			r.Patch(0x0081FDD6, L"31 C0 C3");
			// Autoup
			r.Patch(0x0081FF00, L"31 C0 C3");
			// HSUpdate
			r.Patch(0x0081FBD9, L"31 C0 C3");
			*/
			break;
		}
		case 157:
		{
			RemoveAntiCheat_EasyVer(r);
			Disable_AntiDebug(r);
			FixClient(r);
			// TMS old default DNS (not available cause crash)
			ULONG_PTR uGamania = r.StringPatch("tw.login.maplestory.gamania.com", "127.0.0.1");
			SCANRES(uGamania);
			r.JMP(0xAC0A82, r.Scan(L"68 FF 00 00 00 6A 00 6A 00 8B ?? ?? ?? ?? ?? 83 ?? ?? ?? 6A 03 FF 15"));//TMS157.2 MSCRC ManipulatePacket
			break;
		}
		default:
		{
			break;
		}
		}
		break;
	}
	case MS_CMS: {
		switch (GetMSVersion()) {
		case 86:
		{
			// RemoveCRC_Run: 00733A20 -> 009CCAD9 (00DA7104)
			RemoveCRC(r);
			// 00B69428, 00B69410, 00B693F8, 00B693E0, 00B693C8
			ULONG_PTR uGamania = r.StringPatch("mxdlogin.poptang.com", "221.231.130.70");
			SCANRES(uGamania);
			uGamania = r.StringPatch("mxdlogin2.poptang.com", "221.231.130.70");
			SCANRES(uGamania);
			uGamania = r.StringPatch("mxdlogin3.poptang.com", "221.231.130.70");
			SCANRES(uGamania);
			uGamania = r.StringPatch("mxdlogin5.poptang.com", "221.231.130.70");
			SCANRES(uGamania);
			uGamania = r.StringPatch("mxdlogin6.poptang.com", "221.231.130.70");
			SCANRES(uGamania);
			// Remove HackShield Easy Ver
			r.Patch(0x004A5090, L"31 C0 C3");
			// HideDLL
			r.Patch(0x0045F830, L"31 C0 C3");

			// HackShield_Init
			//r.Patch(0x00A1E6F0, L"31 C0 C2 04 00");
			// EHSvc_Loader_1
			//r.Patch(0x00A25CA6, L"31 C0 C2 10 03");
			// EHSvc_Loader_2
			//r.Patch(0x00A24DD2, L"31 C0 C2 18 00");
			//AOBPatch(MKD25tray, L"31 C0 C3");
			//AOBPatch(Autoup, L"31 C0 C3");
			//AOBPatch(ASPLunchr, L"31 C0 C3");

			// HSUpdate vmed
			//r.Patch(0x00A1F100, L"31 C0 C3");
			// Window Mode
			//r.Patch(0x009CD000, L"31 C9 90 90 90 90");
			break;
		}
		default:
		{
			break;
		}
		}
		break;
	}
	case MS_MSEA: {
		RemoveHackShield(r);

		switch (GetMSVersion()) {
		case 97:
		{
			r.Patch(0x00454AEA, L"31 C0 C3"); // hide DLL MSEA v97
			//r.Patch(0x00AB83A0, L"83 C4 14"); // MSEA v97 MSCRC function
			//r.JMP(0x00AB83A0 + 3, 0x00AB9C8F); // MSEA v97 MSCRC function
			break;
		}
		case 102:
		{
			r.Patch(0x004C1930, L"31 C0 C3"); // hide DLL MSEA v102
			break;
		}
		default:
		{
			break;
		}
		}

		break;
	}
	case MS_KMS: {
		switch (GetMSVersion()) {
		case 100:
		{
			// Artale Client v1.03 (KMS v1.2.100)
			// CWvsApp::ConnectLogin
			r.Patch(0x009D0483, L"31 C0 C3"); // this function is edited and it includes anti debugging or something
			// Window Mode
			r.Patch(0x009D1067 + 3, L"00 00 00 00");
		}
		default:
		{
			break;
		}
		}
		break;
	}
	case MS_GMS: {
		break;
	}
	default:
	{
		RemoveHackShield(r);
		Disable_AntiDebug(r);
		FixClient(r);
		break;
	}
	}
}