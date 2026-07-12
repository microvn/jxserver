#include "stdafx.h"
#include "Global.h"
#include "KSO3World.h"
#include "KPlayer.h"
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

BOOL KRegressionPlayerData::Init(KPlayer* pPlayer)
{
    BOOL bResult = false;
    KGLOG_PROCESS_ERROR(pPlayer);
    m_pPlayer                     = pPlayer;
    m_nAccountRegressionGradeID   = 0;
    m_nAccountRegressionVer       = 0;
    m_nPlayerAccountRegressionVer = 0;
    m_nAccountRegressionEndTime   = 0;
    m_nNewPlayerRegressionEndTime = 0;
    m_nCurrentGradeID             = 0;
    m_nRegressionDailyCount       = 0;
    memset(m_byItemMark, 0, sizeof(m_byItemMark));
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
