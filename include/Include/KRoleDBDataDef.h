/************************************************************************/
/* ������ɫ���ݽṹ����			                                        */
/* Copyright : Kingsoft 2005										    */
/* Author	 : Zhu Jianqiu												*/
/************************************************************************/

#ifndef _KROLE_DB_DATA_DEF_H_
#define _KROLE_DB_DATA_DEF_H_

#include "SO3ProtocolBasic.h"

#define MAX_ROLE_DATA_SIZE          (1024 * 256)

enum ROLE_DATA_BLOCK_TYPE
{
	rbtInvalid = 0,
	rbtSkillList = 1,
	rbtItemList = 2,
	rbtQuestList = 3,
	rbtProfessionList = 4,
    rbtStateInfo = 5,
    rbtBuffList = 6,
	rbtRepute = 7,
	rbtUserPreferences = 8,
	rbtBookState = 9,
    rbtSkillRecipeList = 10,
    rbtRecipeList = 11,
    rbtCoolDownTimer = 12,
    rbtRoadOpenList = 13,
    rbtCustomData = 14,
    rbtVisitedMap = 15,
    rbtPQList = 16,
    rbtHeroData = 17,
    rbtAchievementData = 18,
    rbtDesignationData = 19,
    rbtRandData = 20,
    rbtAntiFarmerData = 21,
    rbtMentorData = 22,
	// Target v2.5 inserted these blocks before the newer source additions.
    rbtPendentData = 23,
    rbtActivityVariables = 24,
    rbtCurrencyData = 25,
    rbtBankPasswordData = 26,
    rbtArenaData = 27,
    rbtDropSurpriseMaskData = 28,
    rbtCampActiveStat = 29,
    rbtExteriorBoxData = 30,
    rbtExteriorSetData = 31,
    rbtExteriorOtherData = 31,
    rbtHairBoxData = 32,
    rbtRegressionData = 33,
    rbtDropSurpriseData = 34,
    rbtSingleDungeonData = 35,
    rbtDelayTradeItemData = 36,
    rbtTimeLimitReturnItemData = 37,
    rbtTimeLimitSoldListInfoData = 38,
    rbtDomesticateData = 39,
    rbtFacePendentData = 40,
    rbtFellowPetData = 41,
    rbtDynamicPackageSize = 42,
    rbtTongExtData = 43,
    rbtTalentSkillData = 44,
    rbtMiniAvatarData = 45,
    rbtVisitTongInviteList = 46,
    rbtRewardsBoxData = 47,
    rbtManualDropData = 48,
	// Source callsites use these historical spellings.
    rbtExteriorData = rbtExteriorBoxData,
	rbtTotal = 49
};

#pragma	pack(1)

// -----��ɫ������Ϣ����------

struct KROLE_POSITION_DB
{
    int     nCenterIndex;
    DWORD	dwMapID;
    int	    nMapCopyIndex;
    int		nX;
    int		nY;
    int		nZ;
    BYTE    byFaceDirection;
};

struct KRoleBaseHeader 
{
    int     nVersion;
};

struct KRoleBaseInfo : KRoleBaseHeader
{
    char                cRoleType;
    KROLE_POSITION_DB   CurrentPos;
    KROLE_POSITION_DB   LastEntry;
    BYTE	            byLevel;
    BYTE                byCamp;
    BYTE                byForceID;
    WORD                wRepresentId[perRepresentCount];
    int                 nLastSaveTime;
    int                 nLastLoginTime;
    int                 nTotalGameTime;
    int                 nCreateTime;
    int                 nDirectApprenticeEndTime;
    BYTE                byCanBeDirectApprentice;
    BYTE                byBeDirectMentor;
    DWORD               dwCorpsSystemID;
    BYTE                bySelectKungfuIndex;
    BYTE                byReserved[11];
};

struct KROLE_LIST_INFO
{
    DWORD	            dwPlayerID;
    char	            szAccount[_NAME_LEN];
    char	            szRoleName[_NAME_LEN];
    KRoleBaseInfo       BaseInfo;
    BYTE                byCanRename;
    time_t              nFreezeTime;
    time_t              nDeleteTime;
};

// -----��ɫ��չ���ݶ���------

struct KRoleDataHeader
{
	DWORD	dwVer;
	DWORD	dwCRC;
	DWORD	dwLen;
};

struct KRoleBlockHeader
{
    int     nType;
    DWORD   dwVer;
    DWORD   dwLen;
};

struct KACTIVITY_VARIABLES
{
    WORD    wPrensentCodeCounters[32][2];
};

typedef char KACTIVITY_VARIABLES_SIZE[(sizeof(KACTIVITY_VARIABLES) == 0x80) ? 1 : -1];

struct KBANK_PASSWORD_DATA
{
    time_t  nResetEndTime;
    char    szPassword[64];
    char    szAnswer[32];
    int     nQuestionID;
    DWORD   dwEffectMask;
    BYTE    byReserved[24];
};

typedef char KBANK_PASSWORD_DATA_SIZE[(sizeof(KBANK_PASSWORD_DATA) == 0x84) ? 1 : -1];

struct KDROP_SURPRISE_DATA
{
    BYTE    byMask[16];
    BYTE    byReserved[16];
};

typedef char KDROP_SURPRISE_DATA_SIZE[(sizeof(KDROP_SURPRISE_DATA) == 0x20) ? 1 : -1];

// ��ɫ��������
struct KROLE_STATE_INFO
{
    BYTE    byMoveState;
    WORD    wCurrentTrack;
    int     nMoveFrameCounter;
    WORD    wFromFlyNode;
    WORD    wTargetCityID;

	int     nExperience;

	int		nCurrentLife;				
	int		nCurrentMana;				
    int     nCurrentStamina;
    int     nCurrentThew;

    int     nAddTrainTimeInToday;
    int     nCurrentTrainValue;
    int     nUsedTrainValue;
    WORD    wReserved;

    BYTE    byPKState;
    WORD    wCloseSlayLeftTime; // ��λ����

    WORD    wLeftReviveFrame;
    time_t  nLastSituReviveTime;
    BYTE    bySituReviveCount;
    DWORD   dwKillerID;

    WORD    wCurrentKillPoint;
    int     nReserved0;

    int     nCurrentPrestige; // ����
    time_t  nBanTime;
    int     nContribution;    // ����ֵ
    int     nMaxLevel;
    BOOL    bHideHat;         // �Ƿ�����ñ��

    BYTE    byTalkWorldDailyCount;
    BYTE    byTalkForceDailyCount;
    BYTE    byTalkCampDailyCount;
    BYTE    byKilledCount;
    time_t  nNextResetKilledCountTime;

    BYTE    byCampFlag;

    BYTE    byReserved[23];
};

// Target v2.5 rbtStateInfo version 2 payload.  The serialized layout is
// target-backed by KROLE_STATE_INFO_V2 (DWARF DIE 0x059e5856), not by the
// legacy source-era KROLE_STATE_INFO declaration above.
struct KROLE_STATE_INFO_V2
{
    BYTE    byMoveState;
    WORD    wCurrentTrack;
    int     nMoveFrameCounter;
    WORD    wFromFlyNode;
    WORD    wTargetCityID;
    int     nExperience;
    int     nCurrentLife;
    int     nCurrentMana;
    int     nAddTrainTimeInToday;
    int     nCurrentTrainValue;
    int     nUsedTrainValue;
    WORD    wReserved;
    WORD    wLeftReviveFrame;
    time_t  nLastSituReviveTime;
    BYTE    bySituReviveCount;
    DWORD   dwKillerID;
    WORD    wCurrentKillPoint;
    time_t  nBanTime;
    int     nMaxLevel;
    BYTE    byHideHat;
    BYTE    byReservedExpired1[3];
    BYTE    byKilledCount;
    time_t  nNextResetKilledCountTime;
    BYTE    byCampFlag;
    int     nLastAddTrainTime;
    BYTE    byBigSwordSelected;
    BYTE    byTitle;
    int     nTitlePoint;
    int     nRankPoint;
    BYTE    byRankPointVersion;
    time_t  nLastGainTitleTime;
    WORD    wReservedExpired3;
    BYTE    byReservedExpired2;
    DWORD   dwKillCount;
    int     nSprintPower;
    BYTE    byApplyExteriorFlag;
    int     nHorseSprintPower;
    WORD    wSendMailDailyCount;
    WORD    wTalkWorldDailyCount;
    WORD    wTalkForceDailyCount;
    WORD    wTalkCampDailyCount;
    WORD    wWhisperDailyCount;
    WORD    wTalkSceneDailyCount;
    WORD    wTalkNearbyDailyCount;
    BYTE    byHunterQualification;
    BYTE    byHuntingFlag;
    BYTE    byPrisonFlag;
    BYTE    byFakeNameState;
    BYTE    byReserved[7];
};

struct KUSER_ROLE_GENERATOR_PARAM
{
	char	szRoleName[_NAME_LEN];
	char    cRoleType;
    DWORD   dwMapID;
    int     nMapCopyIndex;
    WORD    wRepresentId[perRepresentCount];
};

struct KDB_ROLE_GENERATOR_PARAM
{
    char    szAccountName[_NAME_LEN];
    int     nX;
    int     nY;
    int     nZ;
    BYTE    byDirection;
    KUSER_ROLE_GENERATOR_PARAM UserParam;
};

// ��ͨ��浽�ṹ
struct KROLE_ROAD_INFO
{
    int nOpenNodeCount;
    int OpenNodeList[0];
};

struct KHERO_DATA 
{
    BYTE byHeroFlag;
    BYTE byDataLen;
    BYTE byData[0];
};

struct KFORCE_DB_DATA
{
    WORD    wCount;
    struct FORECE_INFO 
    {
        BYTE    byForce;
        BYTE    byFightFlag;
        BYTE    byLevel;
        short   shRepute;
        BYTE    byShow;
    }   ForceInfo[0];
};

struct KMENTOR_DATA_ON_PLAYER
{
    int     nAcquiredMentorValue;
    int     nUsableMentorValue;
    time_t  nLastEvokeMentorTime;
    DWORD   dwTAEquipsScore;
    BYTE    byReserved[16];
    BYTE    byEvokeMentorCount;
    BYTE    byMaxApprenticeCount;
    BYTE    byGraduateMentorCount;
    BYTE    byGraduateApprenticeCount;
    BYTE    GraduateMentorData[0];
};

typedef char KMENTOR_DATA_ON_PLAYER_TARGET_SIZE[(sizeof(KMENTOR_DATA_ON_PLAYER) == 0x24) ? 1 : -1];
typedef char KMENTOR_DATA_ON_PLAYER_USABLE_OFFSET[(offsetof(KMENTOR_DATA_ON_PLAYER, nUsableMentorValue) == 0x04) ? 1 : -1];
typedef char KMENTOR_DATA_ON_PLAYER_SCORE_OFFSET[(offsetof(KMENTOR_DATA_ON_PLAYER, dwTAEquipsScore) == 0x0c) ? 1 : -1];

struct KGRADUATED_MENTOR_DATA_DB
{
    DWORD   dwPlayerID;
    time_t  nMentorTime;
    time_t  nGraduateTime;
};

typedef char KGRADUATED_MENTOR_DATA_DB_SIZE[(sizeof(KGRADUATED_MENTOR_DATA_DB) == 0x0c) ? 1 : -1];

struct KCAMP_ACTIVE_STAT_DB
{
    time_t nRestTime;
    int    nPrestige;
    int    nBaseScore;
    BYTE   byIsActiveBeforeWeek;
    BYTE   byIsActive;
    BYTE   byReserved[2];
};

typedef char KCAMP_ACTIVE_STAT_DB_SIZE[(sizeof(KCAMP_ACTIVE_STAT_DB) == 0x10) ? 1 : -1];

#pragma pack()

#endif	//_KROLE_DB_DATA_DEF_H_
