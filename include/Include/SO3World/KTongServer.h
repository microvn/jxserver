#ifndef _K_TONG_SERVER_H_
#define _K_TONG_SERVER_H_

#include <list>
#include <map>

#include "Engine/KMemory.h"
#include "KTongConstList.h"

/*[target 2.5.2] DWARF TongCacheData: byte_size 7, one member
  BYTE byTechNodeTag[7] indexed by TongTechNodeTag (ttntTotal == 7). */
struct TongCacheData
{
    BYTE byTechNodeTag[ttntTotal];
};

class KTongServer
{
public:
    KTongServer();

    // target parameter order is (dwPlayerID, dwTongID): OnPlayerLogin@081e5938,
    // RegisterTongMember@081e55de
    void OnPlayerLogin(DWORD dwPlayerID, DWORD dwTongID);
    void RegisterTongMember(DWORD dwPlayerID, DWORD dwTongID);
    void OnTongStopAlliance(DWORD dwSrcTongID, DWORD dwDstTongID);
    void OnTongAlly(DWORD dwSrcTongID, DWORD dwDstTongID);

    // target: KTongServer::InsertTongChache@081e5afc (spelling kept as target)
    void InsertTongChache(DWORD dwTongID, const TongCacheData& crCahceData);
    // target: KTongServer::UpdateTongChacheChange@081e5a64
    void UpdateTongChacheChange(DWORD dwTongID, TongTechNodeTag eType, BYTE byValue);
    // target: KTongServer::SyncTongTotalCache@081e5994, caller KPlayer::OnExtDataLoadFinish@0839fb50
    void SyncTongTotalCache(DWORD dwTongID, int nConnIndex);

    // target: KTongServer::Traverse<KFuncTraverseTongSend>@0808fb10
    template <class TFunc>
    void Traverse(DWORD dwTongID, TFunc& Func)
    {
        BOOL                            bRetCode    = false;
        KTONG_MEMBER_LIST_MAP::iterator TongIt;
        KROLE_LIST::iterator            RoleIt;

        /* 0808fb6d-0808fb72: the map-miss branch jumps straight to the epilogue
           at 0808fc2b with no KGLogPrintf, so the miss is silent.
           0808fbbf-0808fbf1: on functor-false the target logs and then FALLS
           THROUGH into ++it at 0808fbf1 - there is no jump out of the loop. The
           format string at 083d0f70 is KGLOG_CHECK_ERROR (line 0x3b = 59), which
           logs without goto Exit0. A failed member must not abort the broadcast
           for the members after it.
           Note KObjectIndex<KPlayer>::Traverse@080ee218 is a different contract:
           it really does break on false, silently. Do not unify the two. */
        TongIt = m_GSTongMemberListMap.find(dwTongID);
        KG_PROCESS_ERROR(TongIt != m_GSTongMemberListMap.end());

        for (RoleIt = TongIt->second.begin(); RoleIt != TongIt->second.end(); ++RoleIt)
        {
            bRetCode = Func(*RoleIt);
            KGLOG_CHECK_ERROR(bRetCode);
        }

Exit0:
        return;
    }

private:
    typedef std::list<DWORD, KMemory::KAllocator<DWORD> > KROLE_LIST;
    typedef std::map<DWORD, KROLE_LIST, std::less<DWORD>,
        KMemory::KAllocator<std::pair<DWORD, KROLE_LIST> > > KTONG_MEMBER_LIST_MAP;
    typedef std::map<DWORD, TongCacheData, std::less<DWORD>,
        KMemory::KAllocator<std::pair<DWORD, TongCacheData> > > KTONG_INFO_CACHE;

private:
    BOOL                    m_bFundRepairEnable;
    BOOL                    m_bRepairDiscountEnable;
    BOOL                    m_bStaminaDiscountEnable;
    KTONG_MEMBER_LIST_MAP   m_GSTongMemberListMap;
    KTONG_INFO_CACHE        m_TongInfoCache;
};

#endif // _K_TONG_SERVER_H_
