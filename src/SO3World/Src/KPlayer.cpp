#include "stdafx.h"
#include "KPlayer.h"
#include "KScene.h"
#include "KAttrModifier.h"
#include "KSkill.h"
#include "KTradingBox.h"
#include "KRecipe.h"
#include "KCraft.h"
#include "KRecipeMaster.h"
#include "KSO3World.h"
#include "KMentorDef.h"

#ifdef _CLIENT
#include "SO3Represent/SO3GameWorldRepresentHandler.h"
#include "SO3UI/SO3GameWorldUIHandler.h"
#include "KPlayerClient.h"
#endif

#ifdef _SERVER
#include "KRoleDBDataDef.h"
#include "KPlayerServer.h"
#include "KRelayClient.h"
#include "Common/CRC32.h"

#pragma pack(1)
struct KARENA_ROLE_DATA
{
    time_t nCorpsChangeTime;
    time_t nCorpsWeekTime;
    time_t nCorpsSeasonTime;
    int    nCorpsLevel[3];
    int    nCorpsRoleLevel[3];
    BYTE   byReserved[30];
};
#pragma pack()
typedef char KARENA_ROLE_DATA_SIZE_CHECK[
    (sizeof(KARENA_ROLE_DATA) == 0x42) ? 1 : -1
];
#endif

#ifdef _CLIENT
BYTE     KPlayer::s_byTalkData[];
unsigned KPlayer::s_uTalkDataSize = 0;
BOOL     KPlayer::s_bFilterTalkText = false;
#endif

void DumpData(char* szFileName, BYTE* pbyData, size_t uDataLen)
{
    int nRetCode = false;
    FILE* fpDumpFile = NULL;
    time_t  nTimeNow = time(NULL);
    char szFullFileName[MAX_PATH];
    struct tm tmNow; 

    localtime_r(&nTimeNow, &tmNow);

    nRetCode = (int)snprintf(
        szFullFileName, sizeof(szFullFileName),
        "dumpdata/%s_%d%2.2d%2.2d_%2.2d_%2.2d_%2.2d",
        szFileName,
        tmNow.tm_year + 1900,
        tmNow.tm_mon + 1,
        tmNow.tm_mday,
        tmNow.tm_hour,
        tmNow.tm_min,
        tmNow.tm_sec
    );
    KGLOG_PROCESS_ERROR(nRetCode > 0 && nRetCode < (int)sizeof(szFullFileName));

    nRetCode = KG_mkdir("dumpdata");
    KGLOG_PROCESS_ERROR((nRetCode != -1) || (errno == EEXIST));

    fpDumpFile = fopen(szFullFileName, "wb");
    KGLOG_PROCESS_ERROR(fpDumpFile);

    nRetCode = (int)fwrite(pbyData, uDataLen, 1, fpDumpFile);
    KGLOG_PROCESS_ERROR(nRetCode == 1);

Exit0:
    if (fpDumpFile)
    {
        fclose(fpDumpFile);
        fpDumpFile = NULL;
    }
    return;
}

KPlayer::KPlayer()
{
    m_dwTeamID              = ERROR_ID;
    m_dwTongID              = ERROR_ID;
    m_nContribution         = 0;
    m_pTradingBox           = NULL;
    m_nMaxLevel             = 0;
    m_bHideHat              = false;
    m_nCoin                 = 0;
    m_nMaxApprenticeNum     = MIN_APPRENTICE_NUM;
    m_nUsableMentorValue    = 0;
    m_nAcquiredMentorValue  = 0;
    m_nLastEvokeMentorTime  = 0;
    m_nEvokeMentorCount     = 0;
    m_dwTAEquipsScore       = 0;
}

KPlayer::~KPlayer()
{
    assert(!m_pTradingBox);
    delete[] m_pbyExtDataBuffer;
}

BOOL KPlayer::AddUsableMentorValue(int nDeltaMentorValue)
{
    BOOL        bResult      = false;
    int         nOldValue    = m_nUsableMentorValue;
    long long   nNewValue    = (long long)m_nUsableMentorValue + nDeltaMentorValue;
    int         nMaxValue    = g_pSO3World->m_Settings.m_ConstList.nMaxUsableMentorValue;

    KGLOG_PROCESS_ERROR(nNewValue >= 0);
    if (nNewValue > nMaxValue)
    {
        nNewValue = nMaxValue;
    }

    m_nUsableMentorValue = (int)nNewValue;
    bResult = true;
    if (m_nUsableMentorValue != nOldValue)
    {
        g_pSO3World->m_ScriptServer.Reset();
        g_pSO3World->m_ScriptServer.PushString("USABLE_MVALUE");
        g_pSO3World->m_ScriptServer.PushNumber(m_nUsableMentorValue);
        g_pSO3World->m_ScriptServer.Call(m_nConnIndex, "OnSyncMentorData");
    }
Exit0:
    return bResult;
}

BOOL KPlayer::Init(void)
{
    BOOL bResult                    = false;
	BOOL bRetCode                   = false;
    BOOL bCharacterInitFlag         = false;
    BOOL bQuestListInitFlag         = false;
    BOOL bTimerListInitFlag         = false;
    BOOL bItemListInitFlag          = false;
    BOOL bUserPreferencesInitFlag   = false;
    BOOL bBookListInitFlag          = false;
    BOOL bPKInitFlag                = false;
    BOOL bScriptTimerListInitFlag   = false;
    BOOL bSkillRecipeListInitFlag   = false;
    BOOL bProfessionListInitFlag    = false;
    BOOL bRecipeListInitFlag        = false;
    BOOL bDesignationFlag           = false;
    BOOL bExteriorBoxFlag           = false;
    BOOL bQuestRandInitFlag         = false;
    BOOL bVenationRandInitFlag      = false;

	bRetCode = KCharacter::Init();
	KGLOG_PROCESS_ERROR(bRetCode);
    bCharacterInitFlag = true;

	bRetCode = m_QuestList.Init(this);
	KGLOG_PROCESS_ERROR(bRetCode);
    bQuestListInitFlag = true;

    bRetCode = m_TimerList.Init(this);
	KGLOG_PROCESS_ERROR(bRetCode);
    bTimerListInitFlag = true;

	bRetCode = m_ItemList.Init(this);
	KGLOG_PROCESS_ERROR(bRetCode);
    bItemListInitFlag = true;

	bRetCode = m_UserPreferences.Init(this);
	KGLOG_PROCESS_ERROR(bRetCode);
    bUserPreferencesInitFlag = true;

	bRetCode = m_BookList.Init(this);
	KGLOG_PROCESS_ERROR(bRetCode);
    bBookListInitFlag = true;

	bRetCode = m_PK.Init(this);
	KGLOG_PROCESS_ERROR(bRetCode);
    bPKInitFlag = true;

    bRetCode = m_Designation.Init(this);
    KGLOG_PROCESS_ERROR(bRetCode);
    bDesignationFlag = true;

    // v2.5 NEW: capped/periodic currencies (infallible init -> no rollback flag).
    m_CurrencyList.Init(this);
    bRetCode = m_CampActiveStat.Init(this);
    KGLOG_PROCESS_ERROR(bRetCode);

    m_nWaistPendentBoxSize = 0;
    m_nBackPendentBoxSize = 0;
    m_nFacePendentBoxSize = 0;
    m_WaistPendent.clear();
    m_BackPendent.clear();
    m_FacePendent.clear();
    m_dwWaistItemIndex = 0;
    m_dwBackItemIndex = 0;
    m_dwFaceItemIndex = 0;
    bRetCode = m_FellowPetBox.Init(this);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_NewExtPointManager.Init(this);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_ExteriorBox.Init(this);
    KGLOG_PROCESS_ERROR(bRetCode);
    bExteriorBoxFlag = true;

    bRetCode = m_HairBox.Init(this);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_MiniAvatar.Init(this);
    KGLOG_PROCESS_ERROR(bRetCode);
    m_dwMiniAvatarID = 0;
    m_dwSingleDungeonMaxLevel = 0;
    memset(m_dwSingleDungeonScore, 0, sizeof(m_dwSingleDungeonScore));
    memset(m_dwSingleDungeonCustomData, 0, sizeof(m_dwSingleDungeonCustomData));
    m_dwCorpsSystemID = 0;
    m_nCorpsChangeTime = 0;
    m_nCorpsWeekTime = 0;
    m_nCorpsSeasonTime = 0;
    memset(m_nCorpsLevel, 0, sizeof(m_nCorpsLevel));
    memset(m_nCorpsRoleLevel, 0, sizeof(m_nCorpsRoleLevel));

    bRetCode = m_RegressionData.Init(this);
    KGLOG_PROCESS_ERROR(bRetCode);

	m_eKind                 = ckPlayer;

    memset(&m_MoveCtrl, 0, sizeof(m_MoveCtrl));

#ifdef _CLIENT
    m_szTongName[0] = '\0';
#endif

#ifdef _SERVER
	m_nConnIndex            = -1;
	m_uSyncPlayerSN         = 0;
	m_uSyncNpcSN            = 0;
	m_uSyncDoodadSN         = 0;
	memset(m_nSyncPlayerCount, 0, sizeof(m_nSyncPlayerCount));
	m_nSyncNpcCount         = 0;
	m_nSyncDoodadCount      = 0;
	m_bExtDataLoadFinish    = false;
	m_uExtDataSectionIndex  = 0;
	m_byCurrentExtDataSectionType = 0;
	m_pbyExtDataBuffer      = NULL;
	m_uExtDataSize          = 0;
	m_uExtDataOffset        = 0;
	m_nTimer                = 0;
    m_bExtPointLock         = false;     
    m_nLastExtPointIndex    = 0;
    m_nLastExtPointValue    = 0;
    m_nEndTimeOfFee         = 0;
    m_nLastClientFrame      = 0;
    m_nNextSaveFrame        = 0;
	m_dwRoundRollCount      = 0;
    m_dwLastScriptID        = 0;
    m_nBanishTime           = 0;
    m_nLastLoginTime        = 0;
    m_nCurrentLoginTime     = 0;
    m_nCreateTime           = 0;
    m_nAccountLastSaveTime  = 0;
    m_nAccContinuousLoginCount = 0;
    m_bContinuousLoginRewardFlag = false;
    m_nTotalGameFrame       = 0;
    m_nNoFeeTime            = 0;

    memset(&m_LastEntry, 0, sizeof(m_LastEntry));

    m_nExpPercent           = 0;
    m_nReputationPercent    = 0;

    memset(&m_DelayedSwitchMapParam, 0, sizeof(m_DelayedSwitchMapParam));

    m_szClientIP[0]         = '\0';
#endif
    m_bChargeFlag           = false;
    m_bFreeLimitFlag        = false;
    m_bFarmerLimit          = false;

	m_eCamp                 = cNeutral;
    m_bCampFlag             = false;
    m_nCloseCampFlagTime    = 0;
    m_nCurrentPrestige      = 0;
#ifdef _SERVER
    m_nKilledCount          = 0;
    m_nNextResetKilledCountTime = 0;

    m_nClientCampInfoVersion = -1;
#endif
    m_dwSystemTeamID        = ERROR_ID;
    m_dwTeamID              = ERROR_ID;
	m_eGameStatus           = gsInvalid;

	m_eRoleType	            = rtInvalid;
    m_bFightState           = false;
    m_nLevel                = 0;

	m_nExperience	        = 0;

	m_dwCubPackageNpcID	= ERROR_ID;
	m_dwBankNpcID			= ERROR_ID;
    memset(m_szBankPassword, 0, sizeof(m_szBankPassword));
    memset(m_szBankPasswordAnswer, 0, sizeof(m_szBankPasswordAnswer));
    m_nBankPasswordResetEndTime = 0;
    m_bIsBankPasswordVerified = false;
    m_bBankPasswordExist = false;
    m_nBankPasswordQuestionID = 0;
    m_dwSafeLockMask = 0;
    m_dwTongRepertoryNpcID  = ERROR_ID;

    m_pTradingBox           = NULL;
    m_dwCubPackageSize     = 0;
    m_dwTradingInviteDst    = ERROR_ID;
    m_dwTradingInviteSrc    = ERROR_ID;

    m_bOnHorse              = false;

    m_bOnPracticeRoom           = false;     
    m_nCurrentTrainValue        = 0;
    m_nMaxTrainValue            = 0;
    m_nUsedTrainValue           = 0;
#ifdef _SERVER
    m_nAddTrainTimeInToday      = 0;
#endif
    m_nBaseVenationCof          = 0;
    m_nPhysicsVenationCof       = 0;
    m_nMagicVenationCof         = 0;
    m_nAssistVenationCof        = 0;

#ifdef _SERVER
    memset(m_nBuffDecayCount, 0, sizeof(m_nBuffDecayCount));
    memset(m_nBuffDecayNextFrame, 0, sizeof(m_nBuffDecayNextFrame));
#endif

    m_SkillRecipeList.Clear();

#ifdef _CLIENT
	m_eRadarType				    = mrtNoRadar;
	m_nRadarParam				    = 0;
#endif
	m_nCurrentStamina		        = INT_MAX / 2;
    m_nMaxStamina                   = 0;
    m_nCurrentThew		            = INT_MAX / 2;
    m_nMaxThew                      = 0;

#ifdef _SERVER
	m_nLastSaveTime                 = 0;
	
    bRetCode = m_ScriptTimerList.Init(this);
    KGLOG_PROCESS_ERROR(bRetCode);
    bScriptTimerListInitFlag = true;
#endif

	memset(m_wRepresentId, 0, sizeof(m_wRepresentId));
    m_dwRepresentIdLock = 0;
    m_dwApplyExteriorFlag = 0;

    m_dwSchoolID = 0;

    m_SkillList.Init(this);

    bRetCode = m_SkillRecipeList.Init(this);
    KGLOG_PROCESS_ERROR(bRetCode);
    bSkillRecipeListInitFlag = true;

	bRetCode = m_ProfessionList.Init(this);
	KGLOG_PROCESS_ERROR(bRetCode);
    bProfessionListInitFlag = true;

	bRetCode = m_RecipeList.Init(this);
	KGLOG_PROCESS_ERROR(bRetCode);
    bRecipeListInitFlag = true;

	bRetCode = m_ReputeList.Init(this);
	KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_Achievement.Init(this);
    KGLOG_PROCESS_ERROR(bRetCode);

#ifdef _SERVER
    m_dwKillerID                            = ERROR_ID;         
    m_nVirtualFrame                         = g_pSO3World->m_nGameLoop;   
    m_nRecordCount                          = 0;
	m_nThreatTime			                = PLAYER_THREAT_TIME;

	m_nLastSyncSelfMaxLife                  = -1;
    m_nLastSyncSelfMaxMana                  = -1; 
    m_nLastSyncSelfMaxRage                  = -1; 
    m_nLastSyncSelfMaxStamina               = -1;
	m_nLastSyncSelfCurrentLife              = -1;
    m_nLastSyncSelfCurrentMana              = -1; 
    m_nLastSyncSelfCurrentRage              = -1; 
    m_nLastSyncSelfCurrentStamina           = -1;
    m_nLastSyncSelfCurrentThew              = -1;
    m_nLastSyncSelfUpperWeak                = -1;
    m_nLastSyncSelfMiddleWeak               = -1;
    m_nLastSyncSelfLowerWeak                = -1;

	m_nLastSyncTargetMaxLife                = -1;
    m_nLastSyncTargetMaxMana                = -1; 
    m_nLastSyncTargetMaxRage                = -1; 
	m_nLastSyncTargetLifePercent            = -1;
    m_nLastSyncTargetManaPercent            = -1;
    m_nLastSyncTargetRagePercent            = -1;
    m_nLastSyncTargetUpperWeak              = -1;
    m_nLastSyncTargetMiddleWeak             = -1;
    m_nLastSyncTargetLowerWeak              = -1;

    m_dwLastSyncTargetTargetID              = ERROR_ID;
    m_nLastSyncTargetTargetMaxLife          = -1;
    m_nLastSyncTargetTargetMaxMana          = -1;
    m_nLastSyncTargetTargetMaxRage          = -1;
    m_nLastSyncTargetTargetLifePercent      = -1;
    m_nLastSyncTargetTargetManaPercent      = -1;
    m_nLastSyncTargetTargetRagePercent      = -1;

    m_dwTargetDropID                        = ERROR_ID;
    memset(m_byDropSurpriseMask, 0, sizeof(m_byDropSurpriseMask));
    memset(m_wPresentCodeCounters, 0, sizeof(m_wPresentCodeCounters));
    m_dwTargetTargetBuffCRC                 = 0;
	m_dwTargetBuffCRC                       = 0;

    ResetTeamLastSyncParamRecord();
#endif

#ifdef _CLIENT
    m_dwFormationEffectID                   = 0;
    m_nFormationEffectLevel                 = 0;

    m_dwMentorFormationEffectID             = 0;
    m_nMentorFormationEffectLevel           = 0;
#endif

    memset(&m_ReviveCtrl, 0, sizeof(m_ReviveCtrl));

    m_OpenRouteNodeList.clear();

    m_nCurrentKillPoint                     = 0;
#ifdef _SERVER
    m_nBanTime                              = 0;
    m_nNextKillPointReduceTime              = 0;
    m_bAbradeEquipmentDouble                = false;
    m_bAbradeEquipmentAll                   = false;
    m_bAttackByGuard                        = false;

    m_nTalkWorldDailyCount                  = 0;
    m_nTalkForceDailyCount                  = 0;
    m_nTalkCampDailyCount                   = 0;
#endif
    m_bOnlyReviveInSitu                     = false;            
    m_bCannotDialogWithNPC                  = false;
    m_bRedName                              = false;

    m_bHeroFlag                             = false;

    m_nVitalityToParryValueCof              = 0;
    m_nVitalityToMaxLifeCof                 = 0;
    m_nVitalityToPhysicsAttackPowerCof      = 0;
    m_nVitalityToSolarAttackPowerCof        = 0;
    m_nVitalityToLunarAttackPowerCof        = 0;
    m_nVitalityToNeutralAttackPowerCof      = 0;
    m_nVitalityToPoisonAttackPowerCof       = 0;
    m_nVitalityToTherapyPowerCof            = 0;
    m_nVitalityToPhysicsShieldCof           = 0;
    m_nSpiritToMaxManaCof                   = 0;
    m_nSpiritToSolarCriticalStrikeCof       = 0;
    m_nSpiritToLunarCriticalStrikeCof       = 0;
    m_nSpiritToNeutralCriticalStrikeCof     = 0;
    m_nSpiritToPoisonCriticalStrikeCof      = 0;
    m_nSpiritToPhysicsAttackPowerCof        = 0;
    m_nSpiritToPhysicsCriticalStrikeCof     = 0;
    m_nSpiritToManaReplenishCof             = 0;
    m_nSpiritToSolarMagicShieldCof          = 0;
    m_nSpiritToLunarMagicShieldCof          = 0;
    m_nSpiritToNeutralMagicShieldCof        = 0;
    m_nSpiritToPoisonMagicShieldCof         = 0;
    m_nSpunkToManaReplenishCof              = 0;
    m_nSpunkToSolarAttackPowerCof           = 0;
    m_nSpunkToLunarAttackPowerCof           = 0;
    m_nSpunkToNeutralAttackPowerCof         = 0;
    m_nSpunkToPoisonAttackPowerCof          = 0;
    m_nStrengthToPhysicsCriticalStrikeCof   = 0;
    m_nStrengthToPhysicsAttackPowerCof      = 0;
    m_nAgilityToPhysicsCriticalStrikeCof    = 0;

#ifdef _SERVER
    m_ViewPoint.pPlayer         = this;
    m_ViewPoint.pRegion         = NULL;
    m_ViewPoint.nDisappearFrame = 0;
#else
    m_pViewPointRegion          = NULL;
#endif

#ifdef _SERVER
    bRetCode = m_QuestRand.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bQuestRandInitFlag = true;

    bRetCode = m_VenationRand.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bVenationRandInitFlag = true;
#endif

	bResult = true;
Exit0:
    if (!bResult)
    {
#ifdef _SERVER
        if (bVenationRandInitFlag)
        {
            m_VenationRand.UnInit();
            bVenationRandInitFlag = false;
        }

        if (bQuestRandInitFlag)
        {
            m_QuestRand.UnInit();
            bQuestRandInitFlag = false;
        }
#endif
        if (bRecipeListInitFlag)
        {
            m_RecipeList.UnInit();
            bRecipeListInitFlag = false;
        }

        if (bProfessionListInitFlag)
        {
            m_ProfessionList.UnInit();
            bProfessionListInitFlag = false;
        }

        if (bSkillRecipeListInitFlag)
        {
            m_SkillRecipeList.UnInit();
            bSkillRecipeListInitFlag = false;
        }

#ifdef _SERVER
        if (bScriptTimerListInitFlag)
        {
            m_ScriptTimerList.UnInit();
            bScriptTimerListInitFlag = false;
        }
#endif

        if (bExteriorBoxFlag)
        {
            m_ExteriorBox.UnInit();
            bExteriorBoxFlag = false;
        }

        if(bDesignationFlag)
        {
            m_Designation.UnInit();
            bDesignationFlag = false;
        }

        if (bPKInitFlag)
        {
            m_PK.UnInit();
            bPKInitFlag = false;
        }

        if (bBookListInitFlag)
        {
            m_BookList.UnInit();
            bBookListInitFlag = false;
        }

        if (bUserPreferencesInitFlag)
        {
            m_UserPreferences.UnInit();
            bUserPreferencesInitFlag = false;
        }

        if (bItemListInitFlag)
        {
            m_ItemList.UnInit();
            bItemListInitFlag = false;
        }

        if (bTimerListInitFlag)
        {
            m_TimerList.UnInit();
            bTimerListInitFlag = false;
        }

        if (bQuestListInitFlag)
        {
            m_QuestList.UnInit();
            bQuestListInitFlag = false;
        }

        if (bCharacterInitFlag)
        {
            KCharacter::UnInit();
            bCharacterInitFlag = false;
        }
    }
	return bResult;
}

void KPlayer::UnInit(void)
{
    m_NewExtPointManager.UnInit();
    m_FellowPetBox.UnInit();
    m_GraduateMentorData.clear();
    m_GraduateApprenticeData.clear();

    if (m_pTradingBox)
    {
        KPlayer*        pOtherPlayer     = NULL;
        KTradingBox*    pOtherTradingBox = NULL;

        pOtherPlayer = m_pTradingBox->GetOtherSide();
        assert(pOtherPlayer);

        pOtherTradingBox = pOtherPlayer->m_pTradingBox;
        assert(pOtherTradingBox);

        pOtherTradingBox->UnInit();
        KMemory::Delete(pOtherTradingBox);
        pOtherTradingBox = NULL;

        pOtherPlayer->m_pTradingBox = NULL;
        pOtherPlayer->m_dwTradingInviteSrc = ERROR_ID;
        pOtherPlayer->m_dwTradingInviteDst = ERROR_ID;

        m_pTradingBox->UnInit();
        KMemory::Delete(m_pTradingBox);
        m_pTradingBox = NULL;

#ifdef _SERVER
	    g_PlayerServer.DoSyncTradingConfirm(pOtherPlayer->m_nConnIndex, m_dwID, false);
#endif	//_SERVER
    }

#ifdef _SERVER
    g_pSO3World->m_FellowshipMgr.UnloadPlayerFellowship(m_dwID);

	m_ScriptTimerList.UnInit();

    if (m_ViewPoint.pRegion != NULL)    // �۲���Ƴ�
    {
        m_ViewPoint.Remove();
        m_ViewPoint.pPlayer = NULL;
		m_ViewPoint.pRegion = NULL;
    }

    m_QuestRand.UnInit();
    m_VenationRand.UnInit();
#else
	m_pViewPointRegion = NULL;
#endif

    if (m_dwTradingInviteSrc)
    {
        KPlayer* pInviteSrc = g_pSO3World->m_PlayerSet.GetObj(m_dwTradingInviteSrc);
        if (pInviteSrc)
        {
            pInviteSrc->m_dwTradingInviteDst = ERROR_ID;
        }
        m_dwTradingInviteSrc = ERROR_ID;
    }

    if (m_dwTradingInviteDst)
    {
        KPlayer* pInviteDst = g_pSO3World->m_PlayerSet.GetObj(m_dwTradingInviteDst);
        if (pInviteDst)
        {
            pInviteDst->m_dwTradingInviteSrc = ERROR_ID;
        }
        m_dwTradingInviteDst = ERROR_ID;
    }

    m_SkillList.UnInit();
    m_SkillRecipeList.UnInit();

	m_RecipeList.UnInit();
	m_ProfessionList.UnInit();
    m_Designation.UnInit();
    m_CurrencyList.UnInit();
    m_CampActiveStat.UnInit();
    m_ExteriorBox.UnInit();
    m_PK.UnInit();
	m_BookList.UnInit();
	m_UserPreferences.UnInit();
	m_ItemList.UnInit();
    m_TimerList.UnInit();
	KCharacter::UnInit();

	return;
}

BOOL KPlayer::Activate(void)
{
    BOOL bResult  = false;
	BOOL bRetCode = false;

	assert(m_pRegion);

	bRetCode = KSceneObject::CheckGameLoop();
    KG_PROCESS_SUCCESS(!bRetCode);

    KG_PROCESS_ERROR(m_eGameStatus == gsPlaying || m_eGameStatus == gsDeleting);

#ifdef _SERVER
    ++m_nTotalGameFrame;

    bRetCode = m_pScene->ValidateRegions(m_pRegion->m_nRegionX, m_pRegion->m_nRegionY);
    KGLOG_PROCESS_ERROR(bRetCode);

    if (m_ViewPoint.pRegion)
    {
        bRetCode = m_pScene->ValidateRegions(m_ViewPoint.pRegion->m_nRegionX, m_ViewPoint.pRegion->m_nRegionY);
        KGLOG_PROCESS_ERROR(bRetCode);
    }
#else
    if (m_dwID == g_pSO3World->m_dwClientPlayerID)
    {
        bRetCode = m_pScene->ValidateRegions(m_pRegion->m_nRegionX, m_pRegion->m_nRegionY);
        KGLOG_PROCESS_ERROR(bRetCode);

        if (m_pViewPointRegion)
        {
            bRetCode = m_pScene->ValidateRegions(m_pViewPointRegion->m_nRegionX, m_pViewPointRegion->m_nRegionY);
            KGLOG_PROCESS_ERROR(bRetCode);
        }
    }
#endif

#ifdef _SERVER
	SyncObjectView();

    if (m_ViewPoint.pRegion && m_ViewPoint.nDisappearFrame <= g_pSO3World->m_nGameLoop)
    {
        m_ViewPoint.Remove();
        m_ViewPoint.pRegion 		= NULL;
        m_ViewPoint.nDisappearFrame = 0;
		
        g_PlayerServer.DoRemoveViewPointRespond(this);
    }

    DoCycleSynchronous();

    if ((g_pSO3World->m_nGameLoop - m_dwID) % (GAME_FPS / 2) == 0)
    {
        m_PK.Activate();
    }
#endif

	// ����״̬�ļ��
	if (m_dwBankNpcID && m_pCell)
	{
		KNpc* pNpc = g_pSO3World->m_NpcSet.GetObj(m_dwBankNpcID);
		if (pNpc)
		{
			//������,�����˾͹ر�����
            bRetCode = g_InRange(this, pNpc, COMMON_PLAYER_OPERATION_DISTANCE);
			if (!bRetCode)
			{
				m_dwBankNpcID = ERROR_ID;
                m_ItemList.m_bBankOpened = false;
			}
		}
		else
		{
			m_dwBankNpcID = ERROR_ID;
            m_ItemList.m_bBankOpened = false;
		}
	}

#ifdef _SERVER
	// ������ѭ���������ʱ����
	m_QuestList.Activate();

	// ��ʱ��ѭ��
	m_ScriptTimerList.Activate();

    if ((g_pSO3World->m_nGameLoop - m_dwID) % (GAME_FPS * 10) == 0)
    {
        SyncFormationCoefficient();

        // v2.5: prune expired timed designations (low-frequency; a no-op while no
        // timed designation is granted, which is the case with the current .tab data).
        m_Designation.Activate();

        // v2.5: periodic (weekly) currency earn-allowance reset. Guarded no-op while the
        // currency config is dormant (reset period unconfigured). See docs/currency_port.
        m_CurrencyList.Activate();
    }
#endif //_SERVER

#ifdef _CLIENT
	if (m_dwID == g_pSO3World->m_dwClientPlayerID)
	{
		m_QuestList.Activate();
	}
#endif //_CLIENT


#ifdef _SERVER
    RecordPosition(&m_PositionRecord[m_nVirtualFrame % MAX_POSITION_RECORD]);

    m_nVirtualFrame++;

    if (m_nRecordCount < MAX_POSITION_RECORD)
    {
        m_nRecordCount++;
    }
#endif

	KCharacter::Activate();

    MoveCtrl();

    MAKE_IN_RANGE(m_nCurrentStamina, 0, m_nMaxStamina);
    MAKE_IN_RANGE(m_nCurrentThew, 0, m_nMaxThew);

    // m_pSceneָ��Ϊ�ձ�ʾ�Լ���ɾ����
    KG_PROCESS_ERROR(m_pScene);

#ifdef _CLIENT
    if (g_pGameWorldRepresentHandler)
        g_pGameWorldRepresentHandler->OnUpdateCharacterFrameData(this);
#endif

#ifdef _SERVER
    if ((g_pSO3World->m_nGameLoop - m_dwID) % g_pSO3World->m_Settings.m_ConstList.nTeamAttractiveInterval == 0)
    {
        ProcessTeamMemberAttraction();
        //KGLOG_CHECK_ERROR(bRetCode);
    }

    if ((g_pSO3World->m_nGameLoop - m_dwID) % g_pSO3World->m_Settings.m_ConstList.nTeamAttractiveIntervalOnlyGroup == 0)
    {
        ProcessTeamMemberOnlyGroupAttraction();
    }

    if ((!m_pScene->m_bSaved) && m_nBanishTime == 0 && (g_pSO3World->m_nGameLoop - (int)m_dwID) % (GAME_FPS * 5) == 0)
    {
        DWORD dwSceneOwner = m_pScene->m_dwOwnerID;

        if (dwSceneOwner != ERROR_ID && m_dwID != dwSceneOwner)
        {
            int nBanishDelay = BANISH_PLAYER_WAIT_SECONDS;

            if (m_dwTeamID == ERROR_ID)
            {
                m_nBanishTime = g_pSO3World->m_nCurrentTime + BANISH_PLAYER_WAIT_SECONDS;
                g_PlayerServer.DoMessageNotify(
                    m_nConnIndex, ectBanishNotifyCode, bpcNotInMapOwnerTeam,
                    &nBanishDelay, sizeof(nBanishDelay)
                );
            }
            else
            {
                // Ϊʲô���ж�Owner�ڲ����ҵĶ�������,�������ж�"���ڲ���Owner�Ķ�������"��?
                // ������ΪOwner������ʱ���ڱ�������
                // "�Ҳ���Owner�Ķ�������" ��Ч�� "Owner�����ҵĶ�������"
                bRetCode = g_pSO3World->m_TeamServer.IsPlayerInTeam(m_dwTeamID, dwSceneOwner);
                if (!bRetCode)
                {
                    m_nBanishTime = g_pSO3World->m_nCurrentTime + BANISH_PLAYER_WAIT_SECONDS;
                    g_PlayerServer.DoMessageNotify(
                        m_nConnIndex, ectBanishNotifyCode, bpcNotInMapOwnerTeam,
                        &nBanishDelay, sizeof(nBanishDelay)
                    );
                }
            }
        }
    }

    if (m_nBanishTime > 0 && g_pSO3World->m_nCurrentTime > m_nBanishTime)
    {
        SwitchMap(
            m_LastEntry.dwMapID, m_LastEntry.nMapCopyIndex, 
            m_LastEntry.nX, m_LastEntry.nY, m_LastEntry.nZ
        );

        m_nBanishTime = 0;
    }

    KG_PROCESS_ERROR(m_pScene);

    if (m_nBanishTime > 0 && (g_pSO3World->m_nGameLoop - (int)m_dwID) % (GAME_FPS * 3) == 0)
    {
        DWORD dwSceneOwner  = m_pScene->m_dwOwnerID;
        BOOL  bInOwnerTeam = false;

        if (m_dwTeamID != ERROR_ID)
        {
            bInOwnerTeam = g_pSO3World->m_TeamServer.IsPlayerInTeam(m_dwTeamID, dwSceneOwner);
        }

        if (m_dwID == dwSceneOwner || bInOwnerTeam)
        {
            m_nBanishTime = 0;
            g_PlayerServer.DoMessageNotify(m_nConnIndex, ectBanishNotifyCode, bpcCancelBanish);
        }
    }

    if (m_bOnHorse && m_pCell->m_BaseInfo.dwRideHorse)
        DownHorse();

    m_ItemList.Activate();

    if (m_eMoveState == cmsOnDeath)
        m_ReviveCtrl.nReviveFrame--;
    else
        m_ReviveCtrl.nNextCheckReviveFrame = 0;

    m_ReviveCtrl.nReviveFrame = max(m_ReviveCtrl.nReviveFrame, 0);

    if (m_ReviveCtrl.nNextCheckReviveFrame == g_pSO3World->m_nGameLoop)
        CheckRevive(false);

    if (m_DelayedSwitchMapParam.dwMapID)
    {
        RealSwitchMap(
            m_DelayedSwitchMapParam.dwMapID, m_DelayedSwitchMapParam.nMapCopyIndex, 
            m_DelayedSwitchMapParam.nX, m_DelayedSwitchMapParam.nY, m_DelayedSwitchMapParam.nZ
        );

        memset(&m_DelayedSwitchMapParam, 0, sizeof(m_DelayedSwitchMapParam));
    }

    if (
        (m_nNextResetKilledCountTime > 0 && g_pSO3World->m_nCurrentTime >= m_nNextResetKilledCountTime) || 
        (m_nNextResetKilledCountTime == 0 && m_nKilledCount != 0)
    )
    {
        m_nKilledCount              = 0;
        m_nNextResetKilledCountTime = 0;
    }

    if ((g_pSO3World->m_nGameLoop - (int)m_dwID) % GAME_FPS == 0)
    {
        CheckReduceKillPoint();
    }
#endif

#ifdef _CLIENT
    if ((m_dwID == g_pSO3World->m_dwClientPlayerID) && (g_pSO3World->m_nGameLoop % GAME_FPS == 0))
    {
        const char* pszFuncName = "Activate";
        int         nTopIndex   = 0;

        bRetCode = g_pSO3World->m_ScriptCenter.IsScriptExist(CLIENT_PLAYER_ACTIVATE_SCRIPT);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = g_pSO3World->m_ScriptCenter.IsFuncExist(CLIENT_PLAYER_ACTIVATE_SCRIPT, pszFuncName);
        KG_PROCESS_ERROR(bRetCode);

        g_pSO3World->m_ScriptCenter.SafeCallBegin(&nTopIndex);

        g_pSO3World->m_ScriptCenter.PushValueToStack(this);
        g_pSO3World->m_ScriptCenter.CallFunction(CLIENT_PLAYER_ACTIVATE_SCRIPT, pszFuncName, 0);

        g_pSO3World->m_ScriptCenter.SafeCallEnd(nTopIndex);
    }
#endif // _CLIENT

#ifdef _SERVER
     ProcessAntiFarmer();

     // ����������12���ӻظ�0.2%����&������ÿСʱ�ظ�1%��
     if ((g_pSO3World->m_nGameLoop - m_dwID) % (GAME_FPS * 12 * 60) == 0)
     {
        AddStaminaAndThew();
     }

     if (m_nCloseCampFlagTime != 0)
     {
         if (m_bFightState)
         {
            m_nCloseCampFlagTime = 0;

            g_pSO3World->m_ScriptServer.Reset();
            g_pSO3World->m_ScriptServer.PushBool(true);
            g_pSO3World->m_ScriptServer.PushNumber(0);
            g_pSO3World->m_ScriptServer.Call(m_nConnIndex, "CloseCampFlagResult");
         }
         else if (g_pSO3World->m_nCurrentTime > m_nCloseCampFlagTime)
         {
            SetCampFlag(false);
            m_nCloseCampFlagTime = 0;
         }
     }
#endif

Exit1:
	bResult = true;
Exit0:
	return bResult;
}

BOOL KPlayer::OpenDoodad(KDoodad* pDoodad)
{
    BOOL             bResult            = false;
	BOOL             bRetCode           = false;
    KDoodadTemplate* pDoodadTemplate    = NULL;
    int              nOpenFrames        = 0;

    assert(pDoodad);

#ifdef _SERVER
	bRetCode = pDoodad->CheckOpen(this);
	KG_PROCESS_ERROR(bRetCode);
#endif

    KGLOG_PROCESS_ERROR(pDoodad->m_pTemplate);
    pDoodadTemplate = pDoodad->m_pTemplate;

    nOpenFrames = pDoodad->GetOpenFrame(this);
    bRetCode    = DoPickPrepare(pDoodad, nOpenFrames);
    KG_PROCESS_SUCCESS(!bRetCode); // Ϊfalse��ʾOTAction���ܸ���,ֱ�ӷ���

#ifdef _SERVER
    if (nOpenFrames > 0 && m_bOnHorse)
    {
        DownHorse();
    }
#else
    if (m_dwID == g_pSO3World->m_dwClientPlayerID)
    {
        if (g_pGameWorldUIHandler)
        {
            KUIEventPickProgress param;
            param.nTotalFrame = nOpenFrames;
            param.dwDoodadID = pDoodad->m_dwID;
            g_pGameWorldUIHandler->OnPickPrepareProgress(param);
        }
    }
#endif

#ifdef _SERVER
	g_PlayerServer.DoCharacterOpen(this, pDoodad);
    pDoodad->m_dwOpenPlayerID = m_dwID;

    if (pDoodad->m_eKind != dkDoor)
        pDoodad->ChangeState(dsBeHit);
#endif

#ifdef _CLIENT
	if (g_pGameWorldRepresentHandler)
        g_pGameWorldRepresentHandler->OnCharacterOpenDoodad(this, pDoodad);
#endif

Exit1:
	bResult = true;
Exit0:
	return bResult;
}

void KPlayer::MoveCtrl()
{
    KG_PROCESS_ERROR(m_pScene);

#ifdef _SERVER
    if (m_MoveCtrl.bUpdate)
    {
        g_PlayerServer.DoSyncMoveCtrl(this);
        m_MoveCtrl.bUpdate = false;
    }
#endif

    KG_PROCESS_ERROR(!m_MoveCtrl.bDisable);

    if (m_MoveCtrl.nTurn)
    {
        if (
            m_eMoveState == cmsOnStand || m_eMoveState == cmsOnWalk || m_eMoveState == cmsOnRun ||
            m_eMoveState == cmsOnJump || m_eMoveState == cmsOnSwimJump || m_eMoveState == cmsOnSwim ||
            m_eMoveState == cmsOnFloat || m_eMoveState == cmsOnSit
        )
        {
            m_nFaceDirection += m_MoveCtrl.nTurn * DIRECTION_TURN_SPEED;
            m_nFaceDirection = m_nFaceDirection % DIRECTION_COUNT;
            if (m_nFaceDirection < 0)
            {
                m_nFaceDirection += DIRECTION_COUNT;
            }
        }

        if (m_eMoveState == cmsOnSit)
        {
            m_nConvergenceSpeed = 0;
            m_nDestX            = m_nX;
            m_nDestY            = m_nY;

            SetMoveState(cmsOnStand);
        }
    }

    if (m_MoveCtrl.bMove)
    {
        int nMoveDirection = m_nFaceDirection + m_MoveCtrl.nDirection8 * DIRECTION_COUNT / 8;
        int nDestX         = m_nX + MOVE_DEST_RANGE * g_Cos(nMoveDirection) / SIN_COS_NUMBER;
        int nDestY         = m_nY + MOVE_DEST_RANGE * g_Sin(nMoveDirection) / SIN_COS_NUMBER;

        if (
            m_eMoveState == cmsOnStand || m_eMoveState == cmsOnWalk || 
            m_eMoveState == cmsOnRun || m_eMoveState == cmsOnJump || m_eMoveState == cmsOnSit
        )
        {
            if (m_MoveCtrl.bWalk)
            {
                WalkTo(nDestX, nDestY, false);
            }
            else
            {
                RunTo(nDestX, nDestY, false);
            }
        }
        else if (m_eMoveState == cmsOnSwim || m_eMoveState == cmsOnFloat)
        {
            SwimTo(nDestX, nDestY, false);
        }
    }
    else
    {
        if (m_eMoveState == cmsOnWalk || m_eMoveState == cmsOnRun || m_eMoveState == cmsOnSwim)
        {
            m_nConvergenceSpeed = 0;
            m_nDestX            = m_nX;
            m_nDestY            = m_nY;

            if (m_eMoveState == cmsOnSwim)
            {
                m_nVelocityXY   = 0; 
                m_nVelocityZ    = 0;

                SetMoveState(cmsOnFloat);
            }
            else
            {
                int nGradientDirection = 0;

                if (m_pCell->m_BaseInfo.dwCellType == ctWater)
                {
                    KCell* pCell = m_pRegion->GetPreCell(m_nXCell, m_nYCell, m_pCell);
                    KGLOG_PROCESS_ERROR(pCell);

                    nGradientDirection = pCell->GetGradientDirection();
                }
                else 
                {
                    nGradientDirection = m_pCell->GetGradientDirection();
                }

                if ((!m_bSlip) || g_Cos(nGradientDirection - m_nDirectionXY) < 0)
                {
                    m_nVelocityXY = 0;
                }

                SetMoveState(cmsOnStand);
            }
        }
    }

Exit0:
    return;
}

BOOL KPlayer::SetCurrentTeamID(BOOL bSystem, DWORD dwTeamID)
{
    BOOL bResult = false;

    if (bSystem)
    {
        m_dwSystemTeamID = dwTeamID;
    }
    else
    {
        m_dwTeamID = dwTeamID;
    }

    bResult = true;
Exit0:
    return bResult;
}

DWORD KPlayer::GetCurrentTeamID()
{
    if (m_dwSystemTeamID != ERROR_ID)
        return m_dwSystemTeamID;

    return m_dwTeamID;
}

int KPlayer::GetCurrentTeamGroupIndex()
{
    int     nResult     = INVALID_TEAM_GROUP;
    DWORD   dwTeamID    = ERROR_ID;
#ifdef _SERVER
    KTeam*  pTeam       = NULL;
#endif
    dwTeamID = GetCurrentTeamID();
    KG_PROCESS_ERROR(dwTeamID != ERROR_ID);

#ifdef _SERVER
    pTeam = g_pSO3World->m_TeamServer.GetTeam(dwTeamID);
    KGLOG_PROCESS_ERROR(pTeam);

    for (int i = 0; i < pTeam->nGroupNum; i++)
    {
        for (
            KTEAM_MEMBER_LIST::iterator it = pTeam->MemberGroup[i].MemberList.begin();
            it != pTeam->MemberGroup[i].MemberList.end(); ++it
        )
        {
            if (it->dwMemberID == m_dwID)
            {
                nResult = i;
                goto Exit0;
            }
        }
    }
#else
    for (int i = 0; i < g_pSO3World->m_TeamClient.m_nGroupNum; i++)
    {
        for (
            KTEAM_MEMBER_LIST::iterator it = g_pSO3World->m_TeamClient.m_TeamGroup[i].MemberList.begin();
            it != g_pSO3World->m_TeamClient.m_TeamGroup[i].MemberList.end(); ++it
        )
        {
            if (it->dwMemberID == m_dwID)
            {
                nResult = i;
                goto Exit0;
            }
        }
    }
#endif

Exit0:
    return nResult;
}

#ifdef _SERVER

BOOL KPlayer::IsInViewRangeByPlayer(KRegion* pRegion)
{
    BOOL bResult = false;

    assert(m_pScene);
    assert(m_pRegion);
    KG_PROCESS_ERROR(pRegion);

    KG_PROCESS_ERROR(abs(pRegion->m_nRegionX - m_pRegion->m_nRegionX) <= m_pScene->m_nBroadcastRegion);
    KG_PROCESS_ERROR(abs(pRegion->m_nRegionY - m_pRegion->m_nRegionY) <= m_pScene->m_nBroadcastRegion);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::IsInViewRangeByViewPoint(KRegion* pRegion)
{
    BOOL bResult = false;

    assert(m_pScene);
    KG_PROCESS_ERROR(pRegion);
    KG_PROCESS_ERROR(m_ViewPoint.pRegion);

    KG_PROCESS_ERROR(abs(pRegion->m_nRegionX - m_ViewPoint.pRegion->m_nRegionX) <= m_pScene->m_nBroadcastRegion);
    KG_PROCESS_ERROR(abs(pRegion->m_nRegionY - m_ViewPoint.pRegion->m_nRegionY) <= m_pScene->m_nBroadcastRegion);

    bResult = true;
Exit0:
    return bResult;
}

#define MAX_VIEW_OBJECT_COUNT   2

void KPlayer::SyncObjectView()
{
    int nCount = 0;     // ��ÿ��ѭ��ͬ���Ķ������������

    assert(m_pScene);
    assert(m_pRegion);

    while (nCount++ < MAX_VIEW_OBJECT_COUNT && !m_PlayerSyncQueue.empty())
    {
        DWORD       dwID    = m_PlayerSyncQueue.front().dwObjectID;
        int         nFrame  = m_PlayerSyncQueue.front().nViewFrame;
        KPlayer*    pPlayer = NULL;

        if (g_pSO3World->m_nGameLoop < nFrame)
            break;

        m_PlayerSyncTable.erase(dwID);
        m_PlayerSyncQueue.pop_front();

        pPlayer = g_pSO3World->m_PlayerSet.GetObj(dwID);

        if (pPlayer == NULL || pPlayer->m_pScene != m_pScene)
            continue;

        if (pPlayer->m_bStealthy)
            continue;

        assert(pPlayer->m_pRegion);

        if (!IsInViewRangeByPlayer(pPlayer->m_pRegion) && !IsInViewRangeByViewPoint(pPlayer->m_pRegion))
            continue;

        g_PlayerServer.DoSyncNewPlayer(m_nConnIndex, pPlayer);
    }

    nCount = 0;

    while (nCount++ < MAX_VIEW_OBJECT_COUNT && !m_NpcSyncQueue.empty())
    {
        DWORD       dwID    = m_NpcSyncQueue.front().dwObjectID;
        int         nFrame  = m_NpcSyncQueue.front().nViewFrame;
        KNpc*       pNpc    = NULL;

        if (g_pSO3World->m_nGameLoop < nFrame)
            break;

        m_NpcSyncTable.erase(dwID);
        m_NpcSyncQueue.pop_front();

        pNpc = g_pSO3World->m_NpcSet.GetObj(dwID);

        if (pNpc == NULL || pNpc->m_pScene != m_pScene)
            continue;

        if (pNpc->m_bStealthy)
            continue;

        if (pNpc->m_dwOwner != ERROR_ID && pNpc->m_dwOwner != m_dwID)
            continue;

        assert(pNpc->m_pRegion);

        if (!IsInViewRangeByPlayer(pNpc->m_pRegion) && !IsInViewRangeByViewPoint(pNpc->m_pRegion))
            continue;

        g_PlayerServer.DoSyncNewNpc(m_nConnIndex, pNpc);
    }

    nCount = 0;

    while (nCount++ < MAX_VIEW_OBJECT_COUNT && !m_DoodadSyncQueue.empty())
    {
        DWORD       dwID    = m_DoodadSyncQueue.front().dwObjectID;
        int         nFrame  = m_DoodadSyncQueue.front().nViewFrame;
        KDoodad*    pDoodad = NULL;

        if (g_pSO3World->m_nGameLoop < nFrame)
            break;

        m_DoodadSyncTable.erase(dwID);
        m_DoodadSyncQueue.pop_front();

        pDoodad = g_pSO3World->m_DoodadSet.GetObj(dwID);

        if (pDoodad == NULL || pDoodad->m_pScene != m_pScene)
            continue;

        assert(pDoodad->m_pRegion);

        if (!IsInViewRangeByPlayer(pDoodad->m_pRegion) && !IsInViewRangeByViewPoint(pDoodad->m_pRegion))
            continue;

        g_PlayerServer.DoSyncNewDoodad(m_nConnIndex, pDoodad);
    }
}

void KPlayer::RegisterViewObjectByPlayer(KRegion* pFromRegion, KRegion* pToRegion)
{
    assert(m_pScene);
    assert(pToRegion);

    m_pScene->SetViewObjectRegisterFlag(pToRegion, true);       // 1. ����Region�����ڽ�Region���Ϊtrue
    m_pScene->SetViewObjectRegisterFlag(pFromRegion, false);    // 2. ����Region�����ڽ�Region���Ϊfalse,�¾���Ұ�Ľ����ᱻ����Ϊfalse

    m_pScene->RegisterPlayerToArea(this, pToRegion);            // ����ע�ᵽ��Region������ViewPoint��playerͬ������

    if (m_ViewPoint.pRegion)
        m_pScene->SetViewObjectRegisterFlag(m_ViewPoint.pRegion, false);    // ��ViewPointRegion�����ڽ�Region���Ϊfalse

    m_pScene->RegisterAreaObjectToPlayer(this, pToRegion);        // ��ȫ��Region������player, doodad, npcע�ᵽ�ҵ�ͬ������

    return;
}

void KPlayer::RegisterViewObjectByViewPoint(KRegion* pFromRegion, KRegion* pToRegion)
{
    assert(m_pScene);
    assert(pToRegion);
    assert(m_ViewPoint.pRegion);

    m_pScene->SetViewObjectRegisterFlag(pToRegion, true);       // 1. ����Region�����ڽ�Region���Ϊtrue
    m_pScene->SetViewObjectRegisterFlag(pFromRegion, false);    // 2. ����Region�����ڽ�Region���Ϊfalse,�¾���Ұ�Ľ����ᱻ����Ϊfalse
    m_pScene->SetViewObjectRegisterFlag(m_pRegion, false);      // 3. ��player����Region�����ڽ�Region���Ϊfalse,�¾���Ұ�Ľ����ᱻ����Ϊfalse
    m_pScene->RegisterAreaObjectToPlayer(this, pToRegion);      // ��ȫ��Region������player, doodad, npcע�ᵽ�ҵ�ͬ������

    return;
}
#endif

#ifdef _CLIENT

void KPlayer::SetDisplayData(S2C_PLAYER_DISPLAY_DATA& rPlayerDisplayData)
{
    KLevelUpData* pLevelUpData = NULL;

	m_eRoleType = (ROLE_TYPE)rPlayerDisplayData.byRoleType;
    m_nLevel    = rPlayerDisplayData.byLevel;

    SetGenderByRoleType(rPlayerDisplayData.byRoleType);

    pLevelUpData = g_pSO3World->m_Settings.m_LevelUpList.GetLevelUpData(m_eRoleType, m_nLevel);   
    KGLOG_PROCESS_ERROR(pLevelUpData);

    m_nHeight   = pLevelUpData->nHeight;
    m_eCamp     = (KCAMP)rPlayerDisplayData.byCamp;
    m_dwTongID  = rPlayerDisplayData.dwTongID;

    g_pSO3World->m_TongClient.TryGetTongName(m_dwTongID, m_szTongName, sizeof(m_szTongName));

	SetForceID((DWORD)rPlayerDisplayData.byForceID);

	strncpy(m_szName, rPlayerDisplayData.szRoleName, _NAME_LEN);
	m_szName[_NAME_LEN - 1] = 0;

    m_dwModelID                             = rPlayerDisplayData.wModelID;
    m_nBattleFieldSide                      = (int)rPlayerDisplayData.byBattleFieldSide;

    m_Designation.m_nCurrentPrefix          = (int)rPlayerDisplayData.uDesignationPrefix;
    m_Designation.m_nCurrentPostfix         = (int)rPlayerDisplayData.uDesignationPostfix;
    m_Designation.m_nGenerationIndex        = (int)rPlayerDisplayData.uGenerationIndex;
    m_Designation.m_nBynameIndex            = (int)rPlayerDisplayData.uBynameIndex;
    m_Designation.m_bBynameDisplay            = (BOOL)rPlayerDisplayData.uDisplayFlag;

    m_wRepresentId[perFaceStyle]            = rPlayerDisplayData.uFaceStyle;
    m_wRepresentId[perHairStyle]            = rPlayerDisplayData.uHairStyle;
    m_wRepresentId[perHelmStyle]            = rPlayerDisplayData.uHelmStyle;
    m_wRepresentId[perHelmColor]            = rPlayerDisplayData.uHelmColor;
    m_wRepresentId[perHelmEnchant]          = rPlayerDisplayData.uHelmEnchant;
    m_wRepresentId[perChestStyle]           = rPlayerDisplayData.uChestStyle;
    m_wRepresentId[perChestColor]           = rPlayerDisplayData.uChestColor;
    m_wRepresentId[perChestEnchant]         = rPlayerDisplayData.uChestEnchant;
    m_wRepresentId[perWaistStyle]           = rPlayerDisplayData.uWaistStyle;
    m_wRepresentId[perWaistColor]           = rPlayerDisplayData.uWaistColor;
    m_wRepresentId[perWaistEnchant]         = rPlayerDisplayData.uWaistEnchant;

    m_wRepresentId[perBangleStyle]          = rPlayerDisplayData.uBangleStyle;
    m_wRepresentId[perBangleColor]          = rPlayerDisplayData.uBangleColor;
    m_wRepresentId[perBangleEnchant]        = rPlayerDisplayData.uBangleEnchant;
    m_wRepresentId[perBootsStyle]           = rPlayerDisplayData.uBootsStyle;
    m_wRepresentId[perBootsColor]           = rPlayerDisplayData.uBootsColor;
    m_wRepresentId[perWeaponStyle]          = rPlayerDisplayData.uWeaponStyle;
    m_wRepresentId[perWeaponEnchant1]       = rPlayerDisplayData.uWeaponEnchant1;
    m_wRepresentId[perWeaponEnchant2]       = rPlayerDisplayData.uWeaponEnchant2;
    m_wRepresentId[perBackExtend]           = rPlayerDisplayData.uBackExtend;
    m_wRepresentId[perWaistExtend]          = rPlayerDisplayData.uWaistExtend;

    m_wRepresentId[perHorseStyle]           = rPlayerDisplayData.uHorseStyle;      
    m_wRepresentId[perHorseAdornment1]      = rPlayerDisplayData.uHorseAdornment1;
    m_wRepresentId[perHorseAdornment2]      = rPlayerDisplayData.uHorseAdornment2; 
    m_wRepresentId[perHorseAdornment3]      = rPlayerDisplayData.uHorseAdornment3; 
    m_wRepresentId[perHorseAdornment4]      = rPlayerDisplayData.uHorseAdornment4;
    m_dwSchoolID                            = (DWORD)rPlayerDisplayData.uSchoolID;

	if (g_pGameWorldUIHandler)
	{
		KUIEventPlayerDisplayDataUpdate Param = { m_dwID };
		g_pGameWorldUIHandler->OnPlayerDisplayDataUpdate(Param);
	}

	if (g_pGameWorldRepresentHandler)
        g_pGameWorldRepresentHandler->OnCharacterDisplayData(this);


Exit0:
    return;
}
#endif

void KPlayer::SetGenderByRoleType(int nRoleType)
{
    m_eGender = cgGay;

    if (nRoleType == rtStandardMale || nRoleType == rtStrongMale || nRoleType == rtLittleBoy)
    {
        m_eGender = cgMale;
    }
    else if (nRoleType == rtStandardFemale || nRoleType == rtSexyFemale || nRoleType == rtLittleGirl)
    {
        m_eGender = cgFemale;
    }
}

void KPlayer::AddExp(int nExpIncrement)
{
	BOOL            bRetCode         = false;
    KLevelUpData*   pLevelUpData     = NULL;

    KG_PROCESS_ERROR(nExpIncrement > 0);

	// ����ﵽ��ߵȼ����������Ӿ���
	KG_PROCESS_ERROR(m_nLevel < m_nMaxLevel);

	m_nExperience += nExpIncrement;

	pLevelUpData = g_pSO3World->m_Settings.m_LevelUpList.GetLevelUpData(m_eRoleType, m_nLevel);
	KGLOG_PROCESS_ERROR(pLevelUpData);

	while (m_nExperience >= pLevelUpData->nExperience && m_nLevel < m_nMaxLevel)
	{
		m_nExperience -= pLevelUpData->nExperience;

		SetLevel(m_nLevel + 1);

#ifdef _SERVER
        g_LogClient.LogPlayerLevelup(m_nLevel, m_szAccount, m_szName);

        CallLevelupScript();
#endif

        m_nCurrentLife = m_nMaxLife;
        m_nCurrentMana = m_nMaxMana;

		pLevelUpData = g_pSO3World->m_Settings.m_LevelUpList.GetLevelUpData(m_eRoleType, m_nLevel);
		KGLOG_PROCESS_ERROR(pLevelUpData);

#ifdef _SERVER
        m_SkillList.OnPlayerLevelUp();
        g_PlayerServer.DoPlayerLevelUp(this);
#endif
	}

#ifdef _SERVER
	g_PlayerServer.DoSyncExperience(this, nExpIncrement);
#endif

Exit0:
	return;
}

#ifdef _SERVER
BOOL KPlayer::AddContribution(int nAddContribution)
{
    BOOL            bResult             = false;
    int             nPreContribution    = m_nContribution;
    KLevelUpData*   pLevelUpData        = NULL;
    int             nMaxContribution    = 0;

    if (nAddContribution >= 0)
    {
        KGLOG_PROCESS_ERROR(m_dwTongID != ERROR_ID);

        pLevelUpData = g_pSO3World->m_Settings.m_LevelUpList.GetLevelUpData(m_eRoleType, m_nLevel);
	    KGLOG_PROCESS_ERROR(pLevelUpData);

        nMaxContribution = pLevelUpData->nMaxContribution;

        if (m_nContribution > nMaxContribution -  nAddContribution)
        {
            m_nContribution = nMaxContribution;
        }
        else
        {
            m_nContribution += nAddContribution;
        }
    }
    else
    {
        KGLOG_PROCESS_ERROR(m_nContribution >= -nAddContribution);
        m_nContribution += nAddContribution;
    }

    if (nPreContribution != m_nContribution)
    {
        g_PlayerServer.DoSyncContribution(m_nConnIndex, m_nContribution);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::CallLevelupScript()
{
    BOOL    bResult         = false;
    BOOL    bRetCode        = false;
    int     nLuaTopIndex    = 0;

    bRetCode = g_pSO3World->m_ScriptCenter.IsScriptExist(PLAYER_SCRIPT);
    KGLOG_PROCESS_ERROR(bRetCode);

    g_pSO3World->m_ScriptCenter.SafeCallBegin(&nLuaTopIndex);

    g_pSO3World->m_ScriptCenter.PushValueToStack(this);

    g_pSO3World->m_ScriptCenter.CallFunction(PLAYER_SCRIPT, "OnPlayerLevelup", 0);

    g_pSO3World->m_ScriptCenter.SafeCallEnd(nLuaTopIndex);

    bResult = true;
Exit0:
    return bResult;
}
#endif

void KPlayer::SetLevel(int nLevel)
{    
    KLevelUpData* pLevelUpData = NULL;

    KGLOG_PROCESS_ERROR(nLevel > 0);

    if (m_nLevel > 0)
    {
	    pLevelUpData = g_pSO3World->m_Settings.m_LevelUpList.GetLevelUpData(m_eRoleType, m_nLevel);   
        KGLOG_PROCESS_ERROR(pLevelUpData);

        CallAttributeFunction(atStrengthBase, false, pLevelUpData->nStrength, 0);
        CallAttributeFunction(atAgilityBase, false, pLevelUpData->nAgility, 0);
        CallAttributeFunction(atVitalityBase, false, pLevelUpData->nVigor, 0);
        CallAttributeFunction(atSpiritBase, false, pLevelUpData->nSpirit, 0);
        CallAttributeFunction(atSpunkBase, false, pLevelUpData->nSpunk, 0);

        CallAttributeFunction(atMaxLifeBase, false, pLevelUpData->nMaxLife, 0);
        CallAttributeFunction(atMaxManaBase, false, pLevelUpData->nMaxMana, 0);

        CallAttributeFunction(atRunSpeedBase, false, pLevelUpData->nRunSpeed, 0);
        CallAttributeFunction(atJumpSpeedBase, false, pLevelUpData->nJumpSpeed, 0);

        // ------------ ���Իظ� ----------------------------
        CallAttributeFunction(atLifeReplenish, false, pLevelUpData->nLifeReplenish, 0);
        CallAttributeFunction(atLifeReplenishPercent, false, pLevelUpData->nLifeReplenishPercent, 0);
        CallAttributeFunction(atLifeReplenishExt, false, pLevelUpData->nLifeReplenishExt, 0);
        CallAttributeFunction(atManaReplenish, false, pLevelUpData->nManaReplenish, 0);
        CallAttributeFunction(atManaReplenishPercent, false, pLevelUpData->nManaReplenishPercent, 0);
        CallAttributeFunction(atManaReplenishExt, false, pLevelUpData->nManaReplenishExt, 0);

        // ------------- ��������ֵ -----------------------------
        CallAttributeFunction(atPhysicsHitBaseRate, false, pLevelUpData->nHitBase, 0);
        CallAttributeFunction(atSolarHitBaseRate, false, pLevelUpData->nHitBase, 0);
        CallAttributeFunction(atNeutralHitBaseRate, false, pLevelUpData->nHitBase, 0);
        CallAttributeFunction(atLunarHitBaseRate, false, pLevelUpData->nHitBase, 0);
        CallAttributeFunction(atPoisonHitBaseRate, false, pLevelUpData->nHitBase, 0);

        // ------------- �мܼ��� -----------------------------
        CallAttributeFunction(atParryBaseRate, false, pLevelUpData->nParryBaseRate, 0);

        // ------------- ���ܱ���ֵ -----------------------------
        CallAttributeFunction(atPhysicsCriticalStrike, false, pLevelUpData->nPhysicsCriticalStrike, 0);
        CallAttributeFunction(atSolarCriticalStrike, false, pLevelUpData->nSolarCriticalStrike, 0);
        CallAttributeFunction(atNeutralCriticalStrike, false, pLevelUpData->nNeutralCriticalStrike, 0);
        CallAttributeFunction(atLunarCriticalStrike, false, pLevelUpData->nLunarCriticalStrike, 0);
        CallAttributeFunction(atPoisonCriticalStrike, false, pLevelUpData->nPoisonCriticalStrike, 0);

        CallAttributeFunction(atNoneWeaponAttackSpeedBase, false, pLevelUpData->nNoneWeaponAttackSpeedBase, 0);

        CallAttributeFunction(atPhysicsDefenceMax, false, pLevelUpData->nMaxPhysicsDefence, 0);
    }
	pLevelUpData = g_pSO3World->m_Settings.m_LevelUpList.GetLevelUpData(m_eRoleType, nLevel);
	KGLOG_PROCESS_ERROR(pLevelUpData);

    CallAttributeFunction(atStrengthBase, true, pLevelUpData->nStrength, 0);
    CallAttributeFunction(atAgilityBase, true, pLevelUpData->nAgility, 0);
    CallAttributeFunction(atVitalityBase, true, pLevelUpData->nVigor, 0);
    CallAttributeFunction(atSpiritBase, true, pLevelUpData->nSpirit, 0);
    CallAttributeFunction(atSpunkBase, true, pLevelUpData->nSpunk, 0);

    CallAttributeFunction(atMaxLifeBase, true, pLevelUpData->nMaxLife, 0);
    CallAttributeFunction(atMaxManaBase, true, pLevelUpData->nMaxMana, 0);

    CallAttributeFunction(atRunSpeedBase, true, pLevelUpData->nRunSpeed, 0);
    CallAttributeFunction(atJumpSpeedBase, true, pLevelUpData->nJumpSpeed, 0);

    // ------------ ���Իظ� ----------------------------
    CallAttributeFunction(atLifeReplenish, true, pLevelUpData->nLifeReplenish, 0);
    CallAttributeFunction(atLifeReplenishPercent, true, pLevelUpData->nLifeReplenishPercent, 0);
    CallAttributeFunction(atLifeReplenishExt, true, pLevelUpData->nLifeReplenishExt, 0);
    CallAttributeFunction(atManaReplenish, true, pLevelUpData->nManaReplenish, 0);
    CallAttributeFunction(atManaReplenishPercent, true, pLevelUpData->nManaReplenishPercent, 0);
    CallAttributeFunction(atManaReplenishExt, true, pLevelUpData->nManaReplenishExt, 0);

    // ------------- ��������ֵ -------------------------
    CallAttributeFunction(atPhysicsHitBaseRate, true, pLevelUpData->nHitBase, 0);
    CallAttributeFunction(atSolarHitBaseRate, true, pLevelUpData->nHitBase, 0);
    CallAttributeFunction(atNeutralHitBaseRate, true, pLevelUpData->nHitBase, 0);
    CallAttributeFunction(atLunarHitBaseRate, true, pLevelUpData->nHitBase, 0);
    CallAttributeFunction(atPoisonHitBaseRate, true, pLevelUpData->nHitBase, 0);

    // ------------- �мܼ��� -----------------------------
    CallAttributeFunction(atParryBaseRate, true, pLevelUpData->nParryBaseRate, 0);

    // --------------------------------------------------
    CallAttributeFunction(atPhysicsCriticalStrike, true, pLevelUpData->nPhysicsCriticalStrike, 0);
    CallAttributeFunction(atSolarCriticalStrike, true, pLevelUpData->nSolarCriticalStrike, 0);
    CallAttributeFunction(atNeutralCriticalStrike, true, pLevelUpData->nNeutralCriticalStrike, 0);
    CallAttributeFunction(atLunarCriticalStrike, true, pLevelUpData->nLunarCriticalStrike, 0);
    CallAttributeFunction(atPoisonCriticalStrike, true, pLevelUpData->nPoisonCriticalStrike, 0);

    CallAttributeFunction(atNoneWeaponAttackSpeedBase, true, pLevelUpData->nNoneWeaponAttackSpeedBase, 0);

    CallAttributeFunction(atPhysicsDefenceMax, true, pLevelUpData->nMaxPhysicsDefence, 0);

	// -----------------------------------------------------------
    m_nHeight = pLevelUpData->nHeight;
	m_nLevel  = nLevel;

    m_nMaxStamina = pLevelUpData->nMaxStamina;
    m_nMaxThew    = pLevelUpData->nMaxThew;

#ifdef _SERVER
    g_RelayClient.DoChangeRoleLevelRequest(this);
#endif // _SERVER

Exit0:
	return;
}

#ifdef _SERVER
BOOL KPlayer::LoadSkillRecipeList(BYTE* pbyData, size_t uDataLen)
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;
    size_t  uLeftSize   = uDataLen;
    size_t  uUsedSize   = 0;
    BYTE*   pbyOffset   = pbyData;

    bRetCode = m_SkillRecipeList.Load(&uUsedSize, pbyOffset, uLeftSize);
    KGLOG_PROCESS_ERROR(bRetCode);
    uLeftSize -= uUsedSize;
    pbyOffset += uUsedSize;

    KGLOG_PROCESS_ERROR(uLeftSize >= 0);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::LoadQuestData(BYTE* pbyData, size_t uDataLen, int nVersion)
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;
    size_t  uLeftSize   = uDataLen;
    size_t  uUsedSize   = 0;
    BYTE*   pbyOffset   = pbyData;

    bRetCode = m_QuestList.LoadQuestState(&uUsedSize, pbyOffset, uLeftSize, nVersion);
	KGLOG_PROCESS_ERROR(bRetCode);

    g_PlayerServer.DoSyncQuestData(m_nConnIndex, m_dwID, pbyOffset, uUsedSize, (BYTE)qdtQuestState);

    uLeftSize -= uUsedSize;
    pbyOffset += uUsedSize;

	bRetCode = m_QuestList.LoadQuestList(&uUsedSize, pbyOffset, uLeftSize);
	KGLOG_PROCESS_ERROR(bRetCode);

    g_PlayerServer.DoSyncQuestData(m_nConnIndex, m_dwID, pbyOffset, uUsedSize, (BYTE)qdtQuestList);

    uLeftSize -= uUsedSize;
    pbyOffset += uUsedSize;

    bRetCode = m_QuestList.LoadAssistQuestList(&uUsedSize, pbyOffset, uLeftSize);
	KGLOG_PROCESS_ERROR(bRetCode);

    uLeftSize -= uUsedSize;
    pbyOffset += uUsedSize;

    bRetCode = m_QuestList.LoadDailyQuest(&uUsedSize, pbyOffset, uLeftSize, nVersion);
    KGLOG_PROCESS_ERROR(bRetCode);

    g_PlayerServer.DoSyncQuestData(m_nConnIndex, m_dwID, pbyOffset, uUsedSize, (BYTE)qdtDailyQuest);

    uLeftSize -= uUsedSize;
    pbyOffset += uUsedSize;

    KGLOG_PROCESS_ERROR(uLeftSize == 0);

    bResult = true;
Exit0:
    return bResult; 
}

BOOL KPlayer::LoadStateInfo(BYTE* pbyData, size_t uDataLen)
{
    BOOL                bResult         = false;
    KROLE_STATE_INFO*   pRoleStateInfo  = NULL;
    size_t              uLeftSize       = uDataLen;
    BYTE*               pbyOffset       = pbyData;
    int                 nReviveFrame    = 0;

    KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(KROLE_STATE_INFO));
    pRoleStateInfo = (KROLE_STATE_INFO*)pbyOffset;
    uLeftSize -= sizeof(KROLE_STATE_INFO);
    pbyOffset += sizeof(KROLE_STATE_INFO);

    m_eMoveState                        = (CHARACTER_MOVE_STATE)pRoleStateInfo->byMoveState;
    m_nCurrentTrack                     = pRoleStateInfo->wCurrentTrack;
    m_nMoveFrameCounter                 = pRoleStateInfo->nMoveFrameCounter;
    m_nFromNode                         = pRoleStateInfo->wFromFlyNode;
    m_nTargetCity                       = pRoleStateInfo->wTargetCityID;
    m_nExperience                       = pRoleStateInfo->nExperience;

    m_nCurrentLife                      = pRoleStateInfo->nCurrentLife;
    m_nCurrentMana                      = pRoleStateInfo->nCurrentMana;
    m_nCurrentStamina                   = pRoleStateInfo->nCurrentStamina;
    m_nCurrentThew                      = pRoleStateInfo->nCurrentThew;

    m_nCurrentTrainValue                = pRoleStateInfo->nCurrentTrainValue;
    m_nUsedTrainValue                   = pRoleStateInfo->nUsedTrainValue;
    m_nAddTrainTimeInToday              = pRoleStateInfo->nAddTrainTimeInToday;

    nReviveFrame = pRoleStateInfo->wLeftReviveFrame - (int)(g_pSO3World->m_nCurrentTime - m_nLastSaveTime) * GAME_FPS;
    m_ReviveCtrl.nReviveFrame           = max(nReviveFrame, 0);
    m_ReviveCtrl.nLastSituReviveTime    = pRoleStateInfo->nLastSituReviveTime;
    m_ReviveCtrl.nSituReviveCount       = pRoleStateInfo->bySituReviveCount;
    m_ReviveCtrl.nNextCheckReviveFrame  = 0;

    if (m_eMoveState == cmsOnDeath)
        m_ReviveCtrl.nNextCheckReviveFrame = g_pSO3World->m_nGameLoop + 1;

    m_dwKillerID						= pRoleStateInfo->dwKillerID;  

    switch (m_eMoveState)
    {
    case cmsOnAutoFly:
    case cmsOnFloat:
    case cmsOnDeath:
        break;

    case cmsOnSwim:
        m_eMoveState = cmsOnFloat;
        break;

    default:
        m_eMoveState = cmsOnStand;
        m_nCurrentTrack = 0;
        break;
    }

    if (pRoleStateInfo->byPKState == pksSlaying)
    {
        m_PK.SetState(pksSlaying);
        g_PlayerServer.DoSyncPKState(m_nConnIndex, this);
    }
    else if (pRoleStateInfo->byPKState == pksExitSlay)
    {
        int nEndFrame = 0;
        int nCloseSlayLeftTime = pRoleStateInfo->wCloseSlayLeftTime;
        MAKE_IN_RANGE(nCloseSlayLeftTime, 0, g_pSO3World->m_Settings.m_ConstList.nCloseSlayTime);

        nEndFrame = g_pSO3World->m_nGameLoop + nCloseSlayLeftTime * GAME_FPS;

        m_PK.SetState(pksExitSlay, nEndFrame);
        g_PlayerServer.DoSyncPKState(m_nConnIndex, this, nEndFrame);
    }

    m_nCurrentKillPoint         = pRoleStateInfo->wCurrentKillPoint;
    m_nCurrentPrestige          = pRoleStateInfo->nCurrentPrestige;
    m_nBanTime                  = pRoleStateInfo->nBanTime > g_pSO3World->m_nCurrentTime ? pRoleStateInfo->nBanTime : 0;
    m_nContribution             = pRoleStateInfo->nContribution;
    m_nMaxLevel                 = pRoleStateInfo->nMaxLevel;
    m_bHideHat                  = pRoleStateInfo->bHideHat;

    if (m_bHideHat)
        m_wRepresentId[perHelmStyle] = 0;

    m_nTalkWorldDailyCount      = pRoleStateInfo->byTalkWorldDailyCount;
    m_nTalkForceDailyCount      = pRoleStateInfo->byTalkForceDailyCount;
    m_nTalkCampDailyCount       = pRoleStateInfo->byTalkCampDailyCount;

    m_nKilledCount              = pRoleStateInfo->byKilledCount;
    m_nNextResetKilledCountTime = pRoleStateInfo->nNextResetKilledCountTime;

    if (m_eCamp != cNeutral)
    {
        SetCampFlag(pRoleStateInfo->byCampFlag);
    }

    KGLOG_PROCESS_ERROR(uLeftSize == 0);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::LoadStateInfoV2(BYTE* pbyData, size_t uDataLen)
{
    BOOL                    bResult         = false;
    KROLE_STATE_INFO_V2*    pRoleStateInfo  = NULL;
    size_t                  uLeftSize       = uDataLen;
    int                     nReviveFrame    = 0;

    typedef char KRoleStateInfoV2SizeCheck[(sizeof(KROLE_STATE_INFO_V2) == 0x80) ? 1 : -1];

    KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(KROLE_STATE_INFO_V2));
    pRoleStateInfo = (KROLE_STATE_INFO_V2*)pbyData;
    uLeftSize -= sizeof(KROLE_STATE_INFO_V2);

    // The assignments below follow target LoadStateInfo_V2 @ 0x0839a01c.
    // Fields present only in the v2 payload but not yet represented by this
    // candidate are still consumed by the packed struct and intentionally
    // remain non-authoritative until their owning subsystem is ported.
    m_eMoveState       = (CHARACTER_MOVE_STATE)pRoleStateInfo->byMoveState;
    m_nCurrentTrack    = pRoleStateInfo->wCurrentTrack;
    m_nMoveFrameCounter = pRoleStateInfo->nMoveFrameCounter;
    m_nFromNode        = pRoleStateInfo->wFromFlyNode;
    m_nTargetCity      = pRoleStateInfo->wTargetCityID;
    m_nExperience      = pRoleStateInfo->nExperience;
    m_nCurrentLife     = pRoleStateInfo->nCurrentLife;
    m_nCurrentMana     = pRoleStateInfo->nCurrentMana;
    m_nAddTrainTimeInToday = pRoleStateInfo->nAddTrainTimeInToday;
    m_nCurrentTrainValue   = pRoleStateInfo->nCurrentTrainValue;
    m_nUsedTrainValue      = pRoleStateInfo->nUsedTrainValue;

    nReviveFrame = pRoleStateInfo->wLeftReviveFrame -
        (int)(g_pSO3World->m_nCurrentTime - m_nLastSaveTime) * GAME_FPS;
    m_ReviveCtrl.nReviveFrame          = max(nReviveFrame, 0);
    m_ReviveCtrl.nLastSituReviveTime   = pRoleStateInfo->nLastSituReviveTime;
    m_ReviveCtrl.nSituReviveCount      = pRoleStateInfo->bySituReviveCount;
    m_ReviveCtrl.nNextCheckReviveFrame = 0;

    if (m_eMoveState == cmsOnDeath)
        m_ReviveCtrl.nNextCheckReviveFrame = g_pSO3World->m_nGameLoop + 1;

    m_dwKillerID = pRoleStateInfo->dwKillerID;

    switch (m_eMoveState)
    {
    case cmsOnAutoFly:
    case cmsOnFloat:
    case cmsOnDeath:
        break;

    case cmsOnSwim:
        m_eMoveState = cmsOnFloat;
        break;

    default:
        m_eMoveState = cmsOnStand;
        m_nCurrentTrack = 0;
        break;
    }

    m_nCurrentKillPoint         = pRoleStateInfo->wCurrentKillPoint;
    m_nBanTime                  = pRoleStateInfo->nBanTime > g_pSO3World->m_nCurrentTime ?
                                  pRoleStateInfo->nBanTime : 0;
    m_nMaxLevel                 = pRoleStateInfo->nMaxLevel;
    m_bHideHat                  = pRoleStateInfo->byHideHat != 0;
    if (m_bHideHat)
        m_wRepresentId[perHelmStyle] = 0;

    m_nTalkWorldDailyCount      = pRoleStateInfo->wTalkWorldDailyCount;
    m_nTalkForceDailyCount      = pRoleStateInfo->wTalkForceDailyCount;
    m_nTalkCampDailyCount       = pRoleStateInfo->wTalkCampDailyCount;
    m_nKilledCount              = pRoleStateInfo->byKilledCount;
    m_nNextResetKilledCountTime = pRoleStateInfo->nNextResetKilledCountTime;

    if (m_eCamp != cNeutral)
        SetCampFlag(pRoleStateInfo->byCampFlag);

    KGLOG_PROCESS_ERROR(uLeftSize == 0);
    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::LoadStateInfo(BYTE* pbyData, size_t uDataLen, int nVersion)
{
    switch (nVersion)
    {
    case 0:
        return LoadStateInfo(pbyData, uDataLen);

    case 2:
        return LoadStateInfoV2(pbyData, uDataLen);

    case 1:
        // Version 1 has a distinct 0x80-byte target layout and currency
        // side effects. Do not reinterpret it as the legacy 117-byte struct.
        KGLogPrintf(KGLOG_INFO, "LoadStateInfo version 1 is not yet ported\n");
        return false;

    default:
        KGLogPrintf(KGLOG_INFO, "LoadStateInfo unsupported version=%d\n", nVersion);
        return false;
    }
}

BOOL KPlayer::LoadRoadOpenNode(BYTE* pbyData, size_t uDataLen)
{
    BOOL                bResult     = false;
    BOOL                bRetCode    = false;
    KROLE_ROAD_INFO*    pLoadRoad   = NULL;
    BYTE*               pbyOffset   = pbyData;
    size_t              uLeftSize   = uDataLen;

    KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(KROLE_ROAD_INFO));
    pLoadRoad = (KROLE_ROAD_INFO*)pbyOffset;
    pbyOffset += sizeof(KROLE_ROAD_INFO);
    uLeftSize -= sizeof(KROLE_ROAD_INFO);

    KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(int) * pLoadRoad->nOpenNodeCount);
    pbyOffset += sizeof(int) * pLoadRoad->nOpenNodeCount;
    uLeftSize -= sizeof(int) * pLoadRoad->nOpenNodeCount;

    for (int i = 0; i < pLoadRoad->nOpenNodeCount; i++)
    {
        m_OpenRouteNodeList.push_back(pLoadRoad->OpenNodeList[i]);
    }

    KGLOG_PROCESS_ERROR(uLeftSize == 0);

    bResult = true;
Exit0:
    return bResult;
}

static BOOL s_bOldPendentMapLoaded = false;
static std::map<int, DWORD> s_OldWaistPendentMap;
static std::map<int, DWORD> s_OldBackPendentMap;
static std::map<int, DWORD> s_OldFacePendentMap;

static BOOL LoadOldPendentMap()
{
    BOOL bResult = false;
    ITabFile* pTabFile = NULL;
    char szFileName[MAX_PATH];
    snprintf(szFileName, sizeof(szFileName), "%s/item/%s", SETTING_DIR, "OldPendentRepresentID2ItemID.tab");
    pTabFile = g_OpenTabFile(szFileName);
    KGLOG_PROCESS_ERROR(pTabFile);
    for (int nLine = 3; nLine <= pTabFile->GetHeight(); ++nLine)
    {
        int nSubType = 0;
        int nRepresentID = 0;
        int nItemID = 0;
        KGLOG_PROCESS_ERROR(pTabFile->GetInteger(nLine, "SubType", 0, &nSubType));
        KGLOG_PROCESS_ERROR(pTabFile->GetInteger(nLine, "RepresentID", 0, &nRepresentID));
        KGLOG_PROCESS_ERROR(pTabFile->GetInteger(nLine, "ItemID", 0, &nItemID));
        if (nSubType == 11) s_OldWaistPendentMap[nRepresentID] = (DWORD)nItemID;
        else if (nSubType == 14) s_OldBackPendentMap[nRepresentID] = (DWORD)nItemID;
        else if (nSubType == 17) s_OldFacePendentMap[nRepresentID] = (DWORD)nItemID;
    }
    bResult = true;
Exit0:
    KG_COM_RELEASE(pTabFile);
    return bResult;
}

static DWORD GetOldPendentItemID(const std::map<int, DWORD>& rMap, int nRepresentID)
{
    std::map<int, DWORD>::const_iterator it = rMap.find(nRepresentID);
    return it == rMap.end() ? 0 : it->second;
}

static BOOL ProcessTimeLimitPendent(KPendentVec& rPendentList, long nDeltaTime)
{
    for (KPendentVec::iterator it = rPendentList.begin(); it != rPendentList.end();)
    {
        KItemInfo* pItemInfo = g_pSO3World->m_ItemManager.GetItemInfo(ittOther, it->dwItemIndex);
        KGLOG_PROCESS_ERROR(pItemInfo);
        if ((pItemInfo->nExistType == ketOffLine && pItemInfo->nMaxExistTime != 0 && pItemInfo->nMaxExistTime < nDeltaTime) ||
            (pItemInfo->nExistType == ketTimeStamp && pItemInfo->nMaxExistTime < g_pSO3World->m_nCurrentTime))
            it = rPendentList.erase(it);
        else
        {
            if (pItemInfo->nExistType == ketOnLine)
                it->nGenTime += nDeltaTime;
            ++it;
        }
    }
    return true;
Exit0:
    return false;
}

BOOL KPlayer::AddPendent(DWORD dwItemID, time_t nGenTime, int nType)
{
    KPendent Pendent = {dwItemID, nGenTime};
    if (nType == 0) m_WaistPendent.push_back(Pendent);
    else if (nType == 1) m_BackPendent.push_back(Pendent);
    else if (nType == 2) m_FacePendent.push_back(Pendent);
    else return false;
    return true;
}

BOOL KPlayer::LoadPendentData(BYTE* pbyData, size_t uDataLen, int nVersion)
{
    BOOL bResult = false;
    if (nVersion == 0) bResult = LoadPendentDataV0(pbyData, uDataLen);
    else if (nVersion == 1 || nVersion == 2) bResult = LoadPendentDataV2(pbyData, uDataLen);
    else KGLogPrintf(KGLOG_WARNING, "Unsupported pendent data version %d", nVersion);
    KGLOG_PROCESS_ERROR(bResult);
    {
        long nDeltaTime = (long)(g_pSO3World->m_nCurrentTime - m_nLastSaveTime);
        KGLOG_PROCESS_ERROR(ProcessTimeLimitPendent(m_WaistPendent, nDeltaTime));
        KGLOG_PROCESS_ERROR(ProcessTimeLimitPendent(m_BackPendent, nDeltaTime));
        KGLOG_PROCESS_ERROR(ProcessTimeLimitPendent(m_FacePendent, nDeltaTime));
    }
    return true;
Exit0:
    return false;
}

static DWORD GetOldWaistPendentItemID(int nRepresentID)
{
    return GetOldPendentItemID(s_OldWaistPendentMap, nRepresentID);
}

static DWORD GetOldBackPendentItemID(int nRepresentID)
{
    return GetOldPendentItemID(s_OldBackPendentMap, nRepresentID);
}

static DWORD GetOldFacePendentItemID(int nRepresentID)
{
    return GetOldPendentItemID(s_OldFacePendentMap, nRepresentID);
}

BOOL KPlayer::LoadPendentDataV0(BYTE* pbyData, size_t uDataLen)
{
    BOOL bResult = false;
    BYTE* pbyOffset = pbyData;
    size_t uLeftSize = uDataLen;
    WORD wBoxSize = 0;
    BYTE byDataLen = 0;
    KCustomData<32> CustomData;
    KGLOG_PROCESS_ERROR(pbyData);
    if (!s_bOldPendentMapLoaded)
    {
        KGLOG_PROCESS_ERROR(LoadOldPendentMap());
        s_bOldPendentMapLoaded = true;
    }
    m_WaistPendent.clear(); m_BackPendent.clear(); m_FacePendent.clear();
#define LOAD_OLD_PENDENT(BoxSize, Vector, EquippedIndex, Type, Lookup) do { \
        KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(WORD)); \
        BoxSize = *(WORD*)pbyOffset; pbyOffset += sizeof(WORD); uLeftSize -= sizeof(WORD); \
        KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(BYTE)); \
        byDataLen = *(BYTE*)pbyOffset; pbyOffset += sizeof(BYTE); uLeftSize -= sizeof(BYTE); \
        KGLOG_PROCESS_ERROR((size_t)byDataLen <= uLeftSize); \
        memset(&CustomData, 0, sizeof(CustomData)); \
        KGLOG_PROCESS_ERROR(CustomData.Load(pbyOffset, byDataLen)); \
        pbyOffset += byDataLen; uLeftSize -= byDataLen; \
        for (int i = 0; i < 256; ++i) { \
            BOOL bSet = false; \
            KGLOG_PROCESS_ERROR(CustomData.GetBit(i, &bSet)); \
            if (bSet) { DWORD dwItemID = Lookup(i + 2); KGLOG_PROCESS_ERROR(dwItemID); KGLOG_PROCESS_ERROR(AddPendent(dwItemID, 0, Type)); if (i == BoxSize) EquippedIndex = dwItemID; } \
        } \
    } while (false)
    LOAD_OLD_PENDENT(m_nWaistPendentBoxSize, m_WaistPendent, m_dwWaistItemIndex, 0, GetOldWaistPendentItemID);
    LOAD_OLD_PENDENT(m_nBackPendentBoxSize, m_BackPendent, m_dwBackItemIndex, 1, GetOldBackPendentItemID);
    LOAD_OLD_PENDENT(m_nFacePendentBoxSize, m_FacePendent, m_dwFaceItemIndex, 2, GetOldFacePendentItemID);
#undef LOAD_OLD_PENDENT
    KGLOG_PROCESS_ERROR(uLeftSize == 0);
    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::LoadPendentDataV2(BYTE* pbyData, size_t uDataLen)
{
    BOOL bResult = false;
    BYTE* pbyOffset = pbyData;
    size_t uLeftSize = uDataLen;
    WORD wCount = 0;

    KGLOG_PROCESS_ERROR(pbyData);

    KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(WORD));
    m_nWaistPendentBoxSize = *(WORD*)pbyOffset;
    pbyOffset += sizeof(WORD);
    uLeftSize -= sizeof(WORD);

    KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(WORD));
    wCount = *(WORD*)pbyOffset;
    pbyOffset += sizeof(WORD);
    uLeftSize -= sizeof(WORD);
    KGLOG_PROCESS_ERROR((size_t)wCount * sizeof(KPendent) <= uLeftSize);
    m_WaistPendent.resize(wCount);
    if (wCount > 0)
    {
        memcpy(&m_WaistPendent[0], pbyOffset, (size_t)wCount * sizeof(KPendent));
        pbyOffset += (size_t)wCount * sizeof(KPendent);
        uLeftSize -= (size_t)wCount * sizeof(KPendent);
    }

    KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(DWORD));
    m_dwWaistItemIndex = *(DWORD*)pbyOffset;
    pbyOffset += sizeof(DWORD);
    uLeftSize -= sizeof(DWORD);

    KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(WORD));
    m_nBackPendentBoxSize = *(WORD*)pbyOffset;
    pbyOffset += sizeof(WORD);
    uLeftSize -= sizeof(WORD);

    KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(WORD));
    wCount = *(WORD*)pbyOffset;
    pbyOffset += sizeof(WORD);
    uLeftSize -= sizeof(WORD);
    KGLOG_PROCESS_ERROR((size_t)wCount * sizeof(KPendent) <= uLeftSize);
    m_BackPendent.resize(wCount);
    if (wCount > 0)
    {
        memcpy(&m_BackPendent[0], pbyOffset, (size_t)wCount * sizeof(KPendent));
        pbyOffset += (size_t)wCount * sizeof(KPendent);
        uLeftSize -= (size_t)wCount * sizeof(KPendent);
    }

    KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(DWORD));
    m_dwBackItemIndex = *(DWORD*)pbyOffset;
    pbyOffset += sizeof(DWORD);
    uLeftSize -= sizeof(DWORD);

    KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(WORD));
    m_nFacePendentBoxSize = *(WORD*)pbyOffset;
    pbyOffset += sizeof(WORD);
    uLeftSize -= sizeof(WORD);

    KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(WORD));
    wCount = *(WORD*)pbyOffset;
    pbyOffset += sizeof(WORD);
    uLeftSize -= sizeof(WORD);
    KGLOG_PROCESS_ERROR((size_t)wCount * sizeof(KPendent) <= uLeftSize);
    m_FacePendent.resize(wCount);
    if (wCount > 0)
    {
        memcpy(&m_FacePendent[0], pbyOffset, (size_t)wCount * sizeof(KPendent));
        pbyOffset += (size_t)wCount * sizeof(KPendent);
        uLeftSize -= (size_t)wCount * sizeof(KPendent);
    }

    KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(DWORD));
    m_dwFaceItemIndex = *(DWORD*)pbyOffset;
    pbyOffset += sizeof(DWORD);
    uLeftSize -= sizeof(DWORD);
    KGLOG_PROCESS_ERROR(uLeftSize == 0);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::SavePendentData(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize)
{
    BOOL bResult = false;
    BYTE* pbyOffset = pbyBuffer;
    size_t uLeftSize = uBufferSize;
    size_t uCount = 0;

    KGLOG_PROCESS_ERROR(puUsedSize);
    KGLOG_PROCESS_ERROR(pbyBuffer);

#define SAVE_PENDENT_VECTOR(BoxSize, Vector, EquippedIndex)                         \
    do {                                                                             \
        KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(WORD));                             \
        *(WORD*)pbyOffset = (WORD)(BoxSize);                                         \
        pbyOffset += sizeof(WORD);                                                   \
        uLeftSize -= sizeof(WORD);                                                   \
        uCount = (Vector).size();                                                    \
        KGLOG_PROCESS_ERROR(uCount <= 0xffff);                                      \
        KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(WORD));                             \
        *(WORD*)pbyOffset = (WORD)uCount;                                            \
        pbyOffset += sizeof(WORD);                                                   \
        uLeftSize -= sizeof(WORD);                                                   \
        KGLOG_PROCESS_ERROR(uLeftSize >= uCount * sizeof(KPendent));                \
        if (uCount > 0)                                                             \
        {                                                                            \
            memcpy(pbyOffset, &(Vector)[0], uCount * sizeof(KPendent));              \
            pbyOffset += uCount * sizeof(KPendent);                                 \
            uLeftSize -= uCount * sizeof(KPendent);                                 \
        }                                                                            \
        KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(DWORD));                            \
        *(DWORD*)pbyOffset = (EquippedIndex);                                        \
        pbyOffset += sizeof(DWORD);                                                  \
        uLeftSize -= sizeof(DWORD);                                                  \
    } while (false)

    SAVE_PENDENT_VECTOR(m_nWaistPendentBoxSize, m_WaistPendent, m_dwWaistItemIndex);
    SAVE_PENDENT_VECTOR(m_nBackPendentBoxSize, m_BackPendent, m_dwBackItemIndex);
    SAVE_PENDENT_VECTOR(m_nFacePendentBoxSize, m_FacePendent, m_dwFaceItemIndex);

#undef SAVE_PENDENT_VECTOR
    *puUsedSize = uBufferSize - uLeftSize;
    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::LoadFellowPetData(BYTE* pbyData, size_t uDataLen, int nVersion)
{
    return m_FellowPetBox.Load(pbyData, uDataLen, (DWORD)nVersion);
}

BOOL KPlayer::SaveFellowPetData(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize)
{
    return m_FellowPetBox.Save(puUsedSize, pbyBuffer, uBufferSize);
}

BOOL KPlayer::LoadHeroData(BYTE* pbyData, size_t uDataLen)
{
    BOOL                bResult     = false;
    BOOL                bRetCode    = false;
    KHERO_DATA*         pHeroData   = NULL;

    KGLOG_PROCESS_ERROR(uDataLen >= sizeof(KHERO_DATA));
    pHeroData = (KHERO_DATA*)pbyData;

    KGLOG_PROCESS_ERROR(pHeroData->byDataLen <= MAX_MAP_ID_DATA_SIZE);

    m_bHeroFlag = pHeroData->byHeroFlag;

    bRetCode = m_HeroMapCopyOpenFlag.Load(pHeroData->byData, pHeroData->byDataLen);
    KGLOG_PROCESS_ERROR(bRetCode);

    g_PlayerServer.DoSyncHeroFlag(this);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::LoadSingleDungeonData(BYTE* pbyData, size_t uDataLen)
{
    BOOL bResult = false;

    KGLOG_PROCESS_ERROR(pbyData);
    KGLOG_PROCESS_ERROR(uDataLen >= 0x424);

    m_dwSingleDungeonMaxLevel = *(DWORD*)pbyData;
    KGLOG_PROCESS_ERROR(m_dwSingleDungeonMaxLevel <= 128);
    memcpy(m_dwSingleDungeonScore, pbyData + 4, sizeof(m_dwSingleDungeonScore));
    memcpy(m_dwSingleDungeonCustomData,
           pbyData + 4 + sizeof(m_dwSingleDungeonScore),
           sizeof(m_dwSingleDungeonCustomData));

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::LoadArenaData(BYTE* pbyData, size_t uDataLen)
{
    BOOL              bResult    = false;
    KARENA_ROLE_DATA* pArenaData = NULL;

    KGLOG_PROCESS_ERROR(pbyData);
    KGLOG_PROCESS_ERROR(uDataLen >= sizeof(KARENA_ROLE_DATA));
    pArenaData = (KARENA_ROLE_DATA*)pbyData;

    if (m_dwCorpsSystemID != 0)
    {
        m_nCorpsChangeTime = pArenaData->nCorpsChangeTime;
        m_nCorpsWeekTime = pArenaData->nCorpsWeekTime;
        m_nCorpsSeasonTime = pArenaData->nCorpsSeasonTime;
        memcpy(m_nCorpsLevel, pArenaData->nCorpsLevel, sizeof(m_nCorpsLevel));
        memcpy(m_nCorpsRoleLevel, pArenaData->nCorpsRoleLevel, sizeof(m_nCorpsRoleLevel));
    }
    else
    {
        m_nCorpsChangeTime = 0;
        m_nCorpsWeekTime = 0;
        m_nCorpsSeasonTime = 0;
        memset(m_nCorpsLevel, 0, sizeof(m_nCorpsLevel));
        memset(m_nCorpsRoleLevel, 0, sizeof(m_nCorpsRoleLevel));
    }
    bResult = true;
Exit0:
    return bResult;
}

// v246 account-state payload is a fixed 0x4c-byte chunk.  The tail remains
// reserved exactly as in KACCOUNT_STATE_INFO.
BOOL KPlayer::LoadAccountStateInfo(BYTE* pbyData, size_t uDataLen)
{
    BOOL bResult = false;

    KGLOG_PROCESS_ERROR(pbyData);
    KGLOG_PROCESS_ERROR(uDataLen >= 0x4c);

    m_nCurrentStamina = *(int*)(pbyData + 0);
    m_nCurrentThew = *(int*)(pbyData + 4);
    m_nAccountLastSaveTime = (time_t)*(DWORD*)(pbyData + 8);
    m_nAccContinuousLoginCount = *(WORD*)(pbyData + 12);
    m_bContinuousLoginRewardFlag = pbyData[14] != 0;
    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::LoadAccountData(BYTE* pbyData, size_t uDataLen)
{
    BOOL bResult = false;
    BYTE* pbyOffset = NULL;
    size_t uLeftSize = 0;
    KRoleDataHeader* pGlobalHeader = NULL;

    KGLOG_PROCESS_ERROR(pbyData);
    if (uDataLen == 0)
    {
        bResult = true;
        goto Exit0;
    }
    KGLOG_PROCESS_ERROR(uDataLen > sizeof(KRoleDataHeader) - 1);

    pGlobalHeader = (KRoleDataHeader*)pbyData;
    KGLOG_PROCESS_ERROR(pGlobalHeader->dwVer == 0);
    pbyOffset = pbyData + sizeof(KRoleDataHeader);
    uLeftSize = uDataLen - sizeof(KRoleDataHeader);
    KGLOG_PROCESS_ERROR(pGlobalHeader->dwLen == uLeftSize);
    KGLOG_PROCESS_ERROR(CRC32(0, pbyOffset, (DWORD)uLeftSize) == pGlobalHeader->dwCRC);

    while (uLeftSize > 0)
    {
        KRoleBlockHeader* pBlock = NULL;

        KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(KRoleBlockHeader));
        pBlock = (KRoleBlockHeader*)pbyOffset;
        pbyOffset += sizeof(KRoleBlockHeader);
        uLeftSize -= sizeof(KRoleBlockHeader);
        KGLOG_PROCESS_ERROR(pBlock->dwLen <= uLeftSize);

        switch (pBlock->nType)
        {
        case 1:
            KGLOG_PROCESS_ERROR(LoadAccountStateInfo(pbyOffset, pBlock->dwLen));
            break;
        case 2:
            (void)m_RegressionData.LoadAccountData(pbyOffset, pBlock->dwLen);
            break;
        default:
            KGLogPrintf(KGLOG_WARNING, "Unknown account data block(%d, %u), Account: %s, Name: %s, ID: %u\n", pBlock->nType, pBlock->dwLen, m_szAccount, m_szName, m_dwID);
            break;
        }

        pbyOffset += pBlock->dwLen;
        uLeftSize -= pBlock->dwLen;
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::Load(BYTE* pbyData, size_t uDataLen)
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;
    KRoleDataHeader* pGlobalHeader = NULL;

    if (uDataLen == 0)
    {
        m_bExtDataLoadFinish = true;
        m_eGameStatus = gsWaitForSyncClientData;
        bRetCode = OnExtDataLoadFinish();
        KGLOG_PROCESS_ERROR(bRetCode);
    }
    else
    {
        KGLOG_PROCESS_ERROR(pbyData);
        KGLOG_PROCESS_ERROR(uDataLen >= sizeof(KRoleDataHeader));

        pGlobalHeader = (KRoleDataHeader*)pbyData;
        KGLOG_PROCESS_ERROR(pGlobalHeader->dwVer == 0);
        KGLOG_PROCESS_ERROR(pGlobalHeader->dwLen == uDataLen - sizeof(KRoleDataHeader));
        KGLOG_PROCESS_ERROR(CRC32(
            0, pbyData + sizeof(KRoleDataHeader), (DWORD)pGlobalHeader->dwLen
        ) == pGlobalHeader->dwCRC);

        delete[] m_pbyExtDataBuffer;
        m_pbyExtDataBuffer = new BYTE[uDataLen];
        KGLOG_PROCESS_ERROR(m_pbyExtDataBuffer);
        memcpy(m_pbyExtDataBuffer, pbyData, uDataLen);
        m_uExtDataSize          = uDataLen;
        m_uExtDataOffset        = 0;
        m_uExtDataSectionIndex  = 0;
        m_byCurrentExtDataSectionType = 0;
        m_bExtDataLoadFinish    = false;
        m_eGameStatus            = gsWaitForSyncClientData;

        bRetCode = PartialLoadExtData();
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    bResult = true;
Exit0:
    if (!bResult)
    {
        KGLogPrintf(KGLOG_ERR, "Role data error, ID(%u), Name(%s)\n", m_dwID, m_szName);
    }
    return bResult;
}

BOOL KDirectMentorCache::AddMentorData(
    DWORD dwMentor, DWORD dwApprentice, const KDirectMentorRecordCache& record
)
{
    uint64_t uMKey = MAKE_INT64(dwApprentice, dwMentor);
    uint64_t uAKey = MAKE_INT64(dwMentor, dwApprentice);
    std::pair<RecordMap::iterator, bool> inserted = m_CacheMTable.insert(std::make_pair(uMKey, record));
    if (!inserted.second)
        return false;
    if (!m_CacheATable.insert(std::make_pair(uAKey, &inserted.first->second)).second)
    {
        m_CacheMTable.erase(inserted.first);
        return false;
    }
    return true;
}

BOOL KDirectMentorCache::UpdateMentorData(
    DWORD dwMentor, DWORD dwApprentice, const KDirectMentorRecordCache& record
)
{
    RecordMap::iterator it = m_CacheMTable.find(MAKE_INT64(dwApprentice, dwMentor));
    if (it == m_CacheMTable.end())
        return AddMentorData(dwMentor, dwApprentice, record);
    it->second = record;
    return true;
}

BOOL KDirectMentorCache::DeleteMentorRecord(uint64_t uKey)
{
    DWORD dwMentor = LOW_DWORD_IN_UINT64(uKey);
    DWORD dwApprentice = HIGH_DWORD_IN_UINT64(uKey);
    m_CacheATable.erase(MAKE_INT64(dwMentor, dwApprentice));
    m_CacheMTable.erase(uKey);
    return true;
}

void KDirectMentorCache::Clear()
{
    m_CacheATable.clear();
    m_CacheMTable.clear();
}

int KDirectMentorCache::PickupTAEquipsScore(DWORD dwMentorID)
{
    int nScore = 0;
    PointerMap::iterator it = m_CacheATable.lower_bound(MAKE_INT64(dwMentorID, 0));
    PointerMap::iterator end = m_CacheATable.lower_bound(MAKE_INT64(dwMentorID + 1, 0));
    for (; it != end; ++it)
        nScore += (int)it->second->dwTAEquipsScore;
    if (nScore != 0)
        g_RelayClient.DoPickupTAEquipsScoreRequest(1, dwMentorID);
    return nScore;
}

BOOL KPlayer::FinishRoleDataLoad()
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;
    int     nTAEquipsScore = 0;
    time_t  nOldDay     = (m_nLastSaveTime - timezone) / (24 * 3600);
    time_t  nNewDay     = (g_pSO3World->m_nCurrentTime - timezone) / (24 * 3600);

    m_ItemList.m_bFinishLoadData = true;
    UpdateFreeLimitFlag();
    m_RegressionData.Calculate(m_nAccountLastSaveTime, m_nLastSaveTime);
    m_ItemList.UpdateItemID();
    KGLOG_PROCESS_ERROR(m_ItemList.AdjustCubPackageSize());
    KGLOG_PROCESS_ERROR(g_PlayerServer.DoSyncCubPackageSize(this));

    if (m_dwSingleDungeonScore[0] != 0)
    {
        if (m_dwSingleDungeonMaxLevel != 0)
            g_RelayClient.DoApplySingleDungeonLastScore(m_dwID);
        SyncSingleDungeonCurrentScore();
    }

    // ������ ExtRoleData ֮���ٴ���
    if (nNewDay != nOldDay)
    {
        bRetCode = RefreshDailyVariable((int)(nNewDay - nOldDay));
        KGLOG_CHECK_ERROR(bRetCode);
    }

    g_RelayClient.DoApplyCoinOperatingFlag(this);

    if (m_dwCorpsSystemID != 0)
    {
        g_RelayClient.DoSyncCorpsChangeDataRequest(
            m_dwID, m_nCorpsChangeTime, m_nCorpsWeekTime, m_nCorpsSeasonTime
        );
        g_PlayerServer.DoSyncCorpsChangeValue(
            m_dwID, m_nCorpsLevel, m_nCorpsRoleLevel
        );
    }

    g_RelayClient.DoUpdateMaxApprenticeNum(m_dwID, m_nMaxApprenticeNum);

    nTAEquipsScore += g_pSO3World->m_MentorCache.PickupTAEquipsScore(m_dwID);
    nTAEquipsScore += g_pSO3World->m_DirectMentorCache.PickupTAEquipsScore(m_dwID);
    KGLOG_PROCESS_ERROR(AddTAEquipsScore(nTAEquipsScore));

    // ͬ��ChargeFlag
    // player sync frontier. Keep this route GS-only until the missing
    // single-dungeon/corps state loaders are ported with their target fields.
    g_RelayClient.DoSyncFellowshipPlayerMiniAvatar(
        m_dwID, m_dwMiniAvatarID, (int)m_eRoleType
    );

    // Target emits protocol 299 before the player-state sync frontier.
    KGLOG_PROCESS_ERROR(g_PlayerServer.DoSyncSprintV2(this));

    // Í¬ï¿½ï¿½ChargeFlag

    // ͬ��״̬
    g_PlayerServer.DoSyncPlayerStateInfo(this);
    g_PlayerServer.DoSyncCurrencyList(this);
    g_PlayerServer.DoSyncKillPoint(this);

    g_pSO3World->m_FellowshipMgr.LoadFellowshipData(m_dwID);

    // ��ͻ���ͬ����������������
    g_PlayerServer.DoLoginTeamSync(this);

    // Buffͬ��:
    g_PlayerServer.DoSyncBuffList(this, this);

    // ͬ���������ʼ�¼
    g_PlayerServer.DoSyncVisitMapInfo(this);

    if (!m_OpenRouteNodeList.empty())
    {
        g_PlayerServer.DoSyncRouteNodeOpenList(
            m_nConnIndex, (int)m_OpenRouteNodeList.size(), &m_OpenRouteNodeList[0]
        );
    }

    g_PlayerServer.DoSyncCampInfo(this);

    g_PlayerServer.DoSyncAchievementData(m_nConnIndex, this);

    g_PlayerServer.DoSyncDesignationData(this);

    g_PlayerServer.DoSyncCoin(this);
    (void)g_PlayerServer.DoSyncRewards(this);
    (void)g_PlayerServer.DoSyncPendentData(this);
    (void)g_PlayerServer.DoSyncFellowPetData(this);

    bResult = true;
Exit0:
    if (!bResult)
    {
        KGLogPrintf(KGLOG_ERR, "Role data completion error, ID(%u), Name(%s)\n", m_dwID, m_szName);
    }
    return bResult;
}

void KPlayer::SyncSingleDungeonCurrentScore()
{
    DWORD dwScore = 0;
    DWORD i;

    for (i = 0; i < m_dwSingleDungeonMaxLevel; ++i)
        dwScore += m_dwSingleDungeonScore[i];

    g_PlayerServer.DoSyncSingleDungeonCurrentScore(
        m_nConnIndex, (int)m_dwSingleDungeonMaxLevel, dwScore
    );
}

BOOL KPlayer::OnClientReady()
{
    BOOL bResult = false;
    BOOL bRetCode = false;

    m_nVirtualFrame = g_pSO3World->m_nGameLoop;
    m_nRecordCount = 0;
    m_eGameStatus = gsPlaying;

    KGLOG_PROCESS_ERROR(m_pScene && m_pRegion);
    bRetCode = m_pScene->ValidateRegions(m_pRegion->m_nRegionX, m_pRegion->m_nRegionY);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::OnExtDataLoadFinish()
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KScene* pScene = NULL;

    KGLOG_PROCESS_ERROR(m_bExtDataLoadFinish);
    KGLOG_PROCESS_ERROR(FinishRoleDataLoad());
    (void)g_pSO3World->m_TongDiplomacyCache.SyncNewClient(this);
    if (m_dwTongID != 0)
        (void)g_pSO3World->m_TongServer.SyncTongTotalCache(m_dwTongID, m_nConnIndex);
    m_nNextSaveFrame = g_pSO3World->m_nGameLoop
        + g_pSO3World->m_Settings.m_ConstList.nSaveInterval * GAME_FPS;
    m_nNextKillPointReduceTime = g_pSO3World->m_nCurrentTime
        + g_pSO3World->m_Settings.m_ConstList.nKillPointReduceCycle;
    pScene = g_pSO3World->GetScene(m_SavePosition.dwMapID, m_SavePosition.nMapCopyIndex);
    KGLOG_PROCESS_ERROR(pScene);
    (void)g_PlayerServer.DoSyncRoleDataOver(m_nConnIndex);
    if (m_ExteriorBox.GetExteriorSetCount() == 0)
        (void)m_ExteriorBox.AddEmptyExteriorSet();
    (void)g_PlayerServer.DoSyncExteriorAllSetData(m_nConnIndex, this);
    (void)g_PlayerServer.DoSyncClientReportConfig(this);
    pScene->CallEnterSceneScript(this);
    bRetCode = CallLoginScript();
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::PartialLoadExtData()
{
    BOOL bResult = false;
    BOOL bRetCode = false;

    KGLogPrintf(
        KGLOG_INFO,
        "W1_PARTIAL_BEGIN id=%u status=%d finish=%d buffer=%p offset=%u size=%u\n",
        m_dwID, m_eGameStatus, m_bExtDataLoadFinish, m_pbyExtDataBuffer,
        (unsigned)m_uExtDataOffset, (unsigned)m_uExtDataSize
    );
    KGLOG_PROCESS_ERROR(m_eGameStatus == gsWaitForSyncClientData);
    KGLOG_PROCESS_ERROR(m_pbyExtDataBuffer);

    bRetCode = LoadExtRoleData(m_pbyExtDataBuffer, m_uExtDataSize);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = g_PlayerServer.DoSyncRoleDataSectionCheckRequest(
        m_nConnIndex, m_byCurrentExtDataSectionType
    );
    KGLOG_PROCESS_ERROR(bRetCode);

    if (m_uExtDataOffset >= m_uExtDataSize)
    {
        delete[] m_pbyExtDataBuffer;
        m_pbyExtDataBuffer = NULL;
        m_uExtDataSize = 0;
        m_uExtDataOffset = 0;
        m_bExtDataLoadFinish = true;
    }

    KGLogPrintf(
        KGLOG_INFO,
        "W1_PARTIAL_OK id=%u type=%u finish=%d next_offset=%u size=%u\n",
        m_dwID, m_byCurrentExtDataSectionType, m_bExtDataLoadFinish,
        (unsigned)m_uExtDataOffset, (unsigned)m_uExtDataSize
    );

    bResult = true;
Exit0:
    if (!bResult)
    {
        KGLogPrintf(
            KGLOG_ERR,
            "W1_PARTIAL_FAIL id=%u status=%d finish=%d buffer=%p offset=%u size=%u\n",
            m_dwID, m_eGameStatus, m_bExtDataLoadFinish, m_pbyExtDataBuffer,
            (unsigned)m_uExtDataOffset, (unsigned)m_uExtDataSize
        );
    }
    return bResult;
}

BOOL KPlayer::SaveActivityVariables(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize)
{
    BOOL bResult = false;
    KACTIVITY_VARIABLES activityVars;
    KGLOG_PROCESS_ERROR(puUsedSize);
    KGLOG_PROCESS_ERROR(pbyBuffer);
    KGLOG_PROCESS_ERROR(uBufferSize >= sizeof(activityVars));
    memcpy(activityVars.wPrensentCodeCounters, m_wPresentCodeCounters, sizeof(activityVars.wPrensentCodeCounters));
    memcpy(pbyBuffer, &activityVars, sizeof(activityVars));
    *puUsedSize = sizeof(activityVars);
    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::LoadActivityVariables(BYTE* pbyData, size_t uDataLen)
{
    BOOL bResult = false;
    KACTIVITY_VARIABLES* pData = NULL;
    KGLOG_PROCESS_ERROR(pbyData);
    KGLOG_PROCESS_ERROR(uDataLen >= sizeof(KACTIVITY_VARIABLES));
    pData = (KACTIVITY_VARIABLES*)pbyData;
    memcpy(m_wPresentCodeCounters, pData->wPrensentCodeCounters, sizeof(m_wPresentCodeCounters));
    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::SaveBankPasswordData(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize)
{
    BOOL bResult = false;
    KBANK_PASSWORD_DATA Data;
    KGLOG_PROCESS_ERROR(puUsedSize);
    KGLOG_PROCESS_ERROR(pbyBuffer);
    KGLOG_PROCESS_ERROR(uBufferSize >= sizeof(Data));
    memset(&Data, 0, sizeof(Data));
    Data.nResetEndTime = m_nBankPasswordResetEndTime;
    strncpy(Data.szPassword, m_szBankPassword, sizeof(Data.szPassword));
    Data.szPassword[sizeof(Data.szPassword) - 1] = 0;
    strncpy(Data.szAnswer, m_szBankPasswordAnswer, sizeof(Data.szAnswer));
    Data.szAnswer[sizeof(Data.szAnswer) - 1] = 0;
    Data.nQuestionID = m_nBankPasswordQuestionID;
    Data.dwEffectMask = m_dwSafeLockMask;
    memcpy(pbyBuffer, &Data, sizeof(Data));
    *puUsedSize = sizeof(Data);
    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::LoadBankPasswordData(BYTE* pbyData, size_t uDataLen)
{
    BOOL bResult = false;
    KBANK_PASSWORD_DATA* pData = NULL;
    KGLOG_PROCESS_ERROR(pbyData);
    KGLOG_PROCESS_ERROR(uDataLen >= sizeof(KBANK_PASSWORD_DATA));
    pData = (KBANK_PASSWORD_DATA*)pbyData;
    m_nBankPasswordResetEndTime = pData->nResetEndTime;
    m_dwSafeLockMask = pData->dwEffectMask;
    m_dwSafeLockMask |= 0x1fff;
    strncpy(m_szBankPassword, pData->szPassword, sizeof(m_szBankPassword));
    m_szBankPassword[sizeof(m_szBankPassword) - 1] = 0;
    if (m_szBankPassword[0] != 0)
        m_bBankPasswordExist = true;
    strncpy(m_szBankPasswordAnswer, pData->szAnswer, sizeof(m_szBankPasswordAnswer));
    m_szBankPasswordAnswer[sizeof(m_szBankPasswordAnswer) - 1] = 0;
    m_nBankPasswordQuestionID = pData->nQuestionID;
    KGLOG_PROCESS_ERROR(g_PlayerServer.DoSyncSafeLockInfo(m_nConnIndex, m_dwSafeLockMask));
    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::SaveDropSurpriseData(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize)
{
    BOOL bResult = false;
    KDROP_SURPRISE_DATA Data;
    KGLOG_PROCESS_ERROR(puUsedSize);
    KGLOG_PROCESS_ERROR(pbyBuffer);
    KGLOG_PROCESS_ERROR(uBufferSize >= sizeof(Data));
    memset(&Data, 0, sizeof(Data));
    memcpy(Data.byMask, m_byDropSurpriseMask, sizeof(Data.byMask));
    memcpy(pbyBuffer, &Data, sizeof(Data));
    *puUsedSize = sizeof(Data);
    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::LoadDropSurpriseData(BYTE* pbyData, size_t uDataLen)
{
    BOOL bResult = false;
    KDROP_SURPRISE_DATA* pData = NULL;
    KGLOG_PROCESS_ERROR(pbyData);
    KGLOG_PROCESS_ERROR(uDataLen >= sizeof(KDROP_SURPRISE_DATA));
    pData = (KDROP_SURPRISE_DATA*)pbyData;
    memcpy(m_byDropSurpriseMask, pData->byMask, sizeof(m_byDropSurpriseMask));
    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::LoadExtRoleData(BYTE* pbyData, size_t uDataLen)
{
    BOOL                bResult         = false;
    BOOL                bRetCode        = false;
    KRoleDataHeader*    pGlobalHeader   = NULL;
    size_t              uLeftSize       = 0;
    BYTE*               pbyOffset       = NULL;
    DWORD               dwCRC           = 0;
    int                 nCurrentRoleBlock = -1;
    DWORD               dwCurrentRoleBlockVersion = 0;
    DWORD               dwCurrentRoleBlockLength = 0;
    unsigned            nRoleBlockIndex = m_uExtDataSectionIndex;

    assert(pbyData);

    if (m_uExtDataOffset == 0)
    {
        KGLOG_PROCESS_ERROR(uDataLen >= sizeof(KRoleDataHeader));
        pGlobalHeader = (KRoleDataHeader*)pbyData;
        KGLOG_PROCESS_ERROR(pGlobalHeader->dwVer == 0);
        KGLOG_PROCESS_ERROR(pGlobalHeader->dwLen == uDataLen - sizeof(KRoleDataHeader));
        dwCRC = CRC32(0, pbyData + sizeof(KRoleDataHeader), (DWORD)pGlobalHeader->dwLen);
        KGLOG_PROCESS_ERROR(dwCRC == pGlobalHeader->dwCRC);
        m_uExtDataOffset = sizeof(KRoleDataHeader);
    }

    KGLOG_PROCESS_ERROR(m_uExtDataOffset < uDataLen);
    pbyOffset = pbyData + m_uExtDataOffset;
    uLeftSize = uDataLen - m_uExtDataOffset;
    KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(KRoleBlockHeader));

    {
        KRoleBlockHeader* pBlock = (KRoleBlockHeader*)pbyOffset;

        uLeftSize -= sizeof(KRoleBlockHeader);
        pbyOffset += sizeof(KRoleBlockHeader);

        KGLOG_PROCESS_ERROR(uLeftSize >= pBlock->dwLen);

        nCurrentRoleBlock = pBlock->nType;
        dwCurrentRoleBlockVersion = pBlock->dwVer;
        dwCurrentRoleBlockLength = pBlock->dwLen;
        m_byCurrentExtDataSectionType = (BYTE)pBlock->nType;
        KGLogPrintf(
            KGLOG_INFO,
            "W1_ROLE_BLOCK_BEGIN id=%u index=%u type=%d ver=%u len=%u remaining=%u\n",
            m_dwID, nRoleBlockIndex, pBlock->nType, pBlock->dwVer,
            pBlock->dwLen, (unsigned)uLeftSize
        );

        switch (pBlock->nType)
        {            
        case rbtSkillList:
            bRetCode = m_SkillList.Load(pbyOffset, pBlock->dwLen);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;

        case rbtSkillRecipeList:
            bRetCode = LoadSkillRecipeList(pbyOffset, pBlock->dwLen);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;

        case rbtItemList:
            KGLogPrintf(KGLOG_INFO,
                "W1_ITEM_BLOCK player=%u type=%d ver=%u len=%u remaining=%u\n",
                m_dwID, pBlock->nType, pBlock->dwVer, pBlock->dwLen,
                (unsigned)uLeftSize);
            bRetCode = m_ItemList.LoadItemList(pbyOffset, pBlock->dwLen, (int)pBlock->dwVer);
		    KGLOG_PROCESS_ERROR(bRetCode);
			break;

        case rbtQuestList:
            bRetCode = LoadQuestData(pbyOffset, pBlock->dwLen, pBlock->dwVer);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;

        case rbtProfessionList:
            bRetCode = m_ProfessionList.Load(pbyOffset, pBlock->dwLen);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;

        case rbtRecipeList:
            bRetCode = m_RecipeList.Load(pbyOffset, pBlock->dwLen, pBlock->dwVer);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;

        case rbtStateInfo:
            KGLogPrintf(
                KGLOG_INFO,
                "W1_ROLE_STATE_BLOCK id=%u type=%d ver=%u len=%u legacy_size=%u remaining=%u\n",
                m_dwID, pBlock->nType, pBlock->dwVer, pBlock->dwLen,
                (unsigned)sizeof(KROLE_STATE_INFO), (unsigned)uLeftSize
            );
            bRetCode = LoadStateInfo(pbyOffset, pBlock->dwLen, (int)pBlock->dwVer);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;

        case rbtBuffList:
            bRetCode = m_BuffList.Load(pbyOffset, pBlock->dwLen);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;

        case rbtRepute:
            bRetCode = m_ReputeList.Load(pbyOffset, pBlock->dwLen);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;

        case rbtUserPreferences:
            bRetCode = m_UserPreferences.Load(pbyOffset, pBlock->dwLen);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;

        case rbtBookState:
            bRetCode = m_BookList.Load(pbyOffset, pBlock->dwLen);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;

        case rbtCoolDownTimer:
            bRetCode = m_TimerList.Load(pbyOffset, pBlock->dwLen);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;

        case rbtRoadOpenList:
            bRetCode = LoadRoadOpenNode(pbyOffset, pBlock->dwLen);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;

        case rbtCustomData:
            bRetCode = m_CustomData.Load(pbyOffset, pBlock->dwLen);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;

        case rbtVisitedMap:
            bRetCode = m_SceneVisitRecord.Load(pbyOffset, pBlock->dwLen);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;

        case rbtPQList:
            bRetCode = m_PQList.Load(pbyOffset, pBlock->dwLen);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;

        case rbtHeroData:
            bRetCode = LoadHeroData(pbyOffset, pBlock->dwLen);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;

        case rbtSingleDungeonData:
            bRetCode = LoadSingleDungeonData(pbyOffset, pBlock->dwLen);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;

        case rbtArenaData:
            bRetCode = LoadArenaData(pbyOffset, pBlock->dwLen);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;

        case rbtCampActiveStat:
            bRetCode = m_CampActiveStat.Load(pbyOffset, pBlock->dwLen);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;

        case rbtPendentData:
            bRetCode = LoadPendentData(pbyOffset, pBlock->dwLen, (int)pBlock->dwVer);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;
        case rbtFellowPetData:
            bRetCode = LoadFellowPetData(pbyOffset, pBlock->dwLen, (int)pBlock->dwVer);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;

        case rbtAchievementData:
            bRetCode = m_Achievement.Load(pbyOffset, pBlock->dwLen);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;

        case rbtDesignationData:
            bRetCode = m_Designation.Load(pbyOffset, pBlock->dwLen);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;

        case rbtRegressionData:
            bRetCode = m_RegressionData.Load(pbyOffset, pBlock->dwLen);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;

        case rbtActivityVariables:
            bRetCode = LoadActivityVariables(pbyOffset, pBlock->dwLen);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;

        case rbtDropSurpriseData:
            bRetCode = LoadDropSurpriseData(pbyOffset, pBlock->dwLen);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;

        case rbtBankPasswordData:
            bRetCode = LoadBankPasswordData(pbyOffset, pBlock->dwLen);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;

        case rbtCurrencyData:
            bRetCode = m_CurrencyList.Load(pbyOffset, pBlock->dwLen);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;

        case rbtRandData:
            bRetCode = LoadRandData(pbyOffset, pBlock->dwLen);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;

        case rbtAntiFarmerData:
            bRetCode = m_AntiFarmer.Load(pbyOffset, pBlock->dwLen);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;

        case rbtMentorData:
            bRetCode = LoadMentorData(pbyOffset, pBlock->dwLen, pBlock->dwVer);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;

        case rbtExteriorBoxData:
            bRetCode = m_ExteriorBox.LoadExteriorBox(pbyOffset, pBlock->dwLen);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;

        case rbtExteriorSetData:
            bRetCode = m_ExteriorBox.Load(pbyOffset, pBlock->dwLen);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;

        case rbtHairBoxData:
            bRetCode = m_HairBox.Load(pbyOffset, pBlock->dwLen);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;

        case rbtMiniAvatarData:
            bRetCode = m_MiniAvatar.Load(pbyOffset, pBlock->dwLen, pBlock->dwVer);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;

        case rbtDelayTradeItemData:
            bRetCode = m_ItemList.LoadDelayTradeInfo(pbyOffset, pBlock->dwLen, (int)pBlock->dwVer);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;

        case rbtTimeLimitReturnItemData:
            bRetCode = m_ItemList.LoadTimeLimitReturnInfo(pbyOffset, pBlock->dwLen, (int)pBlock->dwVer);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;

        case rbtTimeLimitSoldListInfoData:
            bRetCode = m_ItemList.LoadTimeLimitSoldListInfo(pbyOffset, pBlock->dwLen, (int)pBlock->dwVer);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;

        default:
            KGLogPrintf(
                KGLOG_ERR, "Unexpected role data block(%d, %u), Account: %s, Name: %s, ID: %u\n", 
                pBlock->nType, pBlock->dwLen, m_szAccount, m_szName, m_dwID
            );
        }

        KGLogPrintf(
            KGLOG_INFO,
            "W1_ROLE_BLOCK_OK id=%u index=%u type=%d ver=%u len=%u\n",
            m_dwID, nRoleBlockIndex, nCurrentRoleBlock,
            dwCurrentRoleBlockVersion, dwCurrentRoleBlockLength
        );

        uLeftSize -= pBlock->dwLen;
        pbyOffset += pBlock->dwLen;
        m_uExtDataOffset += sizeof(KRoleBlockHeader) + pBlock->dwLen;
    }

    bResult = true;
Exit0:
    if (!bResult)
    {
        KGLogPrintf(
            KGLOG_ERR,
            "W1_ROLE_LOAD_FAIL id=%u index=%u type=%d ver=%u len=%u remaining=%u\n",
            m_dwID, nRoleBlockIndex, nCurrentRoleBlock,
            dwCurrentRoleBlockVersion, dwCurrentRoleBlockLength,
            (unsigned)uLeftSize
        );
    }
    return bResult;
}

BOOL KPlayer::CallLoginScript()
{
    BOOL    bResult         = false;
    BOOL    bRetCode        = false;
    int     nLuaTopIndex    = 0;

    bRetCode = g_pSO3World->m_ScriptCenter.IsScriptExist(PLAYER_SCRIPT);
    KGLOG_PROCESS_ERROR(bRetCode);

	g_pSO3World->m_ScriptCenter.SafeCallBegin(&nLuaTopIndex);

	g_pSO3World->m_ScriptCenter.PushValueToStack(this);

	g_pSO3World->m_ScriptCenter.CallFunction(PLAYER_SCRIPT, "OnPlayerLogin", 0);

	g_pSO3World->m_ScriptCenter.SafeCallEnd(nLuaTopIndex);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::RefreshDailyVariable(int nDays)
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;

    KG_PROCESS_ERROR(nDays > 0);

    g_pSO3World->m_FellowshipMgr.AttractionFalloff(m_dwID, nDays);
    g_pSO3World->m_FellowshipMgr.RefreshDailyCount(m_dwID);
    m_QuestList.ResetAssistDailyCount();

    m_nTalkWorldDailyCount = 0;
    m_nTalkForceDailyCount = 0;
    m_nTalkCampDailyCount  = 0;

    bResult = true;
Exit0:
    return bResult;
}

#endif	// _SERVER

#if defined(_SERVER)
BOOL KPlayer::SaveSkillRecipeList(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize)
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;
    size_t  uUsedSize   = 0;
    size_t  uLeftSize   = uBufferSize;
    BYTE*   pbyOffset   = pbyBuffer;

    bRetCode = m_SkillRecipeList.Save(&uUsedSize, pbyOffset, uLeftSize);
	KGLOG_PROCESS_ERROR(bRetCode);
    uLeftSize -= uUsedSize;
    pbyOffset += uUsedSize;

    *puUsedSize = uBufferSize - uLeftSize;

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::SaveQuestStateAndList(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize)
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;
    size_t  uUsedSize   = 0;
    size_t  uLeftSize   = uBufferSize;
    BYTE*   pbyOffset   = pbyBuffer;

    bRetCode = m_QuestList.SaveQuestState(&uUsedSize, pbyOffset, uLeftSize);
	KGLOG_PROCESS_ERROR(bRetCode);
    uLeftSize -= uUsedSize;
    pbyOffset += uUsedSize;

	bRetCode = m_QuestList.SaveQuestList(&uUsedSize, pbyOffset, uLeftSize);
	KGLOG_PROCESS_ERROR(bRetCode);
    uLeftSize -= uUsedSize;
    pbyOffset += uUsedSize;

    bRetCode = m_QuestList.SaveAssistQuestList(&uUsedSize, pbyOffset, uLeftSize);
	KGLOG_PROCESS_ERROR(bRetCode);
    uLeftSize -= uUsedSize;
    pbyOffset += uUsedSize;

    bRetCode = m_QuestList.SaveDailyQuest(&uUsedSize, pbyOffset, uLeftSize);
	KGLOG_PROCESS_ERROR(bRetCode);
    uLeftSize -= uUsedSize;
    pbyOffset += uUsedSize;

    *puUsedSize = uBufferSize - uLeftSize;

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::SaveStateInfo(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize)
{
    BOOL                bResult         = false;
    BOOL                bRetCode        = false;
    size_t              uLeftSize       = uBufferSize;
    BYTE*               pbyOffset       = pbyBuffer;
    KROLE_STATE_INFO*   pRoleStateInfo  = NULL;

    KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(KROLE_STATE_INFO));
    pRoleStateInfo = (KROLE_STATE_INFO*)pbyOffset;
    uLeftSize -= sizeof(KROLE_STATE_INFO);
    pbyOffset += sizeof(KROLE_STATE_INFO);

    assert(cmsTotal < UCHAR_MAX);

    pRoleStateInfo->byMoveState             = (BYTE)m_eMoveState;
    pRoleStateInfo->wCurrentTrack           = (WORD)m_nCurrentTrack;
    pRoleStateInfo->nMoveFrameCounter       = m_nMoveFrameCounter;

    KGLOG_CHECK_ERROR(m_nFromNode < USHRT_MAX);
    KGLOG_CHECK_ERROR(m_nTargetCity < USHRT_MAX);

    pRoleStateInfo->wFromFlyNode            = (WORD)m_nFromNode;
    pRoleStateInfo->wTargetCityID           = (WORD)m_nTargetCity;

    pRoleStateInfo->nExperience             = m_nExperience;

    pRoleStateInfo->nCurrentLife            = m_nCurrentLife;
    pRoleStateInfo->nCurrentMana            = m_nCurrentMana;
    pRoleStateInfo->nCurrentStamina         = m_nCurrentStamina;
    pRoleStateInfo->nCurrentThew            = m_nCurrentThew;

    pRoleStateInfo->nUsedTrainValue         = m_nUsedTrainValue;

    pRoleStateInfo->nCurrentTrainValue      = m_nCurrentTrainValue;
    pRoleStateInfo->nAddTrainTimeInToday    = m_nAddTrainTimeInToday;

    pRoleStateInfo->byPKState               = (BYTE)m_PK.GetPKState();
    pRoleStateInfo->wCloseSlayLeftTime      = (WORD)m_PK.GetCloseSlayLeftTime();

    pRoleStateInfo->wLeftReviveFrame    = (WORD)m_ReviveCtrl.nReviveFrame;
    pRoleStateInfo->nLastSituReviveTime = m_ReviveCtrl.nLastSituReviveTime;
    pRoleStateInfo->bySituReviveCount   = (BYTE)m_ReviveCtrl.nSituReviveCount;

    pRoleStateInfo->dwKillerID			    = m_dwKillerID;

    pRoleStateInfo->wCurrentKillPoint       = (WORD)m_nCurrentKillPoint;
    pRoleStateInfo->nReserved0              = 0;

    pRoleStateInfo->nCurrentPrestige        = m_nCurrentPrestige;
    pRoleStateInfo->nBanTime                = m_nBanTime > g_pSO3World->m_nCurrentTime ? m_nBanTime : 0;
    pRoleStateInfo->nContribution           = m_nContribution;
    pRoleStateInfo->nMaxLevel               = m_nMaxLevel;
    pRoleStateInfo->bHideHat                = m_bHideHat;

    pRoleStateInfo->byTalkWorldDailyCount   = (BYTE)m_nTalkWorldDailyCount;
    pRoleStateInfo->byTalkForceDailyCount   = (BYTE)m_nTalkForceDailyCount;
    pRoleStateInfo->byTalkCampDailyCount    = (BYTE)m_nTalkCampDailyCount;
    pRoleStateInfo->byKilledCount           = (BYTE)m_nKilledCount;
    pRoleStateInfo->nNextResetKilledCountTime = m_nNextResetKilledCountTime;

    pRoleStateInfo->byCampFlag              = (BYTE)m_bCampFlag;

    memset(pRoleStateInfo->byReserved, 0, sizeof(pRoleStateInfo->byReserved));

    *puUsedSize = uBufferSize - uLeftSize;

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::SaveRoadOpenNode(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize)
{
    BOOL                            bResult     = false;
    size_t                          uLeftSize   = uBufferSize;
    BYTE*                           pbyOffset   = pbyBuffer;
    KROLE_ROAD_INFO*                pSaveRoad   = NULL;

    KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(KROLE_ROAD_INFO) + (sizeof(int) * m_OpenRouteNodeList.size()));
    pSaveRoad = (KROLE_ROAD_INFO*)pbyOffset;
    pbyOffset += sizeof(KROLE_ROAD_INFO) + (sizeof(int) * m_OpenRouteNodeList.size());
    uLeftSize -= sizeof(KROLE_ROAD_INFO) + (sizeof(int) * m_OpenRouteNodeList.size());

    pSaveRoad->nOpenNodeCount = (int)m_OpenRouteNodeList.size();

    for (int i = 0; i < pSaveRoad->nOpenNodeCount; i++)
    {
        pSaveRoad->OpenNodeList[i] = m_OpenRouteNodeList[i];
    }

    *puUsedSize = uBufferSize - uLeftSize;

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::SaveHeroData(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize)
{
    BOOL                bResult                 = false;
    BOOL                bRetCode                = false;
    size_t              uLeftSize               = uBufferSize;
    BYTE*               pbyOffset               = pbyBuffer;
    KHERO_DATA*         pHeroData               = NULL;
    size_t              uHeroDataLen            = sizeof(KHERO_DATA) + MAX_MAP_ID_DATA_SIZE;
    size_t              uHeroMapCopyOpenDataLen = 0;

    KGLOG_PROCESS_ERROR(uLeftSize >= uHeroDataLen);
    pHeroData = (KHERO_DATA*)pbyOffset;

    pbyOffset += uHeroDataLen;
    uLeftSize -= uHeroDataLen;

    pHeroData->byHeroFlag = (BYTE)m_bHeroFlag;
    pHeroData->byDataLen  = MAX_MAP_ID_DATA_SIZE;

    bRetCode = m_HeroMapCopyOpenFlag.Save(&uHeroMapCopyOpenDataLen, pHeroData->byData, MAX_MAP_ID_DATA_SIZE);
    KGLOG_PROCESS_ERROR(bRetCode);

    *puUsedSize = uBufferSize - uLeftSize;

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::SaveSingleDungeonData(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize)
{
    BOOL bResult = false;

    KGLOG_PROCESS_ERROR(puUsedSize);
    KGLOG_PROCESS_ERROR(pbyBuffer);
    KGLOG_PROCESS_ERROR(uBufferSize >= 0x424);
    KGLOG_PROCESS_ERROR(m_dwSingleDungeonMaxLevel <= 128);

    *(DWORD*)pbyBuffer = m_dwSingleDungeonMaxLevel;
    memcpy(pbyBuffer + 4, m_dwSingleDungeonScore, sizeof(m_dwSingleDungeonScore));
    memcpy(pbyBuffer + 4 + sizeof(m_dwSingleDungeonScore),
           m_dwSingleDungeonCustomData, sizeof(m_dwSingleDungeonCustomData));
    *puUsedSize = 0x424;

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::SaveArenaData(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize)
{
    BOOL              bResult    = false;
    KARENA_ROLE_DATA* pArenaData = NULL;

    KGLOG_PROCESS_ERROR(puUsedSize);
    KGLOG_PROCESS_ERROR(pbyBuffer);
    KGLOG_PROCESS_ERROR(uBufferSize >= sizeof(KARENA_ROLE_DATA));
    pArenaData = (KARENA_ROLE_DATA*)pbyBuffer;
    memset(pArenaData, 0, sizeof(*pArenaData));
    pArenaData->nCorpsChangeTime = m_nCorpsChangeTime;
    pArenaData->nCorpsWeekTime = m_nCorpsWeekTime;
    pArenaData->nCorpsSeasonTime = m_nCorpsSeasonTime;
    memcpy(pArenaData->nCorpsLevel, m_nCorpsLevel, sizeof(m_nCorpsLevel));
    memcpy(pArenaData->nCorpsRoleLevel, m_nCorpsRoleLevel, sizeof(m_nCorpsRoleLevel));
    *puUsedSize = sizeof(KARENA_ROLE_DATA);
    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::SaveAccountStateInfo(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize)
{
    BOOL bResult = false;

    KGLOG_PROCESS_ERROR(puUsedSize);
    KGLOG_PROCESS_ERROR(pbyBuffer);
    KGLOG_PROCESS_ERROR(uBufferSize >= 0x4c);

    KGLOG_PROCESS_ERROR(m_nAccContinuousLoginCount <= 0xfffe);
    memset(pbyBuffer, 0, 0x4c);
    *(int*)(pbyBuffer + 0) = m_nCurrentStamina;
    *(int*)(pbyBuffer + 4) = m_nCurrentThew;
    *(DWORD*)(pbyBuffer + 8) = (DWORD)g_pSO3World->m_nCurrentTime;
    *(WORD*)(pbyBuffer + 12) = (WORD)m_nAccContinuousLoginCount;
    pbyBuffer[14] = (BYTE)m_bContinuousLoginRewardFlag;
    *puUsedSize = 0x4c;
    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::SaveAccount(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize)
{
    BOOL bResult = false;
    BYTE* pbyOffset = NULL;
    BYTE* pbyTail = pbyBuffer + uBufferSize;
    KRoleDataHeader* pGlobalHeader = NULL;
    KRoleBlockHeader* pBlock = NULL;
    size_t uUsedSize = 0;
    size_t uPayloadSize = 0;

    KGLOG_PROCESS_ERROR(puUsedSize);
    KGLOG_PROCESS_ERROR(pbyBuffer);
    KGLOG_PROCESS_ERROR(uBufferSize >= sizeof(KRoleDataHeader));

    pGlobalHeader = (KRoleDataHeader*)pbyBuffer;
    pbyOffset = pbyBuffer + sizeof(KRoleDataHeader);
    KGLOG_PROCESS_ERROR((size_t)(pbyTail - pbyOffset) >= sizeof(KRoleBlockHeader));
    pBlock = (KRoleBlockHeader*)pbyOffset;
    pbyOffset += sizeof(KRoleBlockHeader);
    KGLOG_PROCESS_ERROR(SaveAccountStateInfo(&uUsedSize, pbyOffset, pbyTail - pbyOffset));
    pBlock->nType = 1;
    pBlock->dwVer = 0;
    pBlock->dwLen = (DWORD)uUsedSize;
    pbyOffset += uUsedSize;

    KGLOG_PROCESS_ERROR((size_t)(pbyTail - pbyOffset) >= sizeof(KRoleBlockHeader));
    pBlock = (KRoleBlockHeader*)pbyOffset;
    pbyOffset += sizeof(KRoleBlockHeader);
    KGLOG_PROCESS_ERROR((size_t)(pbyTail - pbyOffset) >= 22);
    KGLOG_PROCESS_ERROR(m_RegressionData.SaveAccountData(&uUsedSize, pbyOffset, pbyTail - pbyOffset));
    pBlock->nType = 2;
    pBlock->dwVer = 0;
    pBlock->dwLen = (DWORD)uUsedSize;
    pbyOffset += uUsedSize;

    uPayloadSize = (size_t)(pbyOffset - pbyBuffer - sizeof(KRoleDataHeader));
    pGlobalHeader->dwVer = 0;
    pGlobalHeader->dwLen = (DWORD)uPayloadSize;
    pGlobalHeader->dwCRC = CRC32(0, pbyBuffer + sizeof(KRoleDataHeader), (DWORD)uPayloadSize);
    *puUsedSize = (size_t)(pbyOffset - pbyBuffer);
    m_nAccountLastSaveTime = g_pSO3World->m_nCurrentTime;
    m_nNextSaveFrame = g_pSO3World->m_nGameLoop
        + g_pSO3World->m_Settings.m_ConstList.nSaveInterval * GAME_FPS;
    bResult = true;
Exit0:
    return bResult;
}

#define SAVE_ROLE_BLOCK(Func, BlockType, Version)   \
    do                                                                                  \
    {                                                                                   \
        KRoleBlockHeader* pBlock    = NULL;                                             \
        size_t            uDataLen  = 0;                                                \
                                                                                        \
        KGLOG_PROCESS_ERROR(pbyTail - pbyOffset >= sizeof(KRoleBlockHeader));           \
        pBlock = (KRoleBlockHeader*)pbyOffset;                                          \
        pbyOffset += sizeof(KRoleBlockHeader);                                          \
                                                                                        \
        bRetCode = Func(&uDataLen, pbyOffset, pbyTail - pbyOffset);                     \
        KGLOG_PROCESS_ERROR(bRetCode);                                                  \
        pBlock->nType = BlockType;                                                      \
        pBlock->dwVer = Version;                                                        \
        pBlock->dwLen = (DWORD)uDataLen;                                                \
        pbyOffset    += uDataLen;                                                       \
    } while (false)

BOOL KPlayer::Save(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize)
{
    BOOL                bResult	        = false;
    BOOL                bRetCode        = false;
    BYTE*               pbyOffset       = pbyBuffer;
    BYTE*               pbyTail         = pbyBuffer + uBufferSize;
    KRoleDataHeader*    pGlobalHeader   = NULL;
    DWORD               dwExtDataLen    = 0;
    time_t              nLastCheckTime  = m_nCurrentLoginTime > m_nLastSaveTime ? m_nCurrentLoginTime : m_nLastSaveTime;
    time_t              nOldDay         = (nLastCheckTime - timezone) / (24 * 3600);
    time_t              nNewDay         = (g_pSO3World->m_nCurrentTime - timezone) / (24 * 3600);

    if (nNewDay != nOldDay)
    {
        bRetCode = RefreshDailyVariable((int)(nNewDay - nOldDay));
        KGLOG_CHECK_ERROR(bRetCode);
    }

    KGLOG_PROCESS_ERROR(pbyTail - pbyOffset >= sizeof(KRoleDataHeader));
    pGlobalHeader = (KRoleDataHeader*)pbyOffset;

    pbyOffset += sizeof(KRoleDataHeader);

    SAVE_ROLE_BLOCK(SaveStateInfo, rbtStateInfo, 0);
    SAVE_ROLE_BLOCK(m_ItemList.Save, rbtItemList, 6);
    SAVE_ROLE_BLOCK(m_ItemList.SaveDelayTradeInfo, rbtDelayTradeItemData, 0);
    SAVE_ROLE_BLOCK(m_ItemList.SaveTimeLimitReturnInfo, rbtTimeLimitReturnItemData, 0);
    SAVE_ROLE_BLOCK(m_ItemList.SaveTimeLimitSoldListInfo, rbtTimeLimitSoldListInfoData, 0);
    SAVE_ROLE_BLOCK(m_ProfessionList.Save, rbtProfessionList, 0);
    SAVE_ROLE_BLOCK(m_RecipeList.Save, rbtRecipeList, CURRENT_RECIPE_LIST_VERSION);
    SAVE_ROLE_BLOCK(m_SkillList.Save, rbtSkillList, 0);
    SAVE_ROLE_BLOCK(SaveSkillRecipeList, rbtSkillRecipeList, 0);
    SAVE_ROLE_BLOCK(SaveQuestStateAndList, rbtQuestList, 0);
    SAVE_ROLE_BLOCK(m_BuffList.Save, rbtBuffList, 0);
    SAVE_ROLE_BLOCK(m_ReputeList.Save, rbtRepute, 0);
    SAVE_ROLE_BLOCK(m_UserPreferences.Save, rbtUserPreferences, 0);
    SAVE_ROLE_BLOCK(m_BookList.Save, rbtBookState, 0);
    SAVE_ROLE_BLOCK(m_TimerList.Save, rbtCoolDownTimer, 0);
    SAVE_ROLE_BLOCK(SaveRoadOpenNode, rbtRoadOpenList, 0);
    SAVE_ROLE_BLOCK(SaveHeroData, rbtHeroData, 0);
    SAVE_ROLE_BLOCK(SaveSingleDungeonData, rbtSingleDungeonData, 0);
    SAVE_ROLE_BLOCK(m_CampActiveStat.Save, rbtCampActiveStat, 0);
    SAVE_ROLE_BLOCK(SaveArenaData, rbtArenaData, 0);
    SAVE_ROLE_BLOCK(SavePendentData, rbtPendentData, 2);
    SAVE_ROLE_BLOCK(SaveActivityVariables, rbtActivityVariables, 0);
    SAVE_ROLE_BLOCK(SaveFellowPetData, rbtFellowPetData, 1);
    SAVE_ROLE_BLOCK(m_CustomData.Save, rbtCustomData, 0);
    SAVE_ROLE_BLOCK(m_SceneVisitRecord.Save, rbtVisitedMap, 0);
    SAVE_ROLE_BLOCK(m_PQList.Save, rbtPQList, 0);
    SAVE_ROLE_BLOCK(SaveRandData, rbtRandData, 0);
    SAVE_ROLE_BLOCK(m_Achievement.Save, rbtAchievementData, 0);
    SAVE_ROLE_BLOCK(m_Designation.Save, rbtDesignationData, 0);
    SAVE_ROLE_BLOCK(m_ExteriorBox.Save, rbtExteriorData, 0);
    SAVE_ROLE_BLOCK(m_HairBox.Save, rbtHairBoxData, 0);
    SAVE_ROLE_BLOCK(m_MiniAvatar.Save, rbtMiniAvatarData, 0);
    SAVE_ROLE_BLOCK(m_RegressionData.Save, rbtRegressionData, 0);
    SAVE_ROLE_BLOCK(m_CurrencyList.Save, rbtCurrencyData, 0);
    SAVE_ROLE_BLOCK(SaveBankPasswordData, rbtBankPasswordData, 0);
    SAVE_ROLE_BLOCK(SaveDropSurpriseData, rbtDropSurpriseData, 0);

    SAVE_ROLE_BLOCK(m_AntiFarmer.Save, rbtAntiFarmerData, 0);
    SAVE_ROLE_BLOCK(SaveMentorData, rbtMentorData, 0);

    dwExtDataLen = (DWORD)(pbyOffset - pbyBuffer - sizeof(KRoleDataHeader));

    pGlobalHeader->dwVer = 0;
    pGlobalHeader->dwCRC = CRC32(0, pbyBuffer + sizeof(KRoleDataHeader), dwExtDataLen);
    pGlobalHeader->dwLen = dwExtDataLen;

    *puUsedSize = pbyOffset - pbyBuffer;

    bRetCode = g_pSO3World->m_FellowshipMgr.SaveFellowshipData(m_dwID);
    KGLOG_PROCESS_ERROR(bRetCode);

    m_nLastSaveTime = g_pSO3World->m_nCurrentTime;

    m_nNextSaveFrame = g_pSO3World->m_nGameLoop 
        + g_pSO3World->m_Settings.m_ConstList.nSaveInterval * GAME_FPS;

    bResult = true;
Exit0:
    return bResult;
}

#undef SAVE_ROLE_BLOCK

BOOL KPlayer::SavePosition()
{
    assert(m_pScene);
    assert(m_pCell);

    m_SavePosition.dwMapID         = m_pScene->m_dwMapID;
    m_SavePosition.nMapCopyIndex   = m_pScene->m_nCopyIndex;
    m_SavePosition.nX              = m_nX;
    m_SavePosition.nY              = m_nY;
    m_SavePosition.nZ              = m_nZ;
    m_SavePosition.nFaceDirection  = m_nFaceDirection;

    return true;
}

BOOL KPlayer::SaveBaseInfo(KRoleBaseInfo* pBaseInfo)
{
    BOOL    bResult         = false;

    pBaseInfo->nVersion                     = 0;
	pBaseInfo->cRoleType                    = (char)m_eRoleType;
	pBaseInfo->byLevel                      = (BYTE)m_nLevel;
    pBaseInfo->byCamp                       = (BYTE)m_eCamp;
	pBaseInfo->byForceID                    = (BYTE)m_dwForceID;
	pBaseInfo->nLastSaveTime                = g_pSO3World->m_nCurrentTime;
	pBaseInfo->CurrentPos.nX                = m_SavePosition.nX;
	pBaseInfo->CurrentPos.nY                = m_SavePosition.nY;
	pBaseInfo->CurrentPos.nZ                = m_SavePosition.nZ;
	pBaseInfo->CurrentPos.nCenterIndex      = 0;
	pBaseInfo->CurrentPos.dwMapID           = m_SavePosition.dwMapID;
	pBaseInfo->CurrentPos.nMapCopyIndex     = m_SavePosition.nMapCopyIndex;
    pBaseInfo->CurrentPos.byFaceDirection   = (BYTE)m_nFaceDirection;
    pBaseInfo->LastEntry.dwMapID            = m_LastEntry.dwMapID;
    pBaseInfo->LastEntry.nMapCopyIndex      = m_LastEntry.nMapCopyIndex;
    pBaseInfo->LastEntry.nX                 = m_LastEntry.nX;
    pBaseInfo->LastEntry.nY                 = m_LastEntry.nY;
    pBaseInfo->LastEntry.nZ                 = m_LastEntry.nZ;
    pBaseInfo->LastEntry.nCenterIndex       = 0;
    pBaseInfo->LastEntry.byFaceDirection    = (BYTE)m_LastEntry.nFaceDirection;
    pBaseInfo->nLastLoginTime               = m_nCurrentLoginTime;
    pBaseInfo->nCreateTime                  = m_nCreateTime;
    pBaseInfo->nTotalGameTime               = (time_t)(m_nTotalGameFrame / GAME_FPS);

    memcpy(pBaseInfo->wRepresentId, m_wRepresentId, sizeof(pBaseInfo->wRepresentId));

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::LoadBaseInfo(KRoleBaseInfo* pBaseInfo)
{
    BOOL        bResult         = false;
    BOOL        bRetCode        = false;
    int         nMaxX           = 0;
    int         nMaxY           = 0;
    KScene*     pScene          = NULL;

    assert(pBaseInfo);

    pScene = g_pSO3World->GetScene(pBaseInfo->CurrentPos.dwMapID, pBaseInfo->CurrentPos.nMapCopyIndex);
    KGLOG_PROCESS_ERROR(pScene);

    nMaxX = pScene->GetRegionWidth() * REGION_GRID_WIDTH * CELL_LENGTH - 1;
    nMaxY = pScene->GetRegionHeight() * REGION_GRID_HEIGHT * CELL_LENGTH - 1;

    MAKE_IN_RANGE(pBaseInfo->CurrentPos.nX, 0, nMaxX);
    MAKE_IN_RANGE(pBaseInfo->CurrentPos.nY, 0, nMaxY);
    MAKE_IN_RANGE(pBaseInfo->CurrentPos.nZ, 0, MAX_Z_POINT);

    // ��ҵ�¼������,Load��ʱ��,��û�������İ�������ӵ��������,���ͻ���AckData��ʱ��Ż���������
    m_SavePosition.dwMapID          = pBaseInfo->CurrentPos.dwMapID;
    m_SavePosition.nMapCopyIndex    = pBaseInfo->CurrentPos.nMapCopyIndex;
    m_SavePosition.nX               = pBaseInfo->CurrentPos.nX;
    m_SavePosition.nY               = pBaseInfo->CurrentPos.nY;
    m_SavePosition.nZ               = pBaseInfo->CurrentPos.nZ;
    m_SavePosition.nFaceDirection   = pBaseInfo->CurrentPos.byFaceDirection;

    m_LastEntry.dwMapID             = pBaseInfo->LastEntry.dwMapID;
    m_LastEntry.nMapCopyIndex       = pBaseInfo->LastEntry.nMapCopyIndex;
    m_LastEntry.nX                  = pBaseInfo->LastEntry.nX;
    m_LastEntry.nY                  = pBaseInfo->LastEntry.nY;
    m_LastEntry.nZ                  = pBaseInfo->LastEntry.nZ;
    m_LastEntry.nFaceDirection      = pBaseInfo->LastEntry.byFaceDirection;

    m_nFaceDirection                = pBaseInfo->CurrentPos.byFaceDirection;
    m_eRoleType                     = (ROLE_TYPE)pBaseInfo->cRoleType;
    m_nLastSaveTime                 = pBaseInfo->nLastSaveTime;
    m_nLastLoginTime                = pBaseInfo->nLastLoginTime;
    m_nCurrentLoginTime             = g_pSO3World->m_nCurrentTime;
    m_nTotalGameFrame               = ((int64_t)pBaseInfo->nTotalGameTime) * GAME_FPS;
    m_nCreateTime                   = pBaseInfo->nCreateTime;

    SetGenderByRoleType(pBaseInfo->cRoleType);

    SetLevel(pBaseInfo->byLevel);

    m_eCamp     = (KCAMP)pBaseInfo->byCamp;
    m_dwForceID = pBaseInfo->byForceID;
    m_dwCorpsSystemID = pBaseInfo->dwCorpsSystemID;

    memcpy(m_wRepresentId, pBaseInfo->wRepresentId, sizeof(m_wRepresentId));

    bResult = true;
Exit0:
    return bResult;
}

void KPlayer::SwitchMap(DWORD dwMapID, int nCopyIndex, int nX, int nY, int nZ)
{
    m_DelayedSwitchMapParam.dwMapID = dwMapID;
    m_DelayedSwitchMapParam.nMapCopyIndex = nCopyIndex;
    m_DelayedSwitchMapParam.nX = nX;
    m_DelayedSwitchMapParam.nY = nY;
    m_DelayedSwitchMapParam.nZ = nZ;
}

BOOL KPlayer::RealSwitchMap(DWORD dwMapID, int nCopyIndex, int nX, int nY, int nZ)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;

    KGLOG_PROCESS_ERROR(m_eGameStatus == gsPlaying);

    // ���ｫԭ����λ�ü�¼����,Search mapʧ�ܵĻ���Ҫ�������¼���ָ�ԭ����λ��
    bRetCode = SavePosition();
    KGLOG_PROCESS_ERROR(bRetCode);

    // ֹͣ�ƶ�������ᵼ�¿ͻ��˺ͷ�����״̬��һ�¡�
    if (m_eMoveState != cmsOnDeath && m_eMoveState != cmsOnAutoFly)
    {
        m_nConvergenceSpeed = 0;
        Stop();
    }

    m_nRecordCount   = 0;

    m_MoveCtrl.bMove = false;
    m_MoveCtrl.nTurn = 0;

	bRetCode = g_pSO3World->RemovePlayer(this);
	KGLOG_PROCESS_ERROR(bRetCode);

	g_RelayClient.SaveRoleData(this);
	g_RelayClient.DoSearchMapRequest(m_dwID, dwMapID, nCopyIndex, nX, nY, nZ);

    m_eGameStatus = gsSearchMap;

    m_nBanishTime = 0; // ����ʱ��������������ڵ���ʱ�������뿪��������ʱ���������ʱ

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::CheckRevive(BOOL bFirstCheck)
{
    BOOL            bResult             = false;
    BOOL            bRetCode            = false;
    const char*     pszFunctionName     = "CheckPlayerRevive";
    int             nLuaTopIndex        = 0;

    assert(m_pScene);

    bRetCode = g_pSO3World->m_ScriptCenter.IsScriptExist(m_pScene->m_dwScriptID);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = g_pSO3World->m_ScriptCenter.IsFuncExist(m_pScene->m_dwScriptID, pszFunctionName);
    KGLOG_PROCESS_ERROR(bRetCode);

    g_pSO3World->m_ScriptCenter.SafeCallBegin(&nLuaTopIndex);

    g_pSO3World->m_ScriptCenter.PushValueToStack(this);
    g_pSO3World->m_ScriptCenter.PushValueToStack((bool)bFirstCheck);

    g_pSO3World->m_ScriptCenter.CallFunction(m_pScene->m_dwScriptID, pszFunctionName, 0);

    g_pSO3World->m_ScriptCenter.SafeCallEnd(nLuaTopIndex);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::Revive(int nMode)
{
    BOOL            bResult             = false;
    BOOL            bRetCode            = false;
    const char*     pszFunctionName     = "OnPlayerRevive";
    int             nLuaTopIndex        = 0;

    assert(m_pScene);

    SetMoveState(cmsOnStand);

    InvalidateMoveState(0, true);

	m_bPositiveShield = false;
	m_bNegativeShield = false;

    bRetCode = g_pSO3World->m_ScriptCenter.IsScriptExist(m_pScene->m_dwScriptID);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = g_pSO3World->m_ScriptCenter.IsFuncExist(m_pScene->m_dwScriptID, pszFunctionName);
    KGLOG_PROCESS_ERROR(bRetCode);

    g_pSO3World->m_ScriptCenter.SafeCallBegin(&nLuaTopIndex);

	g_pSO3World->m_ScriptCenter.PushValueToStack(this);
    g_pSO3World->m_ScriptCenter.PushValueToStack(nMode);

    g_pSO3World->m_ScriptCenter.CallFunction(m_pScene->m_dwScriptID, pszFunctionName, 0);

    g_pSO3World->m_ScriptCenter.SafeCallEnd(nLuaTopIndex);

    bResult = true;
Exit0:
    m_dwKillerID = ERROR_ID;
    return bResult;
}

#endif	//_SERVER


BOOL KPlayer::ChangeRegion(KRegion* pDestRegion)
{
    BOOL bResult  = false;
	BOOL bRetCode = false;

	assert(m_pRegion);
	assert(pDestRegion);

#ifdef _CLIENT
    if (m_dwID == g_pSO3World->m_dwClientPlayerID)
    {
        bRetCode = m_pScene->ValidateRegions(pDestRegion->m_nRegionX, pDestRegion->m_nRegionY);
        KGLOG_PROCESS_ERROR(bRetCode);
    }
#else
    bRetCode = m_pScene->ValidateRegions(pDestRegion->m_nRegionX, pDestRegion->m_nRegionY);
    KGLOG_PROCESS_ERROR(bRetCode);

    RegisterViewObjectByPlayer(m_pRegion, pDestRegion);
#endif

	m_RegionObjNode.Remove();
	pDestRegion->m_PlayerList.AddTail((KNode*)&m_RegionObjNode);
	m_pRegion = pDestRegion;

    bResult = true;
Exit0:
	return bResult;
}

#ifdef _SERVER
void KPlayer::SetCell(KCell* pDstCell)
{
    DWORD dwNewScriptID = 0;
    DWORD dwOldScriptID = m_dwLastScriptID;

    assert(m_pScene);
    assert(m_pRegion);

    if (pDstCell)
    {
        dwNewScriptID = m_pRegion->GetScriptIDByIndex(pDstCell->m_BaseInfo.dwScriptIndex);
    }

    if (m_dwLastScriptID != dwNewScriptID && m_dwLastScriptID)
    {
        CallTrapScript(m_dwLastScriptID, SCRIPT_ON_LEAVE_TRAP);
    }

    m_pCell = pDstCell;

    m_dwLastScriptID = dwNewScriptID;

    if (dwNewScriptID != dwOldScriptID && dwNewScriptID)
    {
        CallTrapScript(dwNewScriptID, SCRIPT_ON_ENTER_TRAP);
    }

    return;
}

BOOL KPlayer::CallTrapScript(DWORD dwScript, const char* szFunction)
{
    BOOL    bResult         = false;
    BOOL    bRetCode        = false;
    int     nLuaTopIndex    = 0;

	bRetCode = g_pSO3World->m_ScriptCenter.IsScriptExist(dwScript);
    KG_PROCESS_ERROR(bRetCode);

    bRetCode = g_pSO3World->m_ScriptCenter.IsFuncExist(dwScript, szFunction);
    KG_PROCESS_ERROR(bRetCode);

	g_pSO3World->m_ScriptCenter.SafeCallBegin(&nLuaTopIndex);

	g_pSO3World->m_ScriptCenter.PushValueToStack(this);

	g_pSO3World->m_ScriptCenter.CallFunction(dwScript, szFunction, 0);

	g_pSO3World->m_ScriptCenter.SafeCallEnd(nLuaTopIndex);

    bResult = true;
Exit0:
    return bResult;
}

#endif

BOOL KPlayer::CostStamina(int nStamina)
{
    BOOL bResult        = false;
	BOOL bRetCode       = false;
	int  nOldStamina    = m_nCurrentStamina;

	KGLOG_PROCESS_ERROR(nStamina >= 0);
	KG_PROCESS_SUCCESS(nStamina == 0);
	KG_PROCESS_ERROR(nStamina <= m_nCurrentStamina);

	m_nCurrentStamina -= nStamina;

#ifdef _SERVER
    g_PlayerServer.DoSyncSelfCurrentST(this);
#endif

Exit1:
	bResult = true;
Exit0:
	return bResult;
}

BOOL KPlayer::CostThew(int nThew)
{
    BOOL bResult        = false;
    BOOL bRetCode       = false;
    int  nOldThew       = m_nCurrentThew;

    KGLOG_PROCESS_ERROR(nThew >= 0);
    KG_PROCESS_SUCCESS(nThew == 0);
    KG_PROCESS_ERROR(nThew <= m_nCurrentThew);

    m_nCurrentThew -= nThew;

#ifdef _SERVER    
    g_PlayerServer.DoSyncSelfCurrentST(this);
#endif

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::LearnProfession(DWORD dwProfessionID)
{
    BOOL bResult  = false;
	BOOL bRetCode = false;

	bRetCode = m_ProfessionList.LearnProfession(dwProfessionID);
	KG_PROCESS_ERROR(bRetCode);

#ifdef _SERVER
	g_PlayerServer.DoLearnProfessionNotify(m_nConnIndex, dwProfessionID);
#endif

	bResult = true;
Exit0:
	return bResult;
}

BOOL KPlayer::LearnBranch(DWORD dwProfessionID, DWORD dwBranchID)
{
    BOOL bResult  = false;
	BOOL bRetCode = false;

	bRetCode = m_ProfessionList.LearnBranch(dwProfessionID, dwBranchID);
	KG_PROCESS_ERROR(bRetCode);

#if defined(_SERVER)
    g_pSO3World->m_StatDataServer.UpdateLearnProfessionBranchState(dwProfessionID, dwBranchID);
	g_PlayerServer.DoLearnBranchNotify(m_nConnIndex, dwProfessionID, dwBranchID);
#endif

	bResult = true;
Exit0:
	return bResult;
}

BOOL KPlayer::ForgetProfession(DWORD dwProfessionID)
{
    BOOL bResult  = false;
	BOOL bRetCode = false;

	bRetCode = m_ProfessionList.ForgetProfession(dwProfessionID);
	KG_PROCESS_ERROR(bRetCode);

#if defined(_SERVER)
	g_PlayerServer.DoForgetProfessionNotify(m_nConnIndex, dwProfessionID);
#endif

	bResult = true;
Exit0:
	return bResult;
}

BOOL KPlayer::AddProfessionProficiency(DWORD dwProfessionID, DWORD dwExp)
{
    BOOL bResult  = false;
	BOOL bRetCode = false;

	bRetCode = m_ProfessionList.AddProfessionProficiency(dwProfessionID, dwExp);
	KG_PROCESS_ERROR(bRetCode);

#ifdef _SERVER
	g_PlayerServer.DoAddProfessionProficiency(m_nConnIndex, dwProfessionID, dwExp);
#endif

	bResult = true;
Exit0:
	return bResult;
}

BOOL KPlayer::SetMaxProfessionLevel(DWORD dwProfessionID, DWORD dwLevel)
{
    BOOL bResult  = false;
	BOOL bRetCode = false;

	bRetCode = m_ProfessionList.SetMaxLevel(dwProfessionID, dwLevel);
	KG_PROCESS_ERROR(bRetCode);

#if defined(_SERVER)
	g_PlayerServer.DoSetMaxProfessionLevelNotify(m_nConnIndex,dwProfessionID, dwLevel);
#endif

	bResult = true;
Exit0:
	return bResult;
}

BOOL KPlayer::CanLearnRecipe(DWORD dwCraftID, DWORD dwRecipeID, DWORD dwMasterID)
{
    BOOL                 bResult        = false;
	BOOL				 bRetCode		= false;
	KRecipeMaster*		 pRecipeMaster	= NULL;
	KRecipeLearningInfo* pLearnInfo		= NULL;

	pRecipeMaster = g_pSO3World->m_ProfessionManager.GetRecipeMaster(dwMasterID);
	KG_PROCESS_ERROR(pRecipeMaster);

	pLearnInfo = pRecipeMaster->GetLearnInfo(dwCraftID, dwRecipeID);
	KG_PROCESS_ERROR(pLearnInfo);

	bRetCode = m_RecipeList.CanLearnRecipe(dwCraftID, dwRecipeID, pLearnInfo);
	KG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KPlayer::LearnRecipe(DWORD dwCraftID, DWORD dwRecipeID)
{
    BOOL bResult  = false;
	BOOL bRetCode = false;

	bRetCode = m_RecipeList.LearnRecipe(dwCraftID, dwRecipeID);
	KG_PROCESS_ERROR(bRetCode);

#if defined(_SERVER)
	g_PlayerServer.DoLearnRecipeNotify(m_nConnIndex, dwCraftID, dwRecipeID);
#endif

	bResult = true;
Exit0:
	return bResult;
}

BOOL KPlayer::ForgetRecipe(DWORD dwProfessionID)
{
    BOOL    bResult     = false;
	BOOL    bRetCode    = false;
	int     nIndex      = 0;
	int     nSize       = 0;
	DWORD   dwCraftID   = 0;
	std::vector<DWORD> vecCraftIDList;

	bRetCode = g_pSO3World->m_ProfessionManager.GetCraftIDList(dwProfessionID, vecCraftIDList);
	KG_PROCESS_ERROR(bRetCode);

	nSize = (int)vecCraftIDList.size();
	for (nIndex = 0; nIndex < nSize; nIndex++)
	{
		dwCraftID = vecCraftIDList[nIndex];
		if (dwCraftID == 0)
			continue;

		bRetCode = m_RecipeList.ForgetRecipe(dwCraftID);
		KGLOG_CHECK_ERROR(bRetCode);

#ifdef _SERVER
        g_PlayerServer.DoForgetRecipeNotify(m_nConnIndex, dwCraftID);
#endif
	}

	bResult = true;
Exit0:
	return bResult;
}

CRAFT_RESULT_CODE KPlayer::CanCastProfessionSkill(DWORD dwCraftID, DWORD dwRecipeID, DWORD dwRBookItemID, KTarget& Target)
{
	CRAFT_RESULT_CODE   nResult     = crcFailed;
    CRAFT_RESULT_CODE   eRetCode    = crcFailed;
	KCraft*             pCraft      = NULL;

    KG_PROCESS_ERROR_RET_CODE(m_OTActionParam.eActionType == otActionIdle, crcDoingOTAction);

	pCraft = g_pSO3World->m_ProfessionManager.GetCraft(dwCraftID);
	KGLOG_PROCESS_ERROR(pCraft);
	
	eRetCode = pCraft->CanCast(this, dwRecipeID, dwRBookItemID, Target);
	KG_PROCESS_ERROR_RET_CODE(eRetCode == crcSuccess, eRetCode);

	nResult = crcSuccess;
Exit0:
	if (nResult != crcSuccess)
	{
		//����ʱ֪ͨ����
#ifdef _SERVER
		KCraftCastState CraftCastState;
		CraftCastState.dwCraftID    = dwCraftID;
		CraftCastState.dwRecipeID   = dwRecipeID;
		CraftCastState.eTargetType  = Target.GetTargetType();
		CraftCastState.dwTargetID   = Target.GetTargetID();

		g_PlayerServer.DoMessageNotify(m_nConnIndex, ectCraftErrorCode, nResult, (void*)&CraftCastState, sizeof(CraftCastState));
#endif

#ifdef _CLIENT
        if (g_pGameWorldUIHandler)
        {
            KUIEventCraftCastRespond UIParam;
            UIParam.nCraftCastRespond   = nResult;
            UIParam.dwCraftID           = dwCraftID;
            UIParam.dwRecipeID          = dwRecipeID;
            UIParam.dwTargetType        = Target.GetTargetType();
            UIParam.dwTargetID          = Target.GetTargetID();
            g_pGameWorldUIHandler->OnCraftCastRespond(UIParam);
        }
#endif
	}

	return nResult;
}

CRAFT_RESULT_CODE KPlayer::CastProfessionSkill(DWORD dwCraftID, DWORD dwRecipeID, DWORD dwRBookItemID, KTarget& Target)
{
	CRAFT_RESULT_CODE   nResult         = crcFailed;
    CRAFT_RESULT_CODE   eRetCode        = crcFailed;
	BOOL                bRetCode        = false;
	KCraft*             pCraft          = NULL;
	KRecipeBase*        pRecipeBase     = NULL;
	KProfessionSkill    ProfessionSkill;

	pCraft = g_pSO3World->m_ProfessionManager.GetCraft(dwCraftID);
	KGLOG_PROCESS_ERROR(pCraft);

	pRecipeBase = pCraft->GetRecipe(dwRecipeID);
	KGLOG_PROCESS_ERROR(pRecipeBase);

#ifdef _SERVER
	eRetCode = CanCastProfessionSkill(dwCraftID, dwRecipeID, dwRBookItemID, Target);
	KG_PROCESS_ERROR_RET_CODE(eRetCode == crcSuccess, eRetCode);

    pCraft->CallBeginScriptFunction(this, dwRecipeID);

    if (pRecipeBase->nPrepareFrame == 0)
    {
    	eRetCode = pCraft->Cast(this, dwRecipeID, dwRBookItemID, Target);
        KG_PROCESS_ERROR_RET_CODE(eRetCode == crcSuccess, eRetCode);
        goto Exit1;
    }
#endif

    KGLOG_PROCESS_ERROR(pRecipeBase->nPrepareFrame > 0);

    ProfessionSkill.Target          = Target;
    ProfessionSkill.dwCraftID       = dwCraftID;
    ProfessionSkill.dwRecipeID      = dwRecipeID;
    ProfessionSkill.dwSourceItemID  = dwRBookItemID;

    bRetCode = DoRecipePrepare(&ProfessionSkill, pRecipeBase->nPrepareFrame);
    KG_PROCESS_ERROR(bRetCode);

#ifdef _SERVER
    if (m_bOnHorse)
    {
        DownHorse();
    }

    g_PlayerServer.DoStartUseRecipe(this, dwCraftID, dwRecipeID, Target);
#else
    if (m_dwID == g_pSO3World->m_dwClientPlayerID)
    {
	    //֪ͨUI�¼�,��ʼ�߽���
        if (g_pGameWorldUIHandler)
        {
            KUIEventRecipeProgress UIParam;
            UIParam.nTotalFrame = pRecipeBase->nPrepareFrame;
            UIParam.dwCraftID = dwCraftID;
            UIParam.dwRecipeID = dwRecipeID;
            g_pGameWorldUIHandler->OnRecipePrepareProgress(UIParam);
        }
    }

    if (g_pGameWorldRepresentHandler)
    {
        g_pGameWorldRepresentHandler->OnCharacterBeginCraftAnimation((KCharacter*)this, dwCraftID);
    }
#endif

Exit1:
	nResult = crcSuccess;
Exit0:
	return nResult;
}

int KPlayer::GetCDValue(DWORD dwCooldownID)
{
    int nResult  = 0;

    nResult = g_pSO3World->m_Settings.m_CoolDownList.GetCoolDownValue(dwCooldownID);
    //KG_ASSERT_EXIT(nDuration > 0);

    // TODO: �������,ĳЩ�츳���Լ���ĳЩCool downʱ��
    // ... ...

    return nResult;
}

BOOL KPlayer::MountKungfu(DWORD dwID, DWORD dwLevel)
{
    BOOL            bResult     = false;
    int             nRetCode    = false;
    KSkill*         pKungfu     = NULL;

    KGLOG_PROCESS_ERROR(m_SkillList.m_dwMountKungfuID == INVALID_SKILL_ID);

    pKungfu = g_pSO3World->m_SkillManager.GetSkill_K(dwID, dwLevel);
    KGLOG_PROCESS_ERROR(pKungfu);

    KGLOG_PROCESS_ERROR(pKungfu->m_pBaseInfo->bIsMountable);

    ApplyAttribute(pKungfu->m_pAttrbuteEffectToSelfAndRollback);

    m_SkillList.m_dwMountKungfuID       = dwID;
    m_SkillList.m_dwMountKungfuLevel    = dwLevel;

    m_dwSchoolID = pKungfu->m_pBaseInfo->dwBelongSchool;

#ifdef _CLIENT
    if (g_pGameWorldUIHandler)
	{
        KUIEventSkillUpdate Param;
        KUIEventUpdatePlayerSchoolID UpdateSchoolID;

		Param.dwSkillID     = pKungfu->m_pBaseInfo->dwSkillID;
		Param.dwSkillLevel  = pKungfu->m_dwLevel;
		g_pGameWorldUIHandler->OnSkillMountKungFu(Param);

        UpdateSchoolID.dwPlayerID = m_dwID;
        UpdateSchoolID.dwSchoolID = m_dwSchoolID;
        g_pGameWorldUIHandler->OnUpdatePlayerSchoolID(UpdateSchoolID);
	}	
#endif

#ifdef _SERVER
    g_PlayerServer.DoSyncKungfuMount(m_nConnIndex, dwID, dwLevel);

    g_PlayerServer.DoBroadcastPlayerSchoolID(this);
#endif

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::UmountKungfu()
{
    BOOL    bResult  = false;
    BOOL    bRetCode = false;
    KSkill* pKungfu  = NULL;

#ifdef _CLIENT
	KUIEventSkillUpdate Param;
	Param.dwSkillID = m_SkillList.m_dwMountKungfuID;
	Param.dwSkillLevel = m_SkillList.m_dwMountKungfuLevel;
	int nNotify = m_SkillList.m_dwMountKungfuID != INVALID_SKILL_ID;
#endif

    if (m_SkillList.m_dwMountKungfuID != INVALID_SKILL_ID)
    {
        pKungfu = g_pSO3World->m_SkillManager.GetSkill_K(m_SkillList.m_dwMountKungfuID, m_SkillList.m_dwMountKungfuLevel);
        KGLOG_PROCESS_ERROR(pKungfu);

        UnApplyAttribute(pKungfu->m_pAttrbuteEffectToSelfAndRollback);
    }

    m_SkillList.m_dwMountKungfuID       = INVALID_SKILL_ID;
    m_SkillList.m_dwMountKungfuLevel    = 0;

    m_dwSchoolID = 0;

#ifdef _CLIENT
	if (nNotify)
    {
        if (g_pGameWorldUIHandler)
        {
		    g_pGameWorldUIHandler->OnSkillUnmountKungfu(Param);
        }
    }
#endif

#ifdef _SERVER
    g_PlayerServer.DoSyncKungfuMount(m_nConnIndex, INVALID_SKILL_ID, 0);
#endif

    bResult = true;
Exit0:
    return bResult;
}

#ifdef _SERVER
BOOL KPlayer::CallDeathScript(DWORD dwKiller)
{
    BOOL        bResult             = false;
	BOOL        bRetCode            = false;
	int         nTopIndex           = 0;
	KPlayer*    pKiller             = NULL;

    assert(m_pScene);

	pKiller = g_pSO3World->m_PlayerSet.GetObj(dwKiller);

	bRetCode = g_pSO3World->m_ScriptCenter.IsScriptExist(m_pScene->m_dwScriptID);
    KGLOG_PROCESS_ERROR(bRetCode);

	g_pSO3World->m_ScriptCenter.SafeCallBegin(&nTopIndex);

	g_pSO3World->m_ScriptCenter.PushValueToStack(this);

	if (pKiller)
    {
		g_pSO3World->m_ScriptCenter.PushValueToStack(pKiller);
    }

	g_pSO3World->m_ScriptCenter.CallFunction(m_pScene->m_dwScriptID, SCRIPT_ON_PLAYER_DEATH, 0);

	g_pSO3World->m_ScriptCenter.SafeCallEnd(nTopIndex);

	bResult = true;
Exit0:
	return bResult;
}
#endif

#ifdef _CLIENT
BOOL KPlayer::CanFinishQuestOnNpc(KNpc* pNpc)
{   
    BOOL       bResult      = false;
    BOOL       bRetCode     = false;
    int        nQuestCounet = 0;
    std::vector<DWORD>  vecQuestID;

    assert(pNpc);

	bRetCode = g_pSO3World->m_Settings.m_QuestInfoList.GetNpcQuest(pNpc->m_dwTemplateID, &vecQuestID);
    KGLOG_PROCESS_ERROR(bRetCode);

    nQuestCounet = (int)vecQuestID.size();
	for (int i = 0; i < nQuestCounet; ++i)
	{
		bRetCode = CanFinishTheQuestOnNpc(vecQuestID[i], pNpc);
        if (bRetCode)
        {
            goto Exit1;
        }
	}

	goto Exit0;

Exit1:
    bResult = true;
Exit0:
	return bResult;
}

BOOL KPlayer::CanFinishTheQuestOnNpc(DWORD dwQuestID, KNpc* pNpc)
{
    BOOL                bResult             = false;
    BOOL                bRetCode            = false;
	QUEST_RESULT_CODE   eCanFinishQuest	    = qrcInvalid;
	KQuestInfo*         pQuestInfo			= NULL;
    int                 nRelation           = sortNone;
    KTarget             Target;

    assert(pNpc);

	KG_PROCESS_ERROR(pNpc->m_bDialogFlag);

    nRelation = pNpc->GetPlayerRelation(this);
    KG_PROCESS_ERROR(!(nRelation & sortEnemy));

    pQuestInfo = g_pSO3World->m_Settings.m_QuestInfoList.GetQuestInfo(dwQuestID);
	KGLOG_PROCESS_ERROR(pQuestInfo);

    bRetCode = Target.SetTarget(pNpc);
    KGLOG_PROCESS_ERROR(bRetCode);

	eCanFinishQuest = m_QuestList.CanFinish(pQuestInfo, &Target);
    KG_PROCESS_ERROR(eCanFinishQuest == qrcSuccess);
	
    bResult = true;
Exit0:
	return bResult;
}

QUEST_DIFFICULTY_LEVEL KPlayer::GetQuestDiffcultyLevel(DWORD dwQuestID)
{
    QUEST_DIFFICULTY_LEVEL      nResult				        = qdlInvalid;
    BOOL                        bRetCode                    = false;
    int*                        pnPlayerAndQuestDiffLevel   = NULL;
    int                         nDiffLevel                  = 0;
    KQuestInfo*                 pQuestInfo                  = NULL;

    QUEST_DIFFICULTY_LEVEL eQuestLevel[PLAYER_AND_QUEST_DIFF_LEVEL_COUNT - 1] = {
        qdlHighLevel, qdlProperLevel, qdlLowLevel
    };

    pQuestInfo = g_pSO3World->m_Settings.m_QuestInfoList.GetQuestInfo(dwQuestID);
    KGLOG_PROCESS_ERROR(pQuestInfo);

    if (pQuestInfo->byLevel == 0) // �Ѷȵȼ�Ϊ0������������ʾ��ɫ
    {
        nResult = qdlProperLevel;
        goto Exit0;
    }

    nDiffLevel = m_nLevel - pQuestInfo->byLevel;

    pnPlayerAndQuestDiffLevel = &(g_pSO3World->m_Settings.m_ConstList.nPlayerAndQuestDiffLevel[0]);

    if (nDiffLevel < pnPlayerAndQuestDiffLevel[0])
    {
        nResult = qdlHigherLevel;
        goto Exit0;
    }

    for (int i = 0; i < PLAYER_AND_QUEST_DIFF_LEVEL_COUNT - 1; ++i)
    {
        if (nDiffLevel >= pnPlayerAndQuestDiffLevel[i] && nDiffLevel < pnPlayerAndQuestDiffLevel[i + 1])
        {
            nResult = eQuestLevel[i];
            goto Exit0;
        }
    }

    if (nDiffLevel >= pnPlayerAndQuestDiffLevel[PLAYER_AND_QUEST_DIFF_LEVEL_COUNT - 1])
    {
        nResult = qdlLowerLevel;
        goto Exit0;
    }

Exit0:
	return nResult;
}
#endif //_CLIENT

// ------ Talk about --------------------------------------------------->
#ifdef _CLIENT
BOOL KPlayer::CopyTalkData(DWORD dwTalkerID, size_t uSize, BYTE* pbyData)
{
    BOOL bResult = false;

    KGLOG_PROCESS_ERROR(m_dwID == g_pSO3World->m_dwClientPlayerID);
    KGLOG_PROCESS_ERROR(uSize > 0 && uSize <= MAX_CHAT_TEXT_LEN);
    KGLOG_PROCESS_ERROR(pbyData);

    memcpy(s_byTalkData, pbyData, uSize);
    s_uTalkDataSize = (unsigned)uSize;

    s_bFilterTalkText = false;
    if (dwTalkerID)
        s_bFilterTalkText = IS_PLAYER(dwTalkerID); // ֻ����ҵ�������Ͽ��Ҫ����

    bResult = true;
Exit0:
    return bResult;
}
#endif

int KPlayer::GetNpcRelation(KNpc* pTarget)
{
	int nRelation = 0;

	BOOL bFightFlag = false;
	int nReputeLevel = 0;

	assert(IS_NPC(pTarget->m_dwID));

	nReputeLevel = m_ReputeList.GetReputeLevel(pTarget->m_dwForceID);

	if (nReputeLevel == NEUTRAL_REPUTE_LEVEL)
	{
		nRelation |= sortNeutrality;
	}

	if (nReputeLevel >= ALLY_REPUTE_LEVEL)
	{
		nRelation |= sortAlly;
		nRelation &= ~sortNeutrality;
	}

#ifdef _SERVER
	if (m_SimpThreatList.IsInThreatList(pTarget) || m_SimpThreatList.IsInThreatenList(pTarget))
	{
		nRelation |= sortEnemy;
		nRelation &= ~sortAlly;
		nRelation &= ~sortNeutrality;
	}
#endif

#ifdef _CLIENT
	if (m_ThreatList.IsInThreatList(pTarget->m_dwID) || m_ThreatList.IsInThreatenList(pTarget->m_dwID))
	{
		nRelation |= sortEnemy;
		nRelation &= ~sortAlly;
		nRelation &= ~sortNeutrality;
	}
#endif

	bFightFlag = m_ReputeList.GetFightFlag(pTarget->m_dwForceID);
	if (bFightFlag)
	{
		nRelation |= sortEnemy;
		nRelation &= ~sortAlly;
		nRelation &= ~sortNeutrality;
	}

	return nRelation;
}

BOOL KPlayer::IsInRaid()
{
    BOOL    bResult     = false;
    DWORD   dwTeamID    = ERROR_ID;

#ifdef _SERVER
    KTeam*  pTeam       = NULL;
    dwTeamID = GetCurrentTeamID();
    KG_PROCESS_ERROR(dwTeamID != ERROR_ID);

    pTeam = g_pSO3World->m_TeamServer.GetTeam(dwTeamID);
    KG_PROCESS_ERROR(pTeam);
    KG_PROCESS_ERROR(pTeam->nGroupNum == MAX_TEAM_GROUP_NUM);
#else
    KG_PROCESS_ERROR(g_pSO3World->m_TeamClient.m_dwTeamID != ERROR_ID);
    KG_PROCESS_ERROR(g_pSO3World->m_TeamClient.m_nGroupNum == MAX_TEAM_GROUP_NUM);
#endif

    bResult = true;
Exit0:
    return bResult;
}

int KPlayer::GetPlayerRelation(KPlayer* pTarget)
{
	int     nRelation           = 0;
    BOOL    bRetCode            = false;
    int     nMinBeSlayedLevel   = g_pSO3World->m_Settings.m_ConstList.nMinBeSlayedLevel;
    int     nValue              = 0;
    DWORD   dwTeamID            = GetCurrentTeamID();
    KCell*  pSelfCell           = NULL;
    KCell*  pTargetCell         = NULL;
    KCAMP   eTargetCamp         = cNeutral;

    assert(pTarget);
	assert(IS_PLAYER(pTarget->m_dwID));
    assert(m_pScene);
    assert(pTarget->m_pScene);

    eTargetCamp = pTarget->m_eCamp;
    if (!pTarget->m_bCampFlag || pTarget->m_pScene->m_nCampType == emctAllProtect)
    {
        eTargetCamp = cNeutral;
    }

    nValue = g_pSO3World->m_Settings.m_RelationCampList.GetRelation(m_eCamp, eTargetCamp);
    switch(nValue)
    {
    case -1:
	    nRelation |= sortEnemy;
	    nRelation &= ~sortAlly;
	    nRelation &= ~sortNeutrality;

        assert(m_pRegion);
        assert(pTarget->m_pRegion);

        pSelfCell = m_pRegion->GetLowestObstacle(m_nXCell, m_nYCell);
        pTargetCell = pTarget->m_pRegion->GetLowestObstacle(m_nXCell, m_nYCell);

        if (pSelfCell && pTargetCell && pSelfCell->m_BaseInfo.dwRest || pTargetCell->m_BaseInfo.dwRest) // ��������Ϣ����������
        {
            nRelation |= sortNeutrality; 
	        nRelation &= ~sortAlly;
	        nRelation &= ~sortEnemy;
        }

	    break;
    case 0:
	    nRelation |= sortNeutrality;
	    nRelation &= ~sortAlly;
	    nRelation &= ~sortEnemy;
	    break;
    case 1:
	    nRelation |= sortAlly;
	    nRelation &= ~sortNeutrality;
	    nRelation &= ~sortEnemy;
	    break;
    default:
	    break;
    }

#ifdef _SERVER
	if (m_SimpThreatList.IsInThreatList(pTarget))
	{
		nRelation |= sortEnemy;
		nRelation &= ~sortAlly;
		nRelation &= ~sortNeutrality;
	}
#endif

#ifdef _CLIENT
	if (m_ThreatList.IsInThreatList(pTarget->m_dwID))
	{
        nRelation |= sortEnemy;
        nRelation &= ~sortAlly;
        nRelation &= ~sortNeutrality;
	}
#endif

    if (m_pScene->m_bCanPK)
    {
        if (g_pSO3World->m_FellowshipMgr.GetFoe(m_dwID, pTarget->m_dwID) != NULL)
        {
            nRelation |= sortEnemy;
            nRelation &= ~sortAlly;
            nRelation &= ~sortNeutrality;
        }

	    if (
            ((m_PK.GetState() == pksSlaying || m_PK.GetState() == pksExitSlay) && pTarget->m_nLevel >= nMinBeSlayedLevel) ||
            ((pTarget->m_PK.GetState() == pksSlaying || pTarget->m_PK.GetState() == pksExitSlay) && m_nLevel >= nMinBeSlayedLevel)
        )
	    {
		    nRelation |= sortEnemy;
		    nRelation &= ~sortAlly;
		    nRelation &= ~sortNeutrality;
	    }

	    if (
            (m_PK.GetTargetPlayer() == pTarget && ((m_PK.GetState() == pksDueling) || (m_PK.GetState() == pksDuelOutOfRange))) ||
             pTarget->m_PK.GetTargetPlayer() == this && ((pTarget->m_PK.GetState() == pksDueling) || (pTarget->m_PK.GetState() == pksDuelOutOfRange))
        )
	    {
		    nRelation |= sortEnemy;
		    nRelation &= ~sortAlly;
		    nRelation &= ~sortNeutrality;
	    }

        if (pTarget->m_bRedName)
        {
            nRelation |= sortEnemy;
		    nRelation &= ~sortAlly;
		    nRelation &= ~sortNeutrality;
        }
    }

	if (dwTeamID != ERROR_ID)
	{
#ifdef _SERVER
        bRetCode = g_pSO3World->m_TeamServer.IsPlayerInTeam(dwTeamID, pTarget->m_dwID);
#else
        bRetCode = g_pSO3World->m_TeamClient.IsPlayerInTeam(pTarget->m_dwID);
#endif
        if (bRetCode)
        {
		    nRelation |= sortParty;
		    nRelation |= sortAlly;
		    nRelation &= ~sortEnemy;
		    nRelation &= ~sortNeutrality;
        }
	}
	
	if (this == pTarget)
	{
		nRelation = sortSelf;
	}

	return nRelation;
}

#ifdef _CLIENT
BOOL KPlayer::SwitchSelectTarget(DWORD dwSkillSrcID)
{
    BOOL        bResult     = false;
    BOOL        bRetCode    = false;
    TARGET_TYPE eTargetType = ttInvalid;
    KCharacter* pCaster     = NULL;

    if (IS_PLAYER(dwSkillSrcID))
    {
        int nRelation = 0;
        pCaster = (KCharacter*)g_pSO3World->m_PlayerSet.GetObj(dwSkillSrcID);
        KG_PROCESS_ERROR(pCaster);
        nRelation = ((KPlayer*)pCaster)->GetPlayerRelation(this);
        KG_PROCESS_ERROR(nRelation & sortEnemy);

        eTargetType = ttPlayer;
    }
    else    
    {
        int     nRelation = 0;
        KNpc*   pNpc      = NULL;

        pCaster = (KCharacter*)g_pSO3World->m_NpcSet.GetObj(dwSkillSrcID);
        KG_PROCESS_ERROR(pCaster);

        pNpc = (KNpc*)pCaster;
        nRelation = ((KNpc*)pCaster)->GetPlayerRelation(this);
        KG_PROCESS_ERROR(nRelation & sortEnemy);

        KG_PROCESS_ERROR(pNpc->m_pTemplate);
        KG_PROCESS_ERROR(pNpc->m_pTemplate->bSelectable);

        eTargetType = ttNpc;
    }

    bRetCode = SelectTarget(eTargetType, dwSkillSrcID);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}
#endif

#if defined(_SERVER)
BOOL KPlayer::ReverseFrame(int nFrame)
{
    BOOL                bResult         = false;
    BOOL                bRetCode        = false;
    int                 nReverseFrame   = 0;
    int                 nRecordIndex    = 0;
    KPOSITION_RECORD*   pRecord         = NULL;

    assert(nFrame > 0);

    nReverseFrame = min(m_nRecordCount, nFrame);
    KG_PROCESS_ERROR(nReverseFrame > 0);

    nRecordIndex = (m_nVirtualFrame - nReverseFrame) % MAX_POSITION_RECORD;

    pRecord = &m_PositionRecord[nRecordIndex];

    bRetCode = ReversePosition(pRecord);
    KG_PROCESS_ERROR(bRetCode);

    m_nVirtualFrame -= nReverseFrame;

    bResult = true;
Exit0:
    // ����һ�λع��Ժ�,��ʷ��¼��������,�����ٻع�����֮ǰ
    m_nRecordCount = 0;
    return bResult;
}

BOOL KPlayer::CheckMoveAdjust(int nClientFrame, const KMOVE_CRITICAL_PARAM& crParam)
{
    BOOL bResult        = true;

    if (m_nVirtualFrame > nClientFrame)
    {
        ReverseFrame(m_nVirtualFrame - nClientFrame);
    }
    else if (m_nVirtualFrame < nClientFrame)
    {
        ForwardPosition(nClientFrame - m_nVirtualFrame);
    }

    KG_PROCESS_ERROR(m_eMoveState == crParam.byMoveState);
    KG_PROCESS_ERROR(m_nRunSpeed == crParam.nRunSpeed);
    KG_PROCESS_ERROR(m_nCurrentGravity == crParam.nGravity);
    KG_PROCESS_ERROR(m_nJumpSpeed == crParam.nJumpSpeed);
    KG_PROCESS_ERROR(m_nX == crParam.nX);
    KG_PROCESS_ERROR(m_nY == crParam.nY);
    KG_PROCESS_ERROR(m_nZ == crParam.nZ);

    bResult = false; // ����false��ʾ��������
Exit0:
    return bResult;
}
#endif

#ifdef _SERVER
void KPlayer::DoCycleSynchronous()
{
    int nFrame = g_pSO3World->m_nGameLoop - (int)m_dwID;

    // ���Լ�ͬ�����Ѫ��
    if (nFrame % 5 == 0)
    {
        g_PlayerServer.DoSyncSelfMaxLMRS(this);
    }

    // ���Լ�ͬ����ǰѪ��
    if (nFrame % 3 == 0)
    {
        g_PlayerServer.DoSyncSelfCurrentLMRS(this);
    }

    // ���Լ�ͬ����ǰ����
    if (nFrame % 5 == 0)
    {
        g_PlayerServer.DoSyncSelfWeakInfo(this);
    }

    // ���Լ�ͬ��Ŀ������Ѫ��
    if (nFrame % 8 == 0)
    {
        g_PlayerServer.DoSyncTargetMaxLMR(this);
    }

    // ���Լ�ͬ��Ŀ��ĵ�ǰѪ��
    if (nFrame % 4 == 0)
    {
        g_PlayerServer.DoSyncTargetCurrentLMR(this);
    }

    // ���Լ�ͬ��Ŀ��ĵ�ǰ����
    if (nFrame % 4 == 0)
    {
        g_PlayerServer.DoSyncTargetWeakInfo(this);
    }

    // ͬ�����ѡ��Ŀ���Buff list
    if (nFrame % GAME_FPS == 0)
    {
        g_PlayerServer.DoSyncTargetBuffList(this);
    }

    // ͬ�����ѡ��Ŀ���������
    if (nFrame % GAME_FPS == 0)
    {
        g_PlayerServer.DoSyncTargetDropID(this);
    }

    // ͬ�����ѡ���Ŀ���Ŀ������Ѫ��
    if (nFrame % GAME_FPS == 0)
    {
        g_PlayerServer.DoSyncTargetTargetMaxLMR(this);
    }

    // ͬ�����ѡ���Ŀ���Ŀ��ĵ�ǰѪ��
    if (nFrame % 8 == 0)
    {
        g_PlayerServer.DoSyncTargetTargetCurrentLMR(this);
    }

    // ͬ�����ѡ���Ŀ���Ŀ���Buff List
    if (nFrame % GAME_FPS == 0)
    {
        g_PlayerServer.DoSyncTargetTargetBuffList(this);
    }

    // ȫ��ͬ���������Ѫ����
    if (nFrame % GAME_FPS == 0)
    {
	    g_RelayClient.DoTeamSyncMemberMaxLMR(this);
    }

    // ���ͬ�����ѵ�ǰѪ����
    if (nFrame % GAME_FPS == 0)
    {
	    g_RelayClient.DoTeamSyncMemberCurrentLMR(this);
    }

    // ͬ���������ѵ�ǰ����,ע��,���ͬ����Ƶ�ʻᱻ���ͬ������
    if (nFrame % 7 == 0)
    {
	    g_PlayerServer.DoSyncTeamMemberCurrentLMRLocal(this);
    }

    // ���ͬ�����ѵ�ǰλ��
    if (nFrame % (GAME_FPS * 2) == 0)
    {
	    g_RelayClient.DoSyncTeamMemberPosition(this);
    }

    // ����ͬ�����ѵ�ǰλ��,ע��,���ͬ����Ƶ�ʻᱻ���ͬ������
    if (nFrame % GAME_FPS == 0)
    {
	    g_PlayerServer.DoSyncTeamMemberPositionLocal(this);
    }

    // ȫ��ͬ�������������
    if (nFrame % (2 * GAME_FPS) == 0)
    {
	    g_RelayClient.DoTeamSyncMemberMisc(this);
    }

    // ����Χ�㲥��ǰѪ��
    if (nFrame % GAME_FPS == 0)
    {
        g_PlayerServer.DoBroadcastCharacterLife(this);
    }

    // ͬ����������
    if (nFrame % GAME_FPS == 0)
    {
	    g_PlayerServer.DoSyncSelfCurrentST(this);
    }

    // ͬ����Ӫ��Ϣ
    if (nFrame % (GAME_FPS * 30) == 0)
    {
        g_PlayerServer.DoSyncCampInfo(this);
    }
}

void KPlayer::ResetTeamLastSyncParamRecord()
{
    m_nLastSyncLocalTeamLifePercent            = -1;
    m_nLastSyncLocalTeamManaPercent            = -1;
    m_nLastSyncLocalTeamRagePercent            = -1;
    m_dwLastSyncLocalTeamMemberMapID           = 0;
    m_nLastSyncLocalTeamMemberMapCopyIndex     = 0;
    m_nLastSyncLocalTeamMemberPosX             = -1;
    m_nLastSyncLocalTeamMemberPosY             = -1;

    m_nLastSyncGlobalTeamLifePercent           = -1;
    m_nLastSyncGlobalTeamManaPercent           = -1;
    m_nLastSyncGlobalTeamRagePercent           = -1;
    m_dwLastSyncGlobalTeamMemberMapID          = 0;
    m_nLastSyncGlobalTeamMemberMapCopyIndex    = 0;
    m_nLastSyncGlobalTeamMemberPosX            = -1;
    m_nLastSyncGlobalTeamMemberPosY            = -1;
}

#endif

BOOL KPlayer::RideHorse()
{
    int             nResult     = false;
    int             nRetCode    = false;
    KItem*          pHorse      = NULL;

    pHorse = m_ItemList.GetItem(ibEquip, eitHorse);
    KGLOG_PROCESS_ERROR(pHorse);

    KG_PROCESS_ERROR(!pHorse->m_bApplyed);
    KG_PROCESS_ERROR(!m_bOnHorse);

    nRetCode = m_ItemList.ApplyAttrib(pHorse);
    KGLOG_PROCESS_ERROR(nRetCode);

    m_bOnHorse = true;

#ifdef _SERVER
    g_PlayerServer.DoSyncHorseFlag(this);
#endif

    nResult = true;
Exit0:
    return nResult;    
}

BOOL KPlayer::DownHorse()
{
    int             nResult     = false;
    int             nRetCode    = false;
    KItem*          pHorse      = NULL;

    pHorse = m_ItemList.GetItem(ibEquip, eitHorse);
    KGLOG_PROCESS_ERROR(pHorse);

    KG_PROCESS_ERROR(pHorse->m_bApplyed);
    KG_PROCESS_ERROR(m_bOnHorse);

    nRetCode = m_ItemList.UnApplyAttrib(pHorse);
    KGLOG_PROCESS_ERROR(nRetCode);

    m_bOnHorse = false;

#ifdef _SERVER
    g_PlayerServer.DoSyncHorseFlag(this);
#endif

    nResult = true;
Exit0:
    return nResult;    
}

#ifdef _SERVER
BOOL KPlayer::AddTrain(int nTrain)
{
    int             nResult         = false;
    int             nOldTrainValue  = m_nCurrentTrainValue;

    KG_PROCESS_ERROR(nOldTrainValue + nTrain >= 0);

    m_nCurrentTrainValue += nTrain;

    if (nTrain >= 0)
    {
        m_nCurrentTrainValue = min(m_nMaxTrainValue, m_nCurrentTrainValue); // �Ƿ񳬹�����
        m_nCurrentTrainValue = max(m_nCurrentTrainValue, nOldTrainValue);   // �Ѿ��������޵ģ�ֻ�ǲ�����
    }
    // else �����������������

    g_LogClient.LogTrainvalueChange(m_nCurrentTrainValue - nOldTrainValue, m_szAccount, m_szName);

    g_PlayerServer.DoSyncPlayerTrain(this);
    g_pSO3World->m_StatDataServer.UpdateTrainState(nTrain);

    nResult = true;
Exit0:
    return nResult;
}

BOOL KPlayer::AddTrainNoLimit(int nTrain)
{
    int nResult = false;

    KG_PROCESS_ERROR(m_nCurrentTrainValue + nTrain >= 0);

    m_nCurrentTrainValue += nTrain;

    g_LogClient.LogTrainvalueChange(nTrain, m_szAccount, m_szName);

    g_PlayerServer.DoSyncPlayerTrain(this);
    g_pSO3World->m_StatDataServer.UpdateTrainState(nTrain);

    nResult = true;
Exit0:
    return nResult;
}

BOOL KPlayer::ShareQuest(DWORD dwQuestID)
{
    BOOL                bResult         = false;
    BOOL                bRetCode        = false;
    int                 nCode           = 0;
    QUEST_RESULT_CODE   eQuestRetCode   = qrcFailed;
    KQuestInfo*         pQuestInfo      = NULL;
    KTeam*              pTeam           = NULL;
    KPlayer*            pMember         = NULL;
    KTarget             Target;
    KSHARE_QUEST_RESULT ShareQuestResult;

	pQuestInfo = g_pSO3World->m_Settings.m_QuestInfoList.GetQuestInfo(dwQuestID);
	KGLOG_PROCESS_ERROR(pQuestInfo);

	KG_PROCESS_ERROR(pQuestInfo->bShare);

	KGLOG_PROCESS_ERROR(m_dwTeamID != ERROR_ID);

	pTeam = g_pSO3World->m_TeamServer.GetTeam(m_dwTeamID);
	KGLOG_PROCESS_ERROR(pTeam);

    Target.SetTarget(this);

    ShareQuestResult.dwQuestID = dwQuestID;

    for (int i = 0; i < pTeam->nGroupNum; i++)
	{
        for (
            KTEAM_MEMBER_LIST::iterator it = pTeam->MemberGroup[i].MemberList.begin();
            it != pTeam->MemberGroup[i].MemberList.end(); ++it
        )
        {
            if (it->dwMemberID == m_dwID)
            {
                continue;
            }

            pMember = g_pSO3World->m_PlayerSet.GetObj(it->dwMemberID);
            if (pMember == NULL)
            {
                continue;
            }

            ShareQuestResult.dwTargetPlayerID = pMember->m_dwID;

            bRetCode = g_InRange(this, pMember, CELL_LENGTH * REGION_GRID_WIDTH);
            if (!bRetCode)
            {
                g_PlayerServer.DoMessageNotify(m_nConnIndex, ectShareQuestErrorCode, sqrcTooFar, &ShareQuestResult, sizeof(ShareQuestResult));
                continue;
            }

            eQuestRetCode = pMember->m_QuestList.CanAccept(pQuestInfo, &Target);

            switch (eQuestRetCode)
            {
            case qrcSuccess:
                nCode = sqrcSuccess;
                g_PlayerServer.DoShareQuest(pMember, this, dwQuestID);
                break;
            case qrcAlreadyAcceptQuest:
                nCode = sqrcAlreadyAcceptQuest;
                break;
            case qrcAlreadyFinishedQuest:
                nCode = sqrcAlreadyFinishedQuest;
                break;
            case qrcQuestListFull:
                nCode = sqrcQuestListFull;
                break;
            default:
                nCode = sqrcFailed;
                break;
            }

            g_PlayerServer.DoMessageNotify(m_nConnIndex, ectShareQuestErrorCode, nCode, &ShareQuestResult, sizeof(ShareQuestResult));
        }
	}

    bResult = true;
Exit0:
    return bResult;
}
#endif

#ifdef _SERVER
BOOL KPlayer::IsFormationLeader()
{
    BOOL    bResult     = false;
    KTeam*  pTeam       = NULL;
    DWORD   dwTeamID    = GetCurrentTeamID();

    KG_PROCESS_ERROR(dwTeamID != ERROR_ID);

    pTeam = g_pSO3World->m_TeamServer.GetTeam(dwTeamID);
    KGLOG_PROCESS_ERROR(pTeam);

    for (int i = 0; i < pTeam->nGroupNum; i++)
    {
        if (m_dwID == pTeam->MemberGroup[i].dwFormationLeader)
        {
            bResult = true;
            goto Exit0;
        }
    }

Exit0:
    return bResult;
}
#endif

#ifdef _SERVER
BOOL KPlayer::CanSetFormationLeader()
{
    BOOL    bResult             = false;
    BOOL    bISFormationLeader  = false;
    KTeam*  pTeam               = NULL;
    DWORD   dwTeamID            = GetCurrentTeamID();

    KG_PROCESS_ERROR(dwTeamID != ERROR_ID);

    pTeam = g_pSO3World->m_TeamServer.GetTeam(dwTeamID);
    KGLOG_PROCESS_ERROR(pTeam);

    bISFormationLeader = IsFormationLeader();

    KG_PROCESS_ERROR(bISFormationLeader || m_dwID == pTeam->dwAuthority[tatLeader]);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::SyncFormationCoefficient()
{
    BOOL    bResult  = false;
    KTeam*  pTeam    = NULL;
    DWORD   dwTeamID = GetCurrentTeamID();

    KG_PROCESS_SUCCESS(dwTeamID == ERROR_ID); // û�ж���Ͳ���ͬ����

    pTeam = g_pSO3World->m_TeamServer.GetTeam(dwTeamID);
    if (!pTeam)
    {
        KGLogPrintf(KGLOG_ERR, "[SyncFormationCoefficient] Team:%lu can't found.\n", dwTeamID);
        goto Exit0;
    }

    for (int i = 0; i < pTeam->nGroupNum; i++)
    {
        for (
            KTEAM_MEMBER_LIST::iterator it = pTeam->MemberGroup[i].MemberList.begin();
            it != pTeam->MemberGroup[i].MemberList.end(); ++it
        )
        {
            KPlayer*        pMember         = NULL;
            KGFellowship*   pFellowship     = NULL;
            int             nCoefficient    = 0;

            if (m_dwID == it->dwMemberID)
                continue;

            pMember = g_pSO3World->m_PlayerSet.GetObj(it->dwMemberID);

            if (!pMember)
                continue;

            pFellowship = g_pSO3World->m_FellowshipMgr.GetFellowship(m_dwID, it->dwMemberID);
            if (pFellowship)
            {
                nCoefficient += pFellowship->m_nAttraction;
            }

            pFellowship = g_pSO3World->m_FellowshipMgr.GetFellowship(it->dwMemberID, m_dwID);
            if (pFellowship)
            {
                nCoefficient += pFellowship->m_nAttraction;
            }

            if (nCoefficient > 0)
            {
                g_PlayerServer.DoSyncFormationCoefficient(pMember, m_dwID, nCoefficient);
            }
        }
    }

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::ProcessTeamMemberAttraction()
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;
    int     nGroupIndex = INVALID_TEAM_GROUP;
    KTeam*  pTeam       = NULL;
    DWORD   dwTeamID    = GetCurrentTeamID();

    KGLOG_PROCESS_ERROR(m_pScene);

    pTeam = g_pSO3World->m_TeamServer.GetTeam(dwTeamID);
    KG_PROCESS_ERROR(pTeam);

    nGroupIndex = GetCurrentTeamGroupIndex();
    KGLOG_PROCESS_ERROR(nGroupIndex >= 0 && nGroupIndex < pTeam->nGroupNum);

    for (int i = 0; i < pTeam->nGroupNum; i++)
    {
        if (i == nGroupIndex)
            continue;

        for (
            KTEAM_MEMBER_LIST::iterator it = pTeam->MemberGroup[i].MemberList.begin();
            it != pTeam->MemberGroup[i].MemberList.end(); ++it
        )
        {
            KTEAM_MEMBER_INFO*  pMemberInfo = &(*it);
            KPlayer*            pMember     = NULL;
            KGFellowship*       pFellowship = NULL;

            if (pMemberInfo->dwMemberID == m_dwID)
                continue;

            if (pMemberInfo->dwMapID != m_pScene->m_dwMapID)
                continue;

            pMember = g_pSO3World->m_PlayerSet.GetObj(pMemberInfo->dwMemberID);
            KG_PROCESS_ERROR(pMember);

            bRetCode = g_InRange(this, pMember, g_pSO3World->m_Settings.m_ConstList.nTeamAttractiveRange);
            if (!bRetCode)
                continue;

            pFellowship = g_pSO3World->m_FellowshipMgr.GetFellowship(m_dwID, pMemberInfo->dwMemberID);
            if (!pFellowship)
                continue;

            if (pFellowship->m_nAttraction >= g_pSO3World->m_Settings.m_ConstList.nTeamAttractiveUpLimit)
                continue;

            g_pSO3World->m_FellowshipMgr.AddFellowshipAttraction(
                m_dwID, pMemberInfo->dwMemberID, g_pSO3World->m_Settings.m_ConstList.nTeamAttractiveAdd, false
            );
        }
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::ProcessTeamMemberOnlyGroupAttraction()
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;
    int     nGroupIndex = INVALID_TEAM_GROUP;
    KTeam*  pTeam       = NULL;
    DWORD   dwTeamID    = GetCurrentTeamID();

    KGLOG_PROCESS_ERROR(m_pScene);

    pTeam = g_pSO3World->m_TeamServer.GetTeam(dwTeamID);
    KG_PROCESS_ERROR(pTeam);

    nGroupIndex = GetCurrentTeamGroupIndex();
    KGLOG_PROCESS_ERROR(nGroupIndex >= 0 && nGroupIndex < pTeam->nGroupNum);

    for (
        KTEAM_MEMBER_LIST::iterator it = pTeam->MemberGroup[nGroupIndex].MemberList.begin();
        it != pTeam->MemberGroup[nGroupIndex].MemberList.end(); ++it
    )
    {
        KTEAM_MEMBER_INFO*  pMemberInfo = &(*it);
        KPlayer*            pMember     = NULL;
        KGFellowship*       pFellowship = NULL;

        if (pMemberInfo->dwMemberID == m_dwID)
            continue;

        if (pMemberInfo->dwMapID != m_pScene->m_dwMapID)
            continue;

        pMember = g_pSO3World->m_PlayerSet.GetObj(pMemberInfo->dwMemberID);
        KG_PROCESS_ERROR(pMember);

        bRetCode = g_InRange(this, pMember, g_pSO3World->m_Settings.m_ConstList.nTeamAttractiveRangeOnlyGroup);
        if (!bRetCode)
            continue;

        pFellowship = g_pSO3World->m_FellowshipMgr.GetFellowship(m_dwID, pMemberInfo->dwMemberID);
        if (!pFellowship)
            continue;

        if (pFellowship->m_nAttraction >= g_pSO3World->m_Settings.m_ConstList.nTeamAttractiveUpLimitOnlyGroup)
            continue;

        g_pSO3World->m_FellowshipMgr.AddFellowshipAttraction(
            m_dwID, pMemberInfo->dwMemberID, g_pSO3World->m_Settings.m_ConstList.nTeamAttractiveAddOnlyGroup, false
        );
    }

    bResult = true;
Exit0:
    return bResult;
}
#endif

BOOL KPlayer::CanAddFoe()
{
    BOOL        bResult             = false;
    int         nMinRevengeLevel    = g_pSO3World->m_Settings.m_ConstList.nMinRevengeLevel;

    KG_PROCESS_ERROR(m_nLevel >= nMinRevengeLevel);
    KG_PROCESS_ERROR(!m_bFightState);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::IsFoe(DWORD dwDestPlayerID)
{
    BOOL    bResult     = false;    
    KGFoe*  pFoe        = NULL;

    pFoe = g_pSO3World->m_FellowshipMgr.GetFoe(m_dwID, dwDestPlayerID);
    KG_PROCESS_ERROR(pFoe);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::SetCamp(KCAMP eNewCamp)
{
    BOOL    bResult         = false;
    int     nOldPrestige    = 0;

    KGLOG_PROCESS_ERROR(m_eCamp != eNewCamp);

    m_eCamp = eNewCamp;

#ifdef _SERVER
    nOldPrestige       = m_nCurrentPrestige;
    m_nCurrentPrestige = 0;
    g_PlayerServer.DoSyncCurrentPrestige(this);

    g_PlayerServer.DoSetCamp(this);

    m_ItemList.OnChangeCamp();

    CallChangeCampScript();

    g_pSO3World->m_StatDataServer.UpdatePrestigeStat(this, -nOldPrestige, "OTHER");
#endif

#ifdef _CLIENT
    if (g_pGameWorldUIHandler && g_pGameWorldRepresentHandler)
    {
	    if (m_dwID == g_pSO3World->m_dwClientPlayerID)
	    {
		    KPlayer* pClientPlayer = (KPlayer*)this;
		    KGLOG_PROCESS_ERROR(pClientPlayer);

            pClientPlayer->m_QuestList.UpdateNpcQuestMark(-1); // ����������ΧNpc��������

            g_pGameWorldUIHandler->OnUpdateAllRelation();
            g_pGameWorldRepresentHandler->OnCharacterUpdateAllRelation();
	    }
        else
        {
            KUIEventUpdateRelation param = { m_dwID };

            g_pGameWorldUIHandler->OnUpdateRelation(param);
            g_pGameWorldRepresentHandler->OnCharacterUpdateRelation(this);
        }

        KUIEventChangeCamp param = { m_dwID };
        g_pGameWorldUIHandler->OnChangeCamp(param);
    }
#endif

    if (eNewCamp == cNeutral)
    {
        m_bCampFlag = false;
        m_nCloseCampFlagTime = 0;
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::CanOpenCampFlag()
{
    BOOL bResult = false;

    assert(m_pScene);

    KG_PROCESS_ERROR(m_eCamp != cNeutral);

    KG_PROCESS_ERROR(!m_bCampFlag || m_nCloseCampFlagTime != 0);
    KG_PROCESS_ERROR(m_pScene->m_nCampType != emctAllProtect);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::CanCloseCampFlag()
{
    BOOL bResult = false;

    assert(m_pScene);

    KG_PROCESS_ERROR(m_eCamp != cNeutral);

    KG_PROCESS_ERROR(m_bCampFlag && m_nCloseCampFlagTime == 0);
    KG_PROCESS_ERROR(!m_bFightState);
    KG_PROCESS_ERROR(m_pScene->m_nCampType != emctFight);

    if (m_eCamp == cGood)
    {
        KG_PROCESS_ERROR(m_pScene->m_nCampType != emctProtectEvil);
    }
    else if (m_eCamp == cEvil)
    {
        KG_PROCESS_ERROR(m_pScene->m_nCampType != emctProtectGood);
    }

    bResult = true;
Exit0:
    return bResult;
}

#ifdef _SERVER
BOOL KPlayer::SetCampFlag(BOOL bCampFlag)
{
    BOOL bResult = false;

    KG_PROCESS_ERROR(m_bCampFlag != bCampFlag);

    m_bCampFlag = bCampFlag;

    g_PlayerServer.DoSyncPlayerCampFlag(this);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::AddPrestige(int nAddPrestige)
{
    BOOL        bResult             = false;
    long long   llPrestigePercent   = KILO_NUM;

    KG_PROCESS_SUCCESS(m_eCamp == cNeutral);
    KG_PROCESS_SUCCESS(nAddPrestige == 0);

    if (nAddPrestige > 0)
    {        
        int nCampLevel = g_pSO3World->m_CampInfo.GetCampLevel();
        llPrestigePercent = g_pSO3World->m_CampInfo.GetPrestigePercent(m_eCamp, nCampLevel);
        nAddPrestige = (int)(nAddPrestige * llPrestigePercent / KILO_NUM);

        KG_PROCESS_SUCCESS(m_nCurrentPrestige >= g_pSO3World->m_Settings.m_ConstList.nMaxPrestige);
        KGLOG_PROCESS_ERROR(m_nCurrentPrestige < (m_nCurrentPrestige + nAddPrestige)); // ��ֹ����

        if (m_nCurrentPrestige + nAddPrestige > g_pSO3World->m_Settings.m_ConstList.nMaxPrestige)
        {
            m_nCurrentPrestige = g_pSO3World->m_Settings.m_ConstList.nMaxPrestige;
            nAddPrestige = 0;
        }
    }

    m_nCurrentPrestige += nAddPrestige;
    if (m_nCurrentPrestige < 0)
    {
        m_nCurrentPrestige = 0; // ����ֵ�Ǹ�
    }

Exit1:
    g_PlayerServer.DoSyncCurrentPrestige(this);
    bResult = true;
Exit0:
    return bResult;
}

void KPlayer::ProcessCampPK(DWORD dwKillerID)
{
    BOOL                    bRetCode                = false;
    int                     nReducePrestigeOnDeath  = 0;
    int                     nAddPrestige            = 0;
    const KSimpThreatNode*  cpThreatNode            = NULL;
    size_t                  uKillerCount            = 0;
    KPlayer*                pKiller                 = NULL;
    DWORD                   dwTeamID                = ERROR_ID;
    int                     nDiffLevelPercent       = 0;
    KGWConstList*           pConstList              = &g_pSO3World->m_Settings.m_ConstList;
    int                     nKilledCountPercent     = 0;
    int                     nKillCountIndex         = m_nKilledCount;
    KCAMP                   eMyCamp                 = m_eCamp;
    std::vector<KPlayer*>   vecAllKillerID;

    KGLOG_PROCESS_ERROR(m_pScene);

    if (m_pScene->m_nType == emtBattleField && eMyCamp == cNeutral)
    {
        dwTeamID = GetCurrentTeamID();
        if (dwTeamID != ERROR_ID)
        {
            KTeam* pTeam = g_pSO3World->m_TeamServer.GetTeam(dwTeamID);
            KGLOG_PROCESS_ERROR(pTeam);

            eMyCamp = pTeam->eCamp;
        }
    }

    KG_PROCESS_ERROR(eMyCamp != cNeutral); // ������Ӫ��������

    cpThreatNode = m_SimpThreatList.GetFirstThreat(thtMainThreat);
    KG_PROCESS_ERROR(cpThreatNode);

    while (cpThreatNode)
    {
        if (cpThreatNode->pCharacter && IS_PLAYER(cpThreatNode->pCharacter->m_dwID))
        {
            vecAllKillerID.push_back((KPlayer*)cpThreatNode->pCharacter);
        }

        cpThreatNode = m_SimpThreatList.GetNextThreat(thtMainThreat, cpThreatNode);
    }

    uKillerCount = vecAllKillerID.size();

    // ������ͬ��Ӫ�ĳͷ�
    if (m_pScene->m_nType != emtDungeon && m_pScene->m_nType != emtBattleField)
    {
        for (size_t i = 0; i < uKillerCount; ++i)
        {
            KPlayer* pPlayer = vecAllKillerID[i];
            if (pPlayer->m_eCamp != eMyCamp)
            {
                continue;
            }

            nAddPrestige = pConstList->nSameCampAssistKill;
            if (pPlayer->m_dwID == dwKillerID)
            {
                nAddPrestige = pConstList->nSameCampKill;
            }

            bRetCode = pPlayer->AddPrestige(nAddPrestige);
            KGLOG_PROCESS_ERROR(bRetCode);

            g_pSO3World->m_StatDataServer.UpdatePrestigeStat(pPlayer, nAddPrestige, "OTHER");
        }
    }

    KG_PROCESS_ERROR(dwKillerID != ERROR_ID && IS_PLAYER(dwKillerID));

    pKiller = g_pSO3World->m_PlayerSet.GetObj(m_dwKillerID);
    KG_PROCESS_ERROR(pKiller);

    MAKE_IN_RANGE(nKillCountIndex, 0, MAX_KILL_COUNT - 1);
    nKilledCountPercent = pConstList->nKilledCountPercent[nKillCountIndex];

    // ��Killer������
    if (pKiller->m_eCamp != eMyCamp)
    {
        if (uKillerCount == 1)
        {
            if ((vecAllKillerID[0]->m_dwID == dwKillerID)) // solo kill
            {
                nAddPrestige = pConstList->nSoloKill;
            }
        }
        else
        {
            std::vector<KPlayer*>::iterator it;
            it = std::find(vecAllKillerID.begin(), vecAllKillerID.end(), pKiller);

            if (it != vecAllKillerID.end()) // last hit
            {
                nAddPrestige = pConstList->nLastHit;
            }
        }

        nDiffLevelPercent = GetDiffLevelPercent(pKiller->m_nLevel);
        nAddPrestige = nAddPrestige * nDiffLevelPercent / 100;

        nAddPrestige = nAddPrestige * nKilledCountPercent / 100;

        bRetCode = pKiller->AddPrestige(nAddPrestige);
        KGLOG_PROCESS_ERROR(bRetCode);

        g_pSO3World->m_StatDataServer.UpdatePrestigeStat(pKiller, nAddPrestige, "KILL");
    }

    // ��ͬ����Χ�ڵ�С�Ӷ��Ѽ�����
    dwTeamID = pKiller->GetCurrentTeamID();
    if (dwTeamID != ERROR_ID && pKiller->m_eCamp != eMyCamp)
    {
        KTeam*  pTeam       = NULL;
        int     nGroupIndex = 0;

        pTeam = g_pSO3World->m_TeamServer.GetTeam(dwTeamID);
        KGLOG_PROCESS_ERROR(pTeam);

        nGroupIndex = pKiller->GetCurrentTeamGroupIndex();
        KGLOG_PROCESS_ERROR(nGroupIndex >= 0 && nGroupIndex < pTeam->nGroupNum);

        for (
            KTEAM_MEMBER_LIST::iterator it = pTeam->MemberGroup[nGroupIndex].MemberList.begin();
            it != pTeam->MemberGroup[nGroupIndex].MemberList.end(); ++it
        )
        {
            KPlayer* pMember = NULL;

            if (it->dwMemberID == dwKillerID)
            {
                continue;
            }

            pMember = g_pSO3World->m_PlayerSet.GetObj(it->dwMemberID);
            if (pMember == NULL)
            {
                continue;
            }

            if (pMember->m_eCamp == eMyCamp)
            {
                continue;
            }

            bRetCode = g_InRange(pKiller, pMember, CELL_LENGTH * REGION_GRID_WIDTH);
            if (!bRetCode)
            {
                continue;
            }

            nDiffLevelPercent = GetDiffLevelPercent(pMember->m_nLevel);
            nAddPrestige = pConstList->nKillerTeamMember * nDiffLevelPercent / 100;

            nAddPrestige = nAddPrestige * nKilledCountPercent / 100;

            bRetCode = pMember->AddPrestige(nAddPrestige);
            KGLOG_PROCESS_ERROR(bRetCode);

            g_pSO3World->m_StatDataServer.UpdatePrestigeStat(pMember, nAddPrestige, "KILL");
        }
    }

    // ��AssistKiller������,����ͬʱҲ�Ƕ���
    for (size_t i = 0; i < uKillerCount; ++i)
    {
        KPlayer* pPlayer = vecAllKillerID[i];
        if (pPlayer->m_eCamp == eMyCamp)
        {
            continue;
        }

        if (pPlayer->m_dwID == dwKillerID)
        {
            continue;
        }

        nDiffLevelPercent = GetDiffLevelPercent(pPlayer->m_nLevel);
        nAddPrestige = pConstList->nCommonKill * nDiffLevelPercent / 100;

        nAddPrestige = nAddPrestige * nKilledCountPercent / 100;

        bRetCode = pPlayer->AddPrestige(nAddPrestige);
        KGLOG_PROCESS_ERROR(bRetCode);

        g_pSO3World->m_StatDataServer.UpdatePrestigeStat(pPlayer, nAddPrestige, "KILL");
    }

    // ����������
    if (pKiller->m_eCamp != cNeutral && pKiller->m_eCamp != eMyCamp)
    {
        if (m_pScene->m_nType != emtBattleField)
        {
            nReducePrestigeOnDeath = g_pSO3World->m_CampInfo.GetReducePrestigeOnDeath(eMyCamp, g_pSO3World->m_CampInfo.GetCampLevel());
            nAddPrestige = pConstList->nReducePrestigeOnDeath * nReducePrestigeOnDeath / KILO_NUM;

            bRetCode = AddPrestige(nAddPrestige);
            KGLOG_PROCESS_ERROR(bRetCode);

            g_pSO3World->m_StatDataServer.UpdatePrestigeStat(this, nAddPrestige, "KILLED");
        }

        ++m_nKilledCount;
        if (m_nKilledCount == 1)
        {
            m_nNextResetKilledCountTime = g_pSO3World->m_nCurrentTime + pConstList->nResetKilledCountCycle;
        }
    }

Exit0:
    return;
}

#define PVPSCRIPTS SCRIPT_DIR"/player/PVPScript.lua"

void KPlayer::CallChangeCampScript()
{
    BOOL        bRetCode    = false;
    int         nTopIndex   = 0;
    const char* pszFuncName = "OnChangeCamp";

    bRetCode = g_pSO3World->m_ScriptCenter.IsScriptExist(PVPSCRIPTS);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = g_pSO3World->m_ScriptCenter.IsFuncExist(PVPSCRIPTS, pszFuncName);
    KG_PROCESS_ERROR(bRetCode);

    g_pSO3World->m_ScriptCenter.SafeCallBegin(&nTopIndex);

    g_pSO3World->m_ScriptCenter.PushValueToStack(this);

    g_pSO3World->m_ScriptCenter.CallFunction(PVPSCRIPTS, pszFuncName, 0);

    g_pSO3World->m_ScriptCenter.SafeCallEnd(nTopIndex);

Exit0:
    return;
}

int KPlayer::GetDiffLevelPercent(int nKillerLevel)
{
    int anDiffLevelPercent[]    = { 200, 175, 150, 125, 100, 100, 100, 75, 50, 25, 0 };
	int nDiffLevel              = nKillerLevel - m_nLevel;
	
    MAKE_IN_RANGE(nDiffLevel, -5, 5);

    return anDiffLevelPercent[nDiffLevel + 5];
}

void KPlayer::CheckReduceKillPoint()
{
    BOOL bRetCode = false;
    const char* pszFuncName = "AddKillPoint";

    KG_PROCESS_ERROR(m_nCurrentKillPoint > 0);

    if (g_pSO3World->m_nCurrentTime > m_nNextKillPointReduceTime)
    {
        int nLuaTopIndex = 0;

        m_nNextKillPointReduceTime = g_pSO3World->m_nCurrentTime + g_pSO3World->m_Settings.m_ConstList.nKillPointReduceCycle;

        bRetCode = g_pSO3World->m_ScriptCenter.IsScriptExist(PLAYER_SCRIPT);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = g_pSO3World->m_ScriptCenter.IsFuncExist(PLAYER_SCRIPT, pszFuncName);
        KG_PROCESS_ERROR(bRetCode);

        g_pSO3World->m_ScriptCenter.SafeCallBegin(&nLuaTopIndex);
        g_pSO3World->m_ScriptCenter.PushValueToStack(this);
        g_pSO3World->m_ScriptCenter.PushValueToStack(-g_pSO3World->m_Settings.m_ConstList.nReduceKillPoint);
        g_pSO3World->m_ScriptCenter.CallFunction(PLAYER_SCRIPT, pszFuncName, 0);
        g_pSO3World->m_ScriptCenter.SafeCallEnd(nLuaTopIndex);
    }

Exit0:
    return;
}
#endif

void KPlayer::setHeroFlag(int nValue)
{
    KG_PROCESS_ERROR(m_bHeroFlag != nValue);

    m_bHeroFlag = (BOOL)nValue;

#ifdef _CLIENT
    g_PlayerClient.DoSetHeroFlag(nValue);
#endif

#ifdef _SERVER
    g_PlayerServer.DoSyncHeroFlag(this);
#endif

Exit0:
    return;
}

#ifdef _SERVER
BOOL KPlayer::AddTAEquipsScore(int nDeltaScore)
{
    if (nDeltaScore == 0)
        return true;

    m_dwTAEquipsScore += nDeltaScore;
    return true;
}

BOOL KPlayer::GetExtPoint(int nIndex, int& nValue)
{
    if (nIndex >= 0 && nIndex < MAX_EXT_POINT_COUNT)
    {
        nValue = (int)m_ExtPointInfo.nExtPoint[nIndex];
        return true;
    }

    return m_NewExtPointManager.GetNewExtPoint(nIndex, &nValue);
}

BOOL KPlayer::SetExtPoint(int nIndex, short nChangeValue)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    KGLOG_PROCESS_ERROR(nIndex >= 0 && nIndex < MAX_EXT_POINT_COUNT);
    KGLOG_PROCESS_ERROR(m_ExtPointInfo.nExtPoint[nIndex] != nChangeValue);
    KGLOG_PROCESS_ERROR(!m_bExtPointLock);

    m_bExtPointLock         = true;
    m_nLastExtPointIndex    = nIndex;
    m_nLastExtPointValue    = m_ExtPointInfo.nExtPoint[nIndex];

    m_ExtPointInfo.nExtPoint[nIndex] = nChangeValue;

    KGLogPrintf(
        KGLOG_INFO, "Apply Change ExtPoint. PlayerID = %d, ExtIndex = %d, ChangeValue = %d",
        m_dwID, nIndex, nChangeValue
    );

    bRetCode = g_RelayClient.DoChangeExtPointRequest(m_dwID, (WORD)nIndex, (WORD)nChangeValue);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}
#endif // _SERVER

#ifdef _SERVER
BOOL KPlayer::SaveRandData(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize)
{
    BOOL                bResult     = false;
    BYTE*               pbyOffset   = pbyBuffer;
    BYTE*               pTail       = pbyBuffer + uBufferSize;
    size_t              uLeftSize   = uBufferSize;

	*(PROBABILITY_TYPE*)pbyOffset = eptQuestDrop;
    pbyOffset += sizeof(PROBABILITY_TYPE);

    pbyOffset = m_QuestRand.Save(pbyOffset, pTail - pbyOffset);
    KGLOG_PROCESS_ERROR(pbyOffset);

    *(PROBABILITY_TYPE*)pbyOffset = eptVenation;
    pbyOffset += sizeof(PROBABILITY_TYPE);

    pbyOffset = m_VenationRand.Save(pbyOffset, pTail - pbyOffset);
    KGLOG_PROCESS_ERROR(pbyOffset);

    *puUsedSize = (size_t)(pbyOffset - pbyBuffer);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::LoadRandData(BYTE* pbyData, size_t uDataLen)
{
    BOOL                bResult   = false;
    BYTE*               pbyOffset = pbyData;
    BYTE*               pTail     = pbyData + uDataLen;
    PROBABILITY_TYPE*   peType    = NULL;

    while (pbyOffset < pTail)
    {
        peType = (PROBABILITY_TYPE*)pbyOffset;
        pbyOffset += sizeof(PROBABILITY_TYPE);

        switch (*peType)
        {
        case eptQuestDrop:
            pbyOffset = m_QuestRand.Load(pbyOffset, pTail - pbyOffset);
            KGLOG_PROCESS_ERROR(pbyOffset);
            break;
        case eptVenation:
            pbyOffset = m_VenationRand.Load(pbyOffset, pTail - pbyOffset);
            KGLOG_PROCESS_ERROR(pbyOffset);
            break;
        default:
            assert(false);
        }
    }
    KGLOG_PROCESS_ERROR(pbyOffset == pTail);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::SaveMentorData(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize)
{
    BOOL    bResult = false;
    size_t  uNeedSize = sizeof(KMENTOR_DATA_ON_PLAYER) +
        (m_GraduateMentorData.size() + m_GraduateApprenticeData.size()) * sizeof(KGRADUATED_MENTOR_DATA_DB);
    BYTE*   pbyOffset = pbyBuffer;
    KMENTOR_DATA_ON_PLAYER* pMentorData = NULL;

    KGLOG_PROCESS_ERROR(uBufferSize >= uNeedSize);
    pMentorData = (KMENTOR_DATA_ON_PLAYER*)pbyOffset;
    pMentorData->nAcquiredMentorValue = m_nAcquiredMentorValue;
    pMentorData->nUsableMentorValue = m_nUsableMentorValue;
    pMentorData->nLastEvokeMentorTime = m_nLastEvokeMentorTime;
    pMentorData->dwTAEquipsScore = m_dwTAEquipsScore;
    memset(pMentorData->byReserved, 0, sizeof(pMentorData->byReserved));
    pMentorData->byEvokeMentorCount = (BYTE)m_nEvokeMentorCount;
    pMentorData->byMaxApprenticeCount = (BYTE)m_nMaxApprenticeNum;
    pMentorData->byGraduateMentorCount = (BYTE)m_GraduateMentorData.size();
    pMentorData->byGraduateApprenticeCount = (BYTE)m_GraduateApprenticeData.size();
    pbyOffset += sizeof(KMENTOR_DATA_ON_PLAYER);
    if (!m_GraduateMentorData.empty())
    {
        memcpy(pbyOffset, &m_GraduateMentorData[0], m_GraduateMentorData.size() * sizeof(KGRADUATED_MENTOR_DATA_DB));
        pbyOffset += m_GraduateMentorData.size() * sizeof(KGRADUATED_MENTOR_DATA_DB);
    }
    if (!m_GraduateApprenticeData.empty())
    {
        memcpy(pbyOffset, &m_GraduateApprenticeData[0], m_GraduateApprenticeData.size() * sizeof(KGRADUATED_MENTOR_DATA_DB));
        pbyOffset += m_GraduateApprenticeData.size() * sizeof(KGRADUATED_MENTOR_DATA_DB);
    }
    *puUsedSize = pbyOffset - pbyBuffer;
    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::LoadMentorData(BYTE* pbyData, size_t uDataLen, int nVersion)
{
    BOOL bResult = false;
    size_t uBaseSize = 0;
    size_t uLeftSize = 0;
    BYTE* pbyOffset = pbyData;
    int nMaxApprentice = MIN_APPRENTICE_NUM;
    BYTE byGraduateMentorCount = 0;
    BYTE byGraduateApprenticeCount = 0;

    m_GraduateMentorData.clear();
    m_GraduateApprenticeData.clear();
    if (nVersion == 0)
    {
        uBaseSize = 0x20;
        KGLOG_PROCESS_ERROR(uDataLen >= uBaseSize);
        m_nAcquiredMentorValue = *(int*)(pbyOffset + 0x00);
        m_nUsableMentorValue = *(int*)(pbyOffset + 0x0a);
        m_nLastEvokeMentorTime = *(time_t*)(pbyOffset + 0x04);
        m_nEvokeMentorCount = *(BYTE*)(pbyOffset + 0x08);
        nMaxApprentice = *(BYTE*)(pbyOffset + 0x09);
        m_dwTAEquipsScore = 0;
    }
    else if (nVersion == 1)
    {
        uBaseSize = 0x10;
        KGLOG_PROCESS_ERROR(uDataLen >= uBaseSize);
        m_nAcquiredMentorValue = *(int*)(pbyOffset + 0x00);
        m_nUsableMentorValue = *(int*)(pbyOffset + 0x04);
        m_nLastEvokeMentorTime = *(time_t*)(pbyOffset + 0x08);
        m_nEvokeMentorCount = *(BYTE*)(pbyOffset + 0x0c);
        nMaxApprentice = *(BYTE*)(pbyOffset + 0x0d);
        byGraduateMentorCount = *(BYTE*)(pbyOffset + 0x0e);
        byGraduateApprenticeCount = *(BYTE*)(pbyOffset + 0x0f);
        m_dwTAEquipsScore = 0;
    }
    else if (nVersion == 2)
    {
        uBaseSize = sizeof(KMENTOR_DATA_ON_PLAYER);
        KGLOG_PROCESS_ERROR(uDataLen >= uBaseSize);
        KMENTOR_DATA_ON_PLAYER* pMentorData = (KMENTOR_DATA_ON_PLAYER*)pbyOffset;
        m_nAcquiredMentorValue = pMentorData->nAcquiredMentorValue;
        m_nUsableMentorValue = pMentorData->nUsableMentorValue;
        m_nLastEvokeMentorTime = pMentorData->nLastEvokeMentorTime;
        m_dwTAEquipsScore = pMentorData->dwTAEquipsScore;
        m_nEvokeMentorCount = pMentorData->byEvokeMentorCount;
        nMaxApprentice = pMentorData->byMaxApprenticeCount;
        byGraduateMentorCount = pMentorData->byGraduateMentorCount;
        byGraduateApprenticeCount = pMentorData->byGraduateApprenticeCount;
    }
    else
    {
        KGLOG_PROCESS_ERROR(false);
    }

    if (nVersion == 1 || nVersion == 2)
    {
        size_t uGraduateSize = (byGraduateMentorCount + byGraduateApprenticeCount) * sizeof(KGRADUATED_MENTOR_DATA_DB);
        KGLOG_PROCESS_ERROR(uDataLen - uBaseSize >= uGraduateSize);
        uLeftSize = uDataLen - uBaseSize - uGraduateSize;
        pbyOffset += uBaseSize;
        for (BYTE i = 0; i < byGraduateMentorCount; ++i)
        {
            KGRADUATED_MENTOR_DATA_DB* pData = (KGRADUATED_MENTOR_DATA_DB*)pbyOffset;
            KGRADUATED_MENTOR_DATA data = {pData->dwPlayerID, pData->nMentorTime, pData->nGraduateTime};
            m_GraduateMentorData.push_back(data);
            pbyOffset += sizeof(KGRADUATED_MENTOR_DATA_DB);
        }
        for (BYTE i = 0; i < byGraduateApprenticeCount; ++i)
        {
            KGRADUATED_MENTOR_DATA_DB* pData = (KGRADUATED_MENTOR_DATA_DB*)pbyOffset;
            KGRADUATED_MENTOR_DATA data = {pData->dwPlayerID, pData->nMentorTime, pData->nGraduateTime};
            m_GraduateApprenticeData.push_back(data);
            pbyOffset += sizeof(KGRADUATED_MENTOR_DATA_DB);
        }
    }
    m_nMaxApprenticeNum = std::max(MIN_APPRENTICE_NUM, nMaxApprentice);
    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::IsQuestDrop(DWORD dwID, int nIndex, unsigned uProbability)
{
    BOOL        bResult = false;
    uint64_t    ukey    = 0;

    KGLOG_PROCESS_ERROR(dwID > 0);
    KGLOG_PROCESS_ERROR(nIndex >= 0 && nIndex < QUEST_END_ITEM_COUNT);

    ukey = MAKE_INT64(dwID, nIndex);

    bResult = m_QuestRand.RandomResult(ukey, uProbability);

Exit0:
    return bResult;
}

BOOL KPlayer::IsVenationSuccess(DWORD dwSkillID, DWORD dwSkillLevel, unsigned uProbability)
{
    BOOL        bResult = false;
    uint64_t    ukey    = 0;

    KGLOG_PROCESS_ERROR(dwSkillID > 0);
    KGLOG_PROCESS_ERROR(dwSkillLevel >= 0);

    ukey = MAKE_INT64(dwSkillID, dwSkillLevel);

    bResult = m_VenationRand.RandomResult(ukey, uProbability);
Exit0:
    return bResult;
}

BOOL KPlayer::AddCoin(int nDeltaCoin)
{
    BOOL bResult = false;

    KG_PROCESS_ERROR(m_nCoin + nDeltaCoin >= 0);

    m_nCoin += nDeltaCoin;

    g_PlayerServer.DoSyncCoin(this);

    bResult = true;
Exit0:
    return bResult;
}

void KPlayer::UpdateFreeLimitFlag()
{
    if (!m_bChargeFlag && g_pSO3World->m_bZoneChargeFlag)
    {
        m_bFreeLimitFlag = true;
    }
}

void KPlayer::ProcessAntiFarmer()
{
    if (((DWORD)g_pSO3World->m_nGameLoop - m_dwID) % (GAME_FPS * ANTI_FARMER_CHECK_ONLINE_TIME_INTERVAL) == 0)
    {
        m_AntiFarmer.RecordOnlineTime();
    }

    if (((DWORD)g_pSO3World->m_nGameLoop - m_dwID) % (GAME_FPS * 1024) == 0 && !m_bFarmerLimit)
    {
        BOOL bFamerFlag = m_AntiFarmer.Judge();

        if (bFamerFlag)
        {
            g_RelayClient.DoReportFarmerPlayer(m_dwID,  g_pSO3World->m_Settings.m_ConstList.nAntiFarmerPunishTime);
        }
    }
}

void KPlayer::OpenBox(TItemPos& Pos)
{
    BOOL                bRetCode        = false;
    KOtherItemInfo*     pItemInfo       = NULL;
    KItem*              pItem           = NULL;
    int                 nTopIndex       = 0;
    KUSE_ITEM           UseItemInfo;
    KTarget             Target;

    Target.ClearTarget();

    pItem = m_ItemList.GetItem(Pos.dwBox, Pos.dwX);
    KGLOG_PROCESS_ERROR(pItem);

    pItemInfo = (KOtherItemInfo*)pItem->GetItemInfo();
    KGLOG_PROCESS_ERROR(pItemInfo);

    ZeroMemory(&UseItemInfo, sizeof(UseItemInfo));

    bRetCode = m_ItemList.CanUseItem(pItem, Pos.dwBox, &UseItemInfo, Target);
    if (bRetCode != uircSuccess)
    {
        g_PlayerServer.DoMessageNotify(
            m_nConnIndex, ectUseItemErrorCode, bRetCode, &UseItemInfo, sizeof(UseItemInfo)
        );
        goto Exit0;
    }

    bRetCode = g_pSO3World->m_ScriptCenter.IsScriptExist(pItem->m_dwScriptID);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = g_pSO3World->m_ScriptCenter.IsFuncExist(pItem->m_dwScriptID, OPEN_BOX);
    KGLOG_PROCESS_ERROR(bRetCode);

    g_pSO3World->m_ScriptCenter.SafeCallBegin(&nTopIndex);

    g_pSO3World->m_ScriptCenter.PushValueToStack(this);
    g_pSO3World->m_ScriptCenter.PushValueToStack(Pos.dwBox);
    g_pSO3World->m_ScriptCenter.PushValueToStack(Pos.dwX);

    g_pSO3World->m_ScriptCenter.CallFunction(pItem->m_dwScriptID, OPEN_BOX, 0);
    g_pSO3World->m_ScriptCenter.SafeCallEnd(nTopIndex);

Exit0:
    return;
}

void KPlayer::AddStaminaAndThew()
{
    m_nCurrentStamina +=  (int)(m_nMaxStamina * 0.002);
    m_nCurrentThew += (int)(m_nMaxThew * 0.002);

    MAKE_IN_RANGE(m_nCurrentStamina, 0, m_nMaxStamina);
    MAKE_IN_RANGE(m_nCurrentThew, 0, m_nMaxThew);
}

BOOL KPlayer::CanOpenTongRepertory()
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;
    KNpc*   pNpc        = NULL;

    KG_PROCESS_ERROR(m_dwTongRepertoryNpcID != ERROR_ID);
    pNpc = g_pSO3World->m_NpcSet.GetObj(m_dwTongRepertoryNpcID);
    KGLOG_PROCESS_ERROR(pNpc);

    bRetCode = g_InRange(this, pNpc, COMMON_PLAYER_OPERATION_DISTANCE);
    KG_PROCESS_ERROR(bRetCode);

    KG_PROCESS_ERROR(m_eMoveState != cmsOnDeath);

    bResult = true;
Exit0:
    return bResult;
}
#endif // _SERVER
