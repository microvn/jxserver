#ifndef _KGW_SERVER_CONST_LIST_H_
#define _KGW_SERVER_CONST_LIST_H_
#include <ctime>
struct KGWServerConstList
{
    BOOL bFreeThreeDayWorkFlag; int nFreeThreeDayFlagExtIndex;
    int nFreeThreeDayFlagExtBitIndex; int nFreeThreeDayFlagExtBitLength;
    int nFreeThreeDayRechargeExtIndex; int nFreeThreeDayRechargeExtBitIndex;
    int nFreeThreeDayRechargeExtBitLength; time_t nDungeonTradeItemDelayTime;
    int nTimeLimitReturnItemDelayTime; time_t nTimeLimitSoldListDelayTime[6];
    BOOL Init(const char* pszFileName); BOOL UnInit(); BOOL LoadData(IIniFile* pIniFile);
};
#endif
