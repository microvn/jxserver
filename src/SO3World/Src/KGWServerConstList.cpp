#include "stdafx.h"
#include "Global.h"
#include "KGWServerConstList.h"
BOOL KGWServerConstList::UnInit() { return true; }
BOOL KGWServerConstList::LoadData(IIniFile* pIniFile)
{
    assert(pIniFile);
    KGLOG_PROCESS_ERROR(pIniFile->GetInteger("ServerConst", "FreeThreeDayWorkFlag", 0, (int*)&bFreeThreeDayWorkFlag));
    KGLOG_PROCESS_ERROR(pIniFile->GetInteger("ServerConst", "FreeThreeDayFlagExtIndex", 5, &nFreeThreeDayFlagExtIndex));
    KGLOG_PROCESS_ERROR(pIniFile->GetInteger("ServerConst", "FreeThreeDayFlagExtBitIndex", 13, &nFreeThreeDayFlagExtBitIndex));
    KGLOG_PROCESS_ERROR(pIniFile->GetInteger("ServerConst", "FreeThreeDayFlagExtBitLength", 1, &nFreeThreeDayFlagExtBitLength));
    KGLOG_PROCESS_ERROR(pIniFile->GetInteger("ServerConst", "FreeThreeDayRechargeExtIndex", 9, &nFreeThreeDayRechargeExtIndex));
    KGLOG_PROCESS_ERROR(pIniFile->GetInteger("ServerConst", "FreeThreeDayRechargeExtBitIndex", 0, &nFreeThreeDayRechargeExtBitIndex));
    KGLOG_PROCESS_ERROR(pIniFile->GetInteger("ServerConst", "FreeThreeDayRechargeExtBitLength", 32, &nFreeThreeDayRechargeExtBitLength));
    KGLOG_PROCESS_ERROR(pIniFile->GetInteger("ServerConst", "DungeonTradeItemDelayTime", 0, (int*)&nDungeonTradeItemDelayTime));
    KGLOG_PROCESS_ERROR(pIniFile->GetInteger("ServerConst", "TimeLimitReturnItemDelayTime", 0, &nTimeLimitReturnItemDelayTime));
    KGLOG_PROCESS_ERROR(pIniFile->GetMultiInteger("ServerConst", "TimeLimitSoldListDelayTime", (int*)nTimeLimitSoldListDelayTime, 6) == 6);
    return true;
Exit0: return false;
}
BOOL KGWServerConstList::Init(const char* pszFileName)
{
    BOOL bResult = false; IIniFile* pIniFile = NULL;
    KGLOG_PROCESS_ERROR(pszFileName); pIniFile = g_OpenIniFile(pszFileName); KGLOG_PROCESS_ERROR(pIniFile);
    bResult = LoadData(pIniFile);
Exit0: if (pIniFile) pIniFile->Release(); return bResult;
}
