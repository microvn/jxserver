#ifndef _KTONG_CONST_LIST_H_
#define _KTONG_CONST_LIST_H_

#include <ctime>
#include <map>

#include "KTongDef.h"

struct CRAFT_DISCOUNT_INFO
{
    TongTechNodeTag eTechType;
    int nReduceStaminaRate;
};

class KTongConstList
{
public:
    KTongConstList();
    BOOL Init();
    BOOL UnInit();
    BOOL LoadCraftTech(IIniFile* pIniFile, const char* pszSection,
        TongTechNodeTag eTechType);
    const CRAFT_DISCOUNT_INFO* GetCraftDiscountInfo(unsigned long nCraftType) const;

    int m_nRepairDiscount;
    time_t m_nNewMemberLimitTime;
    int m_nTongWarCD;
    int m_nTongWarSameCampCastDP;
    int m_nTongWarDiffCampCastDP;
    int m_nTongWarTimes[3];
    int m_nTongContractWarCastDP[4];
    int m_nTongContractWarCastMoney[4];
    int m_nTongContractWarTime;
    int m_nTongContractWarWaitTime;
    int m_nTongContractWarTimeSegment[8];
    int m_nTongAllyWaitTime;
    int m_nTongAllyRefusedCoolTime;
    int m_nTongStopAllianceCoolTime;

private:
    std::map<int, CRAFT_DISCOUNT_INFO> m_CraftDiscountMap;
};

#endif
