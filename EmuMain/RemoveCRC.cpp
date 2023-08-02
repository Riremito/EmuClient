#include"RemoveCRC.h"
#include<intrin.h>
#pragma intrinsic(_ReturnAddress)


ULONG_PTR uRenderFrame = 0;
ULONG_PTR uRun_Leave_VM = 0;
void(__thiscall *_RenderFrame)(void *ecx);
void __fastcall RenderFrame_Hook(void *ecx, void *edx) {
	// call IWzGr2D::RenderFrame
	if ((((BYTE *)_ReturnAddress())[-0x05] == 0xE8)) {
		ULONG_PTR call_function = (ULONG_PTR)&((BYTE *)_ReturnAddress())[-0x05] + *(signed long *)&((BYTE *)_ReturnAddress())[-0x04] + 0x05;

		if (call_function == uRenderFrame) {
			return _RenderFrame(ecx);
		}

		// MSEA v102 jmp vmed, vmed: jmp RenderFrame
		// 00B975B5 - call 004172E2 // RederFrame
		// 004172E2 - jmp 0083C5F0 // wtf
		if (((BYTE *)call_function)[0x00] == 0xE9) {
			call_function = call_function + *(signed long *)&((BYTE *)call_function)[0x01] + 0x05;
			if (call_function == uRenderFrame) {
				return _RenderFrame(ecx);
			}
		}

	}
	// CWvsApp::Run MSCRC
	*(ULONG_PTR *)_AddressOfReturnAddress() = uRun_Leave_VM;
	return _RenderFrame(ecx);
}

// MSCRC1
bool RemoveCRC_Run(Rosemary &r) {
	// v188.0 CWvsApp::Run inside
	// IWzGr2D::RenderFrame
	uRenderFrame = r.Scan(L"56 57 8B F9 8B 07 8B 48 1C 57 FF D1 8B F0 85 F6 7D 0E 68 ?? ?? ?? ?? 57 56 E8 ?? ?? ?? ?? 8B C6 5F 5E C3");

	if (!uRenderFrame) {
		uRenderFrame = r.Scan(L"56 57 8B F9 8B 07 8B 48 1C");

		if (!uRenderFrame) {
			uRenderFrame = r.Scan(L"56 8B F1 8B 06 57 56 FF 50 1C");

			if (!uRenderFrame) return false;
		}
	}

	uRun_Leave_VM = r.Scan(L"6A 01 FF 15 ?? ?? ?? ?? 8B ?? 08 83 ?? 00 75");

	if (!uRun_Leave_VM) {
		return false;
	}

	SHookFunction(RenderFrame, uRenderFrame);
	DEBUG(L"RemoveCRC_Run: " + DWORDtoString(uRenderFrame) + L" -> " + DWORDtoString(uRun_Leave_VM));
	return true;
}

// MSCRC2 and MSCRC3
bool RemoveCRC_OnEnterField(Rosemary &r) {
	// v188.0 CWvsContext::OnEnterField
	ULONG_PTR uOnEnterField = r.Scan(L"55 8B EC 83 EC 40 53 56 57 89 4D C8 8B 4D C8 E8 ?? ?? ?? ?? E9");

	if (!uOnEnterField) {
		return false;
	}

	ULONG_PTR uOnEnterField_Enter_VM = uOnEnterField + 0x14;
	ULONG_PTR uOnEnterField_Leave_VM = r.Scan(L"E8 ?? ?? ?? ?? 85 C0 74 ?? E8 ?? ?? ?? ?? 89 45 E8 83 7D E8 00 74");

	if (!uOnEnterField_Leave_VM) {
		return false;
	}

	r.JMP(uOnEnterField_Enter_VM, uOnEnterField_Leave_VM);
	DEBUG(L"RemoveCRC_OnEnterField: " + DWORDtoString(uOnEnterField_Enter_VM) + L" -> " + DWORDtoString(uOnEnterField_Leave_VM));
	return true;
}

bool RemoveCRC_OnEnterField_194(Rosemary &r) {
	// v194.0 CWvsContext::OnEnterField
	ULONG_PTR uOnEnterField = r.Scan(L"55 8B EC 6A FF 68 ?? ?? ?? ?? 64 A1 00 00 00 00 50 83 EC ?? 53 56 57 A1 ?? ?? ?? ?? 33 C5 50 8D 45 F4 64 A3 00 00 00 00 89 4D ?? 8B 4D ?? E8 ?? ?? ?? ?? 6A 28 8B 4D ?? E8 ?? ?? ?? ?? E9");

	if (!uOnEnterField) {
		return false;
	}

	ULONG_PTR uOnEnterField_Enter_VM = uOnEnterField + 0x3D;
	ULONG_PTR uOnEnterField_Leave_VM = r.Scan(L"89 ?? 89 ?? 89 ?? 90 E8 ?? ?? ?? ?? 89 45 ?? E8 ?? ?? ?? ?? 85 C0 74");

	if (!uOnEnterField_Enter_VM || !uOnEnterField_Leave_VM) {
		return false;
	}

	r.JMP(uOnEnterField_Enter_VM, uOnEnterField_Leave_VM);
	DEBUG(L"RemoveCRC_OnEnterField (v194): " + DWORDtoString(uOnEnterField_Enter_VM) + L" -> " + DWORDtoString(uOnEnterField_Leave_VM));
	return true;
}

bool RemoveCRC_OnEnterField_TMS192_2(Rosemary& r) {

	ULONG_PTR uOnEnterField_Enter_VM = r.Scan(L"E9 ?? ?? ?? ?? 50 EB 55 2C 8A 4A 9C AF 79 54 A0");
	//+0x91
	ULONG_PTR uOnEnterField_Leave_VM = r.Scan(L"8B ?? ?? ?? ?? ?? 81 ?? EC 68 00 00 E8 ?? ?? ?? ?? 8B C8 E8 ?? ?? ?? ?? 8B");

	if (!uOnEnterField_Enter_VM || !uOnEnterField_Leave_VM) {
		return false;
	}

	r.JMP(uOnEnterField_Enter_VM, uOnEnterField_Leave_VM);
	DEBUG(L"RemoveCRC_OnEnterField (TMSv192.2): " + DWORDtoString(uOnEnterField_Enter_VM) + L" -> " + DWORDtoString(uOnEnterField_Leave_VM));
	return true;
}

bool RemoveCRC_v334(Rosemary &r) {
	ULONG_PTR uOnSomething = r.Scan(L"55 8B EC 6A FF 68 ?? ?? ?? ?? 64 A1 00 00 00 00 50 81 EC ?? ?? ?? ?? 53 56 57 A1 ?? ?? ?? ?? 33 C5 50 8D 45 F4 64 A3 00 00 00 00 89 8D ?? ?? ?? ?? C7 45 BC 00 00 00 00 33 C0 89 45 C0 89 45 C4 89 45 C8 89 45 CC 89 45 D0 89 45 D4 C7 45 B0 00 00 00 00 33 C9 89 4D B4 89 4D B8 E8");

	if (!uOnSomething) {
		return false;
	}

	ULONG_PTR uOnSomething_Enter_VM = uOnSomething + 0x70;
	ULONG_PTR uOnSomething_Leave_VM = r.Scan(L"68 FF 00 00 00 6A 00 6A 00 8B 85 ?? ?? ?? ?? 83 C0 68 50 6A 03 FF 15");

	if (!uOnSomething_Leave_VM) {
		return false;
	}

	r.JMP(uOnSomething_Enter_VM, uOnSomething_Leave_VM);
	DEBUG(L"RemoveCRC_OnSomething: " + DWORDtoString(uOnSomething_Enter_VM) + L" -> " + DWORDtoString(uOnSomething_Leave_VM));
	return true;
}

bool RemoveCRC(Rosemary &r) {
	int check = 0;

	if (RemoveCRC_Run(r)) {
		check++;
	}

	if (RemoveCRC_OnEnterField_TMS192_2(r)) {
		check++;
	}

	if (RemoveCRC_v334(r)) {
		check++;
	}

	if (RemoveCRC_OnEnterField(r)) {
		check++;
	} else {
		if (RemoveCRC_OnEnterField_194(r)) {
			check++;
		}
	}

	if (!check) {
		return false;
	}

	return true;
}