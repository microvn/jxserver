#include "stdafx.h"
#include "KNewExtPointManager.h"
#include "KPlayer.h"
#include "KRelayClient.h"
#include "Relay_GS_Protocol.h"

KNewExtPointManager::KNewExtPointManager() : m_pPlayer(NULL)
{
}

KNewExtPointManager::~KNewExtPointManager()
{
    UnInit();
}

BOOL KNewExtPointManager::Init(KPlayer* pPlayer)
{
    m_pPlayer = pPlayer;
    return true;
}

void KNewExtPointManager::UnInit()
{
    m_NEPMap.clear();
    m_pPlayer = NULL;
}

BOOL KNewExtPointManager::AddNewExtPoint(int nKey, int nValue, BOOL bLocked)
{
    KNewExtPointInfo info;
    info.nValue = nValue;
    info.bLocked = bLocked;
    return m_NEPMap.insert(std::make_pair(nKey, info)).second;
}

BOOL KNewExtPointManager::CanSetExtPoint(int nIndex)
{
    KNEW_EXT_POINT_MAP::const_iterator it = m_NEPMap.find(nIndex);
    return it == m_NEPMap.end() || !it->second.bLocked;
}

BOOL KNewExtPointManager::GetNewExtPoint(int nIndex, int* pnValue) const
{
    KNEW_EXT_POINT_MAP::const_iterator it;

    KGLOG_PROCESS_ERROR(pnValue);
    it = m_NEPMap.find(nIndex);
    KGLOG_PROCESS_ERROR(it != m_NEPMap.end());
    *pnValue = it->second.nValue;
    return true;

Exit0:
    return false;
}

BOOL KNewExtPointManager::GetNewExtPointByBits(
    int nIndex, int nBitIndex, int nBitLength, int* pnValue
)
const
{
    int nValue = 0;
    unsigned int uMask = 0;

    KGLOG_PROCESS_ERROR(pnValue);
    KGLOG_PROCESS_ERROR(nBitIndex >= 0 && nBitLength > 0);
    KGLOG_PROCESS_ERROR(nBitIndex + nBitLength <= 32);
    KGLOG_PROCESS_ERROR(GetNewExtPoint(nIndex, &nValue));

    uMask = nBitLength == 32 ? 0xffffffffU : ((1U << nBitLength) - 1U);
    *pnValue = (int)(((unsigned int)nValue >> nBitIndex) & uMask);
    return true;

Exit0:
    return false;
}

BOOL KNewExtPointManager::SetNewExtPoint(int nIndex, int nChangeValue)
{
    KNEW_EXT_POINT_MAP::iterator it = m_NEPMap.find(nIndex);
    int nOldValue = 0;
    BOOL bRetCode = false;

    KGLOG_PROCESS_ERROR(it != m_NEPMap.end());
    KGLOG_PROCESS_ERROR(CanSetExtPoint(nIndex));
    nOldValue = it->second.nValue;
    KGLOG_PROCESS_ERROR(nOldValue != nChangeValue);
    it->second.nValue = nChangeValue;
    it->second.bLocked = true;

    KGLOG_PROCESS_ERROR(m_pPlayer);
    bRetCode = g_RelayClient.DoChangeNewExtPointRequest(
        m_pPlayer->m_dwID, nIndex, nChangeValue, nOldValue, 4
    );
    KGLOG_PROCESS_ERROR(bRetCode);
    return true;

Exit0:
    return false;
}

BOOL KNewExtPointManager::SetExtPointByBits(
    int nIndex, int nBitIndex, int nBitLength, int nChangeValue
)
{
    int nValue = 0;
    unsigned int uMask = 0;

    KGLOG_PROCESS_ERROR(nBitIndex >= 0 && nBitLength > 0);
    KGLOG_PROCESS_ERROR(nBitIndex + nBitLength <= 32);
    KGLOG_PROCESS_ERROR(GetNewExtPoint(nIndex, &nValue));
    uMask = nBitLength == 32 ? 0xffffffffU : ((1U << nBitLength) - 1U);
    nValue = (int)(((unsigned int)nValue & ~(uMask << nBitIndex)) |
                   (((unsigned int)nChangeValue & uMask) << nBitIndex));
    return SetNewExtPoint(nIndex, nValue);

Exit0:
    return false;
}

BOOL KNewExtPointManager::GetGSNewExtPoint(
    int nLastKey, KSyncGSNEPInfo* pInfo, int nMaxCount,
    int* pnCount, BOOL* pbSyncFinish
)
const
{
    KNEW_EXT_POINT_MAP::const_iterator it;
    int nCount = 0;

    KGLOG_PROCESS_ERROR(pnCount);
    KGLOG_PROCESS_ERROR(pbSyncFinish);
    KGLOG_PROCESS_ERROR(nMaxCount >= 0);
    KGLOG_PROCESS_ERROR(nMaxCount == 0 || pInfo);

    for (it = m_NEPMap.upper_bound(nLastKey);
         it != m_NEPMap.end() && nCount < nMaxCount; ++it)
    {
        pInfo[nCount].nKey = it->first;
        pInfo[nCount].nValue = it->second.nValue;
        pInfo[nCount].byLocked = (BYTE)it->second.bLocked;
        ++nCount;
    }

    *pnCount = nCount;
    *pbSyncFinish = (it == m_NEPMap.end());
    return true;

Exit0:
    return false;
}

BOOL KNewExtPointManager::OnChangeNewExtPoint(
    int nIndex, int nOldValue, int nChangeValue, int nCurrentValue, int nActionCode
)
{
    KNEW_EXT_POINT_MAP::iterator it = m_NEPMap.find(nIndex);
    BOOL bChangeSuccess = false;
    BOOL bRetCode = false;
    int nTopIndex = 0;

    KGLOG_PROCESS_ERROR(it != m_NEPMap.end());

    it->second.bLocked = false;
    bChangeSuccess = true;
    if (nActionCode != 1)
    {
        it->second.nValue = nChangeValue;
        if (nActionCode == 3)
            it->second.nValue = nCurrentValue;
        bChangeSuccess = false;
    }

    KGLOG_PROCESS_ERROR(bChangeSuccess);
    KGLOG_PROCESS_ERROR(it->second.nValue == nChangeValue);

    KGLOG_PROCESS_ERROR(m_pPlayer);
    KGLOG_PROCESS_ERROR(g_pSO3World->m_ScriptCenter.IsScriptExist(PLAYER_EXT_POINT_SCRIPT));
    KGLOG_PROCESS_ERROR(g_pSO3World->m_ScriptCenter.IsFuncExist(
        PLAYER_EXT_POINT_SCRIPT, "OnSetExtPointSucceed"
    ));

    g_pSO3World->m_ScriptCenter.SafeCallBegin(&nTopIndex);
    g_pSO3World->m_ScriptCenter.PushValueToStack(m_pPlayer);
    g_pSO3World->m_ScriptCenter.PushValueToStack(nOldValue);
    g_pSO3World->m_ScriptCenter.PushValueToStack(nActionCode);
    bRetCode = g_pSO3World->m_ScriptCenter.CallFunction(
        PLAYER_EXT_POINT_SCRIPT, "OnSetExtPointSucceed", 0
    );
    g_pSO3World->m_ScriptCenter.SafeCallEnd(nTopIndex);
    KGLOG_PROCESS_ERROR(bRetCode);
    return true;

Exit0:
    return false;
}
