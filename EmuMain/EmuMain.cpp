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
<<<<<<< Updated upstream
bool RemoveHS_EasyVer(Rosemary &r) {
	ULONG_PTR uCSecurityClient__IsInstantiated = r.Scan(AOB_EasyRemoveHS[0]);
=======
bool RemoveAntiCheat_EasyVer(Rosemary &r) {
	// HS
	ULONG_PTR Call = r.Scan(L"E8 ?? ?? ?? ?? 85 C0 74 0A E8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 83 ?? ?? 00 0F");
	ULONG_PTR uCSecurityClient__IsInstantiated;

	if (Call) {
		uCSecurityClient__IsInstantiated = Call + 0x05 + *(signed long int*)(Call + 0x01);
	} else {/// XIGNCODE
		// TSingleton<CSecurityClient>::GetInstance call
	    auto Call = r.Scan(L"E8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 83 7D 08 00 0F ?? ?? ?? ?? ?? 6A 00 68 00 00 00 80 6A 02 6A");
		//auto Call = r.Scan(L"A1 ?? ?? ?? ?? 3B C3 74 24 8D ?? 44 E8 ?? ?? ?? ?? 3B C3 74");
		uCSecurityClient__IsInstantiated = (Call + 0x05 + *(signed long int*)(Call + 0x01)) + 0x06;//TMS192.2 5C9A49
		if (!Call) return false;
	}

/*	ULONG_PTR uCSecurityClient__IsInstantiated = r.Scan(AOB_EasyRemoveAntiCheat[0]);
>>>>>>> Stashed changes

	if (!uCSecurityClient__IsInstantiated) {
		uCSecurityClient__IsInstantiated = r.Scan(AOB_EasyRemoveAntiCheat[1]);

		if (!uCSecurityClient__IsInstantiated) return false;
	}

	SCANRES(uCSecurityClient__IsInstantiated);

<<<<<<< Updated upstream
	r.Patch(uCSecurityClient__IsInstantiated, L"31 C0 C3");
	//AOBPatch(EasyRemoveHS, L"31 C0 C3");
=======
	//AOBPatch(EasyRemoveAntiCheat, L"31 C0 C3");
>>>>>>> Stashed changes

	AOBPatch(StartKeyCrypt, L"31 C0 C3");
	AOBPatch(StopKeyCrypt, L"31 C0 C3");

	//TMS157.2 HS things check
	ULONG_PTR uHS_things = r.Scan(L"83 ?? ?? ?? ?? ?? 00 74 45 83 ?? ?? 00 A1 ?? ?? ?? ?? FF ?? ?? ?? ?? ?? 8D ?? EC 68 ?? ?? ?? ?? 50 C6 ?? ?? 02 E8");
	r.JMP(uHS_things, uHS_things + 0x4E); //r.JMP(0x60ED4F, 0x60ED9D);

	return true;
}

// Window Mode is needed to run old client with recent displays
// or change display settings 59.9 fps to 60.1 fps, 60.1 fps may allow to use full screen mode
void FixClient(Rosemary &r) {
	// JMS v186 or before
	AOBPatch_ADD(WindowMode_PreBB, 0x03, L"00 00 00 00");
	// JMS v187 only, later version of bigbang client has window mode option (it will cause TMS crash)
	//AOBPatch(WindowMode_PostBB, L"31 C0 C3");

	AOBPatch(Launcher, L"B8 01 00 00 00 C3");
	AOBPatch(Ad, L"B8 01 00 00 00 C3");
	// v194.0 has System Settings causes crash without using this code
	AOBPatch(MapleNetwork, L"31 C0 C2 08 00");

	// TMS old default DNS (not available cause crash)
	r.StringPatch("tw.login.maplestory.gamania.com", "127.0.0.1");
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

	//r.Patch(0x00454AEA, L"31 C0 C3"); // hide DLL MSEA v97
	//r.Patch(0x004C1930, L"31 C0 C3"); // hide DLL MSEA v102
	//r.Patch(0x00AB83A0, L"83 C4 14"); // MSEA v97 MSCRC function
	//r.JMP(0x00AB83A0 + 3, 0x00AB9C8F); // MSEA v97 MSCRC function

	r.JMP(0xAC0A82, r.Scan(L"68 FF 00 00 00 6A 00 6A 00 8B ?? ?? ?? ?? ?? 83 ?? ?? ?? 6A 03 FF 15"));//TMS157.2 MSCRC ManipulatePacket

	RemoveHS_EasyVer(r);
	//RemoveHackShield(r);

<<<<<<< Updated upstream
	Disable_AntiDebug(r);
	FixClient(r);
=======
		SCANRES(uGamania);

		switch (GetMSVersion()) {
		case 157:
		{
			r.JMP(0xAC0A82, r.Scan(L"68 FF 00 00 00 6A 00 6A 00 8B ?? ?? ?? ?? ?? 83 ?? ?? ?? 6A 03 FF 15"));//TMS157.2 MSCRC CWvsApp::Run first VM (ManipulatePacket)
			r.Patch(0x77068C, L"B8 32 00 00 00 C3");//calc_accr ����50%���vMISS
			//r.Patch(0x77068C, L"B8 14 00 00 00 C3");//calc_accr ����80%���vMISS
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
	default:
	{
		RemoveHackShield(r);
		Disable_AntiDebug(r);
		FixClient(r);
		break;
	}
	}
>>>>>>> Stashed changes
}