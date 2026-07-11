#ifndef _KEXTERIOR_BOX_H_
#define _KEXTERIOR_BOX_H_

//////////////////////////////////////////////////////////////////////////
// KExteriorBox -- per-player exterior (wai-guan) wardrobe. Owns the box of
// owned exteriors, the equip-sets, and the "latest buy" list; applies chosen
// exteriors onto the player's represent-ID appearance slots.
// Ported from SO3GameServer v246; see WORKLOG [PORT-3]/[PORT-4].
// Embedded in KPlayer (binary offset 0xb850); recompiled -> compiler-chosen.
//////////////////////////////////////////////////////////////////////////

#include <map>
#include <vector>
#include "KExteriorDef.h"

class KPlayer;

// One equip-set: 5 exterior IDs, one per slot (0 = empty). 20 bytes.
struct KEXTERIOR_SET_INFO
{
    DWORD   dwExteriorID[5];    // [slot] 0..4
};

// 5 exterior equip slots. Each maps to an equip subtype + a pair of
// represent-ID appearance indices (style + color) -- all pinned from the
// KExteriorBox::GetEquip*Index switch tables in v246 (see [PORT-3]).
//   slot 0 = Chest, 1 = Helm, 2 = Waist, 3 = Boots, 4 = Bangle
#define MAX_EXTERIOR_SLOT   5

class KExteriorBox
{
public:
    KExteriorBox();
    ~KExteriorBox();

    BOOL Init(KPlayer* pPlayer);
    void UnInit();

    // Persistence: one role-block = [owned items][sets + trailer][latest-buy].
    // Item byte-layouts pinned from v246; combined into one block because the
    // 2010 KRoleBlockHeader has no chunk key (v246 chunked owned items separately).
    BOOL Save(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize);
    BOOL Load(BYTE* pbyData, size_t uDataLen);

    // Add an owned exterior. nTimeType: ettPermanent/ett7Days/ettLimit (or the
    // stored flag when loading from DB). nExpireTime used for ettLimit / DB load.
    BOOL Add(DWORD dwID, int nTimeType, time_t nExpireTime);

    KEXTERIOR_ITEM* GetExteriorItem(DWORD dwID);   // NULL if not owned
    size_t          GetExteriorCount() const { return m_ExteriorMap.size(); }

    // Remove an owned exterior everywhere (box + sets + latest-buy).
    BOOL Delete(DWORD dwID);
    BOOL DeleteExteriorInSet(DWORD dwID);        // clear the id from every set slot
    BOOL DeleteExteriorInLatestBuy(DWORD dwID);  // drop from the recent-buy list
    BOOL AddLatestBuy(DWORD dwID);               // push onto the recent-buy list (dedup, cap 20)
    void Activate();                             // purge timed exteriors that have expired

    // Free-use counter (box internal currency). Saturating add, cap 60000.
    BOOL  AddExteriorFreeCount(int nCount);
    BOOL  SetExteriorFreeCount(int nCount);
    DWORD GetExteriorFreeCount() const { return m_uExteriorFreeCount; }

    // Reverse mappings (represent/color index -> slot ; subtype -> slot). Pure.
    BOOL GetEquipPos(int nRepresentIndex, int* pnSlot);
    BOOL GetEquipPosBySubType(int nSubType, int* pnSlot);

    // ---- equip-set + apply ----
    KEXTERIOR_SET_INFO* GetExteriorSet(size_t uSetIdx);         // NULL if idx out of range
    size_t              GetExteriorSetCount() const { return m_ExteriorSet.size(); }
    BOOL                AddExteriorSet();                        // push one empty set (no max)
    BOOL                SetExterior(size_t uSetIdx, int nSlot, DWORD dwID);
    BOOL                SetCurrentSetID(size_t uSetIdx);
    DWORD               GetCurrentSetID() const { return m_uCurrentSetID; }

    // TRUE if the current set has a non-zero exterior in nSlot.
    BOOL IsHaveExteriorRepresent(int nSlot);
    // Push slot nSlot's chosen exterior onto the player represent-ID slots
    // (only when its per-slot apply bit is set). Core of the apply chain.
    BOOL ApplyExteriorRepresent(int nSlot);
    // Master apply: set the master bit + apply every owned slot (LuaApplyExterior core).
    void ApplyAllExterior();
    // Master un-apply: clear the master bit + restore every slot's real equip
    // appearance (LuaUnApplyExterior core).
    void UnApplyAllExterior();

    // ---- slot -> constant mappings (pure; leaf helpers for Apply) ----
    // Return TRUE and write *pnOut on success; FALSE if nSlot out of [0,5).
    BOOL GetEquipRepresentIndex(int nSlot, int* pnIndex);   // -> m_wRepresentId[] style index
    BOOL GetEquipColorIndex(int nSlot, int* pnIndex);       // -> m_wRepresentId[] color index
    BOOL GetEquipSubType(int nSlot, int* pnSubType);        // equip subtype guarding the slot

private:
    // _Add: find-or-insert the item, applying the time-type renewal rules to an
    // existing timed entry. Returns the item, or NULL on capacity overflow.
    KEXTERIOR_ITEM* _Add(DWORD dwID, int nTimeType, time_t nExpireTime);

    // Guarded represent-ID setter (== v246 FUN_082009a2 == KItemList::SetRepresentID):
    // honours m_dwRepresentIdLock + the m_bHideHat guard on perHelmStyle, and
    // syncs the client only when the value changes.
    void SetRepresentID(int nRepresentIndex, int nRepresentID);
    // Set the slot's apply bit then apply it (== per-slot wrapper FUN_0838844a).
    void ApplyOneSlot(int nSlot);
    // Push id to the front of the latest-buy list (remove dup, cap 20).
    void _AddLatestBuy(DWORD dwID);
    // Clear the slot's apply bit + restore the real equipped item's appearance
    // (or 0 if the slot is empty) -- == per-slot un-apply FUN_08388252.
    void UnApplyOneSlot(int nSlot);

private:
    typedef std::map<DWORD, KEXTERIOR_ITEM> KEXTERIOR_ITEM_MAP;
    typedef std::vector<KEXTERIOR_SET_INFO> KEXTERIOR_SET_VECTOR;

    KEXTERIOR_ITEM_MAP    m_ExteriorMap;     // owned exteriors, keyed by exterior ID
    KEXTERIOR_SET_VECTOR  m_ExteriorSet;     // equip-sets, indexed 0-based
    std::vector<DWORD>    m_LatestBuy;       // recent-buy exterior IDs (display; cap 20)
    DWORD                 m_uCurrentSetID;   // current set index (box+0x00)
    DWORD                 m_uExteriorFreeCount; // free-use counter (box+0x48)
    KPlayer*              m_pPlayer;
};

#endif  // _KEXTERIOR_BOX_H_
