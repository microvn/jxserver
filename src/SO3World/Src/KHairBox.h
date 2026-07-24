#ifndef _KHAIR_BOX_H_
#define _KHAIR_BOX_H_

//////////////////////////////////////////////////////////////////////////
// KHairBox -- per-player hair (fa-xing) inventory. Owns the two owned-hair
// lists (face / hair slots) + a free-change counter, and applies a chosen
// owned hair onto the player's represent-ID appearance slots.
// Ported from SO3GameServer v246; see WORKLOG [RE-3].
// Embedded in KPlayer (binary offset 0xb8a0); recompiled -> compiler-chosen.
//////////////////////////////////////////////////////////////////////////

#include <vector>
#include "KHairShopDef.h"

class KPlayer;

class KHairBox
{
public:
    KHairBox();
    ~KHairBox();

    BOOL Init(KPlayer* pPlayer);
    void UnInit();

    // Persistence: one role-block = [WORD blockSize][WORD count][8B item x count]
    // per slot (both slots always written), then [WORD freeCount].
    BOOL Save(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize);
    BOOL Load(BYTE* pbyData, size_t uDataLen);

    // Add an owned hair (validates it exists in the shop table), then sync.
    BOOL Add(int nType, DWORD dwID);
    // TRUE if the player owns dwID in slot nType.
    BOOL Find(int nType, DWORD dwID);
    // Apply an owned hair onto the appearance (represent-ID) slot.
    BOOL ChangeHair(int nType, DWORD dwID);

    const std::vector<DWORD>* GetHairList(int nType);
    int  GetHairFreeCount() const { return m_nFreeCount; }
    BOOL AddHairFreeCount(int nCount);   // saturating [0, MAX_HAIR_FREE_COUNT]
    BOOL SetHairFreeCount(int nCount);

private:
    // Raw insert: keep the slot list sorted + unique, cap MAX_HAIR_LIST_SIZE.
    // No shop validation (the path Load uses to rebuild).
    BOOL _Add(int nType, DWORD dwID);

    // Guarded represent-ID setter (== KItemList::SetRepresentID); syncs only on change.
    void SetRepresentID(int nRepresentIndex, int nRepresentID);

private:
    int                 m_nFreeCount;           // +0x00  free-change counter [0,60000]
    std::vector<DWORD>  m_HairList[hsTotal];    // +0x04  owned hair ids per slot (sorted/unique)
    KPlayer*            m_pPlayer;              // +0x1c
};

#endif  // _KHAIR_BOX_H_
