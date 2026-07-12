#ifndef _KMINIAVATAR_SETTINGS_H_
#define _KMINIAVATAR_SETTINGS_H_

//////////////////////////////////////////////////////////////////////////
// KMiniAvatarSettings -- config singleton for the mini-avatar (小玩伴) system.
// Loads MiniAvatarInfo.tab -> map keyed by id. Ported from v2.5 (v246); sizes
// pinned from DWARF (SO3GameServerD) -- see docs/miniavatar_port/WORKLOG.md.
// Embedded in KWorldSettings (like KHairShop). Recompiled -> offsets free.
//////////////////////////////////////////////////////////////////////////

#include <map>

class KMiniAvatar;

// MiniAvatarInfo.tab row. Key = dwID (map key). KMiniAvatarSettings.h, size 0x0c.
struct KMiniAvatarInfo
{
    DWORD   dwID;           // +0x00
    DWORD   dwForceID;      // +0x04  0 = any force
    BOOL    bFreeAcquire;   // +0x08  granted free at login
};                          // sizeof = 0x0c

class KMiniAvatarSettings
{
public:
    BOOL Init();
    void UnInit();

    KMiniAvatarInfo* GetMiniAvatarInfo(DWORD dwID);
    // Grant every free-acquire mini-avatar into a player's box.
    void AcquireAllFreeMiniAvatar(KMiniAvatar* pBox);

private:
    BOOL LoadMiniAvatarInfo();

private:
    typedef std::map<DWORD, KMiniAvatarInfo> KMINIAVATAR_INFO_MAP;
    KMINIAVATAR_INFO_MAP m_MiniAvatarInfoMap;   // MiniAvatarInfo.tab
};

#endif  // _KMINIAVATAR_SETTINGS_H_
