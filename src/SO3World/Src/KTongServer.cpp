#include "stdafx.h"
#include "KPlayerServer.h"
#include "KTongServer.h"

KTongServer::KTongServer()
    : m_bFundRepairEnable(false),
      m_bRepairDiscountEnable(false),
      m_bStaminaDiscountEnable(false)
{
}

BOOL KTongServer::Init()
{
    m_GSTongMemberListMap.clear();
    m_TongInfoCache.clear();
    return true;
}

void KTongServer::UnInit()
{
    m_GSTongMemberListMap.clear();
    m_TongInfoCache.clear();
}

BOOL KTongServer::SyncTongTotalCache(DWORD dwTongID, int nConnIndex)
{
    KTONG_INFO_CACHE_MAP::const_iterator it = m_TongInfoCache.find(dwTongID);
    KGLOG_PROCESS_ERROR(it != m_TongInfoCache.end());
    KGLOG_PROCESS_ERROR(g_PlayerServer.DoSyncTongTotalCache(
        nConnIndex, it->second.byTechNodeTag
    ));
    return true;
Exit0:
    return false;
}
