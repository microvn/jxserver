#ifndef _KCAMP_ACTIVE_STAT_H_
#define _KCAMP_ACTIVE_STAT_H_

#include <windows.h>
#include "SO3ProtocolBasic.h"
#include "KRoleDBDataDef.h"

class KPlayer;

class KCampActiveStat
{
public:
    KCampActiveStat();
    BOOL Init(KPlayer* pPlayer);
    void UnInit();
    BOOL Load(BYTE* pbyData, size_t uDataLen);
    BOOL Save(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize);

private:
    BOOL m_bLastWeekActivity;
    BOOL m_bThisWeekActivity;
    int m_nPrestige;
    int m_nBaseScore;
    time_t m_nResetTime;
    KPlayer* m_pPlayer;
};

typedef char KCAMP_ACTIVE_STAT_LAYOUT[(sizeof(KCampActiveStat) == 0x18) ? 1 : -1];

#endif
