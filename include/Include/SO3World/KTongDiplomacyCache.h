#ifndef _KTONG_DIPLOMACY_CACHE_H_
#define _KTONG_DIPLOMACY_CACHE_H_

#include <map>
#include "KMemory.h"
#include "KTongDef.h"

struct KTongDiplomacyInfo
{
    BOOL    bPlayerDeclare;
    time_t  nStartTime;
    time_t  nEndTime;
    time_t  nCDEndTime;
    WORD    wSubType;
    WORD    wTimeSegment;
};

class KTongDiplomacyCache
{
public:
    typedef std::map<unsigned long long, KTongDiplomacyInfo, std::less<unsigned long long>, KMemory::KAllocator<std::pair<unsigned long long, KTongDiplomacyInfo> > > KDIPLOMACY_MAP;
    typedef std::map<unsigned long, unsigned long, std::less<unsigned long>, KMemory::KAllocator<std::pair<unsigned long, unsigned long> > > KALLIANCE_INDEX;
    typedef std::map<unsigned long long, int, std::less<unsigned long long>, KMemory::KAllocator<std::pair<unsigned long long, int> > > KWAR_KILL_STAT_MAP;

public:
    BOOL Init();
    void UnInit();

    BOOL AddDiplomacyRelation(unsigned long dwSrcTongID, unsigned long dwDstTongID, TONG_DIPLOMACY_RELATION_TYPE eRelationType, BOOL bPlayerDeclare, time_t nStartTime, time_t nEndTime, time_t nCDEndTime, WORD wSubType, WORD wTimeSegment);
    BOOL DelDiplomacyRelation(unsigned long dwSrcTongID, unsigned long dwDstTongID, TONG_DIPLOMACY_RELATION_TYPE eRelationType);
    unsigned long GetAllianceTongID(unsigned long dwTongID);

public:
    KDIPLOMACY_MAP   m_WarMap;
    KDIPLOMACY_MAP   m_AllianceMap;
    KDIPLOMACY_MAP   m_ContractWarMap;
    KALLIANCE_INDEX  m_AllianceIndex;
    KWAR_KILL_STAT_MAP m_CurrentGSWarKillStatMap;
    KWAR_KILL_STAT_MAP m_TotalWarKillCacheMap;
};

#endif
