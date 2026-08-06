#include "stdafx.h"
#include "Global.h"
#include "SO3GlobalDef.h"
#include "KSO3World.h"
#include "KPlayer.h"
#include "KPlayerServer.h"
#include "KHairShop.h"
#include "KHairBox.h"

//////////////////////////////////////////////////////////////////////////
// KHairBox. Layout + serialize byte-format + _Add/ChangeHair logic pinned
// from v246 -- see WORKLOG [RE-3]. Apply reuses the 2010 represent-ID
// mechanism (perFaceStyle/perHairStyle == hsFace/hsHair) via SetRepresentID,
// NOT the raw v246 KPlayer+0x95a8 offset -- see [RE-4].
//////////////////////////////////////////////////////////////////////////

KHairBox::KHairBox()
{
    m_nFreeCount = 0;
    m_pPlayer    = NULL;
}

KHairBox::~KHairBox()
{
}

BOOL KHairBox::Init(KPlayer* pPlayer)
{
    BOOL bResult = false;

    KGLOG_PROCESS_ERROR(pPlayer);

    m_pPlayer    = pPlayer;
    m_nFreeCount = 0;
    m_HairList[hsFace].clear();
    m_HairList[hsHair].clear();

    bResult = true;
Exit0:
    return bResult;
}

void KHairBox::UnInit()
{
    m_HairList[hsFace].clear();
    m_HairList[hsHair].clear();
    m_nFreeCount = 0;
    m_pPlayer    = NULL;
}

//------------------------------------------------------------------------
// _Add / Add / Find  (v246 08206e56 / 082070a6 / 082069b4)
// list kept sorted + unique so Find is a binary search and Save is ordered.
//------------------------------------------------------------------------
BOOL KHairBox::_Add(int nType, DWORD dwID)
{
    BOOL                        bResult = false;
    std::vector<DWORD>*         pList   = NULL;
    std::vector<DWORD>::iterator it;

    KGLOG_PROCESS_ERROR(nType >= 0 && nType < hsTotal);

    pList = &m_HairList[nType];
    it = std::lower_bound(pList->begin(), pList->end(), dwID);
    if (it != pList->end() && *it == dwID)
    {
        bResult = true;         // already owned -> no-op success
        goto Exit0;
    }

    KGLOG_PROCESS_ERROR(pList->size() < MAX_HAIR_LIST_SIZE);
    pList->insert(it, dwID);    // keep sorted

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHairBox::Add(int nType, DWORD dwID)
{
    BOOL              bResult = false;
    KHAIR_PRICE_INFO* pPrice  = NULL;

    if (dwID == 0)
        return true;            // v246: id==0 is a no-op success

    KGLOG_PROCESS_ERROR(nType >= 0 && nType < hsTotal);
    KGLOG_PROCESS_ERROR(m_pPlayer);

    // validate the hair exists in the shop table for this role (v246 Add path)
    pPrice = g_pSO3World->m_HairShop.GetPriceInfo((int)m_pPlayer->m_eRoleType, nType, dwID);
    KGLOG_PROCESS_ERROR(pPrice);

    bResult = _Add(nType, dwID);
Exit0:
    return bResult;
}

BOOL KHairBox::Find(int nType, DWORD dwID)
{
    const std::vector<DWORD>*         pList = NULL;
    std::vector<DWORD>::const_iterator it;

    if (nType < 0 || nType >= hsTotal)
        return false;

    pList = &m_HairList[nType];
    it = std::lower_bound(pList->begin(), pList->end(), dwID);
    return (it != pList->end() && *it == dwID);
}

//------------------------------------------------------------------------
// ChangeHair (v246 08206b76). Apply an owned hair onto the appearance.
// nType (hsFace/hsHair) == represent index (perFaceStyle/perHairStyle).
//------------------------------------------------------------------------
BOOL KHairBox::ChangeHair(int nType, DWORD dwID)
{
    BOOL bResult = false;

    if (dwID == 0)
        return true;

    KGLOG_PROCESS_ERROR(nType >= 0 && nType < hsTotal);
    KGLOG_PROCESS_ERROR(m_pPlayer);
    KGLOG_PROCESS_ERROR(Find(nType, dwID));         // must own it first

    // hsFace==perFaceStyle(0), hsHair==perHairStyle(1) -- verified equal (RE-4).
    SetRepresentID(nType, (int)dwID);

    bResult = true;
Exit0:
    return bResult;
}

// == KItemList::SetRepresentID (copied like KExteriorBox::SetRepresentID).
void KHairBox::SetRepresentID(int nRepresentIndex, int nRepresentID)
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

const std::vector<DWORD>* KHairBox::GetHairList(int nType)
{
    if (nType < 0 || nType >= hsTotal)
        return NULL;
    return &m_HairList[nType];
}

//------------------------------------------------------------------------
// Free-change counter (v246 08206d08 / 08206de2). Saturating, cap 60000.
//------------------------------------------------------------------------
BOOL KHairBox::AddHairFreeCount(int nCount)
{
    long long llValue = 0;

    KGLOG_PROCESS_ERROR(m_pPlayer);

    llValue = (long long)m_nFreeCount + nCount;
    if (llValue < 0)
        llValue = 0;
    if (llValue > MAX_HAIR_FREE_COUNT)
        llValue = MAX_HAIR_FREE_COUNT;
    m_nFreeCount = (int)llValue;

    return true;
Exit0:
    return false;
}

BOOL KHairBox::SetHairFreeCount(int nCount)
{
    KGLOG_PROCESS_ERROR(m_pPlayer);
    m_nFreeCount = nCount;       // v246: set-through, no clamp
    return true;
Exit0:
    return false;
}

//------------------------------------------------------------------------
// Persistence (v246 Save 082077ca / Load 08207466). Per slot:
// [WORD blockSize][WORD count][8B item x count]; blockSize = 2 + count*8.
// Both slots always written, then [WORD freeCount]. Load asserts leftover==2
// before the trailing freeCount (== exactly one WORD left).
//------------------------------------------------------------------------
BOOL KHairBox::Save(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize)
{
    BOOL   bResult   = false;
    BYTE*  pbyOffset = pbyBuffer;
    BYTE*  pbyTail   = pbyBuffer + uBufferSize;
    int    nType     = 0;
    size_t i         = 0;

    KGLOG_PROCESS_ERROR(puUsedSize);

    for (nType = 0; nType < hsTotal; nType++)
    {
        WORD* pwBlockSize = NULL;
        BYTE* pbyBlock    = NULL;

        KGLOG_PROCESS_ERROR((size_t)(pbyTail - pbyOffset) >= sizeof(WORD));
        pwBlockSize = (WORD*)pbyOffset;         // reserve blockSize prefix
        pbyOffset  += sizeof(WORD);
        pbyBlock    = pbyOffset;

        KGLOG_PROCESS_ERROR((size_t)(pbyTail - pbyOffset) >= sizeof(WORD));
        *(WORD*)pbyOffset = (WORD)m_HairList[nType].size();
        pbyOffset += sizeof(WORD);

        for (i = 0; i < m_HairList[nType].size(); i++)
        {
            KHAIR_DB_DATA::KHAIR_INFO Item;
            KGLOG_PROCESS_ERROR((size_t)(pbyTail - pbyOffset) >= sizeof(Item));
            memset(&Item, 0, sizeof(Item));
            Item.wID = (WORD)m_HairList[nType][i];
            memcpy(pbyOffset, &Item, sizeof(Item));
            pbyOffset += sizeof(Item);
        }

        *pwBlockSize = (WORD)(pbyOffset - pbyBlock);    // = 2 + count*8
    }

    KGLOG_PROCESS_ERROR((size_t)(pbyTail - pbyOffset) >= sizeof(WORD));
    *(WORD*)pbyOffset = (WORD)m_nFreeCount;
    pbyOffset += sizeof(WORD);

    *puUsedSize = (size_t)(pbyOffset - pbyBuffer);
    bResult = true;
Exit0:
    return bResult;
}

BOOL KHairBox::Load(BYTE* pbyData, size_t uDataLen)
{
    BOOL   bResult   = false;
    BYTE*  pbyOffset = pbyData;
    BYTE*  pbyTail   = pbyData + uDataLen;
    int    nType     = 0;
    int    nRoleType = 0;

    KGLOG_PROCESS_ERROR(pbyData);
    KGLOG_PROCESS_ERROR(m_pPlayer);
    nRoleType = (int)m_pPlayer->m_eRoleType;

    for (nType = 0; nType < hsTotal; nType++)
    {
        WORD  wBlockSize = 0;
        WORD  wCount     = 0;
        WORD  i          = 0;
        BYTE* pbyBlockEnd = NULL;

        KGLOG_PROCESS_ERROR((size_t)(pbyTail - pbyOffset) >= sizeof(WORD));
        wBlockSize = *(WORD*)pbyOffset; pbyOffset += sizeof(WORD);
        KGLOG_PROCESS_ERROR((size_t)(pbyTail - pbyOffset) >= wBlockSize);
        pbyBlockEnd = pbyOffset + wBlockSize;

        KGLOG_PROCESS_ERROR(wBlockSize >= sizeof(WORD));
        wCount = *(WORD*)pbyOffset; pbyOffset += sizeof(WORD);
        for (i = 0; i < wCount; i++)
        {
            KHAIR_DB_DATA::KHAIR_INFO* pItem = (KHAIR_DB_DATA::KHAIR_INFO*)pbyOffset;
            KGLOG_PROCESS_ERROR((size_t)(pbyBlockEnd - pbyOffset) >= sizeof(*pItem));
            // only re-add hair still present in the shop table (mirror v246 LoadHairList)
            if (g_pSO3World->m_HairShop.GetPriceInfo(nRoleType, nType, pItem->wID))
            {
                KGLOG_PROCESS_ERROR(_Add(nType, pItem->wID));
            }
            pbyOffset += sizeof(*pItem);
        }
        pbyOffset = pbyBlockEnd;    // tolerate trailing pad inside the block
    }

    KGLOG_PROCESS_ERROR((size_t)(pbyTail - pbyOffset) >= sizeof(WORD));
    m_nFreeCount = *(WORD*)pbyOffset; pbyOffset += sizeof(WORD);

    KGLOG_PROCESS_ERROR(pbyOffset == pbyTail);      // leftover == 0
    bResult = true;
Exit0:
    return bResult;
}
