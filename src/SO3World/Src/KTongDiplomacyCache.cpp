#include "stdafx.h"
#include "KSO3World.h"
#include "KPlayer.h"
#include "KPlayerServer.h"
#include "KTongDiplomacyCache.h"

static uint64_t MakeTongRelationKey(DWORD dwSrcTongID, DWORD dwDstTongID)
{
    return ((uint64_t)dwSrcTongID << 32) | (uint64_t)dwDstTongID;
}

BOOL KTongDiplomacyCache::Init()
{
    m_WarMap.clear();
    m_AllianceMap.clear();
    m_ContractWarMap.clear();
    m_AllianceIndex.clear();
    m_CurrentGSWarKillStatMap.clear();
    m_TotalWarKillCacheMap.clear();
    return true;
}

void KTongDiplomacyCache::UnInit()
{
    m_WarMap.clear();
    m_AllianceMap.clear();
    m_ContractWarMap.clear();
    m_AllianceIndex.clear();
    m_CurrentGSWarKillStatMap.clear();
    m_TotalWarKillCacheMap.clear();
}

BOOL KTongDiplomacyCache::AddDiplomacyRelation(
    DWORD dwSrcTongID, DWORD dwDstTongID,
    TONG_DIPLOMACY_RELATION_TYPE eType, BOOL bPlayerDeclare,
    time_t nStartTime, time_t nEndTime, time_t nCDEndTime,
    WORD wSubType, WORD wTimeSegment
)
{
    KTONG_DIPLOMACY_MAP* pMap = NULL;
    KTongDiplomacyInfo Info;

    KGLOG_PROCESS_ERROR(dwSrcTongID != 0 && dwDstTongID != 0);
    switch (eType)
    {
    case tdrtWar:         pMap = &m_WarMap; break;
    case tdrtAlliance:   pMap = &m_AllianceMap; break;
    case tdrtContractWar: pMap = &m_ContractWarMap; break;
    default: KGLOG_PROCESS_ERROR(false);
    }

    Info.bPlayerDeclare = bPlayerDeclare;
    Info.nStartTime = nStartTime;
    Info.nEndTime = nEndTime;
    Info.nCDEndTime = nCDEndTime;
    Info.wSubType = wSubType;
    Info.wTimeSegment = wTimeSegment;
    (*pMap)[MakeTongRelationKey(dwSrcTongID, dwDstTongID)] = Info;
    return true;
Exit0:
    return false;
}

BOOL KTongDiplomacyCache::DelDiplomacyRelation(
    DWORD dwSrcTongID, DWORD dwDstTongID,
    TONG_DIPLOMACY_RELATION_TYPE eType
)
{
    KTONG_DIPLOMACY_MAP* pMap = NULL;
    switch (eType)
    {
    case tdrtWar:          pMap = &m_WarMap; break;
    case tdrtAlliance:    pMap = &m_AllianceMap; break;
    case tdrtContractWar: pMap = &m_ContractWarMap; break;
    default: return false;
    }
    pMap->erase(MakeTongRelationKey(dwSrcTongID, dwDstTongID));
    return true;
}

BOOL KTongDiplomacyCache::GetRelationList(
    DWORD dwTongID, TONG_DIPLOMACY_RELATION_TYPE eType,
    const KTONG_DIPLOMACY_MAP& rMap,
    std::vector<KTONG_DIPLOMACY_RELATION_INFO>& rList
) const
{
    KTONG_DIPLOMACY_MAP::const_iterator it;
    for (it = rMap.begin(); it != rMap.end(); ++it)
    {
        DWORD dwSrcTongID = (DWORD)(it->first >> 32);
        DWORD dwDstTongID = (DWORD)it->first;
        if (dwSrcTongID != dwTongID && dwDstTongID != dwTongID)
            continue;

        KTONG_DIPLOMACY_RELATION_INFO Info;
        memset(&Info, 0, sizeof(Info));
        Info.dwSrcTongID = dwSrcTongID;
        Info.dwDstTongID = dwDstTongID;
        Info.nType = (int)eType;
        Info.nStartTime = it->second.nStartTime;
        Info.nEndTime = it->second.nEndTime;
        Info.nCDEndTime = it->second.nCDEndTime;
        Info.wSubType = it->second.wSubType;
        Info.wTimeSegment = it->second.wTimeSegment;
        Info.byIsAdd = 1;
        rList.push_back(Info);
    }
    return true;
}

BOOL KTongDiplomacyCache::GetWarList(DWORD dwTongID, std::vector<KTONG_DIPLOMACY_RELATION_INFO>& rList)
{
    return GetRelationList(dwTongID, tdrtWar, m_WarMap, rList);
}

BOOL KTongDiplomacyCache::GetAllianceList(DWORD dwTongID, std::vector<KTONG_DIPLOMACY_RELATION_INFO>& rList)
{
    return GetRelationList(dwTongID, tdrtAlliance, m_AllianceMap, rList);
}

BOOL KTongDiplomacyCache::GetContractWarList(DWORD dwTongID, std::vector<KTONG_DIPLOMACY_RELATION_INFO>& rList)
{
    return GetRelationList(dwTongID, tdrtContractWar, m_ContractWarMap, rList);
}

BOOL KTongDiplomacyCache::SyncNewClient(KPlayer* pPlayer)
{
    std::vector<KTONG_DIPLOMACY_RELATION_INFO> RelationList;
    BOOL bResult = false;

    KGLOG_PROCESS_ERROR(pPlayer);
    if (pPlayer->m_dwTongID == 0)
        return true;
    GetWarList(pPlayer->m_dwTongID, RelationList);
    GetAllianceList(pPlayer->m_dwTongID, RelationList);
    GetContractWarList(pPlayer->m_dwTongID, RelationList);
    if (!RelationList.empty())
    {
        KGLOG_PROCESS_ERROR(g_PlayerServer.DoSyncTongDiplomacyData(
            pPlayer->m_nConnIndex, RelationList
        ));
    }
    bResult = true;
Exit0:
    return bResult;
}
