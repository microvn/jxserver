#include "stdafx.h"

#include "KTongServer.h"
#include "KPlayer.h"
#include "KPlayerServer.h"
#include "KRelayClient.h"
#include "KSO3World.h"

KTongServer::KTongServer()
{
    m_bFundRepairEnable      = TRUE;
    m_bRepairDiscountEnable  = TRUE;
    m_bStaminaDiscountEnable = TRUE;
}

/*[target 2.5.2] KTongServer::OnPlayerLogin@081e5938, mangled ...13OnPlayerLoginEmm.
  DWARF parameter order is (dwPlayerID, dwTongID) - not the reverse.
  081e593e cmp [ebp+0xc],0 / jne  -> logged guard on dwPlayerID; the .rodata condition
    string at 08407d4a is literally "dwPlayerID != ERROR_ID" (line 20).
  081e5972 cmp [ebp+0x10],0 / je -> SILENT exit when dwTongID == 0.
  Getting these two the wrong way round logs an error for every guild-less player on
  the login/transfer path, and lets a real dwPlayerID == 0 through without a word. */
void KTongServer::OnPlayerLogin(DWORD dwPlayerID, DWORD dwTongID)
{
    KGLOG_PROCESS_ERROR(dwPlayerID != ERROR_ID);

    if (dwTongID == 0)
        goto Exit0;

    RegisterTongMember(dwPlayerID, dwTongID);

Exit0:
    return;
}

/*[target 2.5.2] KTongServer::RegisterTongMember@081e55de, mangled ...18RegisterTongMemberEmm;
  DWARF parameter order is (dwPlayerID, dwTongID), matching the call at 081e598c. */
void KTongServer::RegisterTongMember(DWORD dwPlayerID, DWORD dwTongID)
{
    KTONG_MEMBER_LIST_MAP::iterator TongIt;
    std::pair<KTONG_MEMBER_LIST_MAP::iterator, bool> InsertRet;
    KROLE_LIST RoleList;

    /* target asserts dwPlayerID first (081e55f8, line 205) then dwTongID
       (081e5623, line 206); the message text on failure is part of the contract. */
    assert(dwPlayerID != 0);
    assert(dwTongID != 0);

    TongIt = m_GSTongMemberListMap.find(dwTongID);
    if (TongIt == m_GSTongMemberListMap.end())
    {
        RoleList.push_back(dwPlayerID);
        InsertRet = m_GSTongMemberListMap.insert(std::make_pair(dwTongID, RoleList));
        KGLOG_PROCESS_ERROR(InsertRet.second);
        TongIt = InsertRet.first;
    }
    else
    {
        TongIt->second.push_back(dwPlayerID);
    }

    /*[target 2.5.2] RegisterTongMember@081e587b -> DoApplyTongCacheRequest@080caa74,
      s2r wire id 129 (0x81), S2R_APPLY_TONG_CACHE_REQUEST (6 bytes). */
    if (m_TongInfoCache.find(dwTongID) == m_TongInfoCache.end())
    {
        g_RelayClient.DoApplyTongCacheRequest(dwTongID);
    }

Exit0:
    return;
}

void KTongServer::OnTongStopAlliance(DWORD dwSrcTongID, DWORD dwDstTongID)
{
    KTONG_MEMBER_LIST_MAP::iterator TongIt;
    KROLE_LIST::iterator RoleIt;
    KPlayer* pPlayer = NULL;

    assert(dwSrcTongID != 0);
    assert(dwDstTongID != 0);

    TongIt = m_GSTongMemberListMap.find(dwSrcTongID);
    if (TongIt != m_GSTongMemberListMap.end())
    {
        for (RoleIt = TongIt->second.begin(); RoleIt != TongIt->second.end(); ++RoleIt)
        {
            pPlayer = g_pSO3World->m_PlayerSet.GetObj(*RoleIt);
            KGLOG_PROCESS_ERROR(pPlayer);
            pPlayer->m_dwAllianceTongID = 0;
        }
    }

    TongIt = m_GSTongMemberListMap.find(dwDstTongID);
    if (TongIt != m_GSTongMemberListMap.end())
    {
        for (RoleIt = TongIt->second.begin(); RoleIt != TongIt->second.end(); ++RoleIt)
        {
            pPlayer = g_pSO3World->m_PlayerSet.GetObj(*RoleIt);
            KGLOG_PROCESS_ERROR(pPlayer);
            pPlayer->m_dwAllianceTongID = 0;
        }
    }

Exit0:
    return;
}

void KTongServer::OnTongAlly(DWORD dwSrcTongID, DWORD dwDstTongID)
{
    KTONG_MEMBER_LIST_MAP::iterator TongIt;
    KROLE_LIST::iterator RoleIt;
    KPlayer* pPlayer = NULL;

    assert(dwSrcTongID != 0);
    assert(dwDstTongID != 0);

    TongIt = m_GSTongMemberListMap.find(dwSrcTongID);
    if (TongIt != m_GSTongMemberListMap.end())
    {
        for (RoleIt = TongIt->second.begin(); RoleIt != TongIt->second.end(); ++RoleIt)
        {
            pPlayer = g_pSO3World->m_PlayerSet.GetObj(*RoleIt);
            KGLOG_PROCESS_ERROR(pPlayer);
            pPlayer->m_dwAllianceTongID = dwDstTongID;
        }
    }

    TongIt = m_GSTongMemberListMap.find(dwDstTongID);
    if (TongIt != m_GSTongMemberListMap.end())
    {
        for (RoleIt = TongIt->second.begin(); RoleIt != TongIt->second.end(); ++RoleIt)
        {
            pPlayer = g_pSO3World->m_PlayerSet.GetObj(*RoleIt);
            KGLOG_PROCESS_ERROR(pPlayer);
            pPlayer->m_dwAllianceTongID = dwSrcTongID;
        }
    }

Exit0:
    return;
}

/*[target 2.5.2] KTongServer::InsertTongChache@081e5afc, size 79, basic_blocks 1.
  Ordered target calls: operator[]@081e5b12 -> DoBroadcastTongTotalCache@081e5b44.
  No guard, no log, no early return on the target path.
  Caller: KRelayClient::OnApplyTongCacheRespond@080dcda8. */
void KTongServer::InsertTongChache(DWORD dwTongID, const TongCacheData& crCahceData)
{
    m_TongInfoCache[dwTongID] = crCahceData;

    g_PlayerServer.DoBroadcastTongTotalCache(dwTongID, crCahceData.byTechNodeTag);
}

/*[target 2.5.2] KTongServer::UpdateTongChacheChange@081e5a64, size 152, basic_blocks 3.
  Ordered target calls: find@081e5a87 -> end@081e5a9f -> operator!=@081e5ab4
  -> operator->@081e5ac6 -> DoBroadcastTongCacheChange@081e5af5.
  Target emits no KGLogPrintf here, so the miss branch is silent.
  Caller: KRelayClient::OnSyncTongCacheChange@080dcd24. */
void KTongServer::UpdateTongChacheChange(DWORD dwTongID, TongTechNodeTag eType, BYTE byValue)
{
    KTONG_INFO_CACHE::iterator CacheIt;

    CacheIt = m_TongInfoCache.find(dwTongID);
    KG_PROCESS_ERROR(CacheIt != m_TongInfoCache.end());

    CacheIt->second.byTechNodeTag[eType] = byValue;

    g_PlayerServer.DoBroadcastTongCacheChange(dwTongID, eType, byValue);

Exit0:
    return;
}

/*[target 2.5.2] KTongServer::SyncTongTotalCache@081e5994, size 207, basic_blocks 4.
  Ordered target calls: find@081e59b1 -> _Rb_tree_const_iterator@081e59c6 -> end@081e59db
  -> operator!=@081e5a02 -> KGLogPrintf@081e5a35 (out-of-line failure block)
  -> operator->@081e5a42 -> DoSyncTongTotalCache@081e5a5c.
  Caller: KPlayer::OnExtDataLoadFinish@0839fb50. */
void KTongServer::SyncTongTotalCache(DWORD dwTongID, int nConnIndex)
{
    KTONG_INFO_CACHE::const_iterator CacheIt;

    CacheIt = m_TongInfoCache.find(dwTongID);
    KGLOG_PROCESS_ERROR(CacheIt != m_TongInfoCache.end());

    g_PlayerServer.DoSyncTongTotalCache(nConnIndex, CacheIt->second.byTechNodeTag);

Exit0:
    return;
}
