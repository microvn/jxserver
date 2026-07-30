#include "stdafx.h"
#include "Global.h"
#include "KSO3World.h"
#include "KRegressionManager.h"
#include <time.h>

//////////////////////////////////////////////////////////////////////////
// KRegressionManager config loader + helpers. Columns/keys/arithmetic pinned
// from v246 -- docs/regression_port/WORKLOG.md [RE-4/RE-5].
// NOTE: v246 adds a timezone bias (DAT_084df688) to the day offsets; the 2010
// equivalent is unconfirmed, so we add only the +345600 anchor and leave the
// tz term out -- affects daily/reset window alignment only, documented in §5.
//////////////////////////////////////////////////////////////////////////

#define REGRESSION_INI          "settings/ServerConstListByVer.ini"
#define REGRESSION_REWARD_TAB   "settings/RegressionReward.tab"
#define REGRESSION_SECTION      "REGRESSION"
#define REGRESSION_OFFSET_ANCHOR 345600

BOOL KRegressionManager::Init()
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    bRetCode = LoadConstList();
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = LoadRegressionReward();
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = CheckRegressionReward();
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

void KRegressionManager::UnInit()
{
    m_RewardMap.clear();
}

BOOL KRegressionManager::LoadConstList()
{
    BOOL      bResult   = false;
    int       nRetCode  = false;
    IIniFile* piIniFile = NULL;
    int       nWorkFlag = 0;
    int       nDailyCycle = 0, nDailyOffset = 0, nResetCycle = 0, nResetOffset = 0;
    char      szStart[64] = {0};
    char      szEnd[64]   = {0};
    struct tm tmStart, tmEnd;

    piIniFile = g_OpenIniFile(REGRESSION_INI);
    KGLOG_PROCESS_ERROR(piIniFile);

    piIniFile->GetInteger(REGRESSION_SECTION, "RegressionWorkFlag",   0,       &nWorkFlag);
    piIniFile->GetInteger(REGRESSION_SECTION, "RegressionDailyCycle", 86400,   &nDailyCycle);
    piIniFile->GetInteger(REGRESSION_SECTION, "RegressionDailyOffset",25200,   &nDailyOffset);
    piIniFile->GetInteger(REGRESSION_SECTION, "RegressionResetCycle", 2592000, &nResetCycle);
    piIniFile->GetInteger(REGRESSION_SECTION, "RegressionResetOffset",25200,   &nResetOffset);
    KGLOG_PROCESS_ERROR(nDailyCycle > 0 && nResetCycle > 0);

    m_bWorkFlag     = (nWorkFlag != 0);
    m_nDailyCycle   = nDailyCycle;
    m_nResetCycle   = nResetCycle;
    m_nDailyOffset  = nDailyOffset + REGRESSION_OFFSET_ANCHOR;
    m_nResetOffset  = nResetOffset + REGRESSION_OFFSET_ANCHOR;

    // Date arrays "Y,M,D,h,m,s" -> mktime. Default to an always-open window on parse miss.
    m_nStartTime = 0;
    m_nEndTime   = 0;
    piIniFile->GetString(REGRESSION_SECTION, "RegressionStartTime", "", szStart, sizeof(szStart));
    piIniFile->GetString(REGRESSION_SECTION, "RegressionEndTime",   "", szEnd,   sizeof(szEnd));
    memset(&tmStart, 0, sizeof(tmStart));
    memset(&tmEnd,   0, sizeof(tmEnd));
    if (sscanf(szStart, "%d,%d,%d,%d,%d,%d", &tmStart.tm_year, &tmStart.tm_mon, &tmStart.tm_mday,
               &tmStart.tm_hour, &tmStart.tm_min, &tmStart.tm_sec) == 6)
    {
        tmStart.tm_year -= 1900; tmStart.tm_mon -= 1;
        m_nStartTime = mktime(&tmStart);
    }
    if (sscanf(szEnd, "%d,%d,%d,%d,%d,%d", &tmEnd.tm_year, &tmEnd.tm_mon, &tmEnd.tm_mday,
               &tmEnd.tm_hour, &tmEnd.tm_min, &tmEnd.tm_sec) == 6)
    {
        tmEnd.tm_year -= 1900; tmEnd.tm_mon -= 1;
        m_nEndTime = mktime(&tmEnd);
    }

    // outside the activity window -> feature off (v246 final step of LoadConstList)
    if (m_bWorkFlag && m_nStartTime && m_nEndTime)
    {
        time_t nNow = g_pSO3World->m_nCurrentTime;
        if (nNow < m_nStartTime || nNow > m_nEndTime)
            m_bWorkFlag = false;
    }

    bResult = true;
Exit0:
    KG_COM_RELEASE(piIniFile);
    return bResult;
}

BOOL KRegressionManager::LoadRegressionReward()
{
    BOOL      bResult   = false;
    int       nRetCode  = false;
    ITabFile* piTabFile = NULL;
    int       nLine     = 0;
    int       nHeight   = 0;
    char      szCol[32];

    piTabFile = g_OpenTabFile(REGRESSION_REWARD_TAB);
    KGLOG_PROCESS_ERROR(piTabFile);
    nHeight = piTabFile->GetHeight();
    KGLOG_PROCESS_ERROR(nHeight > 0);

    for (nLine = 2; nLine <= nHeight; nLine++)
    {
        int nGradeID = -1, nLossDaily = 0, nRegDaily = 0, nFreeLimit = 0, nKungFu = 0;
        int i = 0;
        KRewardItem Item;

        memset(&Item, 0, sizeof(Item));

        nRetCode = piTabFile->GetInteger(nLine, "GradeID", -1, &nGradeID);        KGLOG_PROCESS_ERROR(nRetCode && nGradeID != -1);
        nRetCode = piTabFile->GetInteger(nLine, "LossDaily", 0, &nLossDaily);     KGLOG_PROCESS_ERROR(nRetCode && nLossDaily >= 0);
        nRetCode = piTabFile->GetInteger(nLine, "RegressionDaily", 0, &nRegDaily);KGLOG_PROCESS_ERROR(nRetCode && nRegDaily > 0);
        nRetCode = piTabFile->GetInteger(nLine, "FreeLimit", 0, &nFreeLimit);     KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "KungFu", 0, &nKungFu);           KGLOG_PROCESS_ERROR(nRetCode);
        // (GradeID==0) XOR (LossDaily==0) is the "new player grade" rule
        KGLOG_PROCESS_ERROR((nGradeID == 0) == (nLossDaily == 0));

        for (i = 0; i < REGRESSION_ITEM_MAX_COUNT; i++)
        {
            snprintf(szCol, sizeof(szCol), "ItemType%d", i + 1);
            piTabFile->GetInteger(nLine, szCol, 0, (int*)&Item.dwItemType[i]);
            snprintf(szCol, sizeof(szCol), "ItemIndex%d", i + 1);
            piTabFile->GetInteger(nLine, szCol, 0, &Item.dwItemIndex[i]);
            snprintf(szCol, sizeof(szCol), "ItemStackNum%d", i + 1);
            piTabFile->GetInteger(nLine, szCol, 0, &Item.nItemStackNum[i]);
        }

        KRewardInfo& Grade = m_RewardMap[nGradeID];
        // same GradeID across kungfu rows must agree on LossDaily
        if (!Grade.ItemMap.empty())
            KGLOG_PROCESS_ERROR((int)Grade.nLossDaily == nLossDaily);
        Grade.nLossDaily = nLossDaily;

        KRewardItemInfo& ItemInfo = Grade.ItemMap[nKungFu];
        ItemInfo.bFreeLimit            = (nFreeLimit != 0);
        ItemInfo.nRegressionDailyCount = nRegDaily;
        ItemInfo.ItemInfoMap[(DWORD)nKungFu] = Item;
    }

    bResult = true;
Exit0:
    if (!bResult)
        KGLogPrintf(KGLOG_DEBUG, "Load RegressionReward.tab failed at line %d.", nLine);
    KG_COM_RELEASE(piTabFile);
    return bResult;
}

BOOL KRegressionManager::CheckRegressionReward()
{
    BOOL   bResult = false;
    time_t nPrevLoss = -1;
    std::map<int, KRewardInfo>::iterator it;

    for (it = m_RewardMap.begin(); it != m_RewardMap.end(); ++it)
    {
        // nLossDaily strictly increasing across grades (grade 0 = new-player, loss 0).
        // NOTE: dropped the v246 "ItemMap.size() <= 8" check -- ItemMap is keyed by
        // kungfu and real data has >8 kungfu per grade; the 8-cap is on daily items
        // (array-bounded), not on the kungfu count. Confirmed against RegressionReward.tab.
        KGLOG_PROCESS_ERROR(it->second.nLossDaily > nPrevLoss || (it->first == 0 && it->second.nLossDaily == 0));
        if (it->first != 0)
            nPrevLoss = it->second.nLossDaily;
    }

    bResult = true;
Exit0:
    return bResult;
}

//------------------------------------------------------------------------
// Helpers (v246 [RE-5])
//------------------------------------------------------------------------
BOOL KRegressionManager::IsCrossDays(time_t t1, time_t t2)
{
    time_t nAlign = t1 - ((t1 - m_nDailyOffset) % m_nDailyCycle);
    return (nAlign + m_nDailyCycle) <= t2;
}

time_t KRegressionManager::GetRegressionFinishedTime(time_t nNow)
{
    return m_nResetCycle + (nNow - ((nNow - m_nResetOffset) % 86400)) + 86400;
}

int KRegressionManager::GetRewardGradeID(long nLossDays)
{
    int nResult = -1;
    std::map<int, KRewardInfo>::iterator it;

    if (nLossDays == 0)
        return 0;
    for (it = m_RewardMap.begin(); it != m_RewardMap.end(); ++it)
    {
        if (it->second.nLossDaily != 0 && it->second.nLossDaily <= nLossDays)
            nResult = it->first;    // ascending scan -> highest match wins
    }
    return nResult;
}

KRewardInfo* KRegressionManager::GetRewardInfo(int nGradeID)
{
    std::map<int, KRewardInfo>::iterator it = m_RewardMap.find(nGradeID);
    return (it != m_RewardMap.end()) ? &(it->second) : NULL;
}

KRewardItemInfo* KRegressionManager::GetRewardItemInfo(int nGradeID, int nKungFu)
{
    KRewardInfo* pInfo = GetRewardInfo(nGradeID);
    if (!pInfo)
        return NULL;
    std::map<int, KRewardItemInfo>::iterator it = pInfo->ItemMap.find(nKungFu);
    return (it != pInfo->ItemMap.end()) ? &(it->second) : NULL;
}
