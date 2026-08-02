#ifndef _KFELLOW_PET_BOX_H_
#define _KFELLOW_PET_BOX_H_

#include <map>

#include "Global.h"
#include "KCustomData.h"
#include "Engine/KMemory.h"

class KPlayer;

#pragma pack(push, 1)
struct KPET_TIME_LIMIT_INFO_DB
{
    WORD wPetIndex;
    BYTE byExistType;
    time_t nGenTime;
    time_t nMaxExistTime;
};
#pragma pack(pop)

struct KPetTimeLimitInfo
{
    KITEM_EXIST_TYPE eExistType;
    time_t nGenTime;
    time_t nMaxExistTime;
};

class KFellowPetBox
{
public:
    typedef std::map<
        int,
        KPetTimeLimitInfo,
        std::less<int>,
        KMemory::KAllocator<std::pair<int, KPetTimeLimitInfo> >
    > KPET_TIME_LIMIT_INFO_MAP;

    KFellowPetBox();
    ~KFellowPetBox();

    BOOL Init(KPlayer* pPlayer);
    void UnInit();
    void Activate();
    BOOL Load(BYTE* pbyData, size_t uDataLen, DWORD dwVersion);
    BOOL Save(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize);
    BOOL Acquire(int nPetIndex);
    BOOL Remove(int nPetIndex);
    BOOL IsAcquired(int nPetIndex);
    BOOL GetTimeLimitInfo(int nPetIndex, int* pnExistType, time_t* pnGenTime, time_t* pnMaxExistTime);

private:
    BOOL Load_V0(BYTE* pbyData, size_t uDataLen);
    BOOL Load_V1(BYTE* pbyData, size_t uDataLen);
    BOOL LoadTimeLimitInfo(const KPET_TIME_LIMIT_INFO_DB* pInfo, time_t nTimeNow, time_t nWorldTime);
    BOOL IsOverdue(const KPetTimeLimitInfo& Info, time_t nTimeNow);
    KPlayer* m_pPlayer;
    KCustomData<9> m_PetData;
    KPET_TIME_LIMIT_INFO_MAP m_PetTimeLimitInfoMap;
};

#endif
