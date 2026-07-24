#ifndef _DESIGNATION_H_
#define _DESIGNATION_H_

#include "Global.h"
#include "KCustomData.h"
#include <map>

#define MAX_DESIGNATION_FIX_ID      UCHAR_MAX
#define DESIGNATIONFIX_DATA_SIZE    (MAX_DESIGNATION_FIX_ID / CHAR_BIT + 1)

class KPlayer;

class KDesignation
{
public:
    KDesignation();
    ~KDesignation();

    BOOL Init(KPlayer* pPlayer);
	void UnInit();

    BOOL Save(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize);
    BOOL Load(BYTE* pbyData, size_t uDataLen);

    // v2.5 drift: optional, append-only end-time block (timed designations).
    BOOL SaveEndTimeInfo(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize);
    BOOL LoadEndTimeInfo(size_t* puUsedSize, BYTE* pbyData, size_t uDataLen);

    BOOL SetCurrentDesignation(int nPrefix, int nPostfix, BOOL bDisplayFlag);
    // v2.5 sig drift: prefix can be granted with an explicit expiry (0 = permanent/auto).
    BOOL AcquirePrefix(int nPrefix, time_t nEndTime = 0);
    BOOL AcquirePostfix(int nPostfix);
    BOOL RemovePrefix(int nPrefix);
    BOOL RemovePostfix(int nPostfix);
    BOOL IsPrefixAcquired(int nPrefix);
    BOOL IsPostfixAcquired(int nPostfix);

#ifdef _SERVER
    BOOL SetGeneration(int nGenerationIndex);

    // v2.5 NEW: equip/unequip (own vs wear split), byname-display toggle. Lua-facing.
    BOOL EquipPrefix(int nPrefix);
    BOOL EquipPostfix(int nPostfix);
    BOOL UnEquipPrefix();
    BOOL UnEquipPostfix();
    BOOL SetBynameDisplayFlag(int nFlag);

    // Timed-designation lookups (Lua-facing).
    BOOL GetPrefixEndTime(int nPrefix, int* pnEndTime);
    BOOL GetPostfixEndTime(int nPostfix, int* pnEndTime);

    // Per-tick expiry of timed designations (wired into KPlayer::Activate).
    BOOL Activate();
#endif

private:
    KCustomData<DESIGNATIONFIX_DATA_SIZE>  m_AcquiredPrefix;
    KCustomData<DESIGNATIONFIX_DATA_SIZE>  m_AcquiredPostfix;
    KPlayer*                               m_pPlayer;

    // v2.5 drift (offsets are compiler-free; names/types pinned from DWARF):
    BOOL    m_bAllowBroadcastAnnounceFix;    // Init=TRUE; gates Acquire's announce
    std::map<int, time_t> m_PrefixEndTimeTable;   // id -> expiry (timed designations)
    std::map<int, time_t> m_PostfixEndTimeTable;
    BOOL    m_bCurrentIndependent;           // cached nType of equipped prefix

#ifdef _SERVER
    BOOL BroadcastDesignationAnnounce(int nPrefix, int nPostfix, BYTE byType);

    // v2.5 NEW internal helpers (equip pipeline).
    BOOL SetCurrentPrefix(int nPrefix);
    BOOL SetCurrentPostfix(int nPostfix);
    BOOL CanEquipPrefix(int nPrefix);
    BOOL CanEquipPostfix(int nPostfix);
    BOOL ResetPrefixCDTime();
    BOOL ResetPostfixCDTime();
#endif

public:
    int     m_nCurrentPrefix;   // prefix (equipped)
    int     m_nCurrentPostfix;  // postfix (equipped)
    int     m_nGenerationIndex; // generation
    int     m_nBynameIndex;     // byname
    BOOL    m_bBynameDisplay;   // renamed from 2010 m_bDisplayFlag
};

#endif  //_DESIGNATION_H_
 
