#ifndef __EMUMAIN_H__
#define __EMUMAIN_H__

#include"../Share/Simple/Simple.h"
#include"../Share/Hook/SimpleHook.h"

enum MSRegion {
	MS_JMS, // default
	MS_TWMS,
	MS_MSEA
};

void EmuMain();

MSRegion GetMSRegion();
int GetMSVersion();
bool GetMSDisableMemoryDump();

#endif