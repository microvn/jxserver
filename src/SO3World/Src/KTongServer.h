#ifndef _KTONG_SERVER_H_
#define _KTONG_SERVER_H_

#include <map>

struct TongCacheData
{
    BYTE byTechNodeTag[7];
};

typedef std::map<DWORD, BYTE> KTONG_MEMBER_CACHE_MAP;
typedef std::map<DWORD, TongCacheData> KTONG_INFO_CACHE_MAP;

class KTongServer
{
public:
    KTongServer();
    BOOL Init();
    void UnInit();
    BOOL SyncTongTotalCache(DWORD dwTongID, int nConnIndex);

private:
    BOOL                    m_bFundRepairEnable;
    BOOL                    m_bRepairDiscountEnable;
    BOOL                    m_bStaminaDiscountEnable;
    KTONG_MEMBER_CACHE_MAP  m_GSTongMemberListMap;
    KTONG_INFO_CACHE_MAP    m_TongInfoCache;
};

typedef char TONG_CACHE_DATA_SIZE_CHECK[(sizeof(TongCacheData) == 7) ? 1 : -1];
typedef char KTONG_SERVER_SIZE_CHECK[(sizeof(KTongServer) == 0x3c) ? 1 : -1];

#endif
