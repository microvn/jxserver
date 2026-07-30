#include "stdafx.h"
#include "Global.h"
#include "KSO3World.h"
#include "KPlayer.h"
#include "KPlayerServer.h"
#include "KRegressionPlayerData.h"

//////////////////////////////////////////////////////////////////////////
// KRegressionPlayerData persistence. Byte layout pinned from v246
// Save/LoadAccountData (08209106/08208d8c) + Save/LoadPlayerData
// (08209898/082097de) -- docs/regression_port/WORKLOG.md [RE-3].
//   account 22B: [BYTE grade][DWORD accEnd][BYTE ver][BYTE reserved16]
//   player  46B: [DWORD newEnd][BYTE daily][BYTE mark[8]][BYTE grade][BYTE reserved32]
// Grade/ver/daily are BYTE on disk -> runtime values must stay < 256.
//////////////////////////////////////////////////////////////////////////

#define REG_ACCOUNT_BLOCK   22
#define REG_PLAYER_BLOCK    46

// Target KRegressionPlayerData::LoadAccountData/SaveAccountData use a fixed
// 22-byte account chunk; the 16-byte tail is reserved and must be zeroed.
#define REG_ACCOUNT_DATA_SIZE 22

KRegressionPlayerData::KRegressionPlayerData()
    : m_nAccountRegressionGradeID(0),
      m_nAccountRegressionVer(0),
      m_nPlayerAccountRegressionVer(0),
      m_nAccountRegressionEndTime(0),
      m_nNewPlayerRegressionEndTime(0),
      m_nCurrentGradeID(-1),
      m_nRegressionDailyCount(0),
      m_pPlayer(NULL)
{
    memset(m_byItemMark, 0, sizeof(m_byItemMark));
}

BOOL KRegressionPlayerData::Init(KPlayer* pPlayer)
{
    BOOL bResult = false;
    KGLOG_PROCESS_ERROR(pPlayer);
    m_pPlayer = pPlayer;
    bResult = true;
Exit0:
    return bResult;
}

void KRegressionPlayerData::Calculate(time_t nAccLastSaveTime, time_t nPlayerLastSaveTime)
{
    int nCurrentTime = (int)g_pSO3World->m_nCurrentTime;
    int nGradeID = -1;

    if (!g_pSO3World->m_RegressionManager.IsWork())
        return;

    if (nPlayerLastSaveTime == 0)
    {
        m_nCurrentGradeID = g_pSO3World->m_RegressionManager.GetRewardGradeID(0);
        m_nRegressionDailyCount = 1;
        m_nNewPlayerRegressionEndTime =
            g_pSO3World->m_RegressionManager.GetRegressionFinishedTime(nCurrentTime);
        memset(m_byItemMark, 0, sizeof(m_byItemMark));

        if (nAccLastSaveTime != 0)
        {
            if (m_nAccountRegressionEndTime != 0 && m_nAccountRegressionEndTime >= nCurrentTime)
            {
                m_nAccountRegressionEndTime =
                    g_pSO3World->m_RegressionManager.GetRegressionFinishedTime(nCurrentTime);
            }
            else
            {
                m_nAccountRegressionGradeID = 0;
                m_nAccountRegressionEndTime = 0;
                nGradeID = g_pSO3World->m_RegressionManager.GetRewardGradeID(
                    nCurrentTime - (int)nAccLastSaveTime
                );
                if (nGradeID > 0)
                {
                    m_nCurrentGradeID = nGradeID;
                    m_nAccountRegressionEndTime =
                        g_pSO3World->m_RegressionManager.GetRegressionFinishedTime(nCurrentTime);
                    ++m_nAccountRegressionVer;
                }
            }
        }
    }
    else
    {
        if (m_nNewPlayerRegressionEndTime != 0)
        {
            if (m_nNewPlayerRegressionEndTime >= nCurrentTime)
            {
                m_nCurrentGradeID = g_pSO3World->m_RegressionManager.GetRewardGradeID(0);
                m_nNewPlayerRegressionEndTime =
                    g_pSO3World->m_RegressionManager.GetRegressionFinishedTime(nCurrentTime);
                if (g_pSO3World->m_RegressionManager.IsCrossDays(
                        m_pPlayer->m_nLastLoginTime, m_pPlayer->m_nCurrentLoginTime))
                    ++m_nRegressionDailyCount;
                goto Sync;
            }

            m_nCurrentGradeID = -1;
            m_nRegressionDailyCount = 0;
            m_nNewPlayerRegressionEndTime = 0;
            memset(m_byItemMark, 0, sizeof(m_byItemMark));
        }

        if (m_nAccountRegressionEndTime != 0)
        {
            if (m_nAccountRegressionEndTime >= nCurrentTime)
            {
                if (m_nAccountRegressionVer >= m_nPlayerAccountRegressionVer)
                {
                    m_nCurrentGradeID = m_nAccountRegressionGradeID;
                    m_nAccountRegressionEndTime =
                        g_pSO3World->m_RegressionManager.GetRegressionFinishedTime(nCurrentTime);
                    if (g_pSO3World->m_RegressionManager.IsCrossDays(
                            m_pPlayer->m_nLastLoginTime, m_pPlayer->m_nCurrentLoginTime))
                        ++m_nRegressionDailyCount;
                }
                else
                {
                    m_nCurrentGradeID = m_nAccountRegressionGradeID;
                    m_nRegressionDailyCount = 1;
                    m_nPlayerAccountRegressionVer = m_nAccountRegressionVer;
                    m_nAccountRegressionEndTime =
                        g_pSO3World->m_RegressionManager.GetRegressionFinishedTime(nCurrentTime);
                    memset(m_byItemMark, 0, sizeof(m_byItemMark));
                }
            }
            else
            {
                m_nCurrentGradeID = -1;
                m_nRegressionDailyCount = 0;
                m_nAccountRegressionGradeID = 0;
                m_nAccountRegressionEndTime = 0;
                memset(m_byItemMark, 0, sizeof(m_byItemMark));
            }
        }

        if (nAccLastSaveTime == 0)
            nAccLastSaveTime = nPlayerLastSaveTime;

        nGradeID = g_pSO3World->m_RegressionManager.GetRewardGradeID(
            nCurrentTime - (int)nAccLastSaveTime
        );
        if (nGradeID > 0)
        {
            m_nCurrentGradeID = nGradeID;
            m_nRegressionDailyCount = 1;
            m_nAccountRegressionGradeID = nGradeID;
            m_nAccountRegressionEndTime =
                g_pSO3World->m_RegressionManager.GetRegressionFinishedTime(nCurrentTime);
            ++m_nAccountRegressionVer;
            m_nPlayerAccountRegressionVer = m_nAccountRegressionVer;
            memset(m_byItemMark, 0, sizeof(m_byItemMark));
        }
    }

Sync:
    if (m_nCurrentGradeID < 0)
        return;
    g_PlayerServer.DoSyncRegressionPlayerData(
        m_pPlayer->m_nConnIndex, m_nCurrentGradeID,
        m_nRegressionDailyCount, m_byItemMark
    );
}

BOOL KRegressionPlayerData::CallAddRewardItemScript(
    DWORD dwItemType, DWORD dwItemIndex, int nItemStackNum
)
{
    BOOL bResult = false;
    int nTopIndex = 0;
    BOOL bUsed = false;
    const char* pszScript = "scripts/player/PlayerScript.lua";
    const char* pszFunction = "AddRegressionRewardItem";

    g_pSO3World->m_ScriptCenter.SafeCallBegin(&nTopIndex);
    KGLOG_PROCESS_ERROR(g_pSO3World->m_ScriptCenter.IsScriptExist(pszScript));
    KGLOG_PROCESS_ERROR(g_pSO3World->m_ScriptCenter.IsFuncExist(pszScript, pszFunction));
    g_pSO3World->m_ScriptCenter.PushValueToStack(m_pPlayer);
    g_pSO3World->m_ScriptCenter.PushValueToStack(dwItemType);
    g_pSO3World->m_ScriptCenter.PushValueToStack(dwItemIndex);
    g_pSO3World->m_ScriptCenter.PushValueToStack(nItemStackNum);
    KGLOG_PROCESS_ERROR(g_pSO3World->m_ScriptCenter.CallFunction(pszScript, pszFunction, 1));
    KGLOG_PROCESS_ERROR(g_pSO3World->m_ScriptCenter.GetValuesFromStack("b", &bUsed));
    bResult = bUsed;
Exit0:
    g_pSO3World->m_ScriptCenter.SafeCallEnd(nTopIndex);
    return bResult;
}

BOOL KRegressionPlayerData::AddRewardItem(int nDailyIndex, int nItemIndex, DWORD dwKungFuID)
{
    BOOL bResult = false;
    KRewardItemInfo* pItemInfo = NULL;
    KRewardItem* pRewardItem = NULL;
    std::map<DWORD, KRewardItem>::iterator itItem;
    DWORD dwItemType = 0;
    DWORD dwItemIndex = 0;
    int nItemStackNum = 0;

    KGLOG_PROCESS_ERROR(g_pSO3World->m_RegressionManager.IsWork());
    KGLOG_PROCESS_ERROR(nDailyIndex >= 0 && nDailyIndex < REGRESSION_DAILY_MAX_COUNT);
    KGLOG_PROCESS_ERROR(nItemIndex >= 0 && nItemIndex < REGRESSION_ITEM_MAX_COUNT);
    KGLOG_PROCESS_ERROR(!(m_byItemMark[nDailyIndex] & (1 << nItemIndex)));

    pItemInfo = g_pSO3World->m_RegressionManager.GetRewardItemInfo(
        m_nCurrentGradeID, (int)dwKungFuID
    );
    KGLOG_PROCESS_ERROR(pItemInfo);
    KGLOG_PROCESS_ERROR(pItemInfo->nRegressionDailyCount > m_nRegressionDailyCount);
    KGLOG_PROCESS_ERROR(!(pItemInfo->bFreeLimit && m_pPlayer->m_bFreeLimitFlag));

    itItem = pItemInfo->ItemInfoMap.find(dwKungFuID);
    KGLOG_PROCESS_ERROR(itItem != pItemInfo->ItemInfoMap.end());
    pRewardItem = &itItem->second;
    dwItemType = pRewardItem->dwItemType[nItemIndex];
    dwItemIndex = (DWORD)pRewardItem->dwItemIndex[nItemIndex];
    nItemStackNum = pRewardItem->nItemStackNum[nItemIndex];
    KGLOG_PROCESS_ERROR(dwItemType && dwItemIndex && nItemStackNum);
    KGLOG_PROCESS_ERROR(CallAddRewardItemScript(dwItemType, dwItemIndex, nItemStackNum));

    m_byItemMark[nDailyIndex] |= (BYTE)(1 << nItemIndex);
    g_PlayerServer.DoSyncRegressionPlayerData(
        m_pPlayer->m_nConnIndex, m_nCurrentGradeID,
        m_nRegressionDailyCount, m_byItemMark
    );
    bResult = true;
Exit0:
    return bResult;
}

void KRegressionPlayerData::UnInit()
{
    m_pPlayer = NULL;
}

BYTE KRegressionPlayerData::GetItemMark(int nDailyIndex)
{
    if (nDailyIndex < 0 || nDailyIndex >= REGRESSION_ITEM_MARK_COUNT)
        return 0;
    return m_byItemMark[nDailyIndex];
}

BOOL KRegressionPlayerData::Save(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize)
{
    BOOL  bResult   = false;
    BYTE* pbyOffset = pbyBuffer;
    int   nDaily    = 0;

    KGLOG_PROCESS_ERROR(puUsedSize);
    KGLOG_PROCESS_ERROR(uBufferSize >= REG_ACCOUNT_BLOCK + REG_PLAYER_BLOCK);

    // ---- account block (22B) ----
    memset(pbyOffset, 0, REG_ACCOUNT_BLOCK);
    pbyOffset[0] = (BYTE)m_nAccountRegressionGradeID;
    *(DWORD*)(pbyOffset + 1) = (DWORD)m_nAccountRegressionEndTime;
    pbyOffset[5] = (BYTE)m_nAccountRegressionVer;
    pbyOffset += REG_ACCOUNT_BLOCK;

    // ---- player block (46B) ----
    memset(pbyOffset, 0, REG_PLAYER_BLOCK);
    *(DWORD*)(pbyOffset + 0) = (DWORD)m_nNewPlayerRegressionEndTime;
    nDaily = m_nRegressionDailyCount;
    if (nDaily > 0xFF) nDaily = 0xFF;
    pbyOffset[4] = (BYTE)nDaily;
    memcpy(pbyOffset + 5, m_byItemMark, REGRESSION_ITEM_MARK_COUNT);
    pbyOffset[13] = (BYTE)m_nCurrentGradeID;
    pbyOffset += REG_PLAYER_BLOCK;

    *puUsedSize = (size_t)(pbyOffset - pbyBuffer);
    bResult = true;
Exit0:
    return bResult;
}

BOOL KRegressionPlayerData::Load(BYTE* pbyData, size_t uDataLen)
{
    BOOL  bResult   = false;
    BYTE* pbyOffset = pbyData;

    KGLOG_PROCESS_ERROR(pbyData);
    KGLOG_PROCESS_ERROR(uDataLen >= REG_ACCOUNT_BLOCK + REG_PLAYER_BLOCK);

    // ---- account block ----
    m_nAccountRegressionGradeID = pbyOffset[0];
    m_nAccountRegressionEndTime = (time_t)*(DWORD*)(pbyOffset + 1);
    m_nAccountRegressionVer     = pbyOffset[5];
    pbyOffset += REG_ACCOUNT_BLOCK;

    // ---- player block ----
    m_nNewPlayerRegressionEndTime = (time_t)*(DWORD*)(pbyOffset + 0);
    m_nRegressionDailyCount       = pbyOffset[4];
    memcpy(m_byItemMark, pbyOffset + 5, REGRESSION_ITEM_MARK_COUNT);
    m_nCurrentGradeID             = pbyOffset[13];
    pbyOffset += REG_PLAYER_BLOCK;

    m_nPlayerAccountRegressionVer = 0;   // transient -- reset each session
    bResult = true;
Exit0:
    return bResult;
}

BOOL KRegressionPlayerData::LoadPlayerData(BYTE* pbyData, size_t uDataLen)
{
    BOOL  bResult   = false;
    BYTE* pbyOffset = pbyData;

    KGLOG_PROCESS_ERROR(pbyData);
    KGLOG_PROCESS_ERROR(uDataLen >= REG_PLAYER_BLOCK);

    m_nNewPlayerRegressionEndTime = (time_t)*(DWORD*)(pbyOffset + 0);
    m_nRegressionDailyCount       = pbyOffset[4];
    memcpy(m_byItemMark, pbyOffset + 5, REGRESSION_ITEM_MARK_COUNT);
    m_nCurrentGradeID             = pbyOffset[13];
    m_nPlayerAccountRegressionVer = 0;

    bResult = true;
Exit0:
    return bResult;
}

BOOL KRegressionPlayerData::LoadAccountData(BYTE* pbyData, size_t uDataLen)
{
    BOOL bResult = false;

    KGLOG_PROCESS_ERROR(pbyData);
    KGLOG_PROCESS_ERROR(uDataLen >= REG_ACCOUNT_DATA_SIZE);

    m_nAccountRegressionGradeID = pbyData[0];
    m_nAccountRegressionEndTime = (time_t)*(DWORD*)(pbyData + 1);
    m_nAccountRegressionVer     = pbyData[5];

    bResult = true;
Exit0:
    return bResult;
}

BOOL KRegressionPlayerData::SaveAccountData(
    size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize
)
{
    BOOL bResult = false;

    KGLOG_PROCESS_ERROR(puUsedSize);
    KGLOG_PROCESS_ERROR(pbyBuffer);
    KGLOG_PROCESS_ERROR(uBufferSize >= REG_ACCOUNT_DATA_SIZE);

    pbyBuffer[0] = (BYTE)m_nAccountRegressionGradeID;
    *(DWORD*)(pbyBuffer + 1) = (DWORD)m_nAccountRegressionEndTime;
    pbyBuffer[5] = (BYTE)m_nAccountRegressionVer;
    memset(pbyBuffer + 6, 0, REG_ACCOUNT_DATA_SIZE - 6);
    *puUsedSize = REG_ACCOUNT_DATA_SIZE;

    bResult = true;
Exit0:
    return bResult;
}
