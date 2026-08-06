#include "stdafx.h"
#include "KSO3World.h"

KWorldSettings::KWorldSettings(void)
{
	m_szVersionLineName[0] = '\0';
	m_szVersionEx[0] = '\0';
}

KWorldSettings::~KWorldSettings(void)
{
}

BOOL KWorldSettings::LoadVersionConfig(void)
{
    BOOL bResult = false;
    IIniFile* pIniFile = NULL;
    pIniFile = g_OpenIniFile("version.cfg");
    KGLOG_PROCESS_ERROR(pIniFile);
    KGLOG_PROCESS_ERROR(pIniFile->GetString("Sword3", "VersionLineName", "", m_szVersionLineName, sizeof(m_szVersionLineName)) >= 0);
    KGLOG_PROCESS_ERROR(pIniFile->GetString("Sword3", "Version", "", m_szVersionEx, sizeof(m_szVersionEx)) >= 0);
    bResult = true;
Exit0:
    if (pIniFile) pIniFile->Release();
    return bResult;
}

BOOL KWorldSettings::UnInit_ForEditor(void)
{
    m_NpcTeamList.UnInit();
    m_DoodadTemplateList.UnInit();
    m_NpcTemplateList.UnInit();
    m_PatrolPathList.UnInit();
    return true;
}

BOOL KWorldSettings::Init_ForEditor(void)
{
    BOOL bResult = false;
    BOOL bNpcTemplate = false, bDoodadTemplate = false, bPatrol = false, bTeam = false;
    KGLOG_PROCESS_ERROR(m_NpcTemplateList.Init()); bNpcTemplate = true;
    KGLOG_PROCESS_ERROR(m_DoodadTemplateList.Init()); bDoodadTemplate = true;
    KGLOG_PROCESS_ERROR(m_MapListFile.Init());
    KGLOG_PROCESS_ERROR(m_PatrolPathList.Init()); bPatrol = true;
    KGLOG_PROCESS_ERROR(m_NpcTeamList.Init()); bTeam = true;
    bResult = true;
Exit0:
    if (!bResult)
    {
        if (bTeam) m_NpcTeamList.UnInit();
        if (bPatrol) m_PatrolPathList.UnInit();
        if (bDoodadTemplate) m_DoodadTemplateList.UnInit();
        if (bNpcTemplate) m_NpcTemplateList.UnInit();
    }
    return bResult;
}

#ifdef _PERFORMANCE_OPTIMIZATION
    DWORD g_dwGameFps = 16;
#endif

BOOL KWorldSettings::Init(void)
{
    BOOL bResult                        = false;
	BOOL bRetCode                       = false;
    BOOL bConstListInitFlag             = false;
    BOOL bNpcTemplateListInitFlag       = false;
    BOOL bDoodadTemplateListInitFlag    = false;
    BOOL bQuestInfoListInitFlag         = false;
    BOOL bNpcClassListInitFlag          = false;
    BOOL bDoodadClassListInitFlag       = false;
    BOOL bPatrolPathListInitFlag        = false;
    BOOL bLevelUpListInitFlag           = false;
    BOOL bCoolDownListInitFlag          = false;
    BOOL bWeaponTypeListInitFlag        = false;
    BOOL bReputeLimitInitFlag           = false;
    BOOL bReputeLootBufferListInitFlag  = false;
    BOOL bCharacterActionListInitFlag   = false;
    BOOL bSmartDialogListInitFlag       = false;
    BOOL bNpcAdronTabInitFlag           = false;
    BOOL bServerConstListInitFlag       = false;
    BOOL bOrderManagerInitFlag          = false;
    BOOL bNpcTeamListInitFlag           = false;
    BOOL bAchievementInfoListFlag       = false;
    BOOL bDesignationListInitFlag       = false;
    BOOL bGameCardInfoListInitFlag      = false;
    BOOL bAntiFarmerSettingsInitFlag    = false;
    BOOL bOldPendentDataInitFlag        = false;
    BOOL bTongConstListInitFlag         = false;

	bRetCode = m_ConstList.Init();
	KGLOG_PROCESS_ERROR(bRetCode);
    bConstListInitFlag = true;

	bRetCode = m_NpcTemplateList.Init();
	KGLOG_PROCESS_ERROR(bRetCode);
    bNpcTemplateListInitFlag = true;

	bRetCode = m_DoodadTemplateList.Init();
	KGLOG_PROCESS_ERROR(bRetCode);
    bDoodadTemplateListInitFlag = true;

	bRetCode = m_MapListFile.Init();
	KGLOG_PROCESS_ERROR(bRetCode);

	bRetCode = m_QuestInfoList.Init();
	KGLOG_PROCESS_ERROR(bRetCode);
    bQuestInfoListInitFlag = true;

	bRetCode = m_NpcClassList.Init();
	KGLOG_PROCESS_ERROR(bRetCode);
    bNpcClassListInitFlag = true;

	bRetCode = m_DoodadClassList.Init();
	KGLOG_PROCESS_ERROR(bRetCode);
    bDoodadClassListInitFlag = true;

	bRetCode = m_PatrolPathList.Init();
	KGLOG_PROCESS_ERROR(bRetCode);
    bPatrolPathListInitFlag = true;

	bRetCode = m_LevelUpList.Init();
	KGLOG_PROCESS_ERROR(bRetCode);
    bLevelUpListInitFlag = true;

    bRetCode = m_CoolDownList.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bCoolDownListInitFlag = true;

    bRetCode = m_WeaponTypeList.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bWeaponTypeListInitFlag = true;

	bRetCode = m_RelationCampList.Init(RELATION_CAMP_FILE_NAME);
	KGLOG_PROCESS_ERROR(bRetCode);

	bRetCode = m_RelationForceList.Init(RELATION_FORCE_FILE_NAME);
	KGLOG_PROCESS_ERROR(bRetCode);
    
	bRetCode = m_ForceDefaultList.Init();
	KGLOG_PROCESS_ERROR(bRetCode);

	bRetCode = m_ReputeLimit.Init();
	KGLOG_PROCESS_ERROR(bRetCode);
    bReputeLimitInitFlag = true;
	bRetCode = m_ReputeLootBufferList.Init();
	KGLOG_PROCESS_ERROR(bRetCode);
    bReputeLootBufferListInitFlag = true;

	bRetCode = m_CharacterActionList.Init();
	KGLOG_PROCESS_ERROR(bRetCode);
    bCharacterActionListInitFlag = true;

	bRetCode = m_SmartDialogList.Init();
	KGLOG_PROCESS_ERROR(bRetCode);
    bSmartDialogListInitFlag = true;

#ifdef _SERVER
    bRetCode = m_GMList.Load();
    KGLOG_PROCESS_ERROR(bRetCode);
#endif
	bRetCode = m_ServerConstList.Init(SETTING_DIR "/ServerConstListByVer.ini");
	KGLOG_PROCESS_ERROR(bRetCode);
    bServerConstListInitFlag = true;
	bRetCode = m_NpcAdronTab.Init();
	KGLOG_PROCESS_ERROR(bRetCode);
    bNpcAdronTabInitFlag = true;

	bRetCode = m_OrderManager.Init();
	KGLOG_PROCESS_ERROR(bRetCode);
    bOrderManagerInitFlag = true;

	bRetCode = m_NpcTeamList.Init();
	KGLOG_PROCESS_ERROR(bRetCode);
    bNpcTeamListInitFlag = true;

    bRetCode = m_AchievementInfoList.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bAchievementInfoListFlag = true;

    bRetCode = m_DesignationList.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bDesignationListInitFlag = true;

    bRetCode = m_GameCardInfoList.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bGameCardInfoListInitFlag = true;
	bRetCode = m_AntiFarmerSettings.Init();
	KGLOG_PROCESS_ERROR(bRetCode);
    bAntiFarmerSettingsInitFlag = true;
	bRetCode = m_OldPendentDataInfoList.Init();
	KGLOG_PROCESS_ERROR(bRetCode);
    bOldPendentDataInitFlag = true;
	bRetCode = m_TongConstList.Init();
	KGLOG_PROCESS_ERROR(bRetCode);
    bTongConstListInitFlag = true;
	KGLOG_PROCESS_ERROR(LoadVersionConfig());

    bResult = true;
Exit0:
    if (!bResult)
    {
        if (bTongConstListInitFlag)
        {
            m_TongConstList.UnInit();
            bTongConstListInitFlag = false;
        }

        if (bOldPendentDataInitFlag)
        {
            m_OldPendentDataInfoList.UnInit();
            bOldPendentDataInitFlag = false;
        }

        if (bAntiFarmerSettingsInitFlag)
        {
            m_AntiFarmerSettings.UnInit();
            bAntiFarmerSettingsInitFlag = false;
        }

        if (bGameCardInfoListInitFlag)
        {
            m_GameCardInfoList.UnInit();
            bGameCardInfoListInitFlag = false;
        }

        if (bDesignationListInitFlag)
        {
            m_DesignationList.UnInit();
            bDesignationListInitFlag = false;
        }

        if (bAchievementInfoListFlag)
        {
            m_AchievementInfoList.UnInit();
            bAchievementInfoListFlag = false;
        }

        if (bNpcTeamListInitFlag)
        {
            m_NpcTeamList.UnInit();
            bNpcTeamListInitFlag = false;
        }

        if (bNpcAdronTabInitFlag)
        {
            m_NpcAdronTab.UnInit();
            bNpcAdronTabInitFlag = false;
        }

        if (bOrderManagerInitFlag)
        {
            m_OrderManager.UnInit();
            bOrderManagerInitFlag = false;
        }

        if (bSmartDialogListInitFlag)
        {
            m_SmartDialogList.UnInit();
            bSmartDialogListInitFlag = false;
        }

        if (bCharacterActionListInitFlag)
        {
            m_CharacterActionList.UnInit();
            bCharacterActionListInitFlag = false;
        }

        if (bReputeLootBufferListInitFlag)
        {
            m_ReputeLootBufferList.UnInit();
            bReputeLootBufferListInitFlag = false;
        }

        if (bReputeLimitInitFlag)
        {
            m_ReputeLimit.UnInit();
            bReputeLimitInitFlag = false;
        }

        if (bWeaponTypeListInitFlag)
        {
            m_WeaponTypeList.UnInit();
            bWeaponTypeListInitFlag = false;
        }

        if (bCoolDownListInitFlag)
        {
            m_CoolDownList.UnInit();
            bCoolDownListInitFlag = false;
        }

        if (bLevelUpListInitFlag)
        {
            m_LevelUpList.UnInit();
            bLevelUpListInitFlag = false;
        }

        if (bPatrolPathListInitFlag)
        {
            m_PatrolPathList.UnInit();
            bPatrolPathListInitFlag = false;
        }

        if (bDoodadClassListInitFlag)
        {
            m_DoodadClassList.UnInit();
            bDoodadClassListInitFlag = false;
        }

        if (bNpcClassListInitFlag)
        {
            m_NpcClassList.UnInit();
            bNpcClassListInitFlag = false;
        }

        if (bQuestInfoListInitFlag)
        {
            m_QuestInfoList.UnInit();
            bQuestInfoListInitFlag = false;
        }

        if (bDoodadTemplateListInitFlag)
        {
            m_DoodadTemplateList.UnInit();
            bDoodadTemplateListInitFlag = false;
        }

        if (bNpcTemplateListInitFlag)
        {
            m_NpcTemplateList.UnInit();
            bNpcTemplateListInitFlag = false;
        }

        if (bConstListInitFlag)
        {
            m_ConstList.UnInit();
            bConstListInitFlag = false;
        }
    }
	return bResult;
}

BOOL KWorldSettings::UnInit(void)
{
	m_NpcTeamList.UnInit();
	m_OrderManager.UnInit();
	m_NpcAdronTab.UnInit();

	m_SmartDialogList.UnInit();
	m_CharacterActionList.UnInit();
    m_ReputeLootBufferList.UnInit();
    m_ReputeLimit.UnInit();
    m_WeaponTypeList.UnInit();
    m_CoolDownList.UnInit();
    m_LevelUpList.UnInit();
	m_DoodadTemplateList.UnInit();
	m_NpcTemplateList.UnInit();
	m_QuestInfoList.UnInit();
	m_DoodadClassList.UnInit();
	m_NpcClassList.UnInit();
	m_PatrolPathList.UnInit();
	m_ConstList.UnInit();
	m_AchievementInfoList.UnInit();
    m_DesignationList.UnInit();
    m_GameCardInfoList.UnInit();
    m_AntiFarmerSettings.UnInit();
    m_OldPendentDataInfoList.UnInit();
    m_TongConstList.UnInit();

	return true;
}
