#ifndef __MULTICLIENT_H__
#define __MULTICLIENT_H__

#define MUTEX_MAPLE L"WvsClientMtx"

bool EnableHook();
// Mutexの処理が最初に呼ばれるのでその辺でロードを開始する
bool DelayLoad();

#endif