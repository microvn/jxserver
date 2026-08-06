/************************************************************************/
/* ��Ϸ���������ļ��б�					                                */
/* Copyright : Kingsoft 2004										    */
/* Author	 : Zhu Jianqiu												*/
/* History	 :															*/
/*		2005.04.25	Create												*/
/************************************************************************/
#ifndef _KSETTINGS_H_
#define _KSETTINGS_H_

#include "KMapListFile.h"
#include "KNpcTemplateList.h"
#include "KDoodadTemplateList.h"
#include "KQuestInfoList.h"
#include "KNpcClassList.h"
#include "KDoodadClassList.h"
#include "KPatrolPathList.h"
#include "KLevelUpList.h"
#include "KCoolDownList.h"
#include "KWeaponTypeList.h"
#include "KRelationList.h"
#include "KFightFlagList.h"
#include "KNpcOrderList.h"
#include "KNpcTeamList.h"
#include "KCharacterActionList.h"
#include "KSmartDialogList.h"
#include "KReputationLimit.h"
#include "KReputeLootBuffList.h"
#include "KNpcAdronTab.h"
#include "KGWConstList.h"
#include "KGWServerConstList.h"
#include "KAntiFarmerSettings.h"
#include "KPendentOldDataInfoList.h"
#include "KTongConstList.h"
#include "KGMList.h"
#include "KAchievementInfoList.h"
#include "KDesignationList.h"
#ifdef _CLIENT
#include "../../Source/Common/SO3World/Src/KGameCardInfoList.h"
#else
#include "KGameCardInfoList.h"
#endif

#define MAX_DROP_LEVEL			16
#define MAX_DROP_CLASS			8
#define MAX_SPECIAL_DROP_LIST	2048

#define KMAIL_COST              30

class KWorldSettings
{
public:
	KWorldSettings(void);
	~KWorldSettings(void);

	BOOL Init(void);
	BOOL UnInit(void);
	BOOL Init_ForEditor(void);
	BOOL UnInit_ForEditor(void);

private:
	BOOL LoadVersionConfig(void);

public:

	KGWConstList		m_ConstList;

	//��ͼ��ز���
	KMapListFile		m_MapListFile;

	//�����趨��
	KQuestInfoList		m_QuestInfoList;

	//Npcģ���
	KNpcTemplateList	m_NpcTemplateList;
	//Doodadģ���
	KDoodadTemplateList m_DoodadTemplateList;

	//Npc�����
	KNpcClassList		m_NpcClassList;

	//Doodad�����
	KDoodadClassList	m_DoodadClassList;

	//Ѳ��·����
	KPatrolPathList		m_PatrolPathList;

	//����������ݱ�
	KLevelUpList		m_LevelUpList;

    KCoolDownList       m_CoolDownList;

    KWeaponTypeList     m_WeaponTypeList;

	//��Ӫ��ϵ��
	KRelationList		m_RelationCampList;

	//������ϵ��
	KRelationList		m_RelationForceList;

	//Ĭ������ս������������
	KFightFlagList		m_ForceDefaultList;

	//�����ȼ������
	KReputationLimit	m_ReputeLimit;
	KReputeLootBuffList m_ReputeLootBufferList;

	//����ϵͳ
	KCharacterActionList m_CharacterActionList;

	//Npc˵��
	KSmartDialogList	m_SmartDialogList;
	KNpcAdronTab       m_NpcAdronTab;

#ifdef _SERVER
    KGMList             m_GMList;
#endif
	KGWServerConstList  m_ServerConstList;

	KNpcOrderManager    m_OrderManager;
	KNpcTeamList		m_NpcTeamList;

    KAchievementInfoList m_AchievementInfoList;

    KDesignationList    m_DesignationList;

    KGameCardInfoList   m_GameCardInfoList;
    KAntiFarmerSettings m_AntiFarmerSettings;
    KPendentOldDataInfoList m_OldPendentDataInfoList;
    KTongConstList      m_TongConstList;
    char                m_szVersionLineName[32];
    char                m_szVersionEx[32];

};

#endif	//_KSETTINGS_H_
