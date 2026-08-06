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
    /* Target 08215701 reads the local at -0x34 - the FOURTH argument of the
       GetInteger2 call at 082156c5, i.e. nReduceStaminaRate - and its assert text
       at 0840f1b0 is "nReduceStaminaRate >= 0 && nReduceStaminaRate <= 100".
       The candidate guarded nCraftType (the third argument, the map key), which
       is a different variable; with the shipped "Cooking=4,80" both happen to
       pass, so the drift was latent. */
    KGLOG_PROCESS_ERROR(info.nReduceStaminaRate >= 0 && info.nReduceStaminaRate <= 100);
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

    /* Failure policy re-derived per statement from the target @08215804.  NONE of
       the fourteen reads below has a cmp/branch on its return value, and the only
       KGLOG_PROCESS_ERROR in the whole function is the pIniFile null check at
       0821586b.  Wrapping them in KGLOG_PROCESS_ERROR made three keys that the
       shipped settings/TongConstList.ini simply does not have - TongAllyWaitTime,
       TongAllyRefusedWaitTime and StopAllianceCoolTime (verified absent with
       positive controls) - abort the entire settings load.  The target leaves the
       default in place instead.
       Each GetMultiInteger is preceded by a memset of its destination
       (08215a86 this+0x14/12, 08215ad8 this+0x20/16, 08215b2a this+0x30/16,
       08215be4 this+0x48/32) so a short/absent list leaves zeroes, not garbage. */
    pIniFile->GetInteger("TongDiplomacy", "TongWarCD", 24 * 60 * 60,
        &m_nTongWarCD);
    pIniFile->GetInteger("TongDiplomacy", "TongWarSameCampCastDP", 0,
        &m_nTongWarSameCampCastDP);
    pIniFile->GetInteger("TongDiplomacy", "TongWarDiffCampCastDP", 0,
        &m_nTongWarDiffCampCastDP);
    memset(m_nTongWarTimes, 0, sizeof(m_nTongWarTimes));
    pIniFile->GetMultiInteger("TongDiplomacy", "TongWarTimes",
        m_nTongWarTimes, 3);
    memset(m_nTongContractWarCastDP, 0, sizeof(m_nTongContractWarCastDP));
    pIniFile->GetMultiInteger("TongDiplomacy", "TongContractWarCastDP",
        m_nTongContractWarCastDP, 4);
    memset(m_nTongContractWarCastMoney, 0, sizeof(m_nTongContractWarCastMoney));
    pIniFile->GetMultiInteger("TongDiplomacy", "TongContractWarCastMoney",
        m_nTongContractWarCastMoney, 4);
    pIniFile->GetInteger("TongDiplomacy", "TongContractWarTime", 2 * 60 * 60,
        &m_nTongContractWarTime);
    pIniFile->GetInteger("TongDiplomacy", "TongContractWarWaitTime", 2 * 60 * 60,
        &m_nTongContractWarWaitTime);
    memset(m_nTongContractWarTimeSegment, 0, sizeof(m_nTongContractWarTimeSegment));
    pIniFile->GetMultiInteger("TongDiplomacy", "TongContractWarTimeSegment",
        m_nTongContractWarTimeSegment, 8);
    pIniFile->GetInteger("TongDiplomacy", "TongAllyWaitTime", 15 * 60,
        &m_nTongAllyWaitTime);
    pIniFile->GetInteger("TongDiplomacy", "TongAllyRefusedWaitTime", 30 * 60,
        &m_nTongAllyRefusedCoolTime);
    pIniFile->GetInteger("TongDiplomacy", "StopAllianceCoolTime", 24 * 60 * 60,
        &m_nTongStopAllianceCoolTime);

    pIniFile->GetInteger("Misc", "RepairDiscount", 100, &m_nRepairDiscount);
    /* Both closing guards are KGLOG_CHECK_ERROR, not KGLOG_PROCESS_ERROR: the
       KGLogPrintf calls at 08215d24 and 08215d8e both take format string
       0840f224 == "KGLOG_CHECK_ERROR(%s) at line %d in %s\n" and neither is
       followed by a jmp - control falls straight through to `bResult = 1` at
       08215d93.  They log and continue. */
    KGLOG_CHECK_ERROR(m_nRepairDiscount >= 0 && m_nRepairDiscount <= 100);
    pIniFile->GetInteger("Misc", "NewMemberLimitTime", 0,
        (int*)&m_nNewMemberLimitTime);
    KGLOG_CHECK_ERROR(m_nNewMemberLimitTime >= 0);

    bResult = true;
Exit0:
    if (pIniFile) pIniFile->Release();
    return bResult;
}
