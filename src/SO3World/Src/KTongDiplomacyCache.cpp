#include "stdafx.h"
#include "KSO3World.h"
#include "KTongDiplomacyCache.h"

BOOL KTongDiplomacyCache::Init()
{
    return true;
}

void KTongDiplomacyCache::UnInit()
{
}

unsigned long KTongDiplomacyCache::GetAllianceTongID(unsigned long dwTongID)
{
    unsigned long dwAllianceTongID = 0;
    KALLIANCE_INDEX::iterator it = m_AllianceIndex.find(dwTongID);
    if (it != m_AllianceIndex.end())
        dwAllianceTongID = it->second;
    return dwAllianceTongID;
}

BOOL KTongDiplomacyCache::AddDiplomacyRelation(unsigned long dwSrcTongID, unsigned long dwDstTongID, TONG_DIPLOMACY_RELATION_TYPE eRelationType, BOOL bPlayerDeclare, time_t nStartTime, time_t nEndTime, time_t nCDEndTime, WORD wSubType, WORD wTimeSegment)
{
    unsigned long long ullKey = (unsigned long long)dwSrcTongID + (unsigned long long)dwDstTongID;
    KTongDiplomacyInfo Info;

    Info.bPlayerDeclare = bPlayerDeclare;
    Info.nStartTime = nStartTime;
    Info.nEndTime = nEndTime;
    Info.nCDEndTime = nCDEndTime;
    Info.wSubType = wSubType;
    Info.wTimeSegment = wTimeSegment;

    if (dwSrcTongID == dwDstTongID)
    {
        KGLogPrintf(KGLOG_ERR, "dwSrcTongID != dwDstTongID");
        return true;
    }

    switch (eRelationType)
    {
    case TONG_DIPLOMACY_RELATION_WAR:
        {
            KDIPLOMACY_MAP::iterator it = m_WarMap.find(ullKey);
            if (it == m_WarMap.end())
                it = m_WarMap.find((unsigned long long)dwDstTongID + (unsigned long long)dwSrcTongID);

            if (it != m_WarMap.end())
            {
                if (bPlayerDeclare)
                {
                    KGLogPrintf(KGLOG_ERR, "it == m_WarMap.end()");
                    return true;
                }
                if (it->second.nCDEndTime < nCDEndTime)
                    it->second.nCDEndTime = nCDEndTime;
                return true;
            }

            m_WarMap.insert(std::make_pair(ullKey, Info));
        }
        break;

    case TONG_DIPLOMACY_RELATION_ALLIANCE:
        m_AllianceMap[ullKey] = Info;
        if (bPlayerDeclare)
        {
            m_AllianceIndex[dwSrcTongID] = dwDstTongID;
            m_AllianceIndex[dwDstTongID] = dwSrcTongID;
            g_pSO3World->m_TongServer.OnTongAlly(dwSrcTongID, dwDstTongID);
        }
        break;

    case TONG_DIPLOMACY_RELATION_CONTRACT_WAR:
        m_ContractWarMap[ullKey] = Info;
        break;

    default:
        KGLogPrintf(KGLOG_DEBUG, "[KTongDiplomacyMgr] Add error, can not recognize Diplomacy type %d", eRelationType);
        break;
    }

    return true;
}

BOOL KTongDiplomacyCache::DelDiplomacyRelation(unsigned long dwSrcTongID, unsigned long dwDstTongID, TONG_DIPLOMACY_RELATION_TYPE eRelationType)
{
    unsigned long long ullKey = (unsigned long long)dwSrcTongID + (unsigned long long)dwDstTongID;

    switch (eRelationType)
    {
    case TONG_DIPLOMACY_RELATION_WAR:
        m_WarMap.erase(ullKey);
        m_CurrentGSWarKillStatMap.erase(ullKey);
        m_CurrentGSWarKillStatMap.erase((unsigned long long)dwDstTongID + (unsigned long long)dwSrcTongID);
        m_TotalWarKillCacheMap.erase(ullKey);
        m_TotalWarKillCacheMap.erase((unsigned long long)dwDstTongID + (unsigned long long)dwSrcTongID);
        break;

    case TONG_DIPLOMACY_RELATION_ALLIANCE:
        {
            KDIPLOMACY_MAP::iterator it = m_AllianceMap.find(ullKey);
            if (it != m_AllianceMap.end())
            {
                if (it->second.bPlayerDeclare)
                {
                    g_pSO3World->m_TongServer.OnTongStopAlliance(dwSrcTongID, dwDstTongID);
                    m_AllianceIndex.erase(dwSrcTongID);
                    m_AllianceIndex.erase(dwDstTongID);
                }
                m_AllianceMap.erase(it);
            }
        }
        break;

    case TONG_DIPLOMACY_RELATION_CONTRACT_WAR:
        m_ContractWarMap.erase(ullKey);
        break;

    default:
        KGLogPrintf(KGLOG_DEBUG, "[KTongDiplomacyMgr] Del error, can not recognize Diplomacy type %d", eRelationType);
        break;
    }

    return true;
}
