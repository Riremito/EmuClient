#include"CRCBypass.h"

// v334.0 VMProtect CRC Bypass
DWORD MSCRC1 = 0x015BCBD9;
DWORD MSCRC1_ret = 0x015BCBDE;
DWORD VMCRC1 = 0x006A80DF;
DWORD VMCRC2 = 0x005E9BF1;
DWORD VMCRC3 = 0x00933728;
DWORD VMCRC4 = 0x005F00A1;
DWORD VMCRC5 = 0x005EEE21;
DWORD VMCRC1_ret = 0x006A80E5;
DWORD VMCRC2_ret = 0x005E9BF8;
DWORD VMCRC2_call = 0x005F0468;
DWORD VMCRC3_ret = 0x0093372F;
DWORD VMCRC4_ret = 0x005F00A7;
DWORD VMCRC5_ret = 0x005EEE29;
DWORD VMCRC_Start = 0x00401000;
DWORD VMCRC_End = 0x015BDBD9;

DWORD __stdcall GetBackupVM(DWORD dwAddress) {
	// アドレスの範囲がSection丸ごとだと正常に動かない(Section[0]でもNG)
	if (VMCRC_Start <= dwAddress && dwAddress <= VMCRC_End) {
		return GetBackupVM(dwAddress);
	}

	return dwAddress;
}

// VMProtect Self Memory Scan (VMCRC)
bool AutoDetectVMCRC(DWORD dwVMCRC) {
	BYTE *mem = (BYTE *)dwVMCRC;

	// mov al,[edx]
	if (memcmp(mem, "\x8A\x02", 2) == 0) {
		return true;
	}
	// xor al,[edx]
	if (memcmp(mem, "\x32\x02", 2) == 0) {
		return true;
	}
	// mov eax,[eax]
	if (memcmp(mem, "\x8B\x00", 2) == 0) {
		return true;
	}
	// mov ax,[eax]
	if (memcmp(mem, "\x66\x8B\x00", 3) == 0) {
		return true;
	}

	return false;
}

void _declspec(naked) MSCRC1_Hook() {
	__asm {
		push eax
		push ecx
		push ebx
		push ebp
		push esi
		push edi

		push edx // do not save

		push ecx
		call GetBackup
		pop edx // restore
		mov dl, [eax]

		pop edi
		pop esi
		pop ebp
		pop ebx
		pop ecx
		pop eax

		add dl, 0x01
		jmp dword ptr[MSCRC1_ret]
	}
}

void _declspec(naked) AddrCheck() {
	__asm {
		pushfd
		push eax
		push ecx
		push edx
		push ebx
		push ebp
		push esi
		push edi // addr
		call GetBackupVM
		mov edi, eax
		pop esi
		pop ebp
		pop ebx
		pop edx
		pop ecx
		pop eax
		popfd
		ret
	}
}

void _declspec(naked) VMCRC1_Hook() {
	__asm {
		push edi
		mov edi, edx
		call AddrCheck
		mov al, [edi]
		pop edi
		// original
		pushfd
		push[esp]
		jmp dword ptr[VMCRC1_ret]
	}
}

void _declspec(naked) VMCRC2_Hook() {
	__asm {
		push edi
		mov edi, edx
		call AddrCheck
		xor al, [edi]
		pop edi
		// original
		call dword ptr[VMCRC2_call]
		jmp dword ptr[VMCRC2_ret]
	}
}

void _declspec(naked) VMCRC3_Hook() {
	__asm {
		push edi
		mov edi, edx
		call AddrCheck
		xor al, [edi]
		pop edi
		// original
		mov[esp + 0x10], ax
		jmp dword ptr[VMCRC3_ret]
	}
}

void _declspec(naked) VMCRC4_Hook() {
	__asm {
		push edi
		mov edi, eax
		call AddrCheck
		mov eax, [edi]
		pop edi
		// original
		pushfd
		push[esp]
		jmp dword ptr[VMCRC4_ret]
	}
}

void _declspec(naked) VMCRC5_Hook() {
	__asm {
		push edi
		mov edi, eax
		call AddrCheck
		mov ax, [edi]
		pop edi
		// original
		pushad
		mov byte ptr[esp], 0xBC
		jmp dword ptr[VMCRC5_ret]
	}
}

bool CRCBypass334(Rosemary &r) {
	// MSCRC
	r.Hook(MSCRC1, MSCRC1_Hook, MSCRC1_ret - MSCRC1 - 0x05);
	// VMProtect CRC
	r.Hook(VMCRC1, VMCRC1_Hook, VMCRC1_ret - VMCRC1 - 0x05);
	r.Hook(VMCRC2, VMCRC2_Hook, VMCRC2_ret - VMCRC2 - 0x05);
	r.Hook(VMCRC3, VMCRC3_Hook, VMCRC3_ret - VMCRC3 - 0x05);
	r.Hook(VMCRC4, VMCRC4_Hook, VMCRC4_ret - VMCRC4 - 0x05);
	r.Hook(VMCRC5, VMCRC5_Hook, VMCRC5_ret - VMCRC5 - 0x05);

	// JMS v334
	r.Patch(0x005EC420, L"31 C0 C3");
	//r.Patch(0x0148BF30, L"31 C0 C3");
	r.Patch(0x015678F0, L"C3");
	r.Patch(0x015679C0, L"C3");

	// Launcher Skip
	r.Patch(0x00C30960, L"B8 01 00 00 00 C3");
	return true;
}