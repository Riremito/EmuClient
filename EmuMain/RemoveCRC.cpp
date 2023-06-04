#include"RemoveCRC.h"
#include<intrin.h>
#pragma intrinsic(_ReturnAddress)

ULONG_PTR uRun_Leave_VM = 0;
void(__thiscall *_RenderFrame)(void *ecx);
void __fastcall RenderFrame_Hook(void *ecx, void *edx) {
	// themida CRC
	// push xxxxxxxx
	// jmp xxxxxxxx
	if (
		(((BYTE *)_ReturnAddress())[0] == 0x68 && ((BYTE *)_ReturnAddress())[5] == 0xE9) ||
		// pushfd v334.2
		((BYTE *)_ReturnAddress())[0] == 0x9C ||
		// pushad v334.0
		((BYTE *)_ReturnAddress())[0] == 0x60
		) {
		*(ULONG_PTR *)_AddressOfReturnAddress() = uRun_Leave_VM;
	}
	return _RenderFrame(ecx);
}

// MSCRC1
bool RemoveCRC_Run(Rosemary &r) {
	// v188.0 CWvsApp::Run inside
	// IWzGr2D::RenderFrame
	ULONG_PTR uRenderFrame = r.Scan(L"56 57 8B F9 8B 07 8B 48 1C 57 FF D1 8B F0 85 F6 7D 0E 68 ?? ?? ?? ?? 57 56 E8 ?? ?? ?? ?? 8B C6 5F 5E C3");

	if (!uRenderFrame) {
		return false;
	}

	uRun_Leave_VM = r.Scan(L"6A 01 FF 15 ?? ?? ?? ?? 8B 55 08 83 3A 00 75");

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
	bool check = true;
	check &= RemoveCRC_Run(r);
	if (check && RemoveCRC_v334(r)) {
		return true;
	}
	check &= RemoveCRC_OnEnterField(r);
	return check;
}