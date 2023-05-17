#ifndef __CRCBYPASS_H__
#define __CRCBYPASS_H__

#include"../Share/Simple/Simple.h"
#include"../Share/Hook/SimpleHook.h"


void CRCBypass(Rosemary &r);
DWORD __stdcall GetBackup(DWORD dwAddress);
bool CRCBypass334(Rosemary &r);

#endif
