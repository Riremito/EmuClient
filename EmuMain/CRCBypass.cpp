#include"CRCBypass.h"

std::vector<MEMORY_BASIC_INFORMATION> vSection;
std::vector<void*> vBackup;
ULONG_PTR uMSCRC_Ret = 0;

DWORD __stdcall GetBackup(DWORD dwAddress) {
	for (size_t i = 0; i < vBackup.size(); i++) {
		if ((DWORD)vSection[i].BaseAddress <= dwAddress && dwAddress <= ((DWORD)vSection[i].BaseAddress + vSection[i].RegionSize)) {
			return (DWORD)&((BYTE *)vBackup[i])[dwAddress - (DWORD)vSection[i].BaseAddress];
		}
	}

	return dwAddress;
}

// v186.1
void __declspec(naked) CRCBypass186() {
	__asm {
		mov ecx, [ecx]
		push eax

		push edx
		push ecx
		push ebx
		push esi
		push edi

		lea eax, [esi + edx * 0x4]
		push eax
		call GetBackup

		pop edi
		pop esi
		pop ebx
		pop ecx
		pop edx


		xor ecx, [eax]

		pop eax
		jmp uMSCRC_Ret
	}
}

// 00A2A60D - 0FB6 09  - movzx ecx, byte ptr[ecx]
// 00A2A610 - 8B 55 14 - mov edx, [ebp + 14]
void __declspec(naked) CRCBypass180() {
	__asm {
		push eax
		push edx
		push ecx
		push ebx
		push esi
		push edi
		push ecx // Address
		call GetBackup
		// eax = new address
		pop edi
		pop esi
		pop ebx
		pop ecx
		pop edx
		movzx ecx, byte ptr[eax] // crc bypass
		pop eax
		mov edx, [ebp + 0x14]
		jmp uMSCRC_Ret
	}
}

// 0098A486 - 0FB6 3C 1F - movzx edi, byte ptr[edi + ebx]
// 0098A48A - 8B DA - mov ebx, edx
void __declspec(naked) CRCBypass176() {
	__asm {
		push eax
		push edx
		push ecx
		push ebx
		push esi
		push edi
		add ebx, edi
		push ebx
		call GetBackup
		// eax = new address
		pop edi
		pop esi
		pop ebx
		pop ecx
		pop edx
		movzx edi, byte ptr[eax] // crc bypass
		pop eax
		mov ebx, edx
		jmp uMSCRC_Ret
	}
}

// 00BA2C8D - 33 04 8E - xor eax, [esi + ecx * 4]
// 00BA2C90 - 25 FF000000 - and eax, 000000FF
void __declspec(naked) CRCBypass187() {
	__asm {
		push eax
		push edx
		push ebx
		push esi
		push edi
		lea edi, [esi + ecx * 4]
		push edi
		call GetBackup
		mov ecx, eax // ecxに必要なアドレスを入れる
		pop edi
		pop esi
		pop ebx
		pop edx
		pop eax
		xor eax, [ecx] // ecx保存不要
		and eax, 0x000000FF
		jmp uMSCRC_Ret
	}
}

void CRCBypass(Rosemary &r) {

	r.Backup(vSection, vBackup);

	for (size_t i = 0; i < vSection.size(); i++) {
		DEBUG(L"vSection = " + DWORDtoString((ULONG_PTR)vSection[i].BaseAddress) + L" - " + DWORDtoString((ULONG_PTR)vSection[i].BaseAddress + vSection[i].RegionSize) + L", Backup = " + DWORDtoString((ULONG_PTR)vBackup[i]));
	}

	// 0x00BA2C8D v187.0
	ULONG_PTR uMSCRC = r.Scan(L"33 04 8E 25 FF 00 00 00 33 14 85");
	if (uMSCRC) {
		uMSCRC_Ret = uMSCRC + 8;
		r.Hook(uMSCRC, CRCBypass187, 3);

		ULONG_PTR uThemidaCRC = r.Scan(L"55 8B EC 83 EC 54 53 56 57 89 4D B4 8B 4D B4 E8");
		if (uThemidaCRC) {
			r.Patch(uThemidaCRC, L"31 C0 C3");
		}
		DEBUG(L"uThemidaCRC = " + DWORDtoString(uThemidaCRC));
		ULONG_PTR uLoginCRC = 0;
		do {
			uLoginCRC = r.Scan(L"55 8B EC 83 EC 14 53 56 57 83 4D FC FF C7 45 F8");
			if (uLoginCRC) {
				r.Patch(uLoginCRC, L"31 C0 C3");
			}
			DEBUG(L"uLoginCRC = " + DWORDtoString(uLoginCRC));
		} while (uLoginCRC);
	}
	else {
		// v187.0だと別のアドレスがヒットしてしまうのでv187.0を優先して処理する
		// 0x0098A486 v176.0
		uMSCRC = r.Scan(L"0F B6 3C 1F 8B DA 23 DE 33 FB 8B 3C BD");
		if (uMSCRC) {
			uMSCRC_Ret = uMSCRC + 0x06;
			r.Hook(uMSCRC, CRCBypass176, 1);
		}
		// 0x00A2A60D v180.1
		if (!uMSCRC) {
			uMSCRC = r.Scan(L"0F B6 09 8B 55 14 8B 12 33 D1 81 E2 FF 00 00 00 33 04 95");
			if (uMSCRC) {
				uMSCRC_Ret = uMSCRC + 0x06;
				r.Hook(uMSCRC, CRCBypass180, 1);
			}
		}
		if (!uMSCRC) {
			// 0x00B5D2B0 v186.1
			uMSCRC = r.Scan(L"8B 4D 18 8B 55 E0 8B 75 08 8B 09 33 0C 96 81 E1 FF 00 00 00 33 04 8D");
			if (uMSCRC) {
				uMSCRC += 0x09;
				uMSCRC_Ret = uMSCRC + 0x05;
				r.Hook(uMSCRC, CRCBypass186);
			}
		}
	}
	DEBUG(L"uMSCRC = " + DWORDtoString(uMSCRC));
}