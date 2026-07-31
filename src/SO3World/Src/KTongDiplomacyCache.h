#ifndef _KTONG_DIPLOMACY_CACHE_H_
#define _KTONG_DIPLOMACY_CACHE_H_

#include <map>
#include <vector>

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

typedef std::map<uint64_t, KTongDiplomacyInfo> KTONG_DIPLOMACY_MAP;
typedef std::map<uint64_t, BYTE> KTONG_DIPLOMACY_BYTE_MAP;

class KPlayer;

class KTongDiplomacyCache
{
public:
    BOOL Init();
    void UnInit();

    BOOL AddDiplomacyRelation(
        DWORD dwSrcTongID, DWORD dwDstTongID,
        TONG_DIPLOMACY_RELATION_TYPE eType, BOOL bPlayerDeclare,
        time_t nStartTime, time_t nEndTime, time_t nCDEndTime,
        WORD wSubType, WORD wTimeSegment
    );
    BOOL DelDiplomacyRelation(
        DWORD dwSrcTongID, DWORD dwDstTongID,
        TONG_DIPLOMACY_RELATION_TYPE eType
    );
    BOOL GetWarList(DWORD dwTongID, std::vector<KTONG_DIPLOMACY_RELATION_INFO>& rList);
    BOOL GetAllianceList(DWORD dwTongID, std::vector<KTONG_DIPLOMACY_RELATION_INFO>& rList);
    BOOL GetContractWarList(DWORD dwTongID, std::vector<KTONG_DIPLOMACY_RELATION_INFO>& rList);
    BOOL SyncNewClient(KPlayer* pPlayer);

private:
    BOOL GetRelationList(
        DWORD dwTongID, TONG_DIPLOMACY_RELATION_TYPE eType,
        const KTONG_DIPLOMACY_MAP& rMap,
        std::vector<KTONG_DIPLOMACY_RELATION_INFO>& rList
    ) const;

    KTONG_DIPLOMACY_MAP       m_WarMap;
    KTONG_DIPLOMACY_MAP       m_AllianceMap;
    KTONG_DIPLOMACY_MAP       m_ContractWarMap;
    KTONG_DIPLOMACY_BYTE_MAP  m_AllianceIndex;
    KTONG_DIPLOMACY_BYTE_MAP  m_CurrentGSWarKillStatMap;
    KTONG_DIPLOMACY_BYTE_MAP  m_TotalWarKillCacheMap;
};

typedef char KTONG_DIPLOMACY_INFO_SIZE_CHECK[(sizeof(KTongDiplomacyInfo) == 0x14) ? 1 : -1];
typedef char KTONG_DIPLOMACY_CACHE_SIZE_CHECK[(sizeof(KTongDiplomacyCache) == 0x90) ? 1 : -1];

#endif
