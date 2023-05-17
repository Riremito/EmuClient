#include"../Share/Simple/Simple.h"
#include"../Share/Hook/SimpleHook.h"
#include<intrin.h>
#pragma intrinsic(_ReturnAddress)

HANDLE hMyHeap = NULL;
DWORD dwMainThreadID = 0;
DWORD dwMainSectionStart = 0;
DWORD dwMainSectionEnd = 0;

bool CheckCaller(DWORD dwReturnAddress) {
	if (!hMyHeap) {
		return false;
	}

	if (dwMainThreadID != GetCurrentThreadId()) {
		return false;
	}

	if (dwMainSectionStart <= dwReturnAddress && dwReturnAddress <= dwMainSectionEnd) {
		return true;
	}

	return false;
}

LPVOID(WINAPI *_RtlAllocateHeap)(HANDLE, DWORD, SIZE_T) = NULL;
LPVOID WINAPI RtlAllocateHeap_Hook(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes) {
	if (CheckCaller((DWORD)_ReturnAddress())) {
		return _RtlAllocateHeap(hMyHeap, dwFlags, dwBytes);
	}
	return _RtlAllocateHeap(hHeap, dwFlags, dwBytes);
}

BOOL(WINAPI *_RtlFreeHeap)(HANDLE, DWORD, LPVOID) = NULL;
BOOL WINAPI RtlFreeHeap_Hook(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem) {
	if (CheckCaller((DWORD)_ReturnAddress())) {
		return _RtlFreeHeap(hMyHeap, dwFlags, lpMem);
	}
	return _RtlFreeHeap(hHeap, dwFlags, lpMem);
}

int conf_HeapSize = 0;

#define DLL_NAME L"HeapTest"

bool HeapTest(HMODULE hDll) {
	Config conf(DLL_NAME".ini", hDll);

	std::wstring wHeapSize;
	if (conf.Read(DLL_NAME, L"HeapSize", wHeapSize)) {
		conf_HeapSize = std::stoi(wHeapSize);
		if (conf_HeapSize < 0) {
			conf_HeapSize = 0;
		}
		if (conf_HeapSize > 256) {
			conf_HeapSize = 256;
		}
	}

	Rosemary r;
	std::vector<MEMORY_BASIC_INFORMATION> sections;

	if (!r.GetSectionList(sections) || !sections.size()) {
		DEBUG(L"Error FixHeap (GetSectionList)");
		return false;
	}

	dwMainSectionStart = (DWORD)sections[0].BaseAddress;
	dwMainSectionEnd = dwMainSectionStart + sections[0].RegionSize;

	// memory test
	if (conf_HeapSize) {
		DEBUG(L"HeapTest " + std::to_wstring(conf_HeapSize) + L" MB");

		hMyHeap = HeapCreate(HEAP_NO_SERIALIZE, conf_HeapSize * 1024 * 1024, conf_HeapSize * 1024 * 1024);

		if (!hMyHeap) {
			DEBUG(L"Error FixHeap (HeapCreate)");
			return false;
		}

		dwMainThreadID = GetCurrentThreadId();
		SHookNT(ntdll.dll, RtlAllocateHeap);
		SHookNT(ntdll.dll, RtlFreeHeap);
		return true;
	}

	return false;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
	{
		DisableThreadLibraryCalls(hinstDLL);
		HeapTest(hinstDLL);
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