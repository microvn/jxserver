#include "stdafx.h"
#include "Global.h"
#include "SO3GlobalDef.h"
#include "KSO3World.h"
#include "KPlayer.h"
#include "KPlayerServer.h"
#include "KExterior.h"
#include "KExteriorBox.h"

//////////////////////////////////////////////////////////////////////////
// [PORT-3] slot -> constant mappings. Values pinned verbatim from the v246
// switch tables (FUN_081ffc50/FUN_081ffb24/FUN_081ffbba). The style/color
// indices are indices into KPlayer::m_wRepresentId[] and equal the
// represent-part enum members shown in the comments.
//
// Compile-time guard (C++98 negative-array): the pinned index constants MUST
// stay identical to this build's PLAYER_EQUIP_REPRESENT enum ordinals, else
// exterior would apply onto the wrong appearance slot. Verified equal to v246.
//////////////////////////////////////////////////////////////////////////
typedef char _KEXT_REP_MAP_CHECK[
    (perHelmStyle == 2 && perHelmColor == 3 &&
     perChestStyle == 5 && perChestColor == 6 &&
     perWaistStyle == 8 && perWaistColor == 9 &&
     perBangleStyle == 11 && perBangleColor == 12 &&
     perBootsStyle == 14 && perBootsColor == 15) ? 1 : -1];

KExteriorBox::KExteriorBox()
{
    m_pPlayer = NULL;
    m_uCurrentSetID = 0;
    m_uExteriorFreeCount = 0;
}

KExteriorBox::~KExteriorBox()
{
}

BOOL KExteriorBox::Init(KPlayer* pPlayer)
{
    BOOL bResult = false;

    KGLOG_PROCESS_ERROR(pPlayer);

    m_pPlayer = pPlayer;
    m_ExteriorMap.clear();
    m_ExteriorSet.clear();
    m_LatestBuy.clear();
    m_uCurrentSetID = 0;
    m_uExteriorFreeCount = 0;

    bResult = true;
Exit0:
    return bResult;
}

void KExteriorBox::UnInit()
{
    m_ExteriorMap.clear();
    m_ExteriorSet.clear();
    m_LatestBuy.clear();
    m_uCurrentSetID = 0;
    m_uExteriorFreeCount = 0;
    m_pPlayer = NULL;
}

//------------------------------------------------------------------------
// [PORT-4] _Add / Add  (FUN_082016a6 / FUN_08201ee6)
//------------------------------------------------------------------------
KEXTERIOR_ITEM* KExteriorBox::_Add(DWORD dwID, int nTimeType, time_t nExpireTime)
{
    KEXTERIOR_ITEM*             pResult = NULL;
    KEXTERIOR_ITEM_MAP::iterator it     = m_ExteriorMap.find(dwID);

    if (it != m_ExteriorMap.end())
    {
        // Already owned. Only a timed entry (nFlag != 0) is mutated; a
        // permanent entry (nFlag == 0) is already "forever" -> untouched.
        KEXTERIOR_ITEM* pItem = &(it->second);
        if (pItem->nFlag != 0)
        {
            if (nTimeType == ettPermanent)
            {
                pItem->nExpireTime = 0;
                pItem->nFlag       = 0;
            }
            else if (nTimeType == ett7Days)
            {
                pItem->nExpireTime += EXTERIOR_7DAYS_SECONDS;
            }
            else if (nTimeType == ettLimit)
            {
                if (pItem->nExpireTime < nExpireTime)
                    pItem->nExpireTime = nExpireTime;
            }
        }
        pResult = pItem;
    }
    else
    {
        // New entry. The raw (dwID, nTimeType-as-flag, nExpireTime) is stored;
        // this is the path LoadExteriorBox uses (flag/expire come from DB).
        KEXTERIOR_ITEM Item;

        KGLOG_PROCESS_ERROR(m_ExteriorMap.size() <= MAX_EXTERIOR_ITEM_COUNT);

        Item.dwID        = dwID;
        Item.nFlag       = (DWORD)nTimeType;
        Item.nExpireTime = nExpireTime;
        m_ExteriorMap[dwID] = Item;
        pResult = &(m_ExteriorMap[dwID]);
    }

Exit0:
    return pResult;
}

BOOL KExteriorBox::Add(DWORD dwID, int nTimeType, time_t nExpireTime)
{
    BOOL            bResult = false;
    KEXTERIOR_INFO* pInfo   = NULL;
    KEXTERIOR_ITEM* pItem   = NULL;

    if (dwID == 0)
        return true;    // v246: id==0 is a no-op success

    pInfo = g_pSO3World->m_Settings.m_Exterior.GetExteriorInfo(dwID);
    KGLOG_PROCESS_ERROR(pInfo);

    pItem = _Add(dwID, nTimeType, nExpireTime);
    KGLOG_PROCESS_ERROR(pItem);

    g_PlayerServer.DoSyncExteriorBoxData(m_pPlayer, 1, pItem);   // owner-only

    bResult = true;
Exit0:
    return bResult;
}

KEXTERIOR_ITEM* KExteriorBox::GetExteriorItem(DWORD dwID)
{
    KEXTERIOR_ITEM_MAP::iterator it = m_ExteriorMap.find(dwID);
    return (it != m_ExteriorMap.end()) ? &(it->second) : NULL;
}

//------------------------------------------------------------------------
// [PORT-5] equip-set + apply  (FUN_081ff948 / FUN_081ffaa0 / FUN_08200a4e /
// FUN_082009a2 / FUN_0838844a)
//------------------------------------------------------------------------
KEXTERIOR_SET_INFO* KExteriorBox::GetExteriorSet(size_t uSetIdx)
{
    if (uSetIdx >= m_ExteriorSet.size())
        return NULL;
    return &m_ExteriorSet[uSetIdx];
}

BOOL KExteriorBox::IsHaveExteriorRepresent(int nSlot)
{
    KEXTERIOR_SET_INFO* pSet = NULL;

    if (nSlot < 0 || nSlot >= MAX_EXTERIOR_SLOT)
        return false;

    pSet = GetExteriorSet(m_uCurrentSetID);
    if (pSet == NULL)
        return false;

    return pSet->dwExteriorID[nSlot] != 0;
}

// == v246 FUN_082009a2, identical to KItemList::SetRepresentID.
void KExteriorBox::SetRepresentID(int nRepresentIndex, int nRepresentID)
{
    KG_PROCESS_ERROR(nRepresentIndex >= perFaceStyle && nRepresentIndex < perRepresentCount);
    KG_PROCESS_ERROR(!(m_pPlayer->m_dwRepresentIdLock & (0x1 << nRepresentIndex)));

    if (nRepresentIndex == perHelmStyle)
    {
        KG_PROCESS_ERROR(!m_pPlayer->m_bHideHat);
    }

#ifdef _SERVER
    if (m_pPlayer->m_wRepresentId[nRepresentIndex] != (WORD)nRepresentID)
    {
        g_PlayerServer.DoSyncEquipRepresent(m_pPlayer, nRepresentIndex, nRepresentID);
    }
#endif

    m_pPlayer->m_wRepresentId[nRepresentIndex] = (WORD)nRepresentID;

Exit0:
    return;
}

BOOL KExteriorBox::ApplyExteriorRepresent(int nSlot)
{
    BOOL                bResult        = false;
    BOOL                bRetCode       = false;
    KEXTERIOR_SET_INFO* pSet           = NULL;
    KEXTERIOR_INFO*     pInfo          = NULL;
    KEXTERIOR_ITEM*     pItem          = NULL;
    DWORD               dwID           = 0;
    int                 nRepresentIdx  = 0;
    int                 nColorIdx      = 0;
    int                 nSubType       = 0;
    int                 nRepresentID   = 0;   // 0 = clear (expired)
    int                 nColorID       = 0;

    KGLOG_PROCESS_ERROR(nSlot >= 0 && nSlot < MAX_EXTERIOR_SLOT);

    pSet = GetExteriorSet(m_uCurrentSetID);
    KGLOG_PROCESS_ERROR(pSet);

    dwID = pSet->dwExteriorID[nSlot];
    if (dwID == 0)
        goto Exit0;             // empty slot -> nothing to apply (v246 returns FALSE)

    bRetCode = GetEquipRepresentIndex(nSlot, &nRepresentIdx);
    KGLOG_PROCESS_ERROR(bRetCode);
    bRetCode = GetEquipColorIndex(nSlot, &nColorIdx);
    KGLOG_PROCESS_ERROR(bRetCode);
    bRetCode = GetEquipSubType(nSlot, &nSubType);
    KGLOG_PROCESS_ERROR(bRetCode);

    pInfo = g_pSO3World->m_Settings.m_Exterior.GetExteriorInfo(dwID);
    KGLOG_PROCESS_ERROR(pInfo);

    pItem = GetExteriorItem(dwID);
    KGLOG_PROCESS_ERROR(pItem);

    KGLOG_PROCESS_ERROR((int)pInfo->nSubType == nSubType);

    // Use the exterior appearance only when it is still valid: permanent
    // (nFlag == 0) or not yet expired. Otherwise leave represent/color at 0.
    if (pItem->nFlag == 0 || g_pSO3World->m_nCurrentTime < pItem->nExpireTime)
    {
        nRepresentID = (int)pInfo->nRepresentID;
        nColorID     = (int)pInfo->nColorID;
    }

    if ((m_pPlayer->m_dwApplyExteriorFlag >> nSlot) & 0x1)
    {
        SetRepresentID(nRepresentIdx, nRepresentID);
        SetRepresentID(nColorIdx, nColorID);
    }

    bResult = true;
Exit0:
    return bResult;
}

// == per-slot apply wrapper FUN_0838844a.
void KExteriorBox::ApplyOneSlot(int nSlot)
{
    if (nSlot < 0 || nSlot >= MAX_EXTERIOR_SLOT)
        return;
    if (!IsHaveExteriorRepresent(nSlot))
        return;

    m_pPlayer->m_dwApplyExteriorFlag |= (0x1 << nSlot);
    ApplyExteriorRepresent(nSlot);
}

// == LuaApplyExterior core (FUN_083157b8), minus the client flag-sync packet
// (protocol 0x121) which is added with the packet slice (#7).
void KExteriorBox::ApplyAllExterior()
{
    int nSlot = 0;

    m_pPlayer->m_dwApplyExteriorFlag |= 0x80;   // master apply-on bit

    for (nSlot = 0; nSlot < MAX_EXTERIOR_SLOT; nSlot++)
    {
        if (IsHaveExteriorRepresent(nSlot))
            ApplyOneSlot(nSlot);
    }

    g_PlayerServer.DoSyncApplyExteriorFlag(m_pPlayer, m_pPlayer->m_dwApplyExteriorFlag);
}

// == per-slot un-apply FUN_08388252. Clear the slot bit and restore the real
// equipped item's appearance at that slot (or 0 if nothing equipped), reusing
// the same represent path as KItemList (GetEquipPos -> equipped item).
void KExteriorBox::UnApplyOneSlot(int nSlot)
{
    int    nRepresentIdx = 0;
    int    nColorIdx     = 0;
    int    nRepresentID  = 0;   // default: empty slot -> clear
    int    nColorID      = 0;
    DWORD  dwEquipPos    = 0;
    KItem* pItem         = NULL;

    if (nSlot < 0 || nSlot >= MAX_EXTERIOR_SLOT)
        return;

    m_pPlayer->m_dwApplyExteriorFlag &= ~(0x1 << nSlot);

    if (!GetEquipRepresentIndex(nSlot, &nRepresentIdx)) return;
    if (!GetEquipColorIndex(nSlot, &nColorIdx))         return;

    // restore the appearance of the item actually equipped in this slot's pos
    if (m_pPlayer->m_ItemList.GetEquipPos(nRepresentIdx, &dwEquipPos))
    {
        pItem = m_pPlayer->m_ItemList.GetItem(ibEquip, dwEquipPos);
        if (pItem)
        {
            nRepresentID = pItem->m_nRepresentID;
            nColorID     = pItem->m_nColorID;
        }
    }

    SetRepresentID(nRepresentIdx, nRepresentID);
    SetRepresentID(nColorIdx, nColorID);
}

// == LuaUnApplyExterior core (FUN_083156e0).
void KExteriorBox::UnApplyAllExterior()
{
    int nSlot = 0;

    m_pPlayer->m_dwApplyExteriorFlag &= ~0x80;   // clear master apply-on bit

    for (nSlot = 0; nSlot < MAX_EXTERIOR_SLOT; nSlot++)
        UnApplyOneSlot(nSlot);

    g_PlayerServer.DoSyncApplyExteriorFlag(m_pPlayer, m_pPlayer->m_dwApplyExteriorFlag);
}

// == AddExteriorSet FUN_0820165c: append one empty set. No max (script-driven).
BOOL KExteriorBox::AddExteriorSet()
{
    KEXTERIOR_SET_INFO Set;
    memset(&Set, 0, sizeof(Set));
    m_ExteriorSet.push_back(Set);
    return true;
}

BOOL KExteriorBox::SetExterior(size_t uSetIdx, int nSlot, DWORD dwID)
{
    BOOL                bResult  = false;
    BOOL                bRetCode = false;
    KEXTERIOR_SET_INFO* pSet     = NULL;

    KGLOG_PROCESS_ERROR(nSlot >= 0 && nSlot < MAX_EXTERIOR_SLOT);

    pSet = GetExteriorSet(uSetIdx);
    KGLOG_PROCESS_ERROR(pSet);

    if (pSet->dwExteriorID[nSlot] == dwID)
    {
        bResult = true;
        goto Exit0;
    }

    if (dwID != 0)
    {
        // must be a real exterior of the right subtype AND owned
        KEXTERIOR_INFO* pInfo    = g_pSO3World->m_Settings.m_Exterior.GetExteriorInfo(dwID);
        int             nSubType = 0;

        KGLOG_PROCESS_ERROR(pInfo);
        bRetCode = GetEquipSubType(nSlot, &nSubType);
        KGLOG_PROCESS_ERROR(bRetCode);
        KGLOG_PROCESS_ERROR((int)pInfo->nSubType == nSubType);
        KGLOG_PROCESS_ERROR(GetExteriorItem(dwID));
    }

    pSet->dwExteriorID[nSlot] = dwID;

    // If editing the current set while exterior display is on, re-drive the slot.
    if ((DWORD)uSetIdx == m_uCurrentSetID && (m_pPlayer->m_dwApplyExteriorFlag & 0x80))
    {
        if (dwID == 0)
            UnApplyOneSlot(nSlot);
        else
            ApplyOneSlot(nSlot);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KExteriorBox::SetCurrentSetID(size_t uSetIdx)
{
    BOOL bResult = false;
    int  nSlot   = 0;

    KGLOG_PROCESS_ERROR(uSetIdx < m_ExteriorSet.size());

    m_uCurrentSetID = (DWORD)uSetIdx;

    // Re-drive all 5 slots to the new set when exterior display is on.
    if (m_pPlayer->m_dwApplyExteriorFlag & 0x80)
    {
        for (nSlot = 0; nSlot < MAX_EXTERIOR_SLOT; nSlot++)
        {
            if (IsHaveExteriorRepresent(nSlot))
                ApplyOneSlot(nSlot);
            else
                UnApplyOneSlot(nSlot);
        }
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KExteriorBox::GetEquipRepresentIndex(int nSlot, int* pnIndex)
{
    BOOL bResult = false;

    KGLOG_PROCESS_ERROR(pnIndex);

    switch (nSlot)
    {
    case 0: *pnIndex = 5;  break;   // perChestStyle
    case 1: *pnIndex = 2;  break;   // perHelmStyle
    case 2: *pnIndex = 8;  break;   // perWaistStyle
    case 3: *pnIndex = 14; break;   // perBootsStyle
    case 4: *pnIndex = 11; break;   // perBangleStyle
    default: goto Exit0;
    }
    bResult = true;
Exit0:
    return bResult;
}

BOOL KExteriorBox::GetEquipColorIndex(int nSlot, int* pnIndex)
{
    BOOL bResult = false;

    KGLOG_PROCESS_ERROR(pnIndex);

    switch (nSlot)
    {
    case 0: *pnIndex = 6;  break;   // perChestColor
    case 1: *pnIndex = 3;  break;   // perHelmColor
    case 2: *pnIndex = 9;  break;   // perWaistColor
    case 3: *pnIndex = 15; break;   // perBootsColor
    case 4: *pnIndex = 12; break;   // perBangleColor
    default: goto Exit0;
    }
    bResult = true;
Exit0:
    return bResult;
}

BOOL KExteriorBox::GetEquipSubType(int nSlot, int* pnSubType)
{
    BOOL bResult = false;

    KGLOG_PROCESS_ERROR(pnSubType);

    switch (nSlot)
    {
    case 0: *pnSubType = 2;  break;
    case 1: *pnSubType = 3;  break;
    case 2: *pnSubType = 6;  break;
    case 3: *pnSubType = 9;  break;
    case 4: *pnSubType = 10; break;
    default: goto Exit0;
    }
    bResult = true;
Exit0:
    return bResult;
}

//------------------------------------------------------------------------
// [PORT-6] Persistence. One role-block, three length/count-prefixed segments:
//   owned box  : [WORD count][count x KEXTERIOR_DB_DATA::KEXTERIOR_INFO (16B)]
//   equip sets : [WORD count][count x KEXTERIOR_SET_DB_DATA::KEXTERIOR_SET_INFO (18B)]
//                [BYTE currentSetID][WORD freeCounter]
//   latest buy : [WORD count][count x KEXTERIOR_BUY_LATEST_DB_DATA::KEXTERIOR_INFO (8B)]
// Item byte-layouts are pinned from v246 (FUN_08201fee / FUN_08200ec4 / FUN_08200d72).
//------------------------------------------------------------------------
BOOL KExteriorBox::Save(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize)
{
    BOOL    bResult   = false;
    BYTE*   pbyOffset = pbyBuffer;
    BYTE*   pbyTail   = pbyBuffer + uBufferSize;
    KEXTERIOR_ITEM_MAP::iterator itBox;
    size_t  i = 0;

    KGLOG_PROCESS_ERROR(puUsedSize);

    // ---- owned box ----
    KGLOG_PROCESS_ERROR((size_t)(pbyTail - pbyOffset) >= sizeof(WORD));
    *(WORD*)pbyOffset = (WORD)m_ExteriorMap.size();
    pbyOffset += sizeof(WORD);
    for (itBox = m_ExteriorMap.begin(); itBox != m_ExteriorMap.end(); ++itBox)
    {
        KEXTERIOR_DB_DATA::KEXTERIOR_INFO Item;
        KGLOG_PROCESS_ERROR((size_t)(pbyTail - pbyOffset) >= sizeof(Item));
        memset(&Item, 0, sizeof(Item));
        Item.wID          = (WORD)itBox->second.dwID;
        Item.byFlag       = (BYTE)itBox->second.nFlag;
        Item.dwExpireTime = (DWORD)itBox->second.nExpireTime;
        memcpy(pbyOffset, &Item, sizeof(Item));
        pbyOffset += sizeof(Item);
    }

    // ---- equip sets ----
    KGLOG_PROCESS_ERROR((size_t)(pbyTail - pbyOffset) >= sizeof(WORD));
    *(WORD*)pbyOffset = (WORD)m_ExteriorSet.size();
    pbyOffset += sizeof(WORD);
    for (i = 0; i < m_ExteriorSet.size(); i++)
    {
        KEXTERIOR_SET_DB_DATA::KEXTERIOR_SET_INFO Item;
        int nSlot = 0;
        KGLOG_PROCESS_ERROR((size_t)(pbyTail - pbyOffset) >= sizeof(Item));
        memset(&Item, 0, sizeof(Item));
        for (nSlot = 0; nSlot < MAX_EXTERIOR_SLOT; nSlot++)
            Item.wExteriorID[nSlot] = (WORD)m_ExteriorSet[i].dwExteriorID[nSlot];
        memcpy(pbyOffset, &Item, sizeof(Item));
        pbyOffset += sizeof(Item);
    }
    KGLOG_PROCESS_ERROR((size_t)(pbyTail - pbyOffset) >= sizeof(BYTE) + sizeof(WORD));
    *(BYTE*)pbyOffset = (BYTE)m_uCurrentSetID;
    pbyOffset += sizeof(BYTE);
    *(WORD*)pbyOffset = (WORD)m_uExteriorFreeCount;
    pbyOffset += sizeof(WORD);

    // ---- latest buy ----
    KGLOG_PROCESS_ERROR((size_t)(pbyTail - pbyOffset) >= sizeof(WORD));
    *(WORD*)pbyOffset = (WORD)m_LatestBuy.size();
    pbyOffset += sizeof(WORD);
    for (i = 0; i < m_LatestBuy.size(); i++)
    {
        KEXTERIOR_BUY_LATEST_DB_DATA::KEXTERIOR_INFO Item;
        KGLOG_PROCESS_ERROR((size_t)(pbyTail - pbyOffset) >= sizeof(Item));
        memset(&Item, 0, sizeof(Item));
        Item.wID = (WORD)m_LatestBuy[i];
        memcpy(pbyOffset, &Item, sizeof(Item));
        pbyOffset += sizeof(Item);
    }

    *puUsedSize = (size_t)(pbyOffset - pbyBuffer);
    bResult = true;
Exit0:
    return bResult;
}

BOOL KExteriorBox::Load(BYTE* pbyData, size_t uDataLen)
{
    BOOL    bResult   = false;
    BYTE*   pbyOffset = pbyData;
    BYTE*   pbyTail   = pbyData + uDataLen;
    WORD    wCount    = 0;
    WORD    i         = 0;

    KGLOG_PROCESS_ERROR(pbyData);

    // ---- owned box ----
    KGLOG_PROCESS_ERROR((size_t)(pbyTail - pbyOffset) >= sizeof(WORD));
    wCount = *(WORD*)pbyOffset; pbyOffset += sizeof(WORD);
    for (i = 0; i < wCount; i++)
    {
        KEXTERIOR_DB_DATA::KEXTERIOR_INFO* pItem = (KEXTERIOR_DB_DATA::KEXTERIOR_INFO*)pbyOffset;
        KGLOG_PROCESS_ERROR((size_t)(pbyTail - pbyOffset) >= sizeof(*pItem));
        // only add exteriors that still exist in config (mirror LoadExteriorBox)
        if (g_pSO3World->m_Settings.m_Exterior.GetExteriorInfo(pItem->wID))
        {
            KGLOG_PROCESS_ERROR(_Add(pItem->wID, (int)pItem->byFlag, (time_t)pItem->dwExpireTime));
        }
        pbyOffset += sizeof(*pItem);
    }

    // ---- equip sets ----
    KGLOG_PROCESS_ERROR((size_t)(pbyTail - pbyOffset) >= sizeof(WORD));
    wCount = *(WORD*)pbyOffset; pbyOffset += sizeof(WORD);
    m_ExteriorSet.clear();
    for (i = 0; i < wCount; i++)
    {
        KEXTERIOR_SET_DB_DATA::KEXTERIOR_SET_INFO* pItem = (KEXTERIOR_SET_DB_DATA::KEXTERIOR_SET_INFO*)pbyOffset;
        KEXTERIOR_SET_INFO Set;
        int nSlot = 0;
        KGLOG_PROCESS_ERROR((size_t)(pbyTail - pbyOffset) >= sizeof(*pItem));
        for (nSlot = 0; nSlot < MAX_EXTERIOR_SLOT; nSlot++)
            Set.dwExteriorID[nSlot] = pItem->wExteriorID[nSlot];
        m_ExteriorSet.push_back(Set);
        pbyOffset += sizeof(*pItem);
    }
    KGLOG_PROCESS_ERROR((size_t)(pbyTail - pbyOffset) >= sizeof(BYTE) + sizeof(WORD));
    m_uCurrentSetID = *(BYTE*)pbyOffset; pbyOffset += sizeof(BYTE);
    m_uExteriorFreeCount = *(WORD*)pbyOffset; pbyOffset += sizeof(WORD);

    // ---- latest buy ----
    KGLOG_PROCESS_ERROR((size_t)(pbyTail - pbyOffset) >= sizeof(WORD));
    wCount = *(WORD*)pbyOffset; pbyOffset += sizeof(WORD);
    m_LatestBuy.clear();
    for (i = 0; i < wCount; i++)
    {
        KEXTERIOR_BUY_LATEST_DB_DATA::KEXTERIOR_INFO* pItem = (KEXTERIOR_BUY_LATEST_DB_DATA::KEXTERIOR_INFO*)pbyOffset;
        KGLOG_PROCESS_ERROR((size_t)(pbyTail - pbyOffset) >= sizeof(*pItem));
        m_LatestBuy.push_back(pItem->wID);
        pbyOffset += sizeof(*pItem);
    }

    KGLOG_PROCESS_ERROR(pbyOffset == pbyTail);   // leftover == 0
    bResult = true;
Exit0:
    return bResult;
}

//------------------------------------------------------------------------
// [PORT-9] delete / latest-buy / free-count / activate / reverse-maps.
// Client sync-emit calls (v246 FUN_08059xxx) are deferred to the packet
// follow-up (#10) -- these are owner-only notifies the 2010 client can't
// render; the box state changes below are the observable/persisted part.
//------------------------------------------------------------------------
BOOL KExteriorBox::GetEquipPos(int nRepresentIndex, int* pnSlot)
{
    BOOL bResult = false;
    KG_PROCESS_ERROR(pnSlot);
    switch (nRepresentIndex)
    {
    case perChestStyle:  case perChestColor:  *pnSlot = 0; break;
    case perHelmStyle:   case perHelmColor:   *pnSlot = 1; break;
    case perWaistStyle:  case perWaistColor:  *pnSlot = 2; break;
    case perBootsStyle:  case perBootsColor:  *pnSlot = 3; break;
    case perBangleStyle: case perBangleColor: *pnSlot = 4; break;
    default: goto Exit0;
    }
    bResult = true;
Exit0:
    return bResult;
}

BOOL KExteriorBox::GetEquipPosBySubType(int nSubType, int* pnSlot)
{
    BOOL bResult = false;
    KG_PROCESS_ERROR(pnSlot);
    switch (nSubType)
    {
    case 2:  *pnSlot = 0; break;
    case 3:  *pnSlot = 1; break;
    case 6:  *pnSlot = 2; break;
    case 9:  *pnSlot = 3; break;
    case 10: *pnSlot = 4; break;
    default: goto Exit0;
    }
    bResult = true;
Exit0:
    return bResult;
}

BOOL KExteriorBox::AddExteriorFreeCount(int nCount)
{
    int nNew = (int)m_uExteriorFreeCount;

    if (nCount < 0)
        nNew = (nNew + nCount < 0) ? 0 : nNew + nCount;
    else
        nNew = (0x7fffffff - nCount < nNew) ? 0x7fffffff : nNew + nCount;

    if (nNew > MAX_EXTERIOR_FREE_COUNT)
        nNew = MAX_EXTERIOR_FREE_COUNT;

    m_uExteriorFreeCount = (DWORD)nNew;
    // [PORT-10] DoSyncExteriorFreeCount(m_pPlayer, m_uExteriorFreeCount)
    return true;
}

BOOL KExteriorBox::SetExteriorFreeCount(int nCount)
{
    m_uExteriorFreeCount = (DWORD)nCount;
    // [PORT-10] DoSyncExteriorFreeCount(m_pPlayer, m_uExteriorFreeCount)
    return true;
}

void KExteriorBox::_AddLatestBuy(DWORD dwID)
{
    std::vector<DWORD>::iterator it;
    for (it = m_LatestBuy.begin(); it != m_LatestBuy.end(); ++it)
    {
        if (*it == dwID) { m_LatestBuy.erase(it); break; }   // remove existing dup
    }
    m_LatestBuy.insert(m_LatestBuy.begin(), dwID);           // push front (most recent)
    if (m_LatestBuy.size() > (size_t)MAX_EXTERIOR_LATEST_BUY)
        m_LatestBuy.resize(MAX_EXTERIOR_LATEST_BUY);
}

BOOL KExteriorBox::AddLatestBuy(DWORD dwID)
{
    BOOL bResult = false;
    KGLOG_PROCESS_ERROR(g_pSO3World->m_Settings.m_Exterior.GetExteriorInfo(dwID));
    _AddLatestBuy(dwID);
    // [PORT-10] DoSyncExteriorBuyLatestData(m_pPlayer, 1, &dwID)
    bResult = true;
Exit0:
    return bResult;
}

BOOL KExteriorBox::DeleteExteriorInLatestBuy(DWORD dwID)
{
    BOOL bResult = false;
    std::vector<DWORD>::iterator it;

    KGLOG_PROCESS_ERROR(g_pSO3World->m_Settings.m_Exterior.GetExteriorInfo(dwID));

    for (it = m_LatestBuy.begin(); it != m_LatestBuy.end(); ++it)
    {
        if (*it == dwID)
        {
            m_LatestBuy.erase(it);
            // [PORT-10] DoDeleteExteriorBuyLatestData(m_pPlayer, 1, &dwID)
            break;
        }
    }
    bResult = true;
Exit0:
    return bResult;
}

BOOL KExteriorBox::DeleteExteriorInSet(DWORD dwID)
{
    BOOL            bResult = false;
    KEXTERIOR_INFO* pInfo   = NULL;
    int             nSlot   = 0;
    size_t          i       = 0;

    pInfo = g_pSO3World->m_Settings.m_Exterior.GetExteriorInfo(dwID);
    KGLOG_PROCESS_ERROR(pInfo);
    KGLOG_PROCESS_ERROR(GetEquipPosBySubType((int)pInfo->nSubType, &nSlot));

    for (i = 0; i < m_ExteriorSet.size(); i++)
    {
        if (m_ExteriorSet[i].dwExteriorID[nSlot] == dwID)
            SetExterior(i, nSlot, 0);   // clears the slot (+ re-drive/sync inside)
    }
    bResult = true;
Exit0:
    return bResult;
}

BOOL KExteriorBox::Delete(DWORD dwID)
{
    BOOL bResult = false;
    KEXTERIOR_ITEM_MAP::iterator it;

    if (dwID == 0)
        return true;    // v246: id==0 is a no-op success

    it = m_ExteriorMap.find(dwID);
    KGLOG_PROCESS_ERROR(it != m_ExteriorMap.end());

    m_ExteriorMap.erase(it);
    // [PORT-10] DoDeleteExteriorBoxData(m_pPlayer, dwID)

    KGLOG_PROCESS_ERROR(DeleteExteriorInSet(dwID));
    KGLOG_PROCESS_ERROR(DeleteExteriorInLatestBuy(dwID));

    bResult = true;
Exit0:
    return bResult;
}

void KExteriorBox::Activate()
{
    std::vector<DWORD>            vecExpired;
    KEXTERIOR_ITEM_MAP::iterator it;
    size_t                       i = 0;

    // Collect timed (nFlag != 0) items whose expire time has passed, then delete
    // them (deleting mutates the map, so gather ids first).
    for (it = m_ExteriorMap.begin(); it != m_ExteriorMap.end(); ++it)
    {
        if (it->second.nFlag != 0 && g_pSO3World->m_nCurrentTime >= it->second.nExpireTime)
            vecExpired.push_back(it->first);
    }

    for (i = 0; i < vecExpired.size(); i++)
        Delete(vecExpired[i]);
}
