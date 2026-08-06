#include "stdafx.h"
#include "Global.h"
#include "KGWServerConstList.h"
BOOL KGWServerConstList::UnInit() { return true; }
BOOL KGWServerConstList::LoadData(IIniFile* pIniFile)
{
    /* Target KGWServerConstList::LoadData@082d4926, whole body.
       Three drifts fixed against it:
         1. section names. The first seven keys are in "PLAYER_EXT_OPERATION"
            (.rodata 0842bb1a); DungeonTradeItemDelayTime,
            TimeLimitReturnItemDelayTime and TimeLimitSoldListDelayTime are in
            "ITEM" (0842bbfb). The candidate had "ServerConst" for all ten,
            which appears nowhere in the target.
         2. failure policy. The target NEVER tests a GetInteger/GetMultiInteger
            return value - there is no cmpl and no branch after any of the ten
            calls - and ends with `movl $1, %eax` at 082d4b79, i.e. it always
            returns true and a missing key simply keeps the default. The
            candidate wrapped every call in KGLOG_PROCESS_ERROR, so one absent
            key aborted the whole settings load.
         3. defaults. DungeonTradeItemDelayTime and TimeLimitReturnItemDelayTime
            default to 0x708 == 1800 (082d4af1, 082d4b25), not 0.
       The memset of the 6-int array before GetMultiInteger is at 082d4ad8. */
    assert(pIniFile);

    pIniFile->GetInteger("PLAYER_EXT_OPERATION", "FreeThreeDayWorkFlag",             0,  (int*)&bFreeThreeDayWorkFlag);
    pIniFile->GetInteger("PLAYER_EXT_OPERATION", "FreeThreeDayFlagExtIndex",         5,  &nFreeThreeDayFlagExtIndex);
    pIniFile->GetInteger("PLAYER_EXT_OPERATION", "FreeThreeDayFlagExtBitIndex",      13, &nFreeThreeDayFlagExtBitIndex);
    pIniFile->GetInteger("PLAYER_EXT_OPERATION", "FreeThreeDayFlagExtBitLength",     1,  &nFreeThreeDayFlagExtBitLength);
    pIniFile->GetInteger("PLAYER_EXT_OPERATION", "FreeThreeDayRechargeExtIndex",     9,  &nFreeThreeDayRechargeExtIndex);
    pIniFile->GetInteger("PLAYER_EXT_OPERATION", "FreeThreeDayRechargeExtBitIndex",  0,  &nFreeThreeDayRechargeExtBitIndex);
    pIniFile->GetInteger("PLAYER_EXT_OPERATION", "FreeThreeDayRechargeExtBitLength", 32, &nFreeThreeDayRechargeExtBitLength);

    memset(nTimeLimitSoldListDelayTime, 0, sizeof(nTimeLimitSoldListDelayTime));

    pIniFile->GetInteger("ITEM", "DungeonTradeItemDelayTime",    1800, (int*)&nDungeonTradeItemDelayTime);
    pIniFile->GetInteger("ITEM", "TimeLimitReturnItemDelayTime", 1800, &nTimeLimitReturnItemDelayTime);
    pIniFile->GetMultiInteger("ITEM", "TimeLimitSoldListDelayTime", (int*)nTimeLimitSoldListDelayTime, 6);

    return true;
}
BOOL KGWServerConstList::Init(const char* pszFileName)
{
    /* PORT-DEFERRED_WIRING[CALLER] - target path building deliberately NOT applied.
       Evidence is complete, not missing: the target body is fully recovered below.
       What is deferred is the CALLER, which lives outside this ticket's allowlist.
       Target KGWServerConstList::Init @082d4b80 builds the path itself:
         082d4bdc  pushes "settings" (.rodata 0842bc6e)
         082d4be4  pushes "%s/%s"    (.rodata 0842bc77)
         082d4bfd  calll snprintf into a 260-byte stack buffer, then
                   KGLOG_PROCESS_ERROR(nRetCode > 0 && nRetCode < sizeof(buf))
         082d4c5f  g_OpenIniFile(szFilePath)
       so the target's parameter is a BARE filename.  Its caller confirms this:
       KWorldSettings::Init @081b2431 loads g_pSO3World and passes
       `g_pSO3World + 0x68780`, which DWARF names KSO3World::m_szServerConstListByVer
       (char[260], KSO3World.h:450) - a runtime-configured, version-selectable name.

       Why this is not implemented here: the only live caller in this tree,
       KWorldSettings.cpp:162, passes the already-prefixed literal
       SETTING_DIR "/ServerConstListByVer.ini".  Adding the snprintf would produce
       "settings/settings/ServerConstListByVer.ini", g_OpenIniFile would return
       NULL, and KWorldSettings::Init would abort - i.e. it would break the very
       login -> loading journey this ticket exists to unblock.  KWorldSettings.cpp
       is outside allowed_write_paths for w1-dataload-6file-fix-20260807, and the
       candidate KSO3World has no m_szServerConstListByVer member at all
       (verified absent with positive controls against KSO3World.h).
       Existing proven control flow is therefore preserved unchanged.
       owner=ticket w1-dataload-6file-fix-20260807 -> follow-up (caller + KSO3World member)
       resolution_phase=PRE_BUILD  marker_scope=EXTERNAL_WIRING  root_behavior_impact=NO
       unblock_condition=KSO3World declares m_szServerConstListByVer (char[260]) AND
         KWorldSettings.cpp:162 passes a bare filename; at that point restore the
         snprintf("%s/%s","settings",szFileName) + retcode guard above g_OpenIniFile. */
    BOOL bResult = false; IIniFile* pIniFile = NULL;
    KGLOG_PROCESS_ERROR(pszFileName); pIniFile = g_OpenIniFile(pszFileName); KGLOG_PROCESS_ERROR(pIniFile);
    bResult = LoadData(pIniFile);
Exit0: if (pIniFile) pIniFile->Release(); return bResult;
}
