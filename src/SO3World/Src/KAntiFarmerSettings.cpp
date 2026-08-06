#include "stdafx.h"
#include "Global.h"
#include "KAntiFarmerSettings.h"

KAntiFarmerSettings::KAntiFarmerSettings()
{
    // Target constructs the set first and initializes only m_bEnable here.
    m_bEnable = false;
}

BOOL KAntiFarmerSettings::AppendIgnoreNpcTemplateID(DWORD dwTemplateID)
{
    return m_NpcWhiteList.insert(static_cast<unsigned long>(dwTemplateID)).second;
}

BOOL KAntiFarmerSettings::UnInit()
{
    m_NpcWhiteList.clear();
    return true;
}

BOOL KAntiFarmerSettings::LoadNpcTemplateIDWiteList()
{
    BOOL bResult = false;
    ITabFile* pTabFile = NULL;
    /* Declared with the other locals: KGLOG_PROCESS_ERROR expands to goto Exit0,
       and GCC 4.8 -std=gnu++98 rejects a jump that crosses an initialisation. */
    int nCount = 0;
    char szFile[MAX_PATH];
    snprintf(szFile, sizeof(szFile), "%s/%s", SETTING_DIR, "AntiFarmerWhiteList.tab");
    szFile[sizeof(szFile) - 1] = '\0';
    pTabFile = g_OpenTabFile(szFile);
    KGLOG_PROCESS_ERROR(pTabFile);
    /* Target @08207da8: nCount = GetHeight() - 1 (the `decl %eax` at 08207daa),
       then `for (i = 0; i < nCount; ++i)` (cmp/jl at 08207e86) with the tab row
       computed as i + 2 (`addl $0x2, %eax` at 08207dca).  That covers rows
       2..GetHeight() inclusive; the previous `nLine < GetHeight()` dropped the
       last data row of AntiFarmerWhiteList.tab. */
    nCount = pTabFile->GetHeight() - 1;
    for (int i = 0; i < nCount; ++i)
    {
        int nLine = i + 2;
        DWORD dwTemplateID = 0;
        KGLOG_PROCESS_ERROR(pTabFile->GetInteger(nLine, "NpcTemplateID", 0,
            (int*)&dwTemplateID));
        // Target @08207e2f: `testl %eax,%eax; jne` on the loaded value, assert
        // text "dwNpcTemplateID != ERROR_ID" (ERROR_ID == 0, SO3GlobalDef.h:19).
        KGLOG_PROCESS_ERROR(dwTemplateID != ERROR_ID);
        // Target inserts into the set and ignores the duplicate result.
        m_NpcWhiteList.insert(static_cast<unsigned long>(dwTemplateID));
    }
    bResult = true;
Exit0:
    if (pTabFile) pTabFile->Release();
    return bResult;
}

BOOL KAntiFarmerSettings::Init()
{
    BOOL bResult = false;
    IIniFile* pIniFile = NULL;
    char szFile[MAX_PATH];
    snprintf(szFile, sizeof(szFile), "%s/%s", SETTING_DIR, "AntiFarmer.ini");
    szFile[sizeof(szFile) - 1] = '\0';
    pIniFile = g_OpenIniFile(szFile);
    KGLOG_PROCESS_ERROR(pIniFile);
    /* Target KAntiFarmerSettings::Init @08207eba.
       Three drifts against it, all re-derived from the callsite operands:
         1. sections/keys.  Only "Enable" and "EffectiveTime" live in [Global];
            CheckOnlineTimeRange/OnlineRateLimit/KillNpcRateLimit/KillNpcTypeNum/
            RecordKillNpcNum/PunishRate are in [Rule1] (.rodata 0840cc06) and the
            six detection keys are in [Rule3] (0840cdb4).  Several key names also
            differed from the member names the 2010 source used.  Confirmed
            against shipped settings/AntiFarmer.ini.
         2. getter.  "Enable" (08207fc9) and [Rule3] "Enable" (0820866c) go
            through IIniFile vtable slot 0x48 == GetBool, not slot 0x28
            GetInteger.  The shipped file stores them as `true`/`false`, which
            GetInteger cannot parse.
         3. failure policy.  The target never tests the return value of any of
            the 15 getters - no cmp/branch follows any of the indirect calls -
            and instead guards the resulting *member*.  Wrapping the getters in
            KGLOG_PROCESS_ERROR turned every absent key into a fatal settings
            load.  Each member guard below is KGLOG_PROCESS_ERROR in the target
            too (format string 0840ca94 == "KGLOG_PROCESS_ERROR", each followed
            by `jmp` to the common Exit0), and each sits immediately after the
            read it validates. */
    pIniFile->GetBool("Global", "Enable", (int*)&m_bEnable);
    pIniFile->GetInteger("Global", "EffectiveTime", 864000,
        (int*)&m_nEffectiveTime);
    KGLOG_PROCESS_ERROR(m_nEffectiveTime >= 3600);              // 08208005 cmpl $0xe0f
    pIniFile->GetInteger("Rule1", "CheckOnlineTimeRange", 21600,
        &m_nCheckOnlineTimeRange);
    KGLOG_PROCESS_ERROR(m_nCheckOnlineTimeRange > 30);          // 08208077 cmpl $0x1e
    pIniFile->GetFloat("Rule1", "OnlineRateLimit", .9f, &m_fOnlineRate);
    KGLOG_PROCESS_ERROR(m_fOnlineRate > 0.0f && m_fOnlineRate <= 1.0f);
    pIniFile->GetFloat("Rule1", "KillNpcRateLimit", .95f, &m_fKillNpcRate);
    KGLOG_PROCESS_ERROR(m_fKillNpcRate > 0.0f && m_fKillNpcRate <= 1.0f);
    pIniFile->GetInteger("Rule1", "KillNpcTypeNum", 3, &m_nKillNpcTypeNum);
    KGLOG_PROCESS_ERROR(m_nKillNpcTypeNum > 0 && m_nKillNpcTypeNum <= 10);
    pIniFile->GetInteger("Rule1", "RecordKillNpcNum", 600, &m_nRecordKillNpcNum);
    KGLOG_PROCESS_ERROR(m_nRecordKillNpcNum > 0);
    KGLOG_PROCESS_ERROR(m_nKillNpcTypeNum <= m_nRecordKillNpcNum);
    pIniFile->GetFloat("Rule1", "PunishRate", .8f, &m_fPunishRate);
    KGLOG_PROCESS_ERROR(m_fPunishRate >= 0.0f && m_fPunishRate <= 1.0f);
    // Target calls the white list here (082083ae), between the PunishRate guard
    // and the [Rule3] block - not last.
    KGLOG_PROCESS_ERROR(LoadNpcTemplateIDWiteList());
    pIniFile->GetInteger("Rule3", "SameBehaviorDistanceRange", 384,
        &m_nDetectSameBehaviorDistanceRange);
    KGLOG_PROCESS_ERROR(m_nDetectSameBehaviorDistanceRange >= 0);
    // "AroundSameBehaviorThresholdt": the trailing 't' is in the target string
    // at 0840cde4 and in the shipped [Rule3] section. Do not "correct" it.
    pIniFile->GetInteger("Rule3", "AroundSameBehaviorThresholdt", 5,
        &m_nDetectAroundSameBehaviorThreshold);
    KGLOG_PROCESS_ERROR(m_nDetectAroundSameBehaviorThreshold >= 1);
    pIniFile->GetInteger("Rule3", "ProcessSpeed", 5, &m_nDetectProcessSpeed);
    KGLOG_PROCESS_ERROR(m_nDetectProcessSpeed > 0);
    pIniFile->GetInteger("Rule3", "PunishFreezeFarmerNum", 10,
        &m_nPunishFreezeFarmerNum);
    KGLOG_PROCESS_ERROR(m_nPunishFreezeFarmerNum > 0);
    pIniFile->GetFloat("Rule3", "EquipSimilarityThreshold", .9f,
        &m_fEquipSimilarityThreshold);
    KGLOG_PROCESS_ERROR(m_fEquipSimilarityThreshold >= 0.5f &&
        m_fEquipSimilarityThreshold <= 1.0f);
    // Target reads [Rule3] Enable into m_bEnableIPRule with GetBool and applies
    // no guard at all (0820866c), then falls straight into bResult = 1.
    pIniFile->GetBool("Rule3", "Enable", (int*)&m_bEnableIPRule);
    bResult = true;
Exit0:
    if (pIniFile) pIniFile->Release();
    return bResult;
}

BOOL KAntiFarmerSettings::NeedIgnore(DWORD dwTemplateID) const
{
    return m_NpcWhiteList.find(static_cast<unsigned long>(dwTemplateID)) != m_NpcWhiteList.end();
}
