#include "stdafx.h"
#include "KCampActiveStat.h"
#include "KPlayer.h"
#include "KSO3World.h"

KCampActiveStat::KCampActiveStat()
    : m_bLastWeekActivity(false),
      m_bThisWeekActivity(false),
      m_nPrestige(0),
      m_nBaseScore(0),
      m_nResetTime(0),
      m_pPlayer(NULL)
{
}

BOOL KCampActiveStat::Init(KPlayer* pPlayer)
{
    m_pPlayer = pPlayer;
    m_nResetTime = 0;
    return true;
}

void KCampActiveStat::UnInit()
{
}

BOOL KCampActiveStat::Load(BYTE* pbyData, size_t uDataLen)
{
    KCAMP_ACTIVE_STAT_DB* pData = NULL;

    KG_PROCESS_ERROR(pbyData != NULL);
    KG_PROCESS_ERROR(uDataLen == sizeof(KCAMP_ACTIVE_STAT_DB));

    pData = (KCAMP_ACTIVE_STAT_DB*)pbyData;
    m_nResetTime = pData->nRestTime;

    if (m_nResetTime != 0 && g_pSO3World->m_nCurrentTime < m_nResetTime)
    {
        m_nPrestige = pData->nPrestige;
        m_nBaseScore = pData->nBaseScore;
        m_bLastWeekActivity = pData->byIsActiveBeforeWeek != 0;
        m_bThisWeekActivity = pData->byIsActive != 0;
    }
    else if (m_nResetTime != 0 &&
             g_pSO3World->m_nCurrentTime - m_nResetTime <= 7 * 24 * 60 * 60 - 1)
    {
        m_bLastWeekActivity = pData->byIsActive != 0;
        m_bThisWeekActivity = false;
    }

    const time_t nWeekSeconds = 7 * 24 * 60 * 60;
    const time_t nWeekRemainder =
        (g_pSO3World->m_nCurrentTime - timezone - 0x54600) % nWeekSeconds;
    m_nResetTime = g_pSO3World->m_nCurrentTime - nWeekRemainder + nWeekSeconds;
    return true;
Exit0:
    return false;
}

BOOL KCampActiveStat::Save(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize)
{
    KCAMP_ACTIVE_STAT_DB Data;

    KG_PROCESS_ERROR(puUsedSize != NULL);
    KG_PROCESS_ERROR(pbyBuffer != NULL);
    KG_PROCESS_ERROR(uBufferSize >= sizeof(Data));

    memset(&Data, 0, sizeof(Data));
    Data.nRestTime = m_nResetTime;
    Data.nPrestige = m_nPrestige;
    Data.nBaseScore = m_nBaseScore;
    Data.byIsActiveBeforeWeek = (BYTE)m_bLastWeekActivity;
    Data.byIsActive = (BYTE)m_bThisWeekActivity;
    memcpy(pbyBuffer, &Data, sizeof(Data));
    *puUsedSize = sizeof(Data);
    return true;
Exit0:
    return false;
}
