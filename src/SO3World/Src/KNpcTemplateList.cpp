#include "stdafx.h"
#include "KNpcTemplateList.h"
#include "KAttrModifier.h"
#include "KNpc.h"
#include "KSO3World.h"

#define DEFAULT_NPC_TEMPLATE_ID	0
#define NPC_TEMPLATE_FILENAME	"NpcTemplateList.tab"
#define NPC_TEMPLATE_SHOP_FILENAME "NpcTemplateShopInfo.tab"


#define NPC_TEMPLATE_ID			    "ID"
#define NPC_NAME				    "Name"
#define NPC_TITLE					"Title"
#define NPC_MAP_NAME				"MapName"
#define NPC_KIND				    "Kind"
#define NPC_FORCE_ID				"ForceID"
#define NPC_GUARD_FORCE_ID          "GuardForceID"
#define NPC_SPECIES                 "Species"
#define NPC_LEVEL				    "Level"
#define NPC_ADJUST_LEVEL			"AdjustLevel"
#define NPC_HEIGHT					"Height"
#define NPC_MOVE_MODE				"MoveMode"

#define NPC_TOUCH_RANGE             "TouchRange"
#define NPC_INTENSITY               "Intensity"
#define NPC_ALARM_RANGE             "AlarmRange"
#define NPC_REVIVE_TIME             "ReviveTime"
#define NPC_REVIVE_MIN_TIME         "DynamicReviveMinTime"

#define NPC_IS_SELECTABLE			"IsSelectable"
#define NPC_CAN_SEE_LIFEBAR			"CanSeeLifeBar"
#define NPC_CAN_SEE_NAME            "CanSeeName"

#define NPC_MAX_LIFE			    "MaxLife"
#define NPC_LIFE_REPLENISH          "LifeReplenish"
#define NPC_LIFE_REPLENISH_PERCENT  "LifeReplenishPercent"
#define NPC_MAX_MANA			    "MaxMana"
#define NPC_MANA_REPLENISH          "ManaReplenish"
#define NPC_MANA_REPLENISH_PERCENT  "ManaReplenishPercent"

#define NPC_WALK_SPEED              "WalkSpeed"
#define NPC_RUN_SPEED               "RunSpeed"
#define NPC_JUMP_SPEED              "JumpSpeed"

#define NPC_MELEE_WEAPON_DAMAGE_BASE "MeleeWeaponDamageBase"
#define NPC_MELEE_WEAPON_DAMAGE_RAND "MeleeWeaponDamageRand"
#define NPC_RANGE_WEAPON_DAMAGE_BASE "RangeWeaponDamageBase"
#define NPC_RANGE_WEAPON_DAMAGE_RAND "RangeWeaponDamageRand"

#define NPC_HIT_BASE                "HitBase"
#define NPC_PHYSICS_HIT             "PhysicsAttackHit"
#define NPC_SOLAR_MAGIC_HIT         "SolarMagicHit"
#define NPC_NEUTRAL_MAGIC_HIT       "NeutralMagicHit"
#define NPC_LUNAR_MAGIC_HIT         "LunarMagicHit"
#define NPC_POISON_MAGIC_HIT        "PoisonMagicHit"

#define NPC_DODGE			        "Dodge"
#define NPC_PARRY                   "Parry"
#define NPC_PARRY_VALUE             "ParryValue"
#define NPC_SENSE                   "Sense"
#define NPC_STRAIN                  "Strain"
#define NPC_TOUGHNESS               "Toughness"
#define NPC_CRITICALDAMAGEPOWER     "CriticalDamagePower"
#define NPC_DECRITICALDAMAGEPOWER   "DecriticalDamagePower"

#define NPC_PHYSICS_CRITICAL_STRIKE "PhysicsCriticalStrike"
#define NPC_SOLAR_CRITICAL_STRIKE	"SolarCriticalStrike"	
#define NPC_NEUTRAL_CRITICAL_STRIKE	"NeutralCriticalStrike"	
#define NPC_LUNAR_CRITICAL_STRIKE	"LunarCriticalStrike"		
#define NPC_POISON_CRITICAL_STRIKE	"PoisonCriticalStrike"

#define NPC_PHYSICS_SHIELD_BASE     "PhysicsShieldBase"
#define NPC_SOLAR_MAGIC_DEFENCE	    "SolarMagicDefence"	
#define NPC_NEUTRAL_MAGIC_DEFENCE	"NeutralMagicDefence"	
#define NPC_LUNAR_MAGIC_DEFENCE	    "LunarMagicDefence"		
#define NPC_POISON_MAGIC_DEFENCE	"PoisonMagicDefence"

#define NPC_AI_TYPE					"AIType"
#define NPC_AI_PARAM_ID				"AIParamTemplateID"

#define NPC_CORPSE_DOODAD_ID		"CorpseDoodadID"
#define NPC_DROP_CLASS_ID			"DropClassID"

#define NPC_SKILL_ID1				"SkillID1"
#define NPC_SKILL_ID2				"SkillID2"
#define NPC_SKILL_ID3				"SkillID3"
#define NPC_SKILL_ID4				"SkillID4"
#define NPC_SKILL_ID5				"SkillID5"
#define NPC_SKILL_ID6				"SkillID6"
#define NPC_SKILL_ID7				"SkillID7"
#define NPC_SKILL_ID8				"SkillID8"

#define NPC_SKILL_LEVEL1			"SkillLevel1"
#define NPC_SKILL_LEVEL2			"SkillLevel2"
#define NPC_SKILL_LEVEL3			"SkillLevel3"
#define NPC_SKILL_LEVEL4			"SkillLevel4"
#define NPC_SKILL_LEVEL5			"SkillLevel5"
#define NPC_SKILL_LEVEL6			"SkillLevel6"
#define NPC_SKILL_LEVEL7			"SkillLevel7"
#define NPC_SKILL_LEVEL8			"SkillLevel8"

#define NPC_SKILL_INTERVAL1			"SkillInterval1"
#define NPC_SKILL_INTERVAL2			"SkillInterval2"
#define NPC_SKILL_INTERVAL3			"SkillInterval3"
#define NPC_SKILL_INTERVAL4			"SkillInterval4"
#define NPC_SKILL_INTERVAL5			"SkillInterval5"
#define NPC_SKILL_INTERVAL6			"SkillInterval6"
#define NPC_SKILL_INTERVAL7			"SkillInterval7"
#define NPC_SKILL_INTERVAL8			"SkillInterval8"

#define NPC_SKILL_TYPE1				"SkillType1"
#define NPC_SKILL_TYPE2				"SkillType2"
#define NPC_SKILL_TYPE3				"SkillType3"
#define NPC_SKILL_TYPE4				"SkillType4"
#define NPC_SKILL_TYPE5				"SkillType5"
#define NPC_SKILL_TYPE6				"SkillType6"
#define NPC_SKILL_TYPE7				"SkillType7"
#define NPC_SKILL_TYPE8				"SkillType8"

#define NPC_SKILL_RATE1				"SkillRate1"
#define NPC_SKILL_RATE2				"SkillRate2"
#define NPC_SKILL_RATE3				"SkillRate3"
#define NPC_SKILL_RATE4				"SkillRate4"
#define NPC_SKILL_RATE5				"SkillRate5"
#define NPC_SKILL_RATE6				"SkillRate6"
#define NPC_SKILL_RATE7				"SkillRate7"
#define NPC_SKILL_RATE8				"SkillRate8"

#define NPC_SKILL_ANI_FRAME1		"SkillAniFrame1"
#define NPC_SKILL_ANI_FRAME2		"SkillAniFrame2"
#define NPC_SKILL_ANI_FRAME3		"SkillAniFrame3"
#define NPC_SKILL_ANI_FRAME4		"SkillAniFrame4"
#define NPC_SKILL_ANI_FRAME5		"SkillAniFrame5"
#define NPC_SKILL_ANI_FRAME6		"SkillAniFrame6"
#define NPC_SKILL_ANI_FRAME7		"SkillAniFrame7"
#define NPC_SKILL_ANI_FRAME8		"SkillAniFrame8"

#define NPC_SKILL_REST_FRAME1		"SkillRestFrame1"
#define NPC_SKILL_REST_FRAME2		"SkillRestFrame2"
#define NPC_SKILL_REST_FRAME3		"SkillRestFrame3"
#define NPC_SKILL_REST_FRAME4		"SkillRestFrame4"
#define NPC_SKILL_REST_FRAME5		"SkillRestFrame5"
#define NPC_SKILL_REST_FRAME6		"SkillRestFrame6"
#define NPC_SKILL_REST_FRAME7		"SkillRestFrame7"
#define NPC_SKILL_REST_FRAME8		"SkillRestFrame8"

#define NPC_THREAT_TIME				"ThreatTime"
#define NPC_THREAT_PERCENT			"ThreatPercent"
#define NPC_OVER_THREAT_PERCENT		"OverThreatPercent"
#define NPC_PURSUIT_RANGE		    "PursuitRange"

#define NPC_EXP                     "NpcExp"
#define NPC_MIN_MONEY				"MoneyMin"
#define NPC_MAX_MONEY				"MoneyMax"
#define NPC_MONEY_DROP_RATE			"MoneyDropRate"

#define NPC_REPUTE_ID1				"ReputeID1"
#define NPC_REPUTE_ID2				"ReputeID2"
#define NPC_REPUTE_ID3				"ReputeID3"
#define NPC_REPUTE_ID4				"ReputeID4"

#define NPC_REPUTE_VALUE1			"ReputeValue1"
#define NPC_REPUTE_VALUE2			"ReputeValue2"
#define NPC_REPUTE_VALUE3			"ReputeValue3"
#define NPC_REPUTE_VALUE4			"ReputeValue4"

#define NPC_REPUTE_LOW_LIMIT1		"ReputeLevel1"
#define NPC_REPUTE_LOW_LIMIT2		"ReputeLevel2"
#define NPC_REPUTE_LOW_LIMIT3		"ReputeLevel3"
#define NPC_REPUTE_LOW_LIMIT4		"ReputeLevel4"

#define NPC_REPUTE_HIGH_LIMIT1		"ReputeLevel1"
#define NPC_REPUTE_HIGH_LIMIT2		"ReputeLevel2"
#define NPC_REPUTE_HIGH_LIMIT3		"ReputeLevel3"
#define NPC_REPUTE_HIGH_LIMIT4		"ReputeLevel4"


#define NPC_REPRESENT_ID1		    "RepresentID1"
#define NPC_REPRESENT_ID2		    "RepresentID2"
#define NPC_REPRESENT_ID3		    "RepresentID3"
#define NPC_REPRESENT_ID4		    "RepresentID4"
#define NPC_REPRESENT_ID5		    "RepresentID5"
#define NPC_REPRESENT_ID6		    "RepresentID6"
#define NPC_REPRESENT_ID7		    "RepresentID7"
#define NPC_REPRESENT_ID8		    "RepresentID8"
#define NPC_REPRESENT_ID9		    "RepresentID9"
#define NPC_REPRESENT_ID10		    "RepresentID10"

#define NPC_DIALOG_ID				"NpcDialogID"

#define NPC_DIALOG1					"IdleDialog1"
#define NPC_DIALOG_RATE1			"IdleDialogRate1"
#define NPC_DIALOG2					"IdleDialog2"
#define NPC_DIALOG_RATE2			"IdleDialogRate2"
#define NPC_DIALOG3					"IdleDialog3"
#define NPC_DIALOG_RATE3			"IdleDialogRate3"
#define NPC_DIALOG_AFTER_QUEST		"IdleDialogAfterQuest"
#define NPC_DIALOG_QUEST_ID			"IdleDialogQuestID"

#define NPC_HAS_BANK				"HasBank"
#define NPC_HAS_MAILBOX				"HasMailBox"
#define NPC_HAS_CUB_PACKAGE          "HasCubPackage"
#define NPC_CUB_PACKAGE_TEXT         "CubPackageOptionText"
#define NPC_CUB_PACKAGE_REPUTE_LEVEL "CubPackageRequireReputeLevel"
#define NPC_MASTER_ID               "MasterID"
#define NPC_CRAFT_MASTER_ID			"CraftMasterID"

#define NPC_BANK_TEXT				"BankOptionText"
#define NPC_MAILBOX_TEXT			"MailOptionText"
#define NPC_SHOP_TEXT				"ShopOptionText"
#define NPC_MASTER_TEXT				"SkillMasterOptionText"
#define NPC_CRAFT_MASTER_TEXT		"CraftMasterOptionText"

#define NPC_SHOP_REPUTE_LEVEL           "ShopRequireReputeLevel"
#define NPC_MASTER_REPUTE_LEVEL         "MasterRequireReputeLevel"
#define NPC_CRAFT_MASTER_REPUTE_LEVEL   "CraftMasterRequireReputeLevel"
#define NPC_BANK_REPUTE_LEVEL           "BankRequireReputeLevel"
#define NPC_MAILBOX_REPUTE_LEVEL        "MailBoxRequireReputeLevel"
#define NPC_QUEST_REPUTE_LEVEL          "QuestRequireReputeLevel"

#define NPC_IMMUNITY_MASK           "ImmunityMask"

#define NPC_SCRIPT_NAME				"ScriptName"
#define NPC_DROP_NOT_QUEST_ITEM_FLAG "DropNotQuestItemFlag"
#define NPC_DAILY_QUEST_CYCLE       "DailyQuestCycle"
#define NPC_DAILY_QUEST_OFFSET      "DailyQuestOffset"
#define NPC_PROGRESS_ID             "ProgressID"

#define NPC_HAS_AUCTION                     "HasAuction"
#define NPC_AUCTION_OPTION_TEXT             "AuctionOptionText"
#define NPC_AUCTION_REQUIRE_REPUTE_LEVEL    "AuctionRequireReputeLevel"

#define NPC_HAS_TONG_REPERTORY              "HasTongRepertory"
#define NPC_TONG_REPERTORY_OPTION_TEXT      "TongRepertoryOptionText"
#define NPC_TONG_REPERTORY_REPUTE_LEVEL     "TongRepertoryRequireReputeLevel"

#define NPC_CAMP_LOOT_PRESTIGE "CampLootPrestige"
#define NPC_PRESTIGE           "Prestige"
#define NPC_CONTRIBUTE         "Contribution"
#define NPC_JUSTICE            "DropJustice"
#define NPC_EXAMPRINT          "DropExamPrint"
#define NPC_ACTIVITYAWARD      "DropActivityAward"
#define NPC_ACHIEVEMENT_ID     "AchievementID"

#define NPC_KNOCKED_BACK_RATE   "KnockedBackRate"
#define NPC_KNOCKED_DOWN_RATE   "KnockedDownRate"
#define NPC_KNOCKED_OFF_RATE    "KnockedOffRate"
#define NPC_REPULSED_RATE       "RepulsedRate"
#define NPC_PULL_RATE           "PullRate"
#define NPC_ADD_CAMP_SCORE      "AddCampScore"

#define NPC_HAS_GAME_CARD_SALE                      "HasGameCard"
#define NPC_HAS_GAME_CARD_BUY                       "HasGameCard"
#define NPC_GAME_CARD_SALE_OPTION_TEXT              "GameCardSaleOptionText"
#define NPC_GAME_CARD_BUY_OPTION_TEXT               "GameCardBuyOptionText"
#define NPC_GAME_CARD_TRADE_REQUIRE_REPUTE_LEVEL    "HasGameCard"

static void FreeNpcShopInfo(KSHOP_INFO& shopInfo)
{
    for (int i = 0; i < 16; ++i)
    {
        KMemory::Free(shopInfo.pszShopOptionText[i]);
        shopInfo.pszShopOptionText[i] = NULL;
    }
}

static void FreeNpcShopInfoMap(std::map<DWORD, KSHOP_INFO>& shopInfoList)
{
    for (std::map<DWORD, KSHOP_INFO>::iterator it = shopInfoList.begin(); it != shopInfoList.end(); ++it)
        FreeNpcShopInfo(it->second);
    shopInfoList.clear();
}

char* KNpcTemplate::GetShopOptionText(DWORD dwShopID)
{
	for (int i = 0; i < 16; ++i)
	{
		if (pShopInfo && dwShopID == pShopInfo->dwShopTemplateID[i])
			return pShopInfo->pszShopOptionText[i];
	}
	return NULL;
}

KNpcTemplateList::KNpcTemplateList(void)
{
	m_pNpcTemplateList = NULL;
	m_nNpcTemplateCount = 0;
	m_nLoadNpcTemplateIndex = 0;
	memset(&m_DefaultShopInfo, 0, sizeof(m_DefaultShopInfo));
}

KNpcTemplateList::~KNpcTemplateList(void)
{
	assert(m_pNpcTemplateList == NULL);
	assert(m_nNpcTemplateCount == 0);
}

BOOL KNpcTemplateList::Init(void)
{
	BOOL bResult = false;
	FreeNpcShopInfoMap(m_TmpNpcTemplateShopInfoList);
	FreeNpcShopInfo(m_DefaultShopInfo);
	memset(&m_DefaultNpcTemplate, 0, sizeof(m_DefaultNpcTemplate));
	memset(&m_DefaultShopInfo, 0, sizeof(m_DefaultShopInfo));
	m_mapNpcTemplateList.clear();
	m_TmpNpcTemplateShopInfoList.clear();
	m_nLoadNpcTemplateIndex = 0;
	m_DefaultNpcTemplate.pShopInfo = &m_DefaultShopInfo;
	KGLOG_PROCESS_ERROR(LoadNpcTemplateShopInfo(m_TmpNpcTemplateShopInfoList));
	KGLOG_PROCESS_ERROR(LoadNpcTemplateTabList());
	bResult = true;
Exit0:
	if (!bResult)
	{
		SAFE_DELETE_ARRAY(m_pNpcTemplateList);
		m_nNpcTemplateCount = 0;
		m_mapNpcTemplateList.clear();
	}
	if (!bResult)
		FreeNpcShopInfoMap(m_TmpNpcTemplateShopInfoList);
	return bResult;
}

BOOL KNpcTemplateList::LoadNpcTemplateShopInfo(std::map<DWORD, KSHOP_INFO>& shopInfoList)
{
	BOOL bResult = false;
	ITabFile* pTabFile = NULL;
	char szFilePath[MAX_PATH];
	char szColumn[64];
	int nHeight = 0;
	FreeNpcShopInfoMap(shopInfoList);

	snprintf(szFilePath, sizeof(szFilePath), "%s/%s", SETTING_DIR, NPC_TEMPLATE_SHOP_FILENAME);
	pTabFile = g_OpenTabFile(szFilePath);
	KGLOG_PROCESS_ERROR(pTabFile);
	nHeight = pTabFile->GetHeight();
	KGLOG_PROCESS_ERROR(nHeight >= 2);

	for (int nRow = 2; nRow < nHeight; ++nRow)
	{
		KSHOP_INFO info;
		DWORD dwTemplateID = 0;
		memset(&info, 0, sizeof(info));
		if (!pTabFile->GetInteger(nRow, "ID", 0, (int*)&dwTemplateID))
		{
			FreeNpcShopInfo(info);
			goto Exit0;
		}
		for (int i = 0; i < 16; ++i)
		{
			snprintf(szColumn, sizeof(szColumn), "ShopTemplateID%d", i + 1);
			if (!pTabFile->GetInteger(nRow, szColumn, 0, (int*)&info.dwShopTemplateID[i]))
			{
				FreeNpcShopInfo(info);
				goto Exit0;
			}
			snprintf(szColumn, sizeof(szColumn), "ShopOptionText%d", i + 1);
			if (!LoadOptionText(pTabFile, nRow, szColumn, &info.pszShopOptionText[i]))
			{
				FreeNpcShopInfo(info);
				goto Exit0;
			}
		}
		std::map<DWORD, KSHOP_INFO>::iterator it = shopInfoList.find(dwTemplateID);
		if (it != shopInfoList.end())
			FreeNpcShopInfo(it->second);
		shopInfoList[dwTemplateID] = info;
	}

	bResult = true;
Exit0:
	if (!bResult)
		FreeNpcShopInfoMap(shopInfoList);
	KG_COM_RELEASE(pTabFile);
	return bResult;
}

BOOL KNpcTemplateList::LoadOptionText(ITabFile* piTabFile, int nIndex, const char* pszColumn, char** ppszText)
{
	char szText[NPC_SHOP_OPTION_TEXT_LEN];

	KGLOG_PROCESS_ERROR(piTabFile);
	KGLOG_PROCESS_ERROR(pszColumn);
	KGLOG_PROCESS_ERROR(ppszText);
	*ppszText = NULL;
	memset(szText, 0, sizeof(szText));
	KGLOG_PROCESS_ERROR(piTabFile->GetString(nIndex, pszColumn, "", szText, sizeof(szText)));
	if (szText[0] == '\0')
		return true;

	*ppszText = (char*)KMemory::Alloc(strlen(szText) + 1);
	KGLOG_PROCESS_ERROR(*ppszText);
	strcpy(*ppszText, szText);
	return true;
Exit0:
	if (ppszText && *ppszText)
	{
		KMemory::Free(*ppszText);
		*ppszText = NULL;
	}
	return false;
}

BOOL KNpcTemplateList::LoadNpcTemplateTabList(void)
{
	BOOL bResult = false;
	ITabFile* pTabFile = NULL;
	char szFilePath[MAX_PATH];
	char szRelativePath[MAX_PATH];
	int nHeight = 0;

	snprintf(szFilePath, sizeof(szFilePath), "%s/%s", SETTING_DIR, NPC_TEMPLATE_FILENAME);
	pTabFile = g_OpenTabFile(szFilePath);
	KGLOG_PROCESS_ERROR(pTabFile);
	nHeight = pTabFile->GetHeight();
	KGLOG_PROCESS_ERROR(nHeight >= 2);

	for (int nRow = 2; nRow < nHeight; ++nRow)
	{
		KGLOG_PROCESS_ERROR(pTabFile->GetString(nRow, "FilePath", "", szRelativePath, sizeof(szRelativePath)));
		for (char* p = szRelativePath; *p; ++p)
		{
			if (*p == '\\')
				*p = '/';
		}
		if (strncmp(szRelativePath, "settings/", 9) == 0)
			memmove(szRelativePath, szRelativePath + 9, strlen(szRelativePath + 9) + 1);
		KGLOG_PROCESS_ERROR(LoadNpcTemplateTab(szRelativePath, nRow == 2));
	}

	m_nNpcTemplateCount = m_nLoadNpcTemplateIndex;
	bResult = true;
Exit0:
	KG_COM_RELEASE(pTabFile);
	return bResult;
}

BOOL KNpcTemplateList::LoadNpcTemplateTab(char* pszFilePath, int bDefault)
{
	BOOL bResult = false;
	ITabFile* pTabFile = NULL;
	char szFullPath[MAX_PATH];
	int nHeight = 0;

	KGLOG_PROCESS_ERROR(pszFilePath);
	snprintf(szFullPath, sizeof(szFullPath), "%s/%s", SETTING_DIR, pszFilePath);
	pTabFile = g_OpenTabFile(szFullPath);
	KGLOG_PROCESS_ERROR(pTabFile);
	nHeight = pTabFile->GetHeight();
	KGLOG_PROCESS_ERROR(nHeight >= 2);

	if (bDefault)
	{
		KGLOG_PROCESS_ERROR(LoadNpcTemplate(2, pTabFile, m_DefaultNpcTemplate));
	}
	else
	{
		int nCount = nHeight - 2;
		KNpcTemplate* pNewList = new KNpcTemplate[m_nLoadNpcTemplateIndex + nCount];
		KGLOG_PROCESS_ERROR(pNewList);
		if (m_pNpcTemplateList && m_nLoadNpcTemplateIndex > 0)
		{
			memcpy(pNewList, m_pNpcTemplateList, sizeof(KNpcTemplate) * m_nLoadNpcTemplateIndex);
			for (std::map<DWORD, KNpcTemplate*>::iterator it = m_mapNpcTemplateList.begin(); it != m_mapNpcTemplateList.end(); ++it)
				it->second = pNewList + (it->second - m_pNpcTemplateList);
		}
		SAFE_DELETE_ARRAY(m_pNpcTemplateList);
		m_pNpcTemplateList = pNewList;
		for (int nRow = 2; nRow < nHeight; ++nRow)
		{
			KNpcTemplate& npcTemplate = m_pNpcTemplateList[m_nLoadNpcTemplateIndex];
			memset(&npcTemplate, 0, sizeof(npcTemplate));
			KGLOG_PROCESS_ERROR(LoadNpcTemplate(nRow, pTabFile, npcTemplate));
			KGLOG_PROCESS_ERROR(npcTemplate.dwTemplateID != 0);
			std::map<DWORD, KSHOP_INFO>::iterator shopIt = m_TmpNpcTemplateShopInfoList.find(npcTemplate.dwTemplateID);
			npcTemplate.pShopInfo = shopIt != m_TmpNpcTemplateShopInfoList.end() ? &shopIt->second : &m_DefaultShopInfo;
			std::map<DWORD, KNpcTemplate*>::iterator it = m_mapNpcTemplateList.find(npcTemplate.dwTemplateID);
			KGLOG_PROCESS_ERROR(it == m_mapNpcTemplateList.end());
			m_mapNpcTemplateList[npcTemplate.dwTemplateID] = &npcTemplate;
			++m_nLoadNpcTemplateIndex;
		}
	}

	bResult = true;
Exit0:
	KG_COM_RELEASE(pTabFile);
	return bResult;
}

BOOL KNpcTemplateList::UnInit(void)
{
	KG_DELETE_ARRAY(m_pNpcTemplateList);
	m_nNpcTemplateCount = 0;
	m_mapNpcTemplateList.clear();
	FreeNpcShopInfoMap(m_TmpNpcTemplateShopInfoList);
	FreeNpcShopInfo(m_DefaultShopInfo);
	m_nLoadNpcTemplateIndex = 0;

	return true;
}

BOOL KNpcTemplateList::LoadFromTemplate(DWORD dwNpcTemplateID, KNpc* pNpc)
{
	BOOL			    bRetCode		= false;
	KNpcTemplate*	    pNpcTemplate	= NULL;

    assert(pNpc);
    assert(pNpc->m_pTemplate == NULL);

	pNpcTemplate = GetTemplate(dwNpcTemplateID);
	if (pNpcTemplate == NULL)
	{
        KGLogPrintf(KGLOG_ERR, "Invalid NPC template ID: %u!\n", dwNpcTemplateID);

		pNpcTemplate = GetTemplate(0);
		KGLOG_PROCESS_ERROR(pNpcTemplate);
	}

	pNpc->m_dwTemplateID			= dwNpcTemplateID;
	pNpc->m_dwForceID				= pNpcTemplate->dwForceID;
    pNpc->m_dwGuardForceID          = pNpcTemplate->dwGuardForceID;
	pNpc->m_eKind                   = (CHARACTER_KIND)pNpcTemplate->nKind;
    pNpc->m_eSpecies                = (KNPC_SPECIES)pNpcTemplate->nSpecies;
	pNpc->m_nLevel                  = pNpcTemplate->nLevel;
    pNpc->m_nTouchRange             = pNpcTemplate->nTouchRange;
    pNpc->m_nIntensity              = pNpcTemplate->nIntensity;
    pNpc->m_nReviveTime             = pNpcTemplate->nReviveTime;
	pNpc->m_nHeight					= pNpcTemplate->nHeight;
    pNpc->m_eMoveMode               = pNpcTemplate->eMoveMode;
    pNpc->m_AIData.nAIType          = pNpcTemplate->nAIType;

    pNpc->CallAttributeFunction(atMaxLifeBase, true, pNpcTemplate->nMaxLife, 0);
    pNpc->CallAttributeFunction(atLifeReplenish, true, pNpcTemplate->nLifeReplenish, 0);
    pNpc->CallAttributeFunction(atLifeReplenishPercent, true, pNpcTemplate->nLifeReplenishPercent, 0);

    pNpc->CallAttributeFunction(atMaxManaBase, true, pNpcTemplate->nMaxMana, 0);
    pNpc->CallAttributeFunction(atManaReplenish, true, pNpcTemplate->nManaReplenish, 0);
    pNpc->CallAttributeFunction(atManaReplenishPercent, true, pNpcTemplate->nManaReplenishPercent, 0);

    pNpc->CallAttributeFunction(atRunSpeedBase, true, pNpcTemplate->nRunSpeed, 0);
    pNpc->CallAttributeFunction(atJumpSpeedBase, true, pNpcTemplate->nJumpSpeed, 0);

    pNpc->m_nMeleeWeaponDamageBase = pNpcTemplate->nMeleeWeaponDamageBase;
    pNpc->m_nMeleeWeaponDamageRand = pNpcTemplate->nMeleeWeaponDamageRand;
    pNpc->m_nRangeWeaponDamageBase = pNpcTemplate->nRangeWeaponDamageBase;
    pNpc->m_nRangeWeaponDamageRand = pNpcTemplate->nRangeWeaponDamageRand;

    pNpc->CallAttributeFunction(atPhysicsHitBaseRate, true, pNpcTemplate->nHitBase, 0);
    pNpc->CallAttributeFunction(atSolarHitBaseRate, true, pNpcTemplate->nHitBase, 0);
    pNpc->CallAttributeFunction(atNeutralHitBaseRate, true, pNpcTemplate->nHitBase, 0);
    pNpc->CallAttributeFunction(atLunarHitBaseRate, true, pNpcTemplate->nHitBase, 0);
    pNpc->CallAttributeFunction(atPoisonHitBaseRate, true, pNpcTemplate->nHitBase, 0);

    pNpc->CallAttributeFunction(atDodge, true, pNpcTemplate->nDodge, 0);
    pNpc->CallAttributeFunction(atParryBase, true, pNpcTemplate->nParry, 0);
    pNpc->CallAttributeFunction(atParryValueBase, true, pNpcTemplate->nParryValue, 0);
    pNpc->CallAttributeFunction(atSense, true, pNpcTemplate->nSense, 0);

    pNpc->CallAttributeFunction(atPhysicsCriticalStrike, true, pNpcTemplate->nPhysicsCriticalStrike, 0);
    pNpc->CallAttributeFunction(atSolarCriticalStrike, true, pNpcTemplate->nSolarCriticalStrike, 0);
    pNpc->CallAttributeFunction(atNeutralCriticalStrike, true, pNpcTemplate->nNeutralCriticalStrike, 0);
    pNpc->CallAttributeFunction(atLunarCriticalStrike, true, pNpcTemplate->nLunarCriticalStrike, 0);
    pNpc->CallAttributeFunction(atPoisonCriticalStrike, true, pNpcTemplate->nPoisonCriticalStrike, 0);

    pNpc->CallAttributeFunction(atPhysicsShieldBase, true, pNpcTemplate->nPhysicsShieldBase, 0);
    pNpc->CallAttributeFunction(atSolarMagicShieldBase, true, pNpcTemplate->nSolarMagicDefence, 0);
    pNpc->CallAttributeFunction(atNeutralMagicShieldBase, true, pNpcTemplate->nNeutralMagicDefence, 0);
    pNpc->CallAttributeFunction(atLunarMagicShieldBase, true, pNpcTemplate->nLunarMagicDefence, 0);
    pNpc->CallAttributeFunction(atPoisonMagicShieldBase, true, pNpcTemplate->nPoisonMagicDefence, 0);

    // ��ȡ Boss���� ��������
    assert(sftTotal <= (sizeof(DWORD) * CHAR_BIT + 1));
    for (int nFuncType = sftInvalid + 1; nFuncType < sftTotal; nFuncType++)
    {
        if (pNpcTemplate->dwImmunityMask & ((DWORD)0x1 << (nFuncType - 1)))
        {
            pNpc->m_ImmunityFlag.set(nFuncType - 1);
        }
        else
        {
            pNpc->m_ImmunityFlag.reset(nFuncType - 1);
        }
    }

	strncpy(pNpc->m_szName, pNpcTemplate->szName, _NAME_LEN);
	pNpc->m_szName[_NAME_LEN - 1] = 0;

#ifdef _CLIENT
	strncpy(pNpc->m_szTitle, pNpcTemplate->szTitle, TITLE_LEN);
	pNpc->m_szTitle[TITLE_LEN - 1] = 0;
#endif

	pNpc->m_nCurrentLife = pNpc->m_nMaxLife;
	pNpc->m_nCurrentMana = pNpc->m_nMaxMana;

	pNpc->m_pTemplate = pNpcTemplate;

#if defined(_SERVER)
	pNpc->m_nThreatTime	= pNpcTemplate->nThreatTime;		

	pNpc->m_bBank           = pNpcTemplate->bHasBank;
    pNpc->m_bMailbox        = pNpcTemplate->bHasMailBox;
    pNpc->m_dwSkillMasterID = pNpcTemplate->dwMasterID;
    pNpc->m_dwCraftMasterID = pNpcTemplate->dwCraftMasterID;
    pNpc->m_bAuction        = pNpcTemplate->bHasAuction;
    pNpc->m_bTongRepertory  = pNpcTemplate->bHasTongRepertory;

	if (pNpcTemplate->pShopInfo)
	{
		pNpcTemplate->nShopTemplateID = pNpcTemplate->pShopInfo->dwShopTemplateID[0];
		if (pNpcTemplate->pShopInfo->pszShopOptionText[0])
		{
			strncpy(pNpcTemplate->szShopOptionText, pNpcTemplate->pShopInfo->pszShopOptionText[0], MAX_OPTION_TEXT_LEN);
			pNpcTemplate->szShopOptionText[MAX_OPTION_TEXT_LEN - 1] = '\0';
		}
		else
		{
			pNpcTemplate->szShopOptionText[0] = '\0';
		}
		for (int i = 0; i < 16; ++i)
		{
			if (pNpcTemplate->pShopInfo->dwShopTemplateID[i] != 0)
				g_pSO3World->m_ShopCenter.BindNpcShop(pNpc, pNpcTemplate->pShopInfo->dwShopTemplateID[i]);
		}
	}
	
	if (pNpcTemplate->szScriptName[0] != '\0')
	{
		pNpc->m_dwScriptID = g_FileNameHash(pNpcTemplate->szScriptName);
	}

	pNpc->m_dwModelID  = pNpcTemplate->dwRepresentID[0]; //Ĭ�϶���һ������ID

    pNpc->m_nProgressID = pNpcTemplate->nProgressID;
#endif

    pNpc->m_nKnockedBackRate = pNpcTemplate->nKnockedBackRate;
    pNpc->m_nKnockedDownRate = pNpcTemplate->nKnockedDownRate;
    pNpc->m_nKnockedOffRate = pNpcTemplate->nKnockedOffRate;
    pNpc->m_nRepulsedRate = pNpcTemplate->nRepulsedRate;
    pNpc->m_nPullRate = pNpcTemplate->nPullRate;

	return true;

Exit0:
	KGLogPrintf(KGLOG_ERR, "KNpcTemplateList::LoadFromTemplate(%d)\n", dwNpcTemplateID);
	return false;
}

int KNpcTemplateList::GetNpcTemplateCount()
{
	return m_nNpcTemplateCount;
}

KNpcTemplate* KNpcTemplateList::GetTemplateByIndex(int nIndex)
{
	KGLOG_PROCESS_ERROR(nIndex >= 0 && nIndex < m_nNpcTemplateCount);
	KGLOG_PROCESS_ERROR(m_pNpcTemplateList);

	return &(m_pNpcTemplateList[nIndex]);
Exit0:
	return NULL;
}

KNpcTemplate* KNpcTemplateList::GetTemplate(DWORD dwNpcTemplateID)
{
	KNpcTemplate* pResult = NULL;
	KNPC_TEMPLATE_LIST::iterator iter;

	if (dwNpcTemplateID == 0)
	{
		pResult = &m_DefaultNpcTemplate;
		goto Exit0;
	}

	iter = m_mapNpcTemplateList.find(dwNpcTemplateID);
	if (iter != m_mapNpcTemplateList.end())
	{
		pResult = iter->second;
	}
Exit0:
	return pResult;
}

BOOL KNpcTemplateList::LoadNpcTemplate(int nIndex, ITabFile* piTabFile, KNpcTemplate& fNpcTemplate)
{
    BOOL bResult  = false;
	BOOL bRetCode = false;

#ifdef _SERVER
    char szColName[_NAME_LEN];
#endif

	KGLOG_PROCESS_ERROR(piTabFile);

	bRetCode = piTabFile->GetInteger(nIndex, NPC_TEMPLATE_ID, m_DefaultNpcTemplate.dwTemplateID, (int*)&(fNpcTemplate.dwTemplateID));
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetString(nIndex, NPC_NAME, m_DefaultNpcTemplate.szName, fNpcTemplate.szName, _NAME_LEN);
	(void)bRetCode; /*[endgame] tolerant*/

#ifdef _CLIENT
	bRetCode = piTabFile->GetString(nIndex, NPC_TITLE, m_DefaultNpcTemplate.szTitle, fNpcTemplate.szTitle, TITLE_LEN);
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetString(nIndex, NPC_MAP_NAME, m_DefaultNpcTemplate.szMapName, fNpcTemplate.szMapName, _NAME_LEN);
	(void)bRetCode; /*[endgame] tolerant*/
#endif

	bRetCode = piTabFile->GetInteger(nIndex, NPC_KIND, m_DefaultNpcTemplate.nKind, &(fNpcTemplate.nKind));
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(nIndex, NPC_FORCE_ID, m_DefaultNpcTemplate.dwForceID, (int*)&(fNpcTemplate.dwForceID));
	(void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(nIndex, NPC_GUARD_FORCE_ID, m_DefaultNpcTemplate.dwGuardForceID, (int*)&(fNpcTemplate.dwGuardForceID));
    (void)bRetCode; /*[endgame] tolerant*/
    
    bRetCode = piTabFile->GetInteger(nIndex, NPC_SPECIES, m_DefaultNpcTemplate.nSpecies, &fNpcTemplate.nSpecies);
    (void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(nIndex, NPC_LEVEL, m_DefaultNpcTemplate.nLevel, &(fNpcTemplate.nLevel));
	(void)bRetCode; /*[endgame] tolerant*/
    if (fNpcTemplate.nLevel <= 0)
    {
        KGLogPrintf(KGLOG_ERR, "Npc (%lu) level error!", fNpcTemplate.dwTemplateID);
        goto Exit0;
    }

	bRetCode = piTabFile->GetInteger(nIndex, NPC_ADJUST_LEVEL, m_DefaultNpcTemplate.nAdjustLevel, &(fNpcTemplate.nAdjustLevel));	
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(nIndex, NPC_HEIGHT, m_DefaultNpcTemplate.nHeight, &(fNpcTemplate.nHeight));
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(nIndex, NPC_MOVE_MODE, m_DefaultNpcTemplate.eMoveMode, (int*)&(fNpcTemplate.eMoveMode));
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(nIndex, NPC_TOUCH_RANGE, m_DefaultNpcTemplate.nTouchRange, &(fNpcTemplate.nTouchRange));
	(void)bRetCode; /*[endgame] tolerant*/
    KGLOG_PROCESS_ERROR(fNpcTemplate.nTouchRange >= 0);

	bRetCode = piTabFile->GetInteger(nIndex, NPC_INTENSITY, m_DefaultNpcTemplate.nIntensity, &(fNpcTemplate.nIntensity));
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(nIndex, NPC_REVIVE_TIME, m_DefaultNpcTemplate.nReviveTime, &(fNpcTemplate.nReviveTime));
	(void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(nIndex, NPC_REVIVE_MIN_TIME, m_DefaultNpcTemplate.nReviveMinTime, &(fNpcTemplate.nReviveMinTime));
    (void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(nIndex, NPC_IS_SELECTABLE,	m_DefaultNpcTemplate.bSelectable, (int*)&(fNpcTemplate.bSelectable));
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(nIndex, NPC_CAN_SEE_LIFEBAR, m_DefaultNpcTemplate.bCanSeeLifeBar, (int*)&(fNpcTemplate.bCanSeeLifeBar));
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(nIndex, NPC_CAN_SEE_NAME, m_DefaultNpcTemplate.bCanSeeName, (int*)&(fNpcTemplate.bCanSeeName));
	(void)bRetCode; /*[endgame] tolerant*/
    
	bRetCode = piTabFile->GetInteger(nIndex, NPC_MAX_LIFE, m_DefaultNpcTemplate.nMaxLife, (int*)&(fNpcTemplate.nMaxLife));
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(nIndex, NPC_LIFE_REPLENISH, m_DefaultNpcTemplate.nLifeReplenish, &(fNpcTemplate.nLifeReplenish));
	(void)bRetCode; /*[endgame] tolerant*/

    //�����ظ��ٷֱ�
	bRetCode = piTabFile->GetInteger(nIndex, NPC_LIFE_REPLENISH_PERCENT, m_DefaultNpcTemplate.nLifeReplenishPercent, &(fNpcTemplate.nLifeReplenishPercent));
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_MAX_MANA, 
		m_DefaultNpcTemplate.nMaxMana, (int*)&(fNpcTemplate.nMaxMana)
    );
	(void)bRetCode; /*[endgame] tolerant*/

    //�����ظ�����
	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_MANA_REPLENISH, 
		m_DefaultNpcTemplate.nManaReplenish, (int*)&(fNpcTemplate.nManaReplenish)
    );
	(void)bRetCode; /*[endgame] tolerant*/

    //�����ظ��ٷֱ�
	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_MANA_REPLENISH_PERCENT, 
		m_DefaultNpcTemplate.nManaReplenishPercent, (int*)&(fNpcTemplate.nManaReplenishPercent)
    );
	(void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_WALK_SPEED, 
        m_DefaultNpcTemplate.nWalkSpeed, (int*)&(fNpcTemplate.nWalkSpeed)
    );
    (void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_RUN_SPEED, 
		m_DefaultNpcTemplate.nRunSpeed, (int*)&(fNpcTemplate.nRunSpeed)
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_JUMP_SPEED, 
		m_DefaultNpcTemplate.nJumpSpeed, (int*)&(fNpcTemplate.nJumpSpeed)
    );
	(void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_MELEE_WEAPON_DAMAGE_BASE,
        m_DefaultNpcTemplate.nMeleeWeaponDamageBase, (int*)&(fNpcTemplate.nMeleeWeaponDamageBase)
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_MELEE_WEAPON_DAMAGE_RAND,
        m_DefaultNpcTemplate.nMeleeWeaponDamageRand, (int*)&(fNpcTemplate.nMeleeWeaponDamageRand)
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_RANGE_WEAPON_DAMAGE_BASE,
        m_DefaultNpcTemplate.nRangeWeaponDamageBase, (int*)&(fNpcTemplate.nRangeWeaponDamageBase)
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_RANGE_WEAPON_DAMAGE_RAND,
        m_DefaultNpcTemplate.nRangeWeaponDamageRand, (int*)&(fNpcTemplate.nRangeWeaponDamageRand)
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_HIT_BASE, 
		m_DefaultNpcTemplate.nHitBase, (int*)&(fNpcTemplate.nHitBase)
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_DODGE, 
		m_DefaultNpcTemplate.nDodge, (int*)&(fNpcTemplate.nDodge)
    );
	(void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_PARRY, 
		m_DefaultNpcTemplate.nParry, (int*)&(fNpcTemplate.nParry)
    );
	(void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_PARRY_VALUE, 
		m_DefaultNpcTemplate.nParryValue, (int*)&(fNpcTemplate.nParryValue)
    );
	(void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SENSE, 
		m_DefaultNpcTemplate.nSense, (int*)&(fNpcTemplate.nSense)
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_PHYSICS_CRITICAL_STRIKE, 
		m_DefaultNpcTemplate.nPhysicsCriticalStrike, (int*)&(fNpcTemplate.nPhysicsCriticalStrike)
    );
	(void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SOLAR_CRITICAL_STRIKE, 
        m_DefaultNpcTemplate.nSolarCriticalStrike, (int*)&(fNpcTemplate.nSolarCriticalStrike)
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_NEUTRAL_CRITICAL_STRIKE, 
        m_DefaultNpcTemplate.nNeutralCriticalStrike, (int*)&(fNpcTemplate.nNeutralCriticalStrike)
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_LUNAR_CRITICAL_STRIKE, 
        m_DefaultNpcTemplate.nLunarCriticalStrike, (int*)&(fNpcTemplate.nLunarCriticalStrike)
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_POISON_CRITICAL_STRIKE, 
        m_DefaultNpcTemplate.nPoisonCriticalStrike, (int*)&(fNpcTemplate.nPoisonCriticalStrike)
    );
    (void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_PHYSICS_SHIELD_BASE, 
		m_DefaultNpcTemplate.nPhysicsShieldBase, (int*)&(fNpcTemplate.nPhysicsShieldBase)
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SOLAR_MAGIC_DEFENCE, 
		m_DefaultNpcTemplate.nSolarMagicDefence, (int*)&(fNpcTemplate.nSolarMagicDefence)
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_NEUTRAL_MAGIC_DEFENCE, 
		m_DefaultNpcTemplate.nNeutralMagicDefence, (int*)&(fNpcTemplate.nNeutralMagicDefence)
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_LUNAR_MAGIC_DEFENCE, 
		m_DefaultNpcTemplate.nLunarMagicDefence, (int*)&(fNpcTemplate.nLunarMagicDefence)
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_POISON_MAGIC_DEFENCE, 
		m_DefaultNpcTemplate.nPoisonMagicDefence, (int*)&(fNpcTemplate.nPoisonMagicDefence)
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_AI_TYPE, 
		m_DefaultNpcTemplate.nAIType, (int*)&(fNpcTemplate.nAIType)
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_AI_PARAM_ID,
		m_DefaultNpcTemplate.nAIParamTemplateID, (int*)&(fNpcTemplate.nAIParamTemplateID)
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_CORPSE_DOODAD_ID, 
		m_DefaultNpcTemplate.nCorpseDoodadID, (int*)&(fNpcTemplate.nCorpseDoodadID)
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_DROP_CLASS_ID,
		m_DefaultNpcTemplate.nDropClassID, &fNpcTemplate.nDropClassID
    );
	(void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_IMMUNITY_MASK,
        m_DefaultNpcTemplate.dwImmunityMask, (int*)&fNpcTemplate.dwImmunityMask
    );
    (void)bRetCode; /*[endgame] tolerant*/

#ifdef _SERVER
    for (int i = 0; i < MAX_DROP_PER_NPC; ++i)
    {
        snprintf(szColName, sizeof(szColName), "Drop%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

        bRetCode = piTabFile->GetString(nIndex, szColName, m_DefaultNpcTemplate.szDropName[i], fNpcTemplate.szDropName[i], MAX_PATH);
	    (void)bRetCode; /*[endgame] tolerant*/
        
        snprintf(szColName, sizeof(szColName), "Count%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

        bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultNpcTemplate.nDropCount[i], &fNpcTemplate.nDropCount[i]);
	    (void)bRetCode; /*[endgame] tolerant*/
    }

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_ID1,
		m_DefaultNpcTemplate.dwSkillIDList[0], (int*)&fNpcTemplate.dwSkillIDList[0]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_ID2,
		m_DefaultNpcTemplate.dwSkillIDList[1], (int*)&fNpcTemplate.dwSkillIDList[1]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_ID3,
		m_DefaultNpcTemplate.dwSkillIDList[2], (int*)&fNpcTemplate.dwSkillIDList[2]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_ID4,
		m_DefaultNpcTemplate.dwSkillIDList[3], (int*)&fNpcTemplate.dwSkillIDList[3]
    );
	(void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_ID5,
        m_DefaultNpcTemplate.dwSkillIDList[4], (int*)&fNpcTemplate.dwSkillIDList[4]
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_ID6,
        m_DefaultNpcTemplate.dwSkillIDList[5], (int*)&fNpcTemplate.dwSkillIDList[5]
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_ID7,
        m_DefaultNpcTemplate.dwSkillIDList[6], (int*)&fNpcTemplate.dwSkillIDList[6]
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_ID8,
        m_DefaultNpcTemplate.dwSkillIDList[7], (int*)&fNpcTemplate.dwSkillIDList[7]
    );
    (void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_LEVEL1,
		m_DefaultNpcTemplate.dwSkillLevelList[0], (int*)&fNpcTemplate.dwSkillLevelList[0]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_LEVEL2,
		m_DefaultNpcTemplate.dwSkillLevelList[1], (int*)&fNpcTemplate.dwSkillLevelList[1]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_LEVEL3,
		m_DefaultNpcTemplate.dwSkillLevelList[2], (int*)&fNpcTemplate.dwSkillLevelList[2]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_LEVEL4,
		m_DefaultNpcTemplate.dwSkillLevelList[3], (int*)&fNpcTemplate.dwSkillLevelList[3]
    );
	(void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_LEVEL5,
        m_DefaultNpcTemplate.dwSkillLevelList[4], (int*)&fNpcTemplate.dwSkillLevelList[4]
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_LEVEL6,
        m_DefaultNpcTemplate.dwSkillLevelList[5], (int*)&fNpcTemplate.dwSkillLevelList[5]
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_LEVEL7,
        m_DefaultNpcTemplate.dwSkillLevelList[6], (int*)&fNpcTemplate.dwSkillLevelList[6]
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_LEVEL8,
        m_DefaultNpcTemplate.dwSkillLevelList[7], (int*)&fNpcTemplate.dwSkillLevelList[7]
    );
    (void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_INTERVAL1,
		m_DefaultNpcTemplate.nSkillCastInterval[0], (int*)&fNpcTemplate.nSkillCastInterval[0]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_INTERVAL2,
		m_DefaultNpcTemplate.nSkillCastInterval[1], (int*)&fNpcTemplate.nSkillCastInterval[1]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_INTERVAL3,
		m_DefaultNpcTemplate.nSkillCastInterval[2], (int*)&fNpcTemplate.nSkillCastInterval[2]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_INTERVAL4,
		m_DefaultNpcTemplate.nSkillCastInterval[3], (int*)&fNpcTemplate.nSkillCastInterval[3]
    );
	(void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_INTERVAL5,
        m_DefaultNpcTemplate.nSkillCastInterval[4], (int*)&fNpcTemplate.nSkillCastInterval[4]
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_INTERVAL6,
        m_DefaultNpcTemplate.nSkillCastInterval[5], (int*)&fNpcTemplate.nSkillCastInterval[5]
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_INTERVAL7,
        m_DefaultNpcTemplate.nSkillCastInterval[6], (int*)&fNpcTemplate.nSkillCastInterval[6]
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_INTERVAL8,
        m_DefaultNpcTemplate.nSkillCastInterval[7], (int*)&fNpcTemplate.nSkillCastInterval[7]
    );
    (void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_TYPE1,
		m_DefaultNpcTemplate.nAISkillType[0], &fNpcTemplate.nAISkillType[0]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_TYPE2,
		m_DefaultNpcTemplate.nAISkillType[1], &fNpcTemplate.nAISkillType[1]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_TYPE3,
		m_DefaultNpcTemplate.nAISkillType[2], &fNpcTemplate.nAISkillType[2]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_TYPE4,
		m_DefaultNpcTemplate.nAISkillType[3], &fNpcTemplate.nAISkillType[3]
    );
	(void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_TYPE5,
        m_DefaultNpcTemplate.nAISkillType[4], &fNpcTemplate.nAISkillType[4]
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_TYPE6,
        m_DefaultNpcTemplate.nAISkillType[5], &fNpcTemplate.nAISkillType[5]
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_TYPE7,
        m_DefaultNpcTemplate.nAISkillType[6], &fNpcTemplate.nAISkillType[6]
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_TYPE8,
        m_DefaultNpcTemplate.nAISkillType[7], &fNpcTemplate.nAISkillType[7]
    );
    (void)bRetCode; /*[endgame] tolerant*/
    
	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_RATE1,
		m_DefaultNpcTemplate.nSkillRate[0], & fNpcTemplate.nSkillRate[0]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_RATE2,
		m_DefaultNpcTemplate.nSkillRate[1], & fNpcTemplate.nSkillRate[1]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_RATE3,
		m_DefaultNpcTemplate.nSkillRate[2], & fNpcTemplate.nSkillRate[2]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_RATE4,
		m_DefaultNpcTemplate.nSkillRate[3], & fNpcTemplate.nSkillRate[3]
    );
	(void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_RATE5,
        m_DefaultNpcTemplate.nSkillRate[4], & fNpcTemplate.nSkillRate[4]
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_RATE6,
        m_DefaultNpcTemplate.nSkillRate[5], & fNpcTemplate.nSkillRate[5]
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_RATE7,
        m_DefaultNpcTemplate.nSkillRate[6], & fNpcTemplate.nSkillRate[6]
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_RATE8,
        m_DefaultNpcTemplate.nSkillRate[7], & fNpcTemplate.nSkillRate[7]
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_ANI_FRAME1,
        m_DefaultNpcTemplate.nSkillAniFrame[0], & fNpcTemplate.nSkillAniFrame[0]
    );
    (void)bRetCode; /*[endgame] tolerant*/
    
    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_ANI_FRAME2,
        m_DefaultNpcTemplate.nSkillAniFrame[1], & fNpcTemplate.nSkillAniFrame[1]
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_ANI_FRAME3,
        m_DefaultNpcTemplate.nSkillAniFrame[2], & fNpcTemplate.nSkillAniFrame[2]
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_ANI_FRAME4,
        m_DefaultNpcTemplate.nSkillAniFrame[3], & fNpcTemplate.nSkillAniFrame[3]
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_ANI_FRAME5,
        m_DefaultNpcTemplate.nSkillAniFrame[4], & fNpcTemplate.nSkillAniFrame[4]
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_ANI_FRAME6,
        m_DefaultNpcTemplate.nSkillAniFrame[5], & fNpcTemplate.nSkillAniFrame[5]
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_ANI_FRAME7,
        m_DefaultNpcTemplate.nSkillAniFrame[6], & fNpcTemplate.nSkillAniFrame[6]
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_ANI_FRAME8,
        m_DefaultNpcTemplate.nSkillAniFrame[7], & fNpcTemplate.nSkillAniFrame[7]
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_REST_FRAME1,
        m_DefaultNpcTemplate.nSkillRestFrame[0], & fNpcTemplate.nSkillRestFrame[0]
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_REST_FRAME2,
        m_DefaultNpcTemplate.nSkillRestFrame[1], & fNpcTemplate.nSkillRestFrame[1]
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_REST_FRAME3,
        m_DefaultNpcTemplate.nSkillRestFrame[2], & fNpcTemplate.nSkillRestFrame[2]
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_REST_FRAME4,
        m_DefaultNpcTemplate.nSkillRestFrame[3], & fNpcTemplate.nSkillRestFrame[3]
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_REST_FRAME5,
        m_DefaultNpcTemplate.nSkillRestFrame[4], & fNpcTemplate.nSkillRestFrame[4]
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_REST_FRAME6,
        m_DefaultNpcTemplate.nSkillRestFrame[5], & fNpcTemplate.nSkillRestFrame[5]
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_REST_FRAME7,
        m_DefaultNpcTemplate.nSkillRestFrame[6], & fNpcTemplate.nSkillRestFrame[6]
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SKILL_REST_FRAME8,
        m_DefaultNpcTemplate.nSkillRestFrame[7], & fNpcTemplate.nSkillRestFrame[7]
    );
    (void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_THREAT_TIME,
		m_DefaultNpcTemplate.nThreatTime, (int*)&fNpcTemplate.nThreatTime
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_OVER_THREAT_PERCENT,
		m_DefaultNpcTemplate.nOverThreatPercent, (int*)&fNpcTemplate.nOverThreatPercent
    );
	(void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_PURSUIT_RANGE,
        m_DefaultNpcTemplate.nPursuitRange, (int*)&fNpcTemplate.nPursuitRange
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_EXP,
        m_DefaultNpcTemplate.nExp, (int*)&fNpcTemplate.nExp
    );
    (void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_MIN_MONEY,
		m_DefaultNpcTemplate.nMinMoney, (int*)&fNpcTemplate.nMinMoney
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_MAX_MONEY,
		m_DefaultNpcTemplate.nMaxMoney, (int*)&fNpcTemplate.nMaxMoney
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_MONEY_DROP_RATE,
		m_DefaultNpcTemplate.nMoneyRate, (int*)&fNpcTemplate.nMoneyRate
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_REPUTE_ID1,
		m_DefaultNpcTemplate.dwReputeID[0], (int*)&fNpcTemplate.dwReputeID[0]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_REPUTE_VALUE1,
		m_DefaultNpcTemplate.nReputeValue[0], (int*)&fNpcTemplate.nReputeValue[0]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_REPUTE_LOW_LIMIT1,
		m_DefaultNpcTemplate.nReputeLowLimit[0], (int*)&fNpcTemplate.nReputeLowLimit[0]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_REPUTE_HIGH_LIMIT1,
		m_DefaultNpcTemplate.nReputeHighLimit[0], (int*)&fNpcTemplate.nReputeHighLimit[0]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_REPUTE_ID2,
		m_DefaultNpcTemplate.dwReputeID[1], (int*)&fNpcTemplate.dwReputeID[1]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_REPUTE_VALUE2,
		m_DefaultNpcTemplate.nReputeValue[1], (int*)&fNpcTemplate.nReputeValue[1]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_REPUTE_LOW_LIMIT2,
		m_DefaultNpcTemplate.nReputeLowLimit[1], (int*)&fNpcTemplate.nReputeLowLimit[1]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_REPUTE_HIGH_LIMIT2,
		m_DefaultNpcTemplate.nReputeHighLimit[1], (int*)&fNpcTemplate.nReputeHighLimit[1]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_REPUTE_ID3,
		m_DefaultNpcTemplate.dwReputeID[2], (int*)&fNpcTemplate.dwReputeID[2]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_REPUTE_VALUE3,
		m_DefaultNpcTemplate.nReputeValue[2], (int*)&fNpcTemplate.nReputeValue[2]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_REPUTE_LOW_LIMIT3,
		m_DefaultNpcTemplate.nReputeLowLimit[2], (int*)&fNpcTemplate.nReputeLowLimit[2]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_REPUTE_HIGH_LIMIT3,
		m_DefaultNpcTemplate.nReputeHighLimit[2], (int*)&fNpcTemplate.nReputeHighLimit[2]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_REPUTE_ID4,
		m_DefaultNpcTemplate.dwReputeID[3], (int*)&fNpcTemplate.dwReputeID[3]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_REPUTE_VALUE4,
		m_DefaultNpcTemplate.nReputeValue[3], (int*)&fNpcTemplate.nReputeValue[3]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_REPUTE_LOW_LIMIT4,
		m_DefaultNpcTemplate.nReputeLowLimit[3], (int*)&fNpcTemplate.nReputeLowLimit[3]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_REPUTE_HIGH_LIMIT4,
		m_DefaultNpcTemplate.nReputeHighLimit[3], (int*)&fNpcTemplate.nReputeHighLimit[3]
    );
	(void)bRetCode; /*[endgame] tolerant*/

#endif	//_SERVER 

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_REPRESENT_ID1, 
		m_DefaultNpcTemplate.dwRepresentID[0], (int*)&fNpcTemplate.dwRepresentID[0]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_REPRESENT_ID2, 
		m_DefaultNpcTemplate.dwRepresentID[1], (int*)&fNpcTemplate.dwRepresentID[1]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_REPRESENT_ID3, 
		m_DefaultNpcTemplate.dwRepresentID[2], (int*)&fNpcTemplate.dwRepresentID[2]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_REPRESENT_ID4, 
		m_DefaultNpcTemplate.dwRepresentID[3], (int*)&fNpcTemplate.dwRepresentID[3]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_REPRESENT_ID5, 
		m_DefaultNpcTemplate.dwRepresentID[4], (int*)&fNpcTemplate.dwRepresentID[4]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_REPRESENT_ID6, 
		m_DefaultNpcTemplate.dwRepresentID[5], (int*)&fNpcTemplate.dwRepresentID[5]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_REPRESENT_ID7, 
		m_DefaultNpcTemplate.dwRepresentID[6], (int*)&fNpcTemplate.dwRepresentID[6]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_REPRESENT_ID8, 
		m_DefaultNpcTemplate.dwRepresentID[7], (int*)&fNpcTemplate.dwRepresentID[7]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_REPRESENT_ID9, 
		m_DefaultNpcTemplate.dwRepresentID[8], (int*)&fNpcTemplate.dwRepresentID[8]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_REPRESENT_ID10, 
		m_DefaultNpcTemplate.dwRepresentID[9], (int*)&fNpcTemplate.dwRepresentID[9]
    );
	(void)bRetCode; /*[endgame] tolerant*/

#if defined(_SERVER)
    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_DROP_NOT_QUEST_ITEM_FLAG,
		m_DefaultNpcTemplate.bDropNotQuestItemFlag, (int*)&fNpcTemplate.bDropNotQuestItemFlag
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetString(
        nIndex, NPC_DIALOG1,
		m_DefaultNpcTemplate.szDialog[0], fNpcTemplate.szDialog[0], MAX_NPC_DIALOG_LEN
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_DIALOG_RATE1,
		m_DefaultNpcTemplate.nDialogRate[0], (int*)&fNpcTemplate.nDialogRate[0]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetString(
        nIndex, NPC_DIALOG2,
		m_DefaultNpcTemplate.szDialog[1], fNpcTemplate.szDialog[1], MAX_NPC_DIALOG_LEN
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_DIALOG_RATE2,
		m_DefaultNpcTemplate.nDialogRate[1], (int*)&fNpcTemplate.nDialogRate[1]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetString(
        nIndex, NPC_DIALOG3,
		m_DefaultNpcTemplate.szDialog[2], fNpcTemplate.szDialog[2], MAX_NPC_DIALOG_LEN
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_DIALOG_RATE3,
		m_DefaultNpcTemplate.nDialogRate[2], (int*)&fNpcTemplate.nDialogRate[2]
    );
	(void)bRetCode; /*[endgame] tolerant*/

	fNpcTemplate.nTotalRate = fNpcTemplate.nDialogRate[0] + fNpcTemplate.nDialogRate[1] + fNpcTemplate.nDialogRate[2];
	KGLOG_PROCESS_ERROR(fNpcTemplate.nTotalRate >= 0);
	
	bRetCode = piTabFile->GetString(
        nIndex, NPC_DIALOG_AFTER_QUEST,
		m_DefaultNpcTemplate.szDialogAfterQuest, fNpcTemplate.szDialogAfterQuest, MAX_NPC_DIALOG_LEN
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_DIALOG_QUEST_ID,
		m_DefaultNpcTemplate.dwDialogQuestID, (int*)&fNpcTemplate.dwDialogQuestID
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_HAS_BANK,
		m_DefaultNpcTemplate.bHasBank, (int*)&fNpcTemplate.bHasBank
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_HAS_MAILBOX,
		m_DefaultNpcTemplate.bHasMailBox, (int*)&fNpcTemplate.bHasMailBox
    );
	(void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_HAS_CUB_PACKAGE,
        m_DefaultNpcTemplate.bHasCubPackage, (int*)&fNpcTemplate.bHasCubPackage
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetString(
        nIndex, NPC_CUB_PACKAGE_TEXT, m_DefaultNpcTemplate.szCubPackageOptionText,
        fNpcTemplate.szCubPackageOptionText, MAX_OPTION_TEXT_LEN
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_CUB_PACKAGE_REPUTE_LEVEL, m_DefaultNpcTemplate.nCubPackageRequireReputeLevel,
        &fNpcTemplate.nCubPackageRequireReputeLevel
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_MASTER_ID, 
        m_DefaultNpcTemplate.dwMasterID, (int*)&fNpcTemplate.dwMasterID
    );
    (void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_CRAFT_MASTER_ID,
		m_DefaultNpcTemplate.dwCraftMasterID, (int*)&(fNpcTemplate.dwCraftMasterID)
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetString(
        nIndex, NPC_BANK_TEXT,
		m_DefaultNpcTemplate.szBankOptionText, fNpcTemplate.szBankOptionText, MAX_OPTION_TEXT_LEN
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetString(
        nIndex, NPC_MAILBOX_TEXT,
		m_DefaultNpcTemplate.szMailBoxOptionText, fNpcTemplate.szMailBoxOptionText, MAX_OPTION_TEXT_LEN
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetString(
        nIndex, NPC_MASTER_TEXT,
		m_DefaultNpcTemplate.szMasterOptionText, fNpcTemplate.szMasterOptionText, MAX_OPTION_TEXT_LEN
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetString(
        nIndex, NPC_CRAFT_MASTER_TEXT,
		m_DefaultNpcTemplate.szCraftMasterOptionText, fNpcTemplate.szCraftMasterOptionText, MAX_OPTION_TEXT_LEN
    );
	(void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_SHOP_REPUTE_LEVEL,
        m_DefaultNpcTemplate.nShopRequireReputeLevel, &fNpcTemplate.nShopRequireReputeLevel
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_MASTER_REPUTE_LEVEL,
        m_DefaultNpcTemplate.nMasterRequireReputeLevel, &fNpcTemplate.nMasterRequireReputeLevel
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_CRAFT_MASTER_REPUTE_LEVEL,
        m_DefaultNpcTemplate.nCraftMasterRequireReputeLevel, &fNpcTemplate.nCraftMasterRequireReputeLevel
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_BANK_REPUTE_LEVEL,
        m_DefaultNpcTemplate.nBankRequireReputeLevel, &fNpcTemplate.nBankRequireReputeLevel
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_MAILBOX_REPUTE_LEVEL,
        m_DefaultNpcTemplate.nMailBoxRequireReputeLevel, &fNpcTemplate.nMailBoxRequireReputeLevel
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_QUEST_REPUTE_LEVEL,
        m_DefaultNpcTemplate.nQuestRequireReputeLevel, &fNpcTemplate.nQuestRequireReputeLevel
    );
    (void)bRetCode; /*[endgame] tolerant*/

#endif

	bRetCode = piTabFile->GetString(
        nIndex, NPC_SCRIPT_NAME, 
		m_DefaultNpcTemplate.szScriptName, fNpcTemplate.szScriptName, MAX_SCIRPT_NAME_LEN
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, NPC_DIALOG_ID, 
		m_DefaultNpcTemplate.nNpcDialogID, (int*)&fNpcTemplate.nNpcDialogID
    );
	(void)bRetCode; /*[endgame] tolerant*/
    
    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_DAILY_QUEST_CYCLE, 
		m_DefaultNpcTemplate.nDailyQuestCycle, (int*)&fNpcTemplate.nDailyQuestCycle
    );
	(void)bRetCode; /*[endgame] tolerant*/
    
    fNpcTemplate.nDailyQuestCycle *= 60; // ת����

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_DAILY_QUEST_OFFSET, 
		m_DefaultNpcTemplate.nDailyQuestOffset, (int*)&fNpcTemplate.nDailyQuestOffset
    );
	(void)bRetCode; /*[endgame] tolerant*/
    
    fNpcTemplate.nDailyQuestOffset *= 60; // ת����

#ifdef _SERVER
    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_PROGRESS_ID, m_DefaultNpcTemplate.nProgressID, (int*)&fNpcTemplate.nProgressID
    );
	(void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_HAS_TONG_REPERTORY, m_DefaultNpcTemplate.bHasTongRepertory, (int*)&fNpcTemplate.bHasTongRepertory
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetString(
        nIndex, NPC_TONG_REPERTORY_OPTION_TEXT, m_DefaultNpcTemplate.szTongRepertoryOptionText,
        fNpcTemplate.szTongRepertoryOptionText, MAX_OPTION_TEXT_LEN
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_TONG_REPERTORY_REPUTE_LEVEL, m_DefaultNpcTemplate.nTongRepertoryRequireReputeLevel,
        &fNpcTemplate.nTongRepertoryRequireReputeLevel
    );
    (void)bRetCode; /*[endgame] tolerant*/


    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_HAS_AUCTION, m_DefaultNpcTemplate.bHasAuction, (int*)&fNpcTemplate.bHasAuction
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetString(
        nIndex, NPC_AUCTION_OPTION_TEXT, m_DefaultNpcTemplate.szAuctionOptionText,
        fNpcTemplate.szAuctionOptionText, MAX_OPTION_TEXT_LEN
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_AUCTION_REQUIRE_REPUTE_LEVEL, m_DefaultNpcTemplate.nAuctionRequireReputeLevel,
        &fNpcTemplate.nAuctionRequireReputeLevel
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_CAMP_LOOT_PRESTIGE, m_DefaultNpcTemplate.nCampLootPrestige, &fNpcTemplate.nCampLootPrestige
    );
    (void)bRetCode; /*[endgame] tolerant*/
    
    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_PRESTIGE, m_DefaultNpcTemplate.nPrestige, &fNpcTemplate.nPrestige
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_CONTRIBUTE, m_DefaultNpcTemplate.nContribution, &fNpcTemplate.nContribution
    );
    (void)bRetCode; /*[endgame] tolerant*/

    // v2.5 NEW: currencies dropped on kill (cbtJustice/ExamPrint/ActivityAward). Default 0 = no drop.
    bRetCode = piTabFile->GetInteger(nIndex, NPC_JUSTICE,       0, &fNpcTemplate.nJustice);
    (void)bRetCode; /*[endgame] tolerant*/
    bRetCode = piTabFile->GetInteger(nIndex, NPC_EXAMPRINT,     0, &fNpcTemplate.nExamPrint);
    (void)bRetCode; /*[endgame] tolerant*/
    bRetCode = piTabFile->GetInteger(nIndex, NPC_ACTIVITYAWARD, 0, &fNpcTemplate.nActivityAward);
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_ACHIEVEMENT_ID, m_DefaultNpcTemplate.nAchievementID, &fNpcTemplate.nAchievementID
    );
    (void)bRetCode; /*[endgame] tolerant*/
    
    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_ADD_CAMP_SCORE, m_DefaultNpcTemplate.nAddCampScore, &fNpcTemplate.nAddCampScore
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_HAS_GAME_CARD_SALE, m_DefaultNpcTemplate.bHasGameCardSale, &fNpcTemplate.bHasGameCardSale
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_HAS_GAME_CARD_BUY, m_DefaultNpcTemplate.bHasGameCardBuy, &fNpcTemplate.bHasGameCardBuy
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetString(
        nIndex, NPC_GAME_CARD_SALE_OPTION_TEXT, m_DefaultNpcTemplate.szGameCardSaleOptionText,
        fNpcTemplate.szGameCardSaleOptionText, sizeof(fNpcTemplate.szGameCardSaleOptionText)
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetString(
        nIndex, NPC_GAME_CARD_BUY_OPTION_TEXT, m_DefaultNpcTemplate.szGameCardBuyOptionText,
        fNpcTemplate.szGameCardBuyOptionText, sizeof(fNpcTemplate.szGameCardBuyOptionText)
    );
    (void)bRetCode; /*[endgame] tolerant*/

#endif // _SERVER

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_KNOCKED_BACK_RATE, m_DefaultNpcTemplate.nKnockedBackRate, &fNpcTemplate.nKnockedBackRate
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_KNOCKED_DOWN_RATE, m_DefaultNpcTemplate.nKnockedDownRate, &fNpcTemplate.nKnockedDownRate
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_KNOCKED_OFF_RATE, m_DefaultNpcTemplate.nKnockedOffRate, &fNpcTemplate.nKnockedOffRate
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_REPULSED_RATE, m_DefaultNpcTemplate.nRepulsedRate, &fNpcTemplate.nRepulsedRate
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, NPC_PULL_RATE, m_DefaultNpcTemplate.nPullRate, &fNpcTemplate.nPullRate
    );
    (void)bRetCode; /*[endgame] tolerant*/

	bResult = true;
Exit0:
	return bResult;
}
