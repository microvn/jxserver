#ifndef _KG_GLOBAL_DEF_H_
#define _KG_GLOBAL_DEF_H_

#ifdef __GNUC__
#include <stdint.h>
#endif

// disable: warning C4200: nonstandard extension used : zero-sized array in struct/union
#pragma warning(disable:4200)

#define ID_PREFIX_BIT_NUM	    2
#define MAX_GS_COUNT		    24
#define CLIENT_ITEM_ID_PREFIX	0x01
#define NPC_ID_PREFIX	        0x01

// ����INT64
#define MAKE_INT64(__A__, __B__)	(((uint64_t)(__A__) << 32) + (uint64_t)__B__)

#define ERROR_ID		0

// ��̬�ϰ��������Чֵ
#define INVALID_OBSTACLE_GROUP  (-1)
// ��Ч�Ķ�̬�ϰ�����������ȡֵ��Χ: [0, 4096)
#define MAX_OBSTACLE_GROUP      4096

// ��������Ϸ����Ŀռ仮�ִ�С����
#define MAX_REGION_WIDTH_BIT_NUM	6	//��ͼ��X����������Region����
#define MAX_REGION_HEIGHT_BIT_NUM	6	//��ͼ��Y����������Region����

#define REGION_GRID_WIDTH_BIT_NUM	6	//Region��X����������Cell����
#define REGION_GRID_HEIGHT_BIT_NUM	6	//Region��Y����������Cell����

#define CELL_LENGTH_BIT_NUM			5	//�����е����ص㾫��
#define ALTITUDE_BIT_NUM			6

#define MAX_Z_ALTITUDE_BIT_NUM		16	//Z������ֵλ��(ALTITUDE)
#define MAX_Z_POINT_BIT_NUM         (MAX_Z_ALTITUDE_BIT_NUM + ALTITUDE_BIT_NUM)

#define MOVE_DEST_RANGE_BIT_NUM		12
#define MOVE_DEST_RANGE				(1 << MOVE_DEST_RANGE_BIT_NUM)

#define	MAX_VELOCITY_XY_BIT_NUM		(CELL_LENGTH_BIT_NUM + 2)
#define MAX_VELOCITY_Z_BIT_NUM		(ALTITUDE_BIT_NUM + 6)
// ����ˮƽ�ٶ���������ʱ�����������,ע��,���ֵ��ò�ҪС����������ʱ�ķ�ĸ��ֵ
#define VELOCITY_ZOOM_BIT_NUM      4
#define VELOCITY_ZOOM_COEFFICIENT (1 << VELOCITY_ZOOM_BIT_NUM)

#define MAX_ZOOM_VELOCITY_BIT_NUM   (MAX_VELOCITY_XY_BIT_NUM + VELOCITY_ZOOM_BIT_NUM)
#define MAX_ZOOM_VELOCITY           ((1 << MAX_ZOOM_VELOCITY_BIT_NUM) - 1)

#define MAX_VELOCITY_XY				((1 << MAX_VELOCITY_XY_BIT_NUM) - 1)
// ע��,Z���ٶ����з���,����Ϊ��ֵ��,Ŀǰȡֵ����Ϊ[-512, 511], 10 bits
#define MAX_VELOCITY_Z				((1 << (MAX_VELOCITY_Z_BIT_NUM - 1)) - 1)
#define MIN_VELOCITY_Z				(-(1 << (MAX_VELOCITY_Z_BIT_NUM - 1)))

#define MAX_X_COORDINATE_BIT        (MAX_REGION_WIDTH_BIT_NUM  + REGION_GRID_WIDTH_BIT_NUM  + CELL_LENGTH_BIT_NUM)
#define MAX_Y_COORDINATE_BIT        (MAX_REGION_HEIGHT_BIT_NUM + REGION_GRID_HEIGHT_BIT_NUM + CELL_LENGTH_BIT_NUM)
#define MAX_X_COORDINATE            ((1 << MAX_X_COORDINATE_BIT) - 1)
#define MAX_Y_COORDINATE            ((1 << MAX_Y_COORDINATE_BIT) - 1)

//������ض���
#define MAX_REGION_WIDTH			(1 << MAX_REGION_WIDTH_BIT_NUM)
#define MAX_REGION_HEIGHT			(1 << MAX_REGION_HEIGHT_BIT_NUM)
#define MAX_Z_ALTITUDE				((1 << MAX_Z_ALTITUDE_BIT_NUM) - 1)
#define MAX_Z_POINT				    ((1 << MAX_Z_POINT_BIT_NUM) - 1)

#define REGION_GRID_WIDTH			(1 << REGION_GRID_WIDTH_BIT_NUM)
#define REGION_GRID_HEIGHT			(1 << REGION_GRID_HEIGHT_BIT_NUM)

#define CELL_LENGTH					(1 << CELL_LENGTH_BIT_NUM)
#define POINT_PER_ALTITUDE			(1 << ALTITUDE_BIT_NUM)
#define ALTITUDE_2_CELL				(POINT_PER_ALTITUDE * LOGICAL_CELL_CM_LENGTH/ ALTITUDE_UNIT / CELL_LENGTH)

#define MAX_GRAVITY_BIT_NUM          5
#define MAX_GRAVITY                 ((1 << MAX_GRAVITY_BIT_NUM) - 1)

#define LOGICAL_CELL_CM_LENGTH		50
#define _3D_CELL_CM_LENGTH			100

// �ر��߶ȼ�����λ(����)
#define ALTITUDE_UNIT				((LOGICAL_CELL_CM_LENGTH) / 4.0f)
#define TERRAIN_MIN_HEIGHT			(-65536.0f / 4.0f * ALTITUDE_UNIT)
#define TERRAIN_MAX_HEIGHT			(65536.0f / 4.0f * ALTITUDE_UNIT)

#define ALTITUDE_TO_XYPOINT(Altitude)   ((Altitude) * 8)
#define ZPOINT_TO_XYPOINT(Z)            ((Z) / 8)
#define XYPOINT_TO_ZPOINT(XY)           ((XY) * 8)
#define ZPOINT_TO_ALTITUDE(Z)           ((Z) / POINT_PER_ALTITUDE)

#define AI_PARAM_COUNT		8
#define DIALOG_ACTION_ID	1					//�Ի��ı���ID

#define	MAX_TARGET_RANGE		(CELL_LENGTH * REGION_GRID_WIDTH * 2)

// �����ϰ��߶Ȳ�
#define CLIMB_ABILITY 	    (XYPOINT_TO_ZPOINT(CELL_LENGTH))

// �����ƶ�ʱ��λǰ��Ŀ���ľ���,30��
#define MOVE_TARGET_RANGE   (30 * 100 * 32 / 50)

#define MAX_MOVE_STATE_BIT_NUM  5
#define MAX_MOVE_STATE_VALUE    ((1 << MAX_MOVE_STATE_BIT_NUM) - 1)

#define MAX_JUMP_COUNT_BIT 2
#define MAX_JUMP_COUNT ((1 << MAX_JUMP_COUNT_BIT) - 1)

// ���MapID,MapIDȡֵ��Χ(0, MAX_MAP_ID]
#define MAX_MAP_ID UCHAR_MAX
#define MAX_MAP_ID_DATA_SIZE (((MAX_MAP_ID) - 1) / (CHAR_BIT) + 1)

#define BANISH_PLAYER_WAIT_SECONDS      30
#define DELETE_SCENE_WAIT_SECONDS       (BANISH_PLAYER_WAIT_SECONDS + 5) 

// ���װ������
// С��: ��������Դ(������ǩ)���������������ö����ֵ,�޸Ŀ����������,�������Աȷ��.
enum PLAYER_EQUIP_REPRESENT
{
    perFaceStyle,          // ����              
    
    perHairStyle,          // ����              
    
    perHelmStyle,          // ͷ������          
    perHelmColor,          // ͷ����ɫ          
    perHelmEnchant,        // ͷ����ħЧ��      

    perChestStyle,         // ��װ��ʽ          
    perChestColor,         // ��װ��ɫ          
    perChestEnchant,       // ��װ��ħЧ��      

    perWaistStyle,         // ��������          
    perWaistColor,         // ������ɫ          
    perWaistEnchant,       // ������ħ          

    perBangleStyle,        // ��������          
    perBangleColor,        // ������ɫ          
    perBangleEnchant,      // ����ħ          

    perBootsStyle,         // Ь������          
    perBootsColor,         // Ь����ɫ          

    perWeaponStyle,        // ��������          
    perWeaponColor,
    perWeaponEnchant1,     // ������ħ1         
    perWeaponEnchant2,     // ������ħ2         

    perBigSwordStyle,
    perBigSwordColor,
    perBigSwordEnchant1,
    perBigSwordEnchant2,

    perBackExtend,         // ������չ          
    perWaistExtend,        // ������չ          

    perHorseStyle,         // ��
    perHorseAdornment1,    // ����װ��1
    perHorseAdornment2,    // ����װ��2
    perHorseAdornment3,    // ����װ��3
    perHorseAdornment4,    // ����װ��4
    perFaceExtend,
    perReserved,           // ����  
    perRepresentCount
};

// AI�������ֻ����������
// ���Ҫ�м�����ɾ��,С����Щ����ָ���������
enum AI_TYPE
{
	aitInvalid = 0,

	aitBase,
	aitPlayer,
	aitWood,

	aitTotal
};

// AI����Ϊ״̬
enum KAI_STATE
{
	aisInvalid = 0,

	aisIdle,        // ����,ͨ��ָ����ĳĿ���Ŀ���
	aisWander,		// �й�
	aisPatrol,      // Ѳ�ߵ�ĳ����Ĺ�����
	aisFollow,      // ����
	aisAlert,       // ����Ŀ��,��������aisPatrol�з���һ��Ŀ��,Ȼ������,Ҳ������Idle�Ƿ���Ŀ��,Ȼ������
	aisPursuit,     // ׷��
	aisKeepAway,    // ����
	aisEscape,      // ����
	aisReturn,      // Attack����Ժ󷵻�

	//�����µ�״̬
	aisWait			// NpcС��AI�еȴ����ѵ�״̬ 
};

enum AI_SKILL_TYPE
{
	aistInvalid = 0,

	aistPeriodTarget,
	aistPeriodSelf,
	aistPeriodFriend,
	aistPassiveRespond,
	aistChannelingBreak,
	aistTargetCount,
	aistSelfHP,
    aistPeriodTargetRange,

    aistMainThreatNThreat,
    aistBaseThreatNThreat,
    aistDamageThreatNThreat,
    aistTherapyThreatNThreat,
    aistSpecialThreatNThreat,

    aistBaseRandomTarget,
    aistBaseRandomNotSelectTarget,
    aistInRangeRandomTarget,
    aistInRangeRandomNotSelectTarget,
    aistOutRangeRandomTarget,
    aistOutRangeRandomNotSelectTarget,
    aistNearestTarget,
    aistNearestNotSelectTarget,

    aistPeriodTargetNotClear,
    aistTargetCountNotClear,
    aistPeriodTargetRangeNotClear,

	aistTotal
};

//��ɫ����
enum ROLE_TYPE
{
	rtInvalid = 0,

	rtStandardMale,     // ��׼��
	rtStandardFemale,   // ��׼Ů
    rtStrongMale,       // ������
    rtSexyFemale,       // �Ը�Ů
    rtLittleBoy,        // С�к�
    rtLittleGirl,       // С��Ů

	rtTotal
};

enum KBUFF_REPRESENT_POS
{
    brpTrample = 0,     // ���²��ŵ�
    brpBody,            // ���ϵ�
    brpTopHead,         // ����ͷ�ϵ�

    brpTotal
};


//��ɫ��������
enum CHARACTER_ACTION_TYPE
{
	catInvalid = 0,

	catDialogue,		//�Ի�
	catHowl,			//����

	catTotal
};

//������ض���
enum FORCE_RELATION_TYPE
{
	frtInvalid = 0,

	frtExalted,
	frtRevered,
	frtHonored,
	frtFriendly,

	frtNeutral,

	frtUnfriendly,
	frtHostile,
	frtHated,

	frtTotal
};

enum KCAMP
{
    cNeutral, // ������Ӫ
    cGood,    // ������Ӫ
    cEvil,    // а����Ӫ

    cTotal
};

enum SCENE_OBJ_RELATION_TYPE
{
	sortInvalid		= 0,

	sortNone		= 1,
	sortSelf		= 2,
	sortAlly		= 4,
	sortEnemy		= 8,
	sortNeutrality	= 16,
	sortParty		= 32,

	sortAll	= sortNone | sortAlly | sortEnemy | sortSelf | sortNeutrality | sortParty,

	sortTotal,
};

struct KForceRelation
{
	int						nRelation;
	FORCE_RELATION_TYPE		eRelationType;
};

// ���ܵ����ý������
enum KSKILL_RESULT_TYPE
{
    serPhysicsDamage = 0,
    serSolarMagicDamage,
    serNeutralMagicDamage,
    serLunarMagicDamage,
    serPoisonDamage,

    serReflectiedDamage,    

    serTherapy,             // �ܵ�����
    serStealLife,           // ͵ȡĿ�������  
    serAbsorbDamage,      
    serShieldDamage,
    serParryDamage,
    serInsightDamage,

    serTotal
};


// �������
#define MAX_ITEM_BASE_COUNT		6
#define	MAX_ITEM_REQUIRE_COUNT	6
#define	MAX_ITEM_MAGIC_COUNT	12


#define SETTING_DIR			"settings"

#define MAP_LIST_FILE_NAME	"MapList.tab"

// �ű��ص�������
#define SCRIPT_ON_LOAD		"OnLoad"

// ��ͼ�ϰ��ͳ�������
#define MAP_DIR				"maps"			//Ŀ¼
#define MAP_FILE_EXT		"map"			//��׺

// �ַ������ȶ���
#define TITLE_LEN			32
#define TASK_NAME_LEN		256
#define FUNC_NAME_LEN		32
#define MAX_CHAT_TEXT_LEN	768

//��չ��
#define MAX_EXT_POINT_COUNT   8

//������ض���
#define MAX_FORCE_COUNT			128
#define FORCE_NAME_LEN			32



//��ͼ��ǵ�ע�ͳ���
#define MAX_MAP_MARK_COMMENT_LEN 32
#define MAX_MIDMAP_MARK_COMMENT_LEN 32

enum KMAP_TYPE
{
    emtInvalid = -1,
    
    emtNormalMap,       // ��������
    emtDungeon,         // ���񸱱�
    emtBattleField,     // ս��
    emtBirthMap,        // ���ִ�

    emtTongDungeon,

    emtTotal
};

typedef char _CHK_enum_KMAP_TYPE_emtTotal[(emtTotal == 5) ? 1 : -1];

enum KMAP_CAMP_TYPE
{
    emctInvalid = 0,

    emctAllProtect,  // ȫ������ͼ
    emctProtectGood, // ����������
    emctProtectEvil, // �������˹�
    emctNeutral,     // ������ͼ
    emctFight,       // �����ͼ
    emctTotal
};

#define CLOSE_CAMP_FLAG_TIME 5 * 60 // �ر���Ӫ���ص�ʱ�䣬��λ:��

// ����������ݰ��Ĵ�С
#define MAX_EXTERNAL_PACKAGE_SIZE	    (1024 * 32)

#ifdef _PERFORMANCE_OPTIMIZATION
    extern DWORD g_dwGameFps;
    #define GAME_FPS					g_dwGameFps			//��Ϸ֡��
#else
    #define GAME_FPS                    16
#endif


// �����ض���
// PARTY: ��ӵĶ���
#define INVALID_TEAM_GROUP  (-1)
// ������Ϸ�������������Ķ�����
#define MAX_PARTY_COUNT     4096
// һ֧������������������Ŀ
#define PARTY_MARK_COUNT        16

#define MAX_PLAYER_PER_GROUP    5
#define MIN_TEAM_GROUP_NUM      1
#define MAX_TEAM_GROUP_NUM      5

#define MAX_PARTY_SIZE      (MAX_PLAYER_PER_GROUP * MAX_TEAM_GROUP_NUM)

#define INVALID_SKILL_ID    0

enum KTEAM_AUTHORITY_TYPE
{
    tatInvalid,

    tatLeader,
    tatDistribute,
    tatMark,

    tatTotal
};

enum ITEM_LOOT_MODE
{
    ilmInvalid = 0,

	ilmFreeForAll,		// ����������ʰȡ
	ilmDistribute,		// ����Ʒ���ߵ���Ʒ�����߷��䣬������Ʒ����
	ilmGroupLoot,		// ����Ʒ���ߵ���Ʒroll����������������Ʒ����

	ilmTotal           
};

enum INVENTORY_TYPE
{
	ivtEquipment = 0,
	ivtPackage,
	ivtBank,
	ivtSlot,
	ivtSoldlist,
    ivtSystemPackage,
    ivtBulletPackage,
    ivtTimeLimitSoldlist,
    ivtCubPackage,
    ivtTotal,
};

typedef char _CHK_enum_INVENTORY_TYPE_ivtTotal[(ivtTotal == 9) ? 1 : -1];

enum EQUIPMENT_INVENTORY_TYPE
{
	eitMeleeWeapon = 0,		// ��ս����
	eitRangeWeapon,			// Զ������
	eitChest,				// ����
	eitHelm,				// ͷ��
	eitAmulet,				// ����
	eitLeftRing,			// ���ֽ�ָ
	eitRightRing,			// ���ֽ�ָ
	eitWaist,				// ����
	eitPendant,				// ��׺
	eitPants,				// ����
	eitBoots,				// Ь��
	eitBangle,				// ����
	eitWaistExtend,			// ������չ��װ�����ƺ�֮���
	eitPackage1,            // ��չ����1
	eitPackage2,			// ��չ����2
	eitPackage3,			// ��չ����3
	eitPackage4,			// ��չ����4
	eitBankPackage1,
	eitBankPackage2,
	eitBankPackage3,
	eitBankPackage4,
	eitBankPackage5,
	eitArrow,				// ����
    eitBackExtend,
    eitHorse,               // ���� 
	eitTotal
};

// ----------------- TALK ---------------------------->
#define CLIENT_TALK_LINK_ITEM_BUFFER_SIZE 1024

#define TALK_ERROR_S_OK                     0
#define TALK_ERROR_E_ERROR                  1
#define TALK_ERROR_E_OUT_OF_BUFFER          2
#define TALK_ERROR_E_NOT_IN_PARTY           3
#define TALK_ERROR_E_NOT_IN_SENCE           4
#define TALK_ERROR_E_PLAYER_NOT_FOUND       5
#define TALK_ERROR_E_PLAYER_OFFLINE         6
#define TALK_ERROR_E_YOU_BLACKLIST_TARGET   7
#define TALK_ERROR_E_TARGET_BLACKLIST_YOU   8
#define TALK_ERROR_E_BAN                    9
#define TALK_ERROR_E_CD                     10
#define TALK_ERROR_E_NOT_IN_TONG            11
#define TALK_ERROR_E_TONG_CAN_NOT_SPEAK     12
#define TALK_ERROR_E_DAILY_LIMIT            13
#define TALK_ERROR_E_NOT_IN_FORCE           14
#define TALK_ERROR_E_CHARGE_LIMIT           15

enum TALK_DATA_BLOCK_TYPE
{
    tdbInvalid = 0,
    tdbText,
    tdbLinkName,
    tdbLinkItem,
	tdbLinkItemInfo,
	tdbLinkQuest,
	tdbLinkRecipe,
    tdbLinkEnchant,
	tdbLinkSkill,
	tdbLinkSkillRecipe,
	tdbEmotion,
    tdbLinkBook,
	tdbLinkAchievement,
	tdbLinkDesignation,
    tdbEventLink,

    tdbTotal
};

// ��������
enum CHARACTER_TALK_TYPE
{
    trInvalid = 0,

    // Player to Player chat.
    trNearby,           // ����
    trTeam,             // С������
    trRaid,             // �Ŷ�����
    trBattleField,      // ս���Ŷ�
    trSence,            // ͬ�������죨���ģ�
    trWhisper,          // ����
    trFace,             // ����Ƶ��

    // System message
    trLocalSys,         // ����ϵͳ��Ϣ
    trGlobalSys,        // ȫ��ϵͳ��Ϣ
    trGmMessage,        // GM 

    // NPC to Player chat.
    trNpcNearby,
    trNpcTeam,
    trNpcSence,
    trNpcWhisper,
    trNpcSayTo,         // NPC ��ָ�������˵�����������������ģ�������������ݳ����ڿͻ��˵�NPC����Ƶ����
    trNpcYellTo,        // NPC ��ָ������Ҵ󺰣��������������ģ�������������ݳ����ڿͻ��˵�NPC��ͼƵ����
    trNpcFace,
    trNpcSayToTargetUseSentenceID,
    trNpcSayToCamp,

    // Ognization
    trTong,
    trWorld,
    trForce,
    trCamp,
    trMentor,
    trFriends,

    // Player custom channel.
    trCustomCh1 = 100,  // ����Զ���Ƶ��Ԥ��
    trCustomCh2,
    trCustomCh3,
    trCustomCh4,
    trCustomCh5,
    trCustomCh6,
    trCustomCh7,
    trCustomCh8,

    // Debug message
    trDebugThreat,

    trTotal
};

//������غ궨��
#define KG_FELLOWSHIP_MAX_ATTRACTION        1500
#define KG_FELLOWSHIP_MIN_ATTRACTION       -1500
#define KG_FELLOWSHIP_MAX_RECORD            80
#define KG_FELLOWSHIP_MAX_FOE_RECORD        30
#define KG_FELLOWSHIP_MAX_BLACKLIST_RECORD  20
#define KG_FELLOWSHIP_MAX_CUSTEM_GROUP      8

// ��ɫ״̬����״̬
enum CHARACTER_MOVE_STATE
{
	cmsInvalid = 0,				// ��Ч״̬

	// �ܿ�״̬
	cmsOnStand,					// վ��״̬
	cmsOnWalk,					// ��·״̬
	cmsOnRun,					// �ܲ�״̬
    cmsOnJump,                  // ��Ծ״̬
    cmsOnSwimJump,              // ˮ����Ծ
    cmsOnSwim,                  // ˮ����Ӿ
    cmsOnFloat,                 // ˮ������
	cmsOnSit,					// ��״̬

	// ���ܿ�״̬
	cmsOnKnockedDown,			// ������״̬
	cmsOnKnockedBack,			// ������״̬
	cmsOnKnockedOff,			// ������״̬ 
    cmsOnHalt,                  // ����(ѣ�Ρ�ЦѨ�ȵ�)
    cmsOnFreeze,                // ���ܶ�(�̶�����)
    cmsOnEntrap,                // �����ƶ�
    cmsOnAutoFly,               // �Զ��ƶ�,����������

	cmsOnDeath,					// ����״̬
	cmsOnDash,					// ���״̬
    cmsOnPull,                  // ��ץ״̬��������ץס�Լ������Է�����
    cmsOnRepulsed,              // ����״̬��һ������ٵ�״̬

    cmsOnRise,                  // ����״̬
    cmsOnSkid,                  // ͣ��״̬

    cmsOnSprintBreak,
    cmsOnSprintDash,
    cmsOnSprintKick,
    cmsOnSprintFlash,

	cmsTotal
};

typedef char _CHK_enum_CHARACTER_MOVE_STATE_cmsTotal[(cmsTotal == 26) ? 1 : -1];

enum KPQ_STATISTICS_INDEX 
{
    psiKillCount        = 0,
    psiDecapitateCount,
    psiSoloCount,
    psiHarmOutput,
    psiTreatOutput,
    psiInjury,
    psiDeathCount,
    psiThreatOutput,
    psiSkillMark,
    psiSpecialOp1,
    psiSpecialOp2,
    psiSpecialOp3,
    psiSpecialOp4,
    psiFinalMark,
    psiAwardMoney,
    psiAwardExp,
    psiAward1,
    psiAward2,
    psiAward3,

    psiTotal
};

#define ROLE_NAME_LEN       32

#pragma	pack(1)
// ͷ���������
struct KPORTRAIT_INFO
{
    DWORD   dwFaceStyle : 5;
    DWORD   dwHairStyle : 8;
    DWORD   dwHelmStyle : 8;
    DWORD   dwHelmColor : 3;
    
    DWORD   dwReserved  : 8;    // ����
    // <-- 32 bits --

    BOOL operator != (const KPORTRAIT_INFO& crOther)
    {
        return memcmp(this, &crOther, sizeof(KPORTRAIT_INFO)) != 0;
    }
};

#pragma	pack()
// <---------------------------------------------------

#ifdef WIN32
#define STR_CASE_CMP(szA, szB) _mbsicmp((unsigned char*)(szA), (unsigned char*)(szB))
#else   // linux
#define STR_CASE_CMP(szA, szB) strcasecmp(szA, szB)
#endif


// ս�����
#define MAX_BATTLE_SIDE             4
#define INVALID_BATTLE_SIDE         (-1)

#define INIT_CAMP_LEVEL 5 
#define CAMP_LEVEL_COUNT ((INIT_CAMP_LEVEL) * 2 + 1)

#define MAKE_IN_RANGE(Value, MinValue, MaxValue)        \
    do                                                  \
    {                                                   \
        assert(MinValue <= MaxValue);                   \
        if ((Value) > (MaxValue)) (Value) = (MaxValue); \
        if ((Value) < (MinValue)) (Value) = (MinValue); \
    }                                                   \
    while (false)


enum BANISH_PLAYER_CODE
{
    bpcInvalid = 0,

    bpcMapRefresh,
    bpcMapUnload,
    bpcNotInMapOwnerTeam,
    bpcCancelBanish,

    bpcTotal
};

class KRegion;
struct KCell;

struct KPOSITION_RECORD
{
    CHARACTER_MOVE_STATE    eMoveState;
    int                     nX;
    int                     nY;
    int                     nZ;
    int                     nDestX;
    int                     nDestY;
    int                     nVelocityXY;
    int                     nDirectionXY;
    int                     nVelocityZ;
    int                     nConvergenceSpeed;
    int                     nCurrentGravity;
    int                     nJumpCount;	
    int                     nFaceDirection;
    int                     nDivingCount;
    BOOL                    bTryPathState;
    int                     nTryPathSide;
    BOOL                    bSlip;
    int                     nJumpLimitFrame;
    int                     nCurrentTrack;
    int                     nFromNode;
    int                     nTargetCity;
    int                     nMoveFrameCount;
    KRegion*                pRegion;
    KCell*                  pCell;
};

// lzoѹ��Դ����С��Ŀ�괮��С(�ֽ�)
#define LZO_ORIGINAL_BUFFER_LEN (60 * 1024)
#define LZO_COMPRESS_BUFFER_LEN (LZO_ORIGINAL_BUFFER_LEN + LZO_ORIGINAL_BUFFER_LEN / 16 + 64 + 3)

#endif	//_KG_GLOBAL_DEF_H_
