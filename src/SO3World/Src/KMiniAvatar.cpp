#include "stdafx.h"
#include "Global.h"
#include "KSO3World.h"
#include "KPlayer.h"
#include "KMiniAvatar.h"
#include "KMiniAvatarSettings.h"

//////////////////////////////////////////////////////////////////////////
// KMiniAvatar. set<DWORD> of acquired ids; worn id on KPlayer. Logic + DB
// byte-format pinned from v246 (Save 08216712 / Load 08216a1c) -- see
// docs/miniavatar_port/WORKLOG.md. Client sync packets deferred (v246-only).
//////////////////////////////////////////////////////////////////////////

BOOL KMiniAvatar::Init(KPlayer* pPlayer)
{
    BOOL bResult = false;
    KGLOG_PROCESS_ERROR(pPlayer);
    m_pPlayer = pPlayer;
    m_MiniAvatarSet.clear();
    bResult = true;
Exit0:
    return bResult;
}

void KMiniAvatar::UnInit()
{
    m_MiniAvatarSet.clear();
    m_pPlayer = NULL;
}

BOOL KMiniAvatar::_Add(DWORD dwID)
{
    BOOL bResult = false;
    KGLOG_PROCESS_ERROR(dwID != 0 && dwID <= MAX_MINIAVATAR_ID);
    m_MiniAvatarSet.insert(dwID);
    bResult = true;
Exit0:
    return bResult;
}

BOOL KMiniAvatar::CanAcquire(DWORD dwID)
{
    BOOL             bResult = false;
    KMiniAvatarInfo* pInfo   = NULL;

    KGLOG_PROCESS_ERROR(m_pPlayer);
    pInfo = g_pSO3World->m_Settings.m_MiniAvatarSettings.GetMiniAvatarInfo(dwID);
    KGLOG_PROCESS_ERROR(pInfo);
    // force gate: 0 = any force, else must match the player's force
    KGLOG_PROCESS_ERROR(pInfo->dwForceID == 0 || pInfo->dwForceID == m_pPlayer->m_dwForceID);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KMiniAvatar::Acquire(DWORD dwID)
{
    BOOL bResult = false;

    if (dwID == 0)
        return true;
    KGLOG_PROCESS_ERROR(dwID <= MAX_MINIAVATAR_ID);
    KGLOG_PROCESS_ERROR(CanAcquire(dwID));

    bResult = _Add(dwID);
Exit0:
    return bResult;
}

BOOL KMiniAvatar::IsAcquired(DWORD dwID)
{
    return m_MiniAvatarSet.find(dwID) != m_MiniAvatarSet.end();
}

//------------------------------------------------------------------------
// Persistence: [WORD wornID][WORD count][WORD id x count]; leftover==0.
//------------------------------------------------------------------------
BOOL KMiniAvatar::Save(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize)
{
    BOOL   bResult   = false;
    BYTE*  pbyOffset = pbyBuffer;
    BYTE*  pbyTail   = pbyBuffer + uBufferSize;
    KMINI_AVATAR_SET::iterator it;

    KGLOG_PROCESS_ERROR(puUsedSize && m_pPlayer);

    KGLOG_PROCESS_ERROR((size_t)(pbyTail - pbyOffset) >= sizeof(WORD) * 2);
    *(WORD*)pbyOffset = (WORD)m_pPlayer->m_dwMiniAvatarID;   pbyOffset += sizeof(WORD);
    *(WORD*)pbyOffset = (WORD)m_MiniAvatarSet.size();        pbyOffset += sizeof(WORD);

    for (it = m_MiniAvatarSet.begin(); it != m_MiniAvatarSet.end(); ++it)
    {
        KGLOG_PROCESS_ERROR((size_t)(pbyTail - pbyOffset) >= sizeof(WORD));
        *(WORD*)pbyOffset = (WORD)(*it);
        pbyOffset += sizeof(WORD);
    }

    *puUsedSize = (size_t)(pbyOffset - pbyBuffer);
    bResult = true;
Exit0:
    return bResult;
}

BOOL KMiniAvatar::Load(BYTE* pbyData, size_t uDataLen, unsigned long nVersion)
{
    BOOL   bResult   = false;
    BYTE*  pbyOffset = pbyData;
    BYTE*  pbyTail   = pbyData + uDataLen;
    WORD   wWornID   = 0;
    WORD   wCount    = 0;
    WORD   i         = 0;

    KGLOG_PROCESS_ERROR(pbyData && m_pPlayer);
    KGLOG_PROCESS_ERROR(nVersion <= 1);

    KGLOG_PROCESS_ERROR((size_t)(pbyTail - pbyOffset) >= sizeof(WORD) * 2);
    wWornID = *(WORD*)pbyOffset; pbyOffset += sizeof(WORD);
    wCount  = *(WORD*)pbyOffset; pbyOffset += sizeof(WORD);

    for (i = 0; i < wCount; i++)
    {
        WORD wID = 0;
        KGLOG_PROCESS_ERROR((size_t)(pbyTail - pbyOffset) >= sizeof(WORD));
        wID = *(WORD*)pbyOffset; pbyOffset += sizeof(WORD);
        if (wID != 0)
            m_MiniAvatarSet.insert((DWORD)wID);   // raw insert (config may have changed)
    }

    m_pPlayer->m_dwMiniAvatarID = wWornID;         // worn id restored as-is
    KGLOG_PROCESS_ERROR(pbyOffset == pbyTail);     // leftover == 0
    bResult = true;
Exit0:
    return bResult;
}
