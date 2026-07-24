#ifndef _KDESIGNATION_LIST_H_
#define _KDESIGNATION_LIST_H_

#include <map>

// v2.5 drift: prefix/postfix info gained cooldown/duration/buff columns.
// Layout pinned from v246 binary (LoadPrefixInfo 081d49f6, GetPrefixInfo store order):
//   6 dwords for prefix (0x18=24B), 5 dwords for postfix (0x14=20B). All 4-byte fields.
struct KPrefixInfo
{
    int   nAnnounceType;    // @0x00  (2010 was BYTE byAnnounceType; widen to int)
    DWORD dwCoolDownID;     // @0x04  0 = no cooldown
    int   nOwnDuration;     // @0x08  seconds; 0 = permanent
    DWORD dwBuffID;         // @0x0c  0 = no buff on equip
    int   nBuffLevel;       // @0x10
    int   nType;            // @0x14  !=0 => independent/exclusive prefix (prefix ONLY)
};

struct KPostfixInfo
{
    int   nAnnounceType;    // @0x00
    DWORD dwCoolDownID;     // @0x04
    int   nOwnDuration;     // @0x08
    DWORD dwBuffID;         // @0x0c
    int   nBuffLevel;       // @0x10
};

class KDesignationList
{
public:
    BOOL Init();
    void UnInit();

    KPrefixInfo* GetPrefixInfo(int nPrefix);
    KPostfixInfo* GetPostfixInfo(int nPostfix);

private:
    BOOL LoadPrefixInfo();
    BOOL LoadPostfixInfo();

private:
    typedef std::map<int, KPrefixInfo> KPREFIX_MAP;
    KPREFIX_MAP m_PrefixList;

    typedef std::map<int, KPostfixInfo> KPOSTFIX_MAP;
    KPOSTFIX_MAP m_PostfixList;
};

#endif  //_KDESIGNATION_LIST_H_

