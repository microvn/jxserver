#ifndef _KMINIAVATAR_H_
#define _KMINIAVATAR_H_

//////////////////////////////////////////////////////////////////////////
// KMiniAvatar -- per-player mini-avatar (小玩伴) collection: the set of
// acquired mini-avatar ids. The "worn" id lives on KPlayer (m_dwMiniAvatarID).
// Ported from v2.5 (v246); layout pinned from DWARF -- docs/miniavatar_port/.
// Lua = Luna<KMiniAvatar> object, reached via KPlayer:GetMiniAvatarMgr().
// Embedded in KPlayer; recompiled -> offset compiler-chosen.
//////////////////////////////////////////////////////////////////////////

#include <set>
#ifdef _SERVER
#include "Luna.h"
#endif

class KPlayer;

#define MAX_MINIAVATAR_ID   0xFFFF  // ids serialize as WORD

class KMiniAvatar
{
public:
    BOOL Init(KPlayer* pPlayer);
    void UnInit();

    BOOL Save(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize);
    BOOL Load(BYTE* pbyData, size_t uDataLen, unsigned long nVersion);

    BOOL Acquire(DWORD dwID);        // validate (shop + force gate) then add
    BOOL CanAcquire(DWORD dwID);     // exists in config + force matches
    BOOL IsAcquired(DWORD dwID);
    size_t GetCount() const { return m_MiniAvatarSet.size(); }

#ifdef _SERVER
public:
    int LuaAcquireMiniAvatar(Lua_State* L);
    int LuaIsMiniAvatarAcquired(Lua_State* L);
    int LuaGetMiniAvatarCount(Lua_State* L);
    int LuaGetAllMiniAvatar(Lua_State* L);

    DECLARE_LUA_CLASS(KMiniAvatar);
#endif

private:
    BOOL _Add(DWORD dwID);           // raw insert (Load path), no validation

private:
    typedef std::set<DWORD, std::less<DWORD>, KMemory::KAllocator<DWORD> > KMINI_AVATAR_SET;
    KMINI_AVATAR_SET m_MiniAvatarSet; // +0x00  acquired ids
    KPlayer*        m_pPlayer;       // +0x18
};

#endif  // _KMINIAVATAR_H_
