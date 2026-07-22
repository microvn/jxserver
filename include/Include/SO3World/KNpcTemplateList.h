/************************************************************************/
/* ����װ����ģ���б�					                            */
/* Copyright : Kingsoft 2004										    */
/* Author	 : Zhu Jianqiu												*/
/* History	 :															*/
/*		2005.04.25	Create												*/
/* Comment	 :															*/
/*		��װ�˶�NPCģ�������ļ��Ķ�д��Cache						*/
/************************************************************************/
#ifndef _KNPC_TEMPLATE_LIST_
#define _KNPC_TEMPLATE_LIST_

#include "Global.h"
#include "Luna.h"

#include <map>

#define MAX_NPC_REPRESENT_ID_COUNT 10
#define MAX_NPC_DIALOG_COUNT 3			//Npc���ϵ�����Ի�����
#define MAX_NPC_DIALOG_LEN	1024
#define MAX_OPTION_TEXT_LEN	128
#define MAX_NPC_REPUTATION	4			//ÿ��Npc����Ӱ�����������

class KNpc;

struct KNpcTemplate
{
	DWORD						dwTemplateID;
	char						szName[_NAME_LEN];		    // ��ɫ��
#ifdef _CLIENT
	char						szTitle[TITLE_LEN];			// �ƺ�
	char						szMapName[_NAME_LEN];		// ������ͼ
#endif

	int							nKind;					    // ��CHARACTER_KIND
	DWORD						dwForceID;				    // ��������
    DWORD                       dwGuardForceID;             // �ܸ���������������
    int                         nSpecies;                   // NPC���֣���KNPC_SPECIES����
	int							nLevel;					    // ��ɫ��׼�ȼ�
	int							nAdjustLevel;				// ��ɫ�ȼ�������ģʽ��0��������1���ϣ�2���£�3���¸���
	int							nHeight;					// Npc�߶�
	MOVE_MODE					eMoveMode;					// Npc���ƶ�ģʽ				

    int                         nTouchRange;                //�ܻ���Χ��������Χ������������뾶
    int                         nIntensity;                 //ǿ��
    int                         nReviveTime;                //����ʱ��
    int                         nReviveMinTime;

	BOOL						bSelectable;				// �ܷ��ѡ
	BOOL						bCanSeeLifeBar;				// �ܷ���ʾѪ��
    BOOL                        bCanSeeName;                // �Ƿ���ʾ����

	int							nMaxLife;				    // �������ֵ
	int							nLifeReplenish;			    // ���������ظ��ٶ�
	int							nLifeReplenishPercent;	    // �����ٷֱȻظ��ٶ�

	int							nMaxMana;				    // �������ֵ
	int							nManaReplenish;			
	int							nManaReplenishPercent;	

    int                         nWalkSpeed;
    int                         nRunSpeed;
    int                         nJumpSpeed;

    int                         nMeleeWeaponDamageBase;     
    int                         nMeleeWeaponDamageRand;     
    int                         nRangeWeaponDamageBase;
    int                         nRangeWeaponDamageRand;

	int							nDodge;			            //���ܵ���
    int                         nParry;                     //�мܼ���
    int                         nParryValue;                //�м�ֵ���ͼ����˺����
    int                         nSense;                     //����ֵ
    int                         nHitBase;                   //���л���ֵ

    int                         nPhysicsCriticalStrike;     //�⹦�ػ����� 
    int                         nSolarCriticalStrike;       //�����ڹ��ػ�����
    int                         nNeutralCriticalStrike;     //�����ڹ��ػ�����
    int                         nLunarCriticalStrike;       //�����ڹ��ػ�����
    int                         nPoisonCriticalStrike;      //�����ڹ��ػ�����

    int							nPhysicsShieldBase;         //�⹦������
    int							nSolarMagicDefence;			//�����ڹ�����ϵ��
    int							nNeutralMagicDefence;		//�����ڹ�����ϵ��
    int							nLunarMagicDefence;			//�����ڹ�����ϵ��
    int							nPoisonMagicDefence;		//�����ڹ�����ϵ��

	DWORD                       nAIType;					//AI����
	int							nAIParamTemplateID;			//AI����ģ��

	int							nCorpseDoodadID;			//ʬ��ģ��ID
	int							nDropClassID;				//npc���������ʹ��		

	DWORD						dwRepresentID[MAX_NPC_REPRESENT_ID_COUNT];
	int							nNpcDialogID;				//Npc˵���õ�ID,��Ӧ��SmartDialog.tab

    DWORD                       dwImmunityMask;             // Boss���� ��������

#if defined(_SERVER)
    BOOL                        bDropNotQuestItemFlag;      // ��ʾ���NPC����Ҵ�����NPC,����NPC�Ƿ�Ҫ������������
	char						szDropName[MAX_DROP_PER_NPC][MAX_PATH];//������ļ���
	int							nDropCount[MAX_DROP_PER_NPC];

	DWORD						dwSkillIDList[MAX_NPC_AI_SKILL];		//AI����ID
	DWORD						dwSkillLevelList[MAX_NPC_AI_SKILL];		//AI���ܵȼ�
	int							nSkillCastInterval[MAX_NPC_AI_SKILL];	//AI���ܼ��
	int							nAISkillType[MAX_NPC_AI_SKILL];			//AI��������
	int							nSkillRate[MAX_NPC_AI_SKILL];			//AI�����ͷŵĸ���
    int							nSkillAniFrame[MAX_NPC_AI_SKILL];       //AI���ܶ���֡��
    int							nSkillRestFrame[MAX_NPC_AI_SKILL];      //AI����ֹͣ֡��

	int							nThreatTime;				            //������˼��
	int							nOverThreatPercent;
    int                         nPursuitRange;                          //׷������

	//------  �����Ǯ  ------------------------------------------------->
    int                         nExp;
	int							nMinMoney;
	int							nMaxMoney;
	int							nMoneyRate;
	DWORD						dwReputeID[MAX_NPC_REPUTATION];
	int							nReputeValue[MAX_NPC_REPUTATION];
	int							nReputeLowLimit[MAX_NPC_REPUTATION];
	int							nReputeHighLimit[MAX_NPC_REPUTATION];



	//<-------------------------------------------------------------------

	//------  OpenWindow ------------------------------------------------->
	char						szDialog[MAX_NPC_DIALOG_COUNT][MAX_NPC_DIALOG_LEN];
	int							nDialogRate[MAX_NPC_DIALOG_COUNT];
	int							nTotalRate;
	char						szDialogAfterQuest[MAX_NPC_DIALOG_LEN];
	DWORD						dwDialogQuestID;

    int 						nShopTemplateID;
	DWORD                       dwMasterID;
	DWORD						dwCraftMasterID;
	BOOL						bHasBank;
	BOOL						bHasMailBox;
    BOOL                        bHasAuction;
    BOOL                        bHasTongRepertory;
    BOOL                        bHasGameCardSale;
    BOOL                        bHasGameCardBuy;

	char						szShopOptionText[MAX_OPTION_TEXT_LEN];
	char						szMasterOptionText[MAX_OPTION_TEXT_LEN];
	char						szCraftMasterOptionText[MAX_OPTION_TEXT_LEN];
	char						szBankOptionText[MAX_OPTION_TEXT_LEN];
	char						szMailBoxOptionText[MAX_OPTION_TEXT_LEN];
    char                        szAuctionOptionText[MAX_OPTION_TEXT_LEN];
    char                        szTongRepertoryOptionText[MAX_OPTION_TEXT_LEN];
    char                        szGameCardSaleOptionText[MAX_OPTION_TEXT_LEN];
    char                        szGameCardBuyOptionText[MAX_OPTION_TEXT_LEN];

    // OpenWindows ʱ����ģ��������ȼ�������
    int                         nShopRequireReputeLevel;
    int                         nMasterRequireReputeLevel;
    int                         nCraftMasterRequireReputeLevel;
    int                         nBankRequireReputeLevel;
    int                         nMailBoxRequireReputeLevel;
    int                         nQuestRequireReputeLevel;
    int                         nAuctionRequireReputeLevel;
    int                         nTongRepertoryRequireReputeLevel;
    int                         nGameCardTradeRequireReputeLevel;

	//<---------------------------------------------------------------------
#endif	//_SERVER
    int                         nDailyQuestCycle;
    int                         nDailyQuestOffset;
    BOOL                        bIsRandomDailyQuest;

	char						szScriptName[MAX_SCIRPT_NAME_LEN];
    
#ifdef _SERVER
    int                         nProgressID;
    int                         nCampLootPrestige; // ��Ӫ����, 0,1,2λ�ֱ��ʾ��Ӧ��Ӫ�ܷ������
    int                         nPrestige;
    int                         nContribution;
    int                         nJustice;           // v2.5: cbtJustice dropped on kill
    int                         nExamPrint;         // v2.5: cbtExamPrint dropped on kill
    int                         nActivityAward;     // v2.5: cbtActivityAward dropped on kill
    int                         nAchievementID;     // ��ɱ����ĳɾ�ID
    int                         nAddCampScore;      // ��ɱ�����ӵ�ʿ��ֵ,�����Ӻ�����,�����Ӷ��˹�
#endif

    int                         nKnockedBackRate; // �����ƶ����ʣ����ɵ�
    int                         nKnockedDownRate;
    int                         nKnockedOffRate;
    int                         nRepulsedRate;
    int                         nPullRate;

	DECLARE_LUA_CLASS(KNpcTemplate);

	DECLARE_LUA_STRUCT_DWORD(TemplateID, dwTemplateID);
	DECLARE_LUA_STRUCT_STRING(Name, _NAME_LEN, szName);
	DECLARE_LUA_STRUCT_INTEGER(Kind, nKind);		
	DECLARE_LUA_STRUCT_DWORD(ForceID, dwForceID);
	DECLARE_LUA_STRUCT_INTEGER(Species, nSpecies);
	DECLARE_LUA_STRUCT_INTEGER(Level, nLevel);	
	DECLARE_LUA_STRUCT_INTEGER(Height, nHeight);
    DECLARE_LUA_STRUCT_INTEGER(Intensity, nIntensity);
};

class KNpcTemplateList
{
public:
	KNpcTemplateList(void);
	~KNpcTemplateList(void);

	BOOL Init(void);
	BOOL UnInit(void);

	BOOL LoadFromTemplate(DWORD dwNpcTemplateID, KNpc* pNpc);

	KNpcTemplate* GetTemplate(DWORD dwNpcTemplateID);

	KNpcTemplate* GetTemplateByIndex(int nIndex);

	int GetNpcTemplateCount();

private:
	typedef std::map<DWORD, KNpcTemplate*> KNPC_TEMPLATE_LIST;
	KNPC_TEMPLATE_LIST	m_mapNpcTemplateList;
	KNpcTemplate		m_DefaultNpcTemplate;
	KNpcTemplate*		m_pNpcTemplateList;
	int					m_nNpcTemplateCount;

	BOOL LoadNpcTemplate(int nIndex, ITabFile* piTabFile, KNpcTemplate& fNpcTemplate);
};

// Npc��������ľ�������ǿ�Ⱥ;���������


#endif	//_KNPC_TEMPLATE_LIST_
