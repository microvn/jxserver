#include "stdafx.h"
#include "Global.h"
#include "KTongConstList.h"

KTongConstList::KTongConstList()
{
    // Target constructor initializes only the first scalar; the remaining
    // fields are populated by Init() and are not zero-filled here.
    m_nRepairDiscount = 0;
}

BOOL KTongConstList::UnInit()
{
    return true;
}

const CRAFT_DISCOUNT_INFO* KTongConstList::GetCraftDiscountInfo(unsigned long nCraftType) const
{
    std::map<int, CRAFT_DISCOUNT_INFO>::const_iterator it = m_CraftDiscountMap.find(nCraftType);
    return it == m_CraftDiscountMap.end() ? NULL : &it->second;
}

BOOL KTongConstList::LoadCraftTech(IIniFile* pIniFile, const char* pszSection,
    TongTechNodeTag eTechType)
{
    CRAFT_DISCOUNT_INFO info;
    int nCraftType = 0;
    KGLOG_PROCESS_ERROR(pIniFile->GetInteger2("CraftConsumeReduceStamina", pszSection,
        &nCraftType, &info.nReduceStaminaRate));
    KGLOG_PROCESS_ERROR(nCraftType >= 0 && nCraftType <= 100);
    info.eTechType = eTechType;
    KGLOG_PROCESS_ERROR(m_CraftDiscountMap.insert(std::make_pair(nCraftType, info)).second);
    return true;
Exit0:
    return false;
}

BOOL KTongConstList::Init()
{
    BOOL bResult = false;
    /* Declared with the other locals: KGLOG_PROCESS_ERROR below expands to
       goto Exit0, and a later `BOOL bRetCode = false;` made that jump cross an
       initialisation (GCC 4.8 -std=gnu++98 hard error). Pre-existing defect in
       the 2026-08-05 KTongConstList port, found by the VPS compile gate. */
    BOOL bRetCode = false;
    IIniFile* pIniFile = NULL;
    pIniFile = g_OpenIniFile(SETTING_DIR "/TongConstList.ini");
    KGLOG_PROCESS_ERROR(pIniFile);

    // The target attempts all four craft sections first.  A failed craft
    // section is logged and the scalar load continues; it is not the
    // Init() failure edge used by the remaining fields.
    bRetCode = LoadCraftTech(pIniFile, "Cooking", ttntCookingConsumeReduce);
    KGLOG_CHECK_ERROR(bRetCode);
    bRetCode = LoadCraftTech(pIniFile, "Tailoring", ttntTailoringConsumeReduce);
    KGLOG_CHECK_ERROR(bRetCode);
    bRetCode = LoadCraftTech(pIniFile, "Founding", ttntFoundingConsumeReduce);
    KGLOG_CHECK_ERROR(bRetCode);
    bRetCode = LoadCraftTech(pIniFile, "Medicine", ttntMedicineConsumeReduce);
    KGLOG_CHECK_ERROR(bRetCode);

    KGLOG_PROCESS_ERROR(pIniFile->GetInteger("TongDiplomacy", "TongWarCD", 24 * 60 * 60,
        &m_nTongWarCD));
    KGLOG_PROCESS_ERROR(pIniFile->GetInteger("TongDiplomacy", "TongWarSameCampCastDP", 0,
        &m_nTongWarSameCampCastDP));
    KGLOG_PROCESS_ERROR(pIniFile->GetInteger("TongDiplomacy", "TongWarDiffCampCastDP", 0,
        &m_nTongWarDiffCampCastDP));
    KGLOG_PROCESS_ERROR(pIniFile->GetMultiInteger("TongDiplomacy", "TongWarTimes",
        m_nTongWarTimes, 3) == 3);
    KGLOG_PROCESS_ERROR(pIniFile->GetMultiInteger("TongDiplomacy", "TongContractWarCastDP",
        m_nTongContractWarCastDP, 4) == 4);
    KGLOG_PROCESS_ERROR(pIniFile->GetMultiInteger("TongDiplomacy", "TongContractWarCastMoney",
        m_nTongContractWarCastMoney, 4) == 4);
    KGLOG_PROCESS_ERROR(pIniFile->GetInteger("TongDiplomacy", "TongContractWarTime", 2 * 60 * 60,
        &m_nTongContractWarTime));
    KGLOG_PROCESS_ERROR(pIniFile->GetInteger("TongDiplomacy", "TongContractWarWaitTime", 2 * 60 * 60,
        &m_nTongContractWarWaitTime));
    KGLOG_PROCESS_ERROR(pIniFile->GetMultiInteger("TongDiplomacy", "TongContractWarTimeSegment",
        m_nTongContractWarTimeSegment, 8) == 8);
    KGLOG_PROCESS_ERROR(pIniFile->GetInteger("TongDiplomacy", "TongAllyWaitTime", 15 * 60,
        &m_nTongAllyWaitTime));
    KGLOG_PROCESS_ERROR(pIniFile->GetInteger("TongDiplomacy", "TongAllyRefusedWaitTime", 30 * 60,
        &m_nTongAllyRefusedCoolTime));
    KGLOG_PROCESS_ERROR(pIniFile->GetInteger("TongDiplomacy", "StopAllianceCoolTime", 24 * 60 * 60,
        &m_nTongStopAllianceCoolTime));

    KGLOG_PROCESS_ERROR(pIniFile->GetInteger("Misc", "RepairDiscount", 100,
        &m_nRepairDiscount));
    KGLOG_PROCESS_ERROR(m_nRepairDiscount >= 0 && m_nRepairDiscount <= 100);
    KGLOG_PROCESS_ERROR(pIniFile->GetInteger("Misc", "NewMemberLimitTime", 0,
        (int*)&m_nNewMemberLimitTime));
    KGLOG_PROCESS_ERROR(m_nNewMemberLimitTime >= 0);

    bResult = true;
Exit0:
    if (pIniFile) pIniFile->Release();
    return bResult;
}
