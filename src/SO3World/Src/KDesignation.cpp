#include "stdafx.h"
#include "KDesignation.h"
#include "KPlayerServer.h"
#include "KPlayerClient.h"
#include "KRelayClient.h"
#include "KPlayer.h"
#include "KDesignationList.h"
#include "KBuff.h"           // KBUFF_RECIPE_KEY (buff-on-equip)

#pragma pack(1)
struct KDesignationDB
{
    BYTE    byCurrentPrefix;   // ǰ׺
    BYTE    byCurrentPostfix;  // ��׺
    BYTE    byGenerationIndex; // generation
    int     nBynameIndex;      // byname
    BYTE    byBynameDisplay;   // renamed from 2010 byDisplayFlag (byte layout identical)
};
#pragma pack()

// Drift locks (gnu++98: negative-array trick, no static_assert).
// Header MUST stay 8 bytes (byte-identical to 2010). time_t MUST be 4 bytes on the
// wire (32-bit build) or the end-time block byte layout diverges from v246.
typedef char _CHK_DB_SIZE[(sizeof(KDesignationDB) == 8) ? 1 : -1];
typedef char _CHK_TIME_T[(sizeof(time_t) == 4) ? 1 : -1];

KDesignation::KDesignation()
{
}

KDesignation::~KDesignation()
{
}

BOOL KDesignation::Init(KPlayer* pPlayer)
{
	assert(pPlayer);

	m_pPlayer = pPlayer;

    m_nCurrentPrefix    = 0;
    m_nCurrentPostfix   = 0;
    m_nGenerationIndex  = 0;
    m_nBynameIndex      = 0;
    m_bBynameDisplay    = false;

    // v2.5 drift (RE-6 Init 081cd99e): announce-fix on, no independent prefix, empty maps.
    m_bAllowBroadcastAnnounceFix = true;
    m_bCurrentIndependent        = false;
    m_PrefixEndTimeTable.clear();
    m_PostfixEndTimeTable.clear();

	return true;
}

void KDesignation::UnInit()
{
    m_pPlayer = NULL;
    return;
}

BOOL KDesignation::Save(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize)
{
    BOOL            bResult             = false;
    BOOL            bRetCode            = false;
    KDesignationDB* pDBHeader           = NULL;
    BYTE*           pbyOffset           = pbyBuffer;
    size_t          uLeftSize           = uBufferSize;
    size_t          uDataLen            = 0;

    assert(puUsedSize);
    assert(pbyBuffer);

    KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(KDesignationDB));
    pDBHeader = (KDesignationDB*)pbyOffset;

    pDBHeader->byCurrentPrefix      = (BYTE)m_nCurrentPrefix;
    pDBHeader->byCurrentPostfix     = (BYTE)m_nCurrentPostfix;
    pDBHeader->byGenerationIndex    = (BYTE)m_nGenerationIndex;
    pDBHeader->nBynameIndex         = m_nBynameIndex;
    pDBHeader->byBynameDisplay        = (BYTE)m_bBynameDisplay;

    pbyOffset += sizeof(KDesignationDB);
    uLeftSize -= sizeof(KDesignationDB);

    bRetCode = m_AcquiredPrefix.Save(&uDataLen, pbyOffset, uLeftSize);
    KGLOG_PROCESS_ERROR(bRetCode);
    pbyOffset += uDataLen;
    uLeftSize -= uDataLen;

    bRetCode = m_AcquiredPostfix.Save(&uDataLen, pbyOffset, uLeftSize);
    KGLOG_PROCESS_ERROR(bRetCode);
    pbyOffset += uDataLen;
    uLeftSize -= uDataLen;

    // v2.5: emit the optional end-time block ONLY when a timed designation exists.
    // No timed designations => byte-identical to the old 72-byte 2010 blob.
    if (!m_PrefixEndTimeTable.empty() || !m_PostfixEndTimeTable.empty())
    {
        bRetCode = SaveEndTimeInfo(&uDataLen, pbyOffset, uLeftSize);
        KGLOG_PROCESS_ERROR(bRetCode);
        pbyOffset += uDataLen;
        uLeftSize -= uDataLen;
    }

    *puUsedSize = uBufferSize - uLeftSize;

    bResult = true;
Exit0:
    return bResult;
}

BOOL KDesignation::Load(BYTE* pbyData, size_t uDataLen)
{
    BOOL            bResult             = false;
    BOOL            bRetCode            = false;
    KDesignationDB* pDBHeader           = NULL;
    BYTE*           pbyOffset           = pbyData;
    size_t          uLeftSize           = uDataLen;

    KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(KDesignationDB));
    pDBHeader = (KDesignationDB*)pbyOffset;

    m_nCurrentPrefix    = pDBHeader->byCurrentPrefix;
    m_nCurrentPostfix   = pDBHeader->byCurrentPostfix;
    m_nGenerationIndex  = pDBHeader->byGenerationIndex;
    m_nBynameIndex      = pDBHeader->nBynameIndex;
    m_bBynameDisplay      = pDBHeader->byBynameDisplay;

#ifdef _SERVER
    // v2.5 LoadCurrentDesignation (081cddea) routes the header through SetCurrentPrefix to
    // rebuild m_bCurrentIndependent + re-add the equip buff. We keep the direct field-set
    // above (byte-verified, non-fatal) and reconstruct ONLY the independent flag here:
    // a since-removed prefix leaves the flag false instead of bricking character load, and
    // the equip-buff is owned by KBuffList's own persistence (not re-added here to avoid a
    // double-apply). Observationally identical for this leak's dormant buff/nType data.
    if (m_nCurrentPrefix != 0)
    {
        KPrefixInfo* pPrefixInfo = g_pSO3World->m_Settings.m_DesignationList.GetPrefixInfo(m_nCurrentPrefix);
        if (pPrefixInfo && pPrefixInfo->nType != 0)
            m_bCurrentIndependent = true;
    }
#endif

    pbyOffset += sizeof(KDesignationDB);
    uLeftSize -= sizeof(KDesignationDB);

    KGLOG_PROCESS_ERROR(uLeftSize >= DESIGNATIONFIX_DATA_SIZE);
    bRetCode = m_AcquiredPrefix.Load(pbyOffset, DESIGNATIONFIX_DATA_SIZE);
    KGLOG_PROCESS_ERROR(bRetCode);
    pbyOffset += DESIGNATIONFIX_DATA_SIZE;
    uLeftSize -= DESIGNATIONFIX_DATA_SIZE;

    // v2.5: relaxed from exact-tail (== ) to >= so an optional end-time block may follow.
    KGLOG_PROCESS_ERROR(uLeftSize >= DESIGNATIONFIX_DATA_SIZE);
    bRetCode = m_AcquiredPostfix.Load(pbyOffset, DESIGNATIONFIX_DATA_SIZE);
    KGLOG_PROCESS_ERROR(bRetCode);
    pbyOffset += DESIGNATIONFIX_DATA_SIZE;
    uLeftSize -= DESIGNATIONFIX_DATA_SIZE;

    // Old 72-byte 2010 blob => uLeftSize==0 here, end-time skipped (backward compatible).
    if (uLeftSize != 0)
    {
        size_t uEndTimeUsed = 0;
        bRetCode = LoadEndTimeInfo(&uEndTimeUsed, pbyOffset, uLeftSize);
        KGLOG_PROCESS_ERROR(bRetCode);
        pbyOffset += uEndTimeUsed;
        uLeftSize -= uEndTimeUsed;
        KGLOG_PROCESS_ERROR(uLeftSize == 0);
    }

    bResult = true;
Exit0:
    return bResult;
}

// v2.5 (SaveEndTimeInfo 081ce64e): serialize the optional timed-designation block.
// Format: [BYTE nPre]{BYTE id; time_t(4B)} x nPre [BYTE nPost]{BYTE id; time_t(4B)} x nPost
BOOL KDesignation::SaveEndTimeInfo(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize)
{
    BOOL   bResult   = false;
    BYTE*  pbyOffset = pbyBuffer;
    size_t uLeftSize = uBufferSize;
    std::map<int, time_t>::iterator it;

    assert(puUsedSize);
    assert(pbyBuffer);
    KGLOG_PROCESS_ERROR(uBufferSize != 0);

    *pbyOffset = (BYTE)m_PrefixEndTimeTable.size();
    pbyOffset += sizeof(BYTE);
    uLeftSize -= sizeof(BYTE);

    for (it = m_PrefixEndTimeTable.begin(); it != m_PrefixEndTimeTable.end(); ++it)
    {
        KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(BYTE) + sizeof(time_t));
        *pbyOffset = (BYTE)it->first;
        pbyOffset += sizeof(BYTE);
        uLeftSize -= sizeof(BYTE);
        memcpy(pbyOffset, &it->second, sizeof(time_t));
        pbyOffset += sizeof(time_t);
        uLeftSize -= sizeof(time_t);
    }

    KGLOG_PROCESS_ERROR(uLeftSize != 0);
    *pbyOffset = (BYTE)m_PostfixEndTimeTable.size();
    pbyOffset += sizeof(BYTE);
    uLeftSize -= sizeof(BYTE);

    for (it = m_PostfixEndTimeTable.begin(); it != m_PostfixEndTimeTable.end(); ++it)
    {
        KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(BYTE) + sizeof(time_t));
        *pbyOffset = (BYTE)it->first;
        pbyOffset += sizeof(BYTE);
        uLeftSize -= sizeof(BYTE);
        memcpy(pbyOffset, &it->second, sizeof(time_t));
        pbyOffset += sizeof(time_t);
        uLeftSize -= sizeof(time_t);
    }

    *puUsedSize = uBufferSize - uLeftSize;

    bResult = true;
Exit0:
    return bResult;
}

// v2.5 (LoadEndTimeInfo 081ce386): deserialize the optional timed-designation block.
BOOL KDesignation::LoadEndTimeInfo(size_t* puUsedSize, BYTE* pbyData, size_t uDataLen)
{
    BOOL   bResult        = false;
    BYTE*  pbyOffset      = pbyData;
    size_t uLeftSize      = uDataLen;
    int    nPrefixCount   = 0;
    int    nPostfixCount  = 0;
    int    i              = 0;

    assert(puUsedSize);
    assert(pbyData);
    KGLOG_PROCESS_ERROR(uDataLen != 0);

    nPrefixCount = (int)*pbyOffset;
    pbyOffset += sizeof(BYTE);
    uLeftSize -= sizeof(BYTE);

    assert(m_PrefixEndTimeTable.size() == 0);
    for (i = 0; i < nPrefixCount; i++)
    {
        int    nID      = 0;
        time_t nEndTime = 0;

        KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(BYTE) + sizeof(time_t));
        nID = (int)*pbyOffset;
        pbyOffset += sizeof(BYTE);
        uLeftSize -= sizeof(BYTE);
        memcpy(&nEndTime, pbyOffset, sizeof(time_t));
        pbyOffset += sizeof(time_t);
        uLeftSize -= sizeof(time_t);
        m_PrefixEndTimeTable[nID] = nEndTime;
    }

    KGLOG_PROCESS_ERROR(uLeftSize != 0);
    nPostfixCount = (int)*pbyOffset;
    pbyOffset += sizeof(BYTE);
    uLeftSize -= sizeof(BYTE);

    assert(m_PostfixEndTimeTable.size() == 0);
    for (i = 0; i < nPostfixCount; i++)
    {
        int    nID      = 0;
        time_t nEndTime = 0;

        KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(BYTE) + sizeof(time_t));
        nID = (int)*pbyOffset;
        pbyOffset += sizeof(BYTE);
        uLeftSize -= sizeof(BYTE);
        memcpy(&nEndTime, pbyOffset, sizeof(time_t));
        pbyOffset += sizeof(time_t);
        uLeftSize -= sizeof(time_t);
        m_PostfixEndTimeTable[nID] = nEndTime;
    }

    *puUsedSize = uDataLen - uLeftSize;

    bResult = true;
Exit0:
    return bResult;
}

// �ı���ҵ�ǰ�õĳƺż���ʾ
BOOL KDesignation::SetCurrentDesignation(int nPrefix, int nPostfix, BOOL bDisplayFlag)
{
    BOOL bResult            = false;
    BOOL bRetCode           = false;
    int  nGenerationIndex   = 0;

    if (nPrefix != 0)
    {
        bRetCode = IsPrefixAcquired(nPrefix);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    if (nPostfix != 0)
    {
        bRetCode = IsPostfixAcquired(nPostfix);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    m_nCurrentPrefix   = nPrefix;
    m_nCurrentPostfix  = nPostfix;
    m_bBynameDisplay     = bDisplayFlag;

#ifdef _SERVER
    g_PlayerServer.DoSyncPlayerDesignation(
        m_pPlayer, m_nCurrentPrefix, m_nCurrentPostfix, 
        m_nGenerationIndex, m_nBynameIndex, m_bBynameDisplay
    );
#endif

    bResult = true;
Exit0:
    return bResult;
}

// v2.5 (AcquirePrefix 081cec1c): grant a prefix, optionally with an explicit expiry.
//   nEndTime==0 : permanent (or config-timed if nOwnDuration!=0). Skips if already owned.
//   nEndTime!=0 : explicit expiry, capped to now+nOwnDuration, deduped.
BOOL KDesignation::AcquirePrefix(int nPrefix, time_t nEndTime)
{
    BOOL            bResult     = false;
    BOOL            bRetCode    = false;
    KPrefixInfo*    pPrefixInfo = NULL;

    KGLOG_PROCESS_ERROR(nPrefix > 0 && nPrefix <= MAX_DESIGNATION_FIX_ID);

#ifdef _SERVER
    pPrefixInfo = g_pSO3World->m_Settings.m_DesignationList.GetPrefixInfo(nPrefix);
    KGLOG_PROCESS_ERROR(pPrefixInfo);

    if (nEndTime == 0)
    {
        // permanent grant: already owned -> nothing to do (avoids a duplicate announce).
        if (IsPrefixAcquired(nPrefix))
        {
            bResult = true;
            goto Exit0;
        }
        // config-driven auto-expiry.
        if (pPrefixInfo->nOwnDuration != 0)
        {
            nEndTime = g_pSO3World->m_nCurrentTime + pPrefixInfo->nOwnDuration;
            m_PrefixEndTimeTable[nPrefix] = nEndTime;
        }
    }
    else
    {
        // explicit-expiry grant: only valid for a duration-limited prefix.
        KGLOG_PROCESS_ERROR(pPrefixInfo->nOwnDuration != 0);
        KGLOG_PROCESS_ERROR(nEndTime > g_pSO3World->m_nCurrentTime);

        {
            time_t nCap = g_pSO3World->m_nCurrentTime + pPrefixInfo->nOwnDuration;
            if (nEndTime > nCap)
                nEndTime = nCap;
        }

        // dedup: identical expiry already recorded -> nothing to do.
        {
            std::map<int, time_t>::iterator it = m_PrefixEndTimeTable.find(nPrefix);
            if (it != m_PrefixEndTimeTable.end() && it->second == nEndTime)
            {
                bResult = true;
                goto Exit0;
            }
        }
        m_PrefixEndTimeTable[nPrefix] = nEndTime;
    }
#endif

    bRetCode = m_AcquiredPrefix.SetBit(nPrefix, true);
    KGLOG_PROCESS_ERROR(bRetCode);

#ifdef _SERVER
    g_PlayerServer.DoAcquireDesignation(m_pPlayer, nPrefix, 0);

    if (m_bAllowBroadcastAnnounceFix)
    {
        BroadcastDesignationAnnounce(nPrefix, 0, pPrefixInfo->nAnnounceType);
    }

    g_pSO3World->m_StatDataServer.UpdateDesignationStat(true, nPrefix);
#endif

    bResult = true;
Exit0:
    return bResult;
}

// v2.5 (AcquirePostfix 081cea26): grant a postfix. Already owned -> no-op. Auto-expiry
// from nOwnDuration. No explicit-expiry path (postfix is always config-timed or permanent).
BOOL KDesignation::AcquirePostfix(int nPostfix)
{
    BOOL            bResult      = false;
    BOOL            bRetCode     = false;
    KPostfixInfo*   pPostfixInfo = NULL;

    KGLOG_PROCESS_ERROR(nPostfix > 0 && nPostfix <= MAX_DESIGNATION_FIX_ID);

#ifdef _SERVER
    // already owned -> skip the whole grant (avoids a duplicate announce).
    if (IsPostfixAcquired(nPostfix))
    {
        bResult = true;
        goto Exit0;
    }

    pPostfixInfo = g_pSO3World->m_Settings.m_DesignationList.GetPostfixInfo(nPostfix);
    KGLOG_PROCESS_ERROR(pPostfixInfo);

    if (pPostfixInfo->nOwnDuration != 0)
    {
        m_PostfixEndTimeTable[nPostfix] = g_pSO3World->m_nCurrentTime + pPostfixInfo->nOwnDuration;
    }
#endif

    bRetCode = m_AcquiredPostfix.SetBit(nPostfix, true);
    KGLOG_PROCESS_ERROR(bRetCode);

#ifdef _SERVER
    g_PlayerServer.DoAcquireDesignation(m_pPlayer, 0, nPostfix);

    if (m_bAllowBroadcastAnnounceFix)
    {
        BroadcastDesignationAnnounce(0, nPostfix, pPostfixInfo->nAnnounceType);
    }

    g_pSO3World->m_StatDataServer.UpdateDesignationStat(false, nPostfix);
#endif

    bResult = true;
Exit0:
    return bResult;
}

// v2.5 (RemovePrefix 081cf75c): if the removed prefix is equipped, force-unequip it by
// clearing its cooldown first (bypasses UnEquip's CD-gate), then drop the acquired bit.
BOOL KDesignation::RemovePrefix(int nPrefix)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;

    KGLOG_PROCESS_ERROR(nPrefix > 0 && nPrefix <= MAX_DESIGNATION_FIX_ID);

    if (IsPrefixAcquired(nPrefix))
    {
#ifdef _SERVER
        if (m_nCurrentPrefix == nPrefix)
        {
            KPrefixInfo* pPrefixInfo = g_pSO3World->m_Settings.m_DesignationList.GetPrefixInfo(nPrefix);
            KGLOG_PROCESS_ERROR(pPrefixInfo);

            m_pPlayer->m_TimerList.ClearTimer(pPrefixInfo->dwCoolDownID);

            bRetCode = UnEquipPrefix();
            KGLOG_PROCESS_ERROR(bRetCode);
        }
#endif

        bRetCode = m_AcquiredPrefix.SetBit(nPrefix, false);
        KGLOG_PROCESS_ERROR(bRetCode);

#ifdef _SERVER
        g_PlayerServer.DoRemoveDesignation(m_pPlayer, nPrefix, 0);
#endif
    }

    bResult = true;
Exit0:
    return bResult;
}

// v2.5 (RemovePostfix 081cf3f6): symmetric.
BOOL KDesignation::RemovePostfix(int nPostfix)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;

    KGLOG_PROCESS_ERROR(nPostfix > 0 && nPostfix <= MAX_DESIGNATION_FIX_ID);

    if (IsPostfixAcquired(nPostfix))
    {
#ifdef _SERVER
        if (m_nCurrentPostfix == nPostfix)
        {
            KPostfixInfo* pPostfixInfo = g_pSO3World->m_Settings.m_DesignationList.GetPostfixInfo(nPostfix);
            KGLOG_PROCESS_ERROR(pPostfixInfo);

            m_pPlayer->m_TimerList.ClearTimer(pPostfixInfo->dwCoolDownID);

            bRetCode = UnEquipPostfix();
            KGLOG_PROCESS_ERROR(bRetCode);
        }
#endif

        bRetCode = m_AcquiredPostfix.SetBit(nPostfix, false);
        KGLOG_PROCESS_ERROR(bRetCode);

#ifdef _SERVER
        g_PlayerServer.DoRemoveDesignation(m_pPlayer, 0, nPostfix);
#endif
    }

    bResult = true;
Exit0:
    return bResult;
}

// �ж�����Ƿ���ĳ���ƺ�
BOOL KDesignation::IsPrefixAcquired(int nPrefix)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;

    KGLOG_PROCESS_ERROR(nPrefix > 0 && nPrefix <= MAX_DESIGNATION_FIX_ID);

    bRetCode = m_AcquiredPrefix.GetBit(nPrefix, &bResult);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return bResult;
}

BOOL KDesignation::IsPostfixAcquired(int nPostfix)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;

    KGLOG_PROCESS_ERROR(nPostfix > 0 && nPostfix <= MAX_DESIGNATION_FIX_ID);

    bRetCode = m_AcquiredPostfix.GetBit(nPostfix, &bResult);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return bResult;
}

#ifdef _SERVER
// v2.5 (SetCurrentPrefix 081cdc04): equip a prefix in memory + add its buff + cache nType.
// nPrefix==0 clears. Buff/nType columns are blank in this leak -> those branches dormant.
BOOL KDesignation::SetCurrentPrefix(int nPrefix)
{
    BOOL            bResult     = false;
    BOOL            bRetCode    = false;
    KPrefixInfo*    pPrefixInfo = NULL;

    KGLOG_PROCESS_ERROR(nPrefix >= 0 && nPrefix <= MAX_DESIGNATION_FIX_ID);

    if (nPrefix != 0)
    {
        assert(m_pPlayer);

        pPrefixInfo = g_pSO3World->m_Settings.m_DesignationList.GetPrefixInfo(nPrefix);
        KGLOG_PROCESS_ERROR(pPrefixInfo);

        if (pPrefixInfo->dwBuffID != 0)
        {
            KBUFF_RECIPE_KEY BuffRecipeKey;
            memset(&BuffRecipeKey, 0, sizeof(BuffRecipeKey));
            BuffRecipeKey.dwID   = pPrefixInfo->dwBuffID;
            BuffRecipeKey.nLevel = pPrefixInfo->nBuffLevel;

            bRetCode = m_pPlayer->m_BuffList.CallBuff(
                m_pPlayer->m_dwID, m_pPlayer->m_nLevel, BuffRecipeKey, 0, 0);
            KGLOG_PROCESS_ERROR(bRetCode);
        }

        if (pPrefixInfo->nType != 0)
        {
            m_bCurrentIndependent = true;
        }

        m_nCurrentPrefix = nPrefix;
    }

    bResult = true;
Exit0:
    return bResult;
}

// v2.5 (SetCurrentPostfix 081cda30): symmetric, no nType branch.
BOOL KDesignation::SetCurrentPostfix(int nPostfix)
{
    BOOL            bResult      = false;
    BOOL            bRetCode     = false;
    KPostfixInfo*   pPostfixInfo = NULL;

    KGLOG_PROCESS_ERROR(nPostfix >= 0 && nPostfix <= MAX_DESIGNATION_FIX_ID);

    if (nPostfix != 0)
    {
        assert(m_pPlayer);

        pPostfixInfo = g_pSO3World->m_Settings.m_DesignationList.GetPostfixInfo(nPostfix);
        KGLOG_PROCESS_ERROR(pPostfixInfo);

        if (pPostfixInfo->dwBuffID != 0)
        {
            KBUFF_RECIPE_KEY BuffRecipeKey;
            memset(&BuffRecipeKey, 0, sizeof(BuffRecipeKey));
            BuffRecipeKey.dwID   = pPostfixInfo->dwBuffID;
            BuffRecipeKey.nLevel = pPostfixInfo->nBuffLevel;

            bRetCode = m_pPlayer->m_BuffList.CallBuff(
                m_pPlayer->m_dwID, m_pPlayer->m_nLevel, BuffRecipeKey, 0, 0);
            KGLOG_PROCESS_ERROR(bRetCode);
        }

        m_nCurrentPostfix = nPostfix;
    }

    bResult = true;
Exit0:
    return bResult;
}

// v2.5 (CanEquipPrefix 081cdee2): gate = nothing equipped + owned + (independent => no postfix/byname).
BOOL KDesignation::CanEquipPrefix(int nPrefix)
{
    BOOL            bResult     = false;
    BOOL            bRetCode    = false;
    KPrefixInfo*    pPrefixInfo = NULL;

    KGLOG_PROCESS_ERROR(nPrefix > 0 && nPrefix <= MAX_DESIGNATION_FIX_ID);
    KGLOG_PROCESS_ERROR(m_nCurrentPrefix == 0);

    bRetCode = IsPrefixAcquired(nPrefix);
    KGLOG_PROCESS_ERROR(bRetCode);

    pPrefixInfo = g_pSO3World->m_Settings.m_DesignationList.GetPrefixInfo(nPrefix);
    KGLOG_PROCESS_ERROR(pPrefixInfo);

    if (pPrefixInfo->nType != 0)
    {
        // independent prefix: cannot coexist with a postfix or a displayed byname.
        KGLOG_PROCESS_ERROR(m_nCurrentPostfix == 0);
        KGLOG_PROCESS_ERROR(m_bBynameDisplay == false);
    }

    bResult = true;
Exit0:
    return bResult;
}

// v2.5 (CanEquipPostfix 081cd878): gate = nothing equipped + owned + no independent prefix worn.
BOOL KDesignation::CanEquipPostfix(int nPostfix)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    KGLOG_PROCESS_ERROR(nPostfix > 0 && nPostfix <= MAX_DESIGNATION_FIX_ID);
    KGLOG_PROCESS_ERROR(m_nCurrentPostfix == 0);

    bRetCode = IsPostfixAcquired(nPostfix);
    KGLOG_PROCESS_ERROR(bRetCode);

    KGLOG_PROCESS_ERROR(!m_bCurrentIndependent);

    bResult = true;
Exit0:
    return bResult;
}

// v2.5 (ResetPrefixCDTime 081ce17a): arm the equipped prefix's cooldown from its recipe.
BOOL KDesignation::ResetPrefixCDTime()
{
    BOOL            bResult     = false;
    KPrefixInfo*    pPrefixInfo = NULL;

    if (m_nCurrentPrefix != 0)
    {
        pPrefixInfo = g_pSO3World->m_Settings.m_DesignationList.GetPrefixInfo(m_nCurrentPrefix);
        KGLOG_PROCESS_ERROR(pPrefixInfo);

        if (pPrefixInfo->dwCoolDownID != 0)
        {
            int nInterval = g_pSO3World->m_Settings.m_CoolDownList.GetCoolDownValue(pPrefixInfo->dwCoolDownID);
            m_pPlayer->m_TimerList.ResetTimer(pPrefixInfo->dwCoolDownID, nInterval);
        }
    }

    bResult = true;
Exit0:
    return bResult;
}

// v2.5 (ResetPostfixCDTime 081ce0a4): symmetric.
BOOL KDesignation::ResetPostfixCDTime()
{
    BOOL             bResult      = false;
    KPostfixInfo*    pPostfixInfo = NULL;

    if (m_nCurrentPostfix != 0)
    {
        pPostfixInfo = g_pSO3World->m_Settings.m_DesignationList.GetPostfixInfo(m_nCurrentPostfix);
        KGLOG_PROCESS_ERROR(pPostfixInfo);

        if (pPostfixInfo->dwCoolDownID != 0)
        {
            int nInterval = g_pSO3World->m_Settings.m_CoolDownList.GetCoolDownValue(pPostfixInfo->dwCoolDownID);
            m_pPlayer->m_TimerList.ResetTimer(pPostfixInfo->dwCoolDownID, nInterval);
        }
    }

    bResult = true;
Exit0:
    return bResult;
}

// v2.5 (EquipPrefix 081cf132): CanEquip -> SetCurrent -> arm CD -> sync to client.
BOOL KDesignation::EquipPrefix(int nPrefix)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    KGLOG_PROCESS_ERROR(nPrefix > 0 && nPrefix <= MAX_DESIGNATION_FIX_ID);

    bRetCode = CanEquipPrefix(nPrefix);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = SetCurrentPrefix(nPrefix);
    KGLOG_PROCESS_ERROR(bRetCode);

    ResetPrefixCDTime();

    g_PlayerServer.DoSyncPlayerDesignation(
        m_pPlayer, m_nCurrentPrefix, m_nCurrentPostfix,
        m_nGenerationIndex, m_nBynameIndex, m_bBynameDisplay);

    bResult = true;
Exit0:
    return bResult;
}

// v2.5 (EquipPostfix 081cf002): symmetric.
BOOL KDesignation::EquipPostfix(int nPostfix)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    KGLOG_PROCESS_ERROR(nPostfix > 0 && nPostfix <= MAX_DESIGNATION_FIX_ID);

    bRetCode = CanEquipPostfix(nPostfix);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = SetCurrentPostfix(nPostfix);
    KGLOG_PROCESS_ERROR(bRetCode);

    ResetPostfixCDTime();

    g_PlayerServer.DoSyncPlayerDesignation(
        m_pPlayer, m_nCurrentPrefix, m_nCurrentPostfix,
        m_nGenerationIndex, m_nBynameIndex, m_bBynameDisplay);

    bResult = true;
Exit0:
    return bResult;
}

// v2.5 (UnEquipPrefix 081cf5be): CD-gate -> remove buff -> clear -> sync.
BOOL KDesignation::UnEquipPrefix()
{
    BOOL            bResult     = false;
    BOOL            bRetCode    = false;
    KPrefixInfo*    pPrefixInfo = NULL;

    if (m_nCurrentPrefix != 0)
    {
        pPrefixInfo = g_pSO3World->m_Settings.m_DesignationList.GetPrefixInfo(m_nCurrentPrefix);
        KGLOG_PROCESS_ERROR(pPrefixInfo);

        // cooldown gate: must be ready (CheckTimer TRUE) to take it off.
        if (pPrefixInfo->dwCoolDownID != 0)
        {
            bRetCode = m_pPlayer->m_TimerList.CheckTimer(pPrefixInfo->dwCoolDownID);
            if (!bRetCode)
            {
                g_PlayerServer.DoMessageNotify(m_pPlayer->m_nConnIndex, ectDesignationNotifyCode, 3);
                goto Exit0;
            }
        }

        if (pPrefixInfo->dwBuffID != 0)
        {
            m_pPlayer->m_BuffList.DelSingleBuff(pPrefixInfo->dwBuffID, pPrefixInfo->nBuffLevel);
        }

        if (pPrefixInfo->nType != 0)
        {
            m_bCurrentIndependent = false;
        }

        m_nCurrentPrefix = 0;

        g_PlayerServer.DoSyncPlayerDesignation(
            m_pPlayer, m_nCurrentPrefix, m_nCurrentPostfix,
            m_nGenerationIndex, m_nBynameIndex, m_bBynameDisplay);
    }

    bResult = true;
Exit0:
    return bResult;
}

// v2.5 (UnEquipPostfix 081cf262): symmetric, no nType.
BOOL KDesignation::UnEquipPostfix()
{
    BOOL             bResult      = false;
    BOOL             bRetCode     = false;
    KPostfixInfo*    pPostfixInfo = NULL;

    if (m_nCurrentPostfix != 0)
    {
        pPostfixInfo = g_pSO3World->m_Settings.m_DesignationList.GetPostfixInfo(m_nCurrentPostfix);
        KGLOG_PROCESS_ERROR(pPostfixInfo);

        if (pPostfixInfo->dwCoolDownID != 0)
        {
            bRetCode = m_pPlayer->m_TimerList.CheckTimer(pPostfixInfo->dwCoolDownID);
            if (!bRetCode)
            {
                g_PlayerServer.DoMessageNotify(m_pPlayer->m_nConnIndex, ectDesignationNotifyCode, 3);
                goto Exit0;
            }
        }

        if (pPostfixInfo->dwBuffID != 0)
        {
            m_pPlayer->m_BuffList.DelSingleBuff(pPostfixInfo->dwBuffID, pPostfixInfo->nBuffLevel);
        }

        m_nCurrentPostfix = 0;

        g_PlayerServer.DoSyncPlayerDesignation(
            m_pPlayer, m_nCurrentPrefix, m_nCurrentPostfix,
            m_nGenerationIndex, m_nBynameIndex, m_bBynameDisplay);
    }

    bResult = true;
Exit0:
    return bResult;
}

// v2.5 (SetBynameDisplayFlag 081cef60): toggle byname display; blocked while an independent prefix is worn.
BOOL KDesignation::SetBynameDisplayFlag(int nFlag)
{
    BOOL bResult = false;

    KGLOG_PROCESS_ERROR(!m_bCurrentIndependent);

    if (m_bBynameDisplay != nFlag)
    {
        m_bBynameDisplay = nFlag;

        g_PlayerServer.DoSyncPlayerDesignation(
            m_pPlayer, m_nCurrentPrefix, m_nCurrentPostfix,
            m_nGenerationIndex, m_nBynameIndex, m_bBynameDisplay);
    }

    bResult = true;
Exit0:
    return bResult;
}

// v2.5 (GetPrefixEndTime 081cd7c2): owned -> *pnEndTime = expiry (0 if permanent).
BOOL KDesignation::GetPrefixEndTime(int nPrefix, int* pnEndTime)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    std::map<int, time_t>::iterator it;

    assert(pnEndTime);

    bRetCode = IsPrefixAcquired(nPrefix);
    KGLOG_PROCESS_ERROR(bRetCode);

    *pnEndTime = 0;
    it = m_PrefixEndTimeTable.find(nPrefix);
    if (it != m_PrefixEndTimeTable.end())
    {
        *pnEndTime = (int)it->second;
    }

    bResult = true;
Exit0:
    return bResult;
}

// v2.5 (GetPostfixEndTime 081cd664): symmetric.
BOOL KDesignation::GetPostfixEndTime(int nPostfix, int* pnEndTime)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    std::map<int, time_t>::iterator it;

    assert(pnEndTime);

    bRetCode = IsPostfixAcquired(nPostfix);
    KGLOG_PROCESS_ERROR(bRetCode);

    *pnEndTime = 0;
    it = m_PostfixEndTimeTable.find(nPostfix);
    if (it != m_PostfixEndTimeTable.end())
    {
        *pnEndTime = (int)it->second;
    }

    bResult = true;
Exit0:
    return bResult;
}

// v2.5 (Activate 081cf920): prune expired timed designations. Called per player tick.
// RemovePrefix/Postfix does NOT touch the end-time map, so we erase the entry here.
BOOL KDesignation::Activate()
{
    std::map<int, time_t>::iterator it;

    it = m_PrefixEndTimeTable.begin();
    while (it != m_PrefixEndTimeTable.end())
    {
        if (it->second < g_pSO3World->m_nCurrentTime)
        {
            RemovePrefix(it->first);
            m_PrefixEndTimeTable.erase(it++);   // erase old node, advance to next (pre-C++11 idiom)
        }
        else
        {
            ++it;
        }
    }

    it = m_PostfixEndTimeTable.begin();
    while (it != m_PostfixEndTimeTable.end())
    {
        if (it->second < g_pSO3World->m_nCurrentTime)
        {
            RemovePostfix(it->first);
            m_PostfixEndTimeTable.erase(it++);
        }
        else
        {
            ++it;
        }
    }

    return true;
}
#endif // _SERVER

// ����������ɱ���(�������ݺź��������������)
#ifdef _SERVER
BOOL KDesignation::SetGeneration(int nGenerationIndex)
{
    BOOL bResult = false;

    m_nGenerationIndex  = nGenerationIndex;
    m_nBynameIndex = g_Random(g_pSO3World->m_Settings.m_ConstList.nMaxDesignationBynameIndex) + 1;

    g_PlayerServer.DoSetGenerationNotify(m_pPlayer, nGenerationIndex, m_nBynameIndex);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KDesignation::BroadcastDesignationAnnounce(int nPrefix, int nPostfix, BYTE byType)
{
    BOOL bResult = false;

    assert(m_pPlayer);

    if (byType == datGlobal)
    {
        g_RelayClient.DoDesignationGlobalAnnounceRequest(m_pPlayer->m_dwID, nPrefix, nPostfix);
    }
    else
    {
        g_PlayerServer.BroadcastDesignationAnnounce(m_pPlayer->m_dwID, m_pPlayer->m_szName, nPrefix, nPostfix, byType);
    }

    bResult = true;
Exit0:
    return bResult;
}
#endif // _SERVER
