#include"../Share/Simple/Simple.h"
#include"../Share/Hook/SimpleHook.h"

#pragma pack(push, 1)
// JMS v20
typedef struct {
	BYTE unk1[0x70];
	void *Platform; // 0x70
	BYTE unk2[0x2C];
	DWORD JumpFlag; // 0xA0
} MapleCharacterObject;

typedef struct {
	BYTE unk1[0x3C];
	MapleCharacterObject * Object; // 0x3C
	BYTE unk2[0x3A0];
	int X; // 0x3E0
	int Y; // 0x3E4
	/*
	struct Teleport {
		int toggle; // 0x3F8
		int x; // 0x400
		int y; // 0x404
	};
	*/
} MapleCharacter;
#pragma pack(pop)

MapleCharacter **MyCharacter = (decltype(MyCharacter))0x0065F40C;
void(__stdcall *_TeleportObject)(MapleCharacterObject *, int X, int Y) = (decltype(_TeleportObject))0x005B17A0;
WCHAR *JumpSoundName = (decltype(JumpSoundName))0x0065B91C;
void (*_PlaySound)(WCHAR *, int) = (decltype(_PlaySound))0x00599380;

void (__thiscall *_MoveCharacter)(MapleCharacterObject *, DWORD) = NULL;
void __fastcall MoveCharacter_Hook(MapleCharacterObject *Object, void *edx, DWORD dwCommand) {
	if (Object->JumpFlag && Object->Platform) {
		// DOWN ARROW key, Key Down
		if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
			MapleCharacter *character = (MapleCharacter *)(*(DWORD *)0x0065F40C);
			if (character) {
				Object->JumpFlag = 0; // clear flag
				_PlaySound(JumpSoundName, 100); // jump SE
				_TeleportObject(Object, character->X, character->Y + 5); // teleport
				return; // do not call original
			}
		}
	}
	// original
	_MoveCharacter(Object, dwCommand);
}

void MemoryPatch() {
	Rosemary r;

	// MapleStory AcGuardian Bypass
	r.Patch(0x005D147D, L"EB");
	r.Patch(0x005D15C5, L"90 90 90 90 90");
	r.Patch(0x004FC0D0, L"C3");
	r.Patch(0x005D37FA, L"90 90 90 90 90");
	r.Patch(0x005D3801, L"90 90 90 90 90");
	r.Patch(0x005D4664, L"EB");
	r.Patch(0x005D5511, L"90 90 90 90 90");
	// Japanese OS CP Check Bypass
	r.Patch(0x005D15A1, L"90 90");
	r.Patch(0x005D15AE, L"90 90");
	// Enable Multi-Client
	r.Patch(0x005D14CA, L"EB");
	// Enable Window Mode
	r.Patch(0x005D47AE + 7, L"00 00 00 00");
	// Modifying Client Resolution
	//r.Patch(0x005D487C + 1, L"00 04 00 00"); // 1024
	//r.Patch(0x005D486D + 1, L"00 03 00 00"); // 768
	// Enable Droppable NX
	r.Patch(0x0048613B, L"90 90 90 90 90 90");
	r.Patch(0x00486147, L"90 90 90 90 90 90");
	// Re-Enable Admin Actions
	r.Patch(0x004860AF, L"EB");
	r.Patch(0x0052A0BC, L"EB");
	r.Patch(0x005E0681, L"EB");
	r.Patch(0x00497604, L"EB");
	// Enable Cash Shop
	r.Patch(0x00463900, L"C2 04 00"); //idk
	// Fix Pets
	// Fix Messengers

	// Add Jump Down
	SHookFunction(MoveCharacter, 0x005ACBF0);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
	{
		DisableThreadLibraryCalls(hinstDLL);
		MemoryPatch();
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