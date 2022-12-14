#include"MultiClient.h"
#include"../Share/Simple/Simple.h"
#include"../Share/Hook/SimpleHook.h"

bool IsMapleMutex(LPCWSTR lpName) {
	if (!lpName) {
		return false;
	}

	if (wcsstr(lpName, MUTEX_MAPLE)) {
		return true;
	}

	return false;
}

bool CloseMutex(HANDLE hMutex) {
	HANDLE hDuplicatedMutex = NULL;
	if (DuplicateHandle(GetCurrentProcess(), hMutex, 0, &hDuplicatedMutex, 0, FALSE, DUPLICATE_CLOSE_SOURCE)) {
		CloseHandle(hDuplicatedMutex);
		DEBUG(L"Mutex is closed");
		return true;
	}
	return false;
}

decltype(CreateMutexExW) *_CreateMutexExW = NULL;
HANDLE WINAPI CreateMutexExW_Hook(LPSECURITY_ATTRIBUTES lpMutexAttributes, LPCWSTR lpName, DWORD dwFlags, DWORD dwDesiredAccess) {
	HANDLE hRet = _CreateMutexExW(lpMutexAttributes, lpName, dwFlags, dwDesiredAccess);

	if (IsMapleMutex(lpName)) {
		CloseMutex(hRet);
		static bool bAlreadyLoaded = false;
		if (!bAlreadyLoaded) {
			bAlreadyLoaded = true;
			// ÉÅÉÇÉäìWäJå„ÇÃéwíËDLLÇÃì«Ç›çûÇ›
			DelayLoad();
		}
	}

	return hRet;
}