/************************************************************************/
/* ȫ��������							                                */
/* Copyright : Kingsoft 2005										    */
/* Author	 : Zhu Jianqiu												*/
/* Comment	 : �˴���������SO3World��ʹ�õĳ�����һЩ������Ҫ���õĳ��� */
/*			   ����������KG_GlobalDef.h�У�����Project�������		*/
/* History	 :															*/
/*		2005.01.12	Create												*/
/************************************************************************/
#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "SO3GlobalDef.h"
#include "ProtocolBasic.h"

#define DIRECTION_BIT_NUM       8
#define DIRECTION_COUNT			(1 << DIRECTION_BIT_NUM)
#define DIRECTION_TURN_SPEED    8

#define	CURRENT_ITEM_VERSION	1
#define	ITEM_SETTING_DIR	SETTING_DIR"/item"
#define	NORMAL_WEAPON_FILE	"Weapon.tab"
#define	NORMAL_ARMOR_FILE	"Armor.tab"
#define NORMAL_TRINKET_FILE	"Trinket.tab"
#define CUST_WEAPON_FILE	"Custom_Weapon.tab"
#define	CUST_ARMOR_FILE		"Custom_Armor.tab"
#define	CUST_TRINKET_FILE	"Custom_Trinket.tab"
#define	ATTRIB_FILE			"Attrib.tab"
#define	COMMON_ITEM_FILE	"Other.tab"
#define SET_FILE			"Set.tab"
#define ITEM_BOX_FILE       "Box.tab"            

//���ߵ��������
#define DROP_LIST_DIR		"DropList"
#define DOODAD_DROP_LIST_DIR "DoodadDropList"
#define MONEY_DROP_FILE		"ClassDrop\\MoneyDrop.tab"

#define MAX_DROP_PER_NPC	10
#define MAX_DROP_PER_DOODAD	10
#define MAX_LOOT_SIZE		32

#define CUSTOM_OTACTION_NAME_LEN 16
#define USE_ITEM_PROGRESS_NAME_LEN 16

#define SCRIPT_DIR		"scripts"


// �ű��ص�������
#define SCRIPT_ON_ENTER_TRAP	"OnEnterTrap"		//Param1:player;
#define SCRIPT_ON_LEAVE_TRAP	"OnLeaveTrap"		//Param1:player;

#define SCRIPT_ON_INIT_SCENE	"OnInitScene"		//Param1:Scene
#define SCRIPT_ON_ENTER_SCENE	"OnEnterScene"		//Param1:player;Param2:DestScene
#define SCRIPT_ON_LEAVE_SCENE	"OnLeaveScene"		//Param1:player;Param2:SrcScene

#define SCRIPT_ON_USE_ITEM		"OnUse"				//Param1:player;Param2:item

#define SCRIPT_ON_OPEN_DOODAD	    "OnOpen"			//Param1:doodad;Param2:player
#define SCRIPT_ON_BREAK_OPEN	    "OnBreak"			//Param1:doodad;Param2:player
#define SCRIPT_ON_PICK_DOODAD	    "OnPick"			//Param1:doodad;Param2:player
#define SCRIPT_ON_PER_OPEN_DOODAD	"OnPerOpen"			//Param1:doodad;Param2:player

#define SCRIPT_ON_DIALOGUE		"OnDialogue"		//Param1:npc;	Param2:player
#define SCRIPT_ON_NPC_DEATH		"OnDeath"			//Param1:npc;	Param2:killer
#define SCRIPT_ON_PLAYER_DEATH	"OnDeath"			//Param1:player Param2:killer

#define SCRIPT_ON_ACCEPT_QUEST	"OnAcceptQuest"		//Param1:player	 Param2:dwQuestID	
#define SCRIPT_ON_FINISH_QUEST	"OnFinishQuest"		//Param1:player  Param2:nPresentChoice  Param3:nPresentChoice2 Param4:dwQuestID	
#define SCRIPT_ON_CANCEL_QUEST	"OnCancelQuest"		//Param1:player  Param2:dwQuestID	
#define SCRIPT_ON_START_QUEST	"OnStartQuest"		//

#define SCRIPT_ON_CUSTOM_EVENT	"OnCustomEvent"		//Param1:player Param2:target
#define SCRIPT_ON_CUSTOM_BREAK	"OnCustomBreak"		//Param1:player

//AI�ýű��ص�������
#define SCRIPT_ON_PATROL		"OnPatrol"			//Param1:npc
#define SCRIPT_ON_AI_EVENT		"OnAIEvent"			//Param1:npc

//������ʱ���Ļص�������
#define SCRIPT_ON_TIMER			"OnTimer"			//Param1:int Param2:int

//�����	
#define SCRIPT_ON_MEMORIZE_BOOK	"OnMemorizeBook"	//Param1:player Param2:BookID Param3:BookSubID
#define SCRIPT_ON_GET_ALL_BOOK	"OnGetAllBook"		//Param1:player Param2:BookID 

#define SCRIPT_ON_BEGIN_CAST_CRAFT  "OnBeginCastCraft"
#define SCRIPT_ON_BREAK_CAST_CRAFT  "OnBreakCastCraft"
#define SCRIPT_ON_END_CAST_CRAFT    "OnEndCastCraft"

#define OPEN_BOX            "OpenBox"               // ������, Param1:player Param2:dwBox Param3:dwPos
#define START_USE_ITEM      "OnStartUse"
#define USE_ITEM_SUCCEED    "OnUseSucceed"
#define USE_ITEM_FAILD      "OnUseFailed"

#define MAX_WINDOW_SELECT_COUNT	32					//�ͻ��˴���ѡ�����
#define MAX_SELECT_FUNCNAME_LEN	32					//ѡ��ص�����������

//�������������
#define DICE_MAX				100
#define DICE_MIN				1

//��������
#define MAX_PROFESSION_COUNT		128	
#define MAX_CRAFT_COUNT 16

#define OLD_MAX_RECIPE_ID 256
#define MAX_RECIPE_ID 1024
#define MAX_RECIPE_NAME_LEN 64
#define MAX_BELONG_NAME_LEN 32

#define MAX_MATERIAL 8
#define MAX_RESULTANT 8

#define MAX_READ_BOOK_ID_BIT 9			
#define MAX_READ_BOOK_SUB_ID_BIT 3		

#define MAX_READ_BOOK_ID (1 << MAX_READ_BOOK_ID_BIT)
#define MAX_READ_BOOK_SUB_ID (1 << MAX_READ_BOOK_SUB_ID_BIT)

// �������
#define	MAX_ITEM_COUNT         65536

#define INIT_BANK_SIZE		   28	//��ʼ���и��Ӵ�С
#define MAX_BANK_PACKAGE_COUNT 5	//���е���չ��������

//AI���
#define MAX_AI_EVENT 6
#define INIT_AI_EVENT_FUNC_NAME "InitAIEvent"

#define PLAYER_THREAT_TIME	(GAME_FPS * 6) //��ҵĳ������ʱ��

#define AI_RETURN_OVERTIME_FRAME 160

// �ַ������ȶ���
#define ITEM_NAME_LEN			64
#define QUEST_NAME_LEN			32
#define QUEST_LINK_NAME_LEN		32

// NPC AIӵ�еļ�������
#define MAX_NPC_AI_SKILL		8

#define MAX_OPEN_WINDOW_TEXT_LEN 1000

// �û����������̳���
#define MAX_USER_PREFERENCES_LEN 4096

// С��ͼ��ǵĸ��¼��
#define UPDATE_MINIMAP_MARK_INTERVAL 16

#define START_ESCORT_QUEST_DELAY (GAME_FPS * 8)
// ֻ��׷��
enum DOODAD_KIND
{
	dkInvalid = 0,

	dkNormal,					// ��ͨ��Doodad,��Tip,���ܲ���
	dkCorpse,					// ʬ��
	dkQuest,					// ������ص�Doodad
	dkRead,						// ���Կ���Doodad
	dkDialog,					// ���ԶԻ���Doodad
	dkAcceptQuest,				// ���Խ������Doodad,����������3��������һ����,ֻ��ͼ�겻ͬ����
	dkTreasure,					// ����
	dkOrnament,					// װ����,���ܲ���
	dkCraftTarget,				// ����ܵĲɼ���
	dkClientOnly,				// �ͻ�����
    dkNpcDrop,                  // ʹ��NPC����ģʽ��doodad
	dkChair,					// ��������Doodad
    dkGuide,                    // ·��
    dkDoor,                     // ��֮���ж�̬�ϰ���Doodad
    

	dkTotal
};

//��ɫ���ͣ����ڸ�ö��ֵ���̣������Ͻ�����˳��ֻ��׷�ӣ�������
enum CHARACTER_KIND
{
	ckInvalid = 0,

	ckPlayer,
	ckFightNpc,
	ckDialogNpc,
	ckCannotOperateNpc,	//���ܱ�������������Npc
	ckClientNpc,

	ckTotal
};

enum PATROL_REPEAT_STYLE
{
	prsInvalid = 0,

	prsNoRepeat,
	prsCircle,
	prsLine,

	prsTotal
};

// �Ա�
enum CHARACTER_GENDER
{
	cgMale = 1,
	cgFemale = 2,
    cgGay = 3
};

// ��ɫ����
enum CHARACTER_RACE
{
	crInvalid = 0,

	crNone,				//������
	crHuman,			//����
	crAnimal,			//����
	crRobot,			//��е
	crMonster,			//����
	crHolyBeast,		//����	ע��HolyBeast�ɺ���ԭ��

	crTotal
};

enum QUEST_DIFFICULTY_LEVEL  // ��������ѳ̶�
{   
    qdlInvalid = 0,

    qdlProperLevel,
    qdlHighLevel,
    qdlHigherLevel,
    qdlLowLevel,
    qdlLowerLevel
};

enum QUEST_EVENT_TYPE
{
	qetInvalid = -1,

	qetUpdateAll = 0,
	qetKillNpc,
	qetGetItem,
	qetDeleteItem,
	qetSetQuestValue,

	qetTotal
};

//��������
enum ALL_CRAFT_TYPE
{
	actInvalid = 0,

	actCollection,						// �ռ�����,����ɻ�,�ɽ�
	actProduce,							// ���켼��,��������,ҽ��,���
	actRead,							// �Ķ�����,�Ե���ʹ��
	actEnchant,							// ��ħ����
	actRadar,							// �״＼��(���ͻ���ʵ��)
    actCopy,                            // ��¼

	actEquipBreak,

	actTotal
};

typedef char _CHK_enum_ALL_CRAFT_TYPE_actTotal[(actTotal == 8) ? 1 : -1];

enum ITEM_BOX
{
	ibInvalid = 0x000000ff,		// ��ǿת��bye��ͬ��ʱ������ֻ�ܶ���0xffΪ�Ƿ���
	ibEquip = 0,				// ���뱣֤����ǵ�һ����Ч�ģ�����˳���Ӱ��������ʱ��װ������
	ibPackage,
	ibPackage1,
	ibPackage2,
	ibPackage3,
	ibPackage4,
	ibBank,
	ibBankPackage1,
	ibBankPackage2,
	ibBankPackage3,
	ibBankPackage4,
	ibBankPackage5,
	ibSoldList,
	ibEquipBackUp1,
	ibEquipBackUp2,
	ibSystemPackage,
	ibBulletPackage,
	ibTimeLimitSoldList,
	ibCubPackage,
	ibTotal
};

typedef char _CHK_enum_ITEM_BOX_ibTotal[(ibTotal == 19) ? 1 : -1];

//Npc����Ի���ʱ��
enum NPC_TALK_ACTION_TYPE
{
	ntatTurnToFight = 0,
	ntatDeath,
	ntatTeammateDeath,
	ntatIdle,
	ntatCustom,
	ntatTurnToIdle,
	ntatKillEnemy,
	ntatEscape,
	
	ntatTotal
};

enum DROP_TYPE
{
	dtInvalid = -1,

	dtNpcDrop,
	dtDoodadDrop,

	dtTotal
};

enum TARGET_TYPE
{
    ttInvalid,

    ttNoTarget,
    ttCoordination,
    ttNpc,
    ttPlayer,
    ttDoodad,
    ttItem,
	ttItemPos,

    ttTotal
};

enum DATE_FORMAT
{
	dfInvalid = 0x0000,

	dfSecond = 0x0001,
	dfMinute = 0x0002,
	dfHour = 0x0004,
	dfDay = 0x0008,
	dfMonth = 0x0010,
	dfYear = 0x0020,
	dfWeek = 0x0040
};

enum MINIMAP_RADAR_TYPE
{
	mrtInvalid = -1,

	mrtNoRadar = 0,
	mrtFindNpc,
	mrtFindCraftDoodad,

	mrtTotal
};

enum AI_EVENT_CONDITION
{
	aecInvalid = 0,

	//������ÿ֡��������
	aecSelfHPBelow,
	aecTargetHPBelow,
	aecFrameInterval,
	aecRandom,

	//��������������������
	aecEvent,
	aecTurnToFight = aecEvent,
	aecTurnToIdle,
	aecReturnToOrigin,
	aecSelfDead,
	aecTargetDead,

	aecTotal
};

enum AI_EVENT_ACTION
{
	aeaInvalid = 0,

	aeaEscape,
	aeaEscapeForHelp,
	aeaCallHelp,
	aeaDeath,
	aeaRunScript,

	aeaTotal
};

enum MOVE_MODE
{
	mmInvalid = 0,
	mmGround,
	mmWater,
	mmGroundAndWater,

	mmOnWaterLine,

	mmTotal
};

typedef char _CHK_enum_MOVE_MODE_mmTotal[(mmTotal == 5) ? 1 : -1];

class KPlayer;
class KNpc;
class KDoodad;

struct KCaster
{
	TARGET_TYPE	eTargetType;

	DWORD dwCharacterID;

	union 
	{
		ROLE_TYPE nRoleType;
		DWORD dwRepresentID;
	};
};

//����
#define TEN_THOUSAND_NUM        10000
#define KILO_NUM                1024            // �˺����㣬���Լ���ı�������������1/1024ȡ��1/100
#define MILLION_NUM				1048576		    // ��Ʒ�����ʰٷֱȾ��Ȼ���
#define MAX_SYNC_PER_REGION		64	            // ͬ�������Ʋ���
#define VIEW_SYNC_DELAY	        GAME_FPS        // ��Ұͬ����ʱ
#define STATE_UPDATE_TIME		8			    // ħ��״̬ˢ�¼��
#define DEFAULT_MASS			128			    // ��λ��g

#define DEFAULT_GRAVITY			(8 * 512 / GAME_FPS / GAME_FPS)	//��λ����/֡^2

#define LOGOUT_WAITING_TIME		(GAME_FPS * 20)	    // ������ߵȴ�20��
#define LOGIN_TIMEOUT			(GAME_FPS * 8)	    // ��½��ʱʱ��

#define MAX_LIFE_VALUE          (INT_MAX / 2)
#define MAX_MANA_VALUE          (INT_MAX / 2)

// �̵�ÿҳҪ��ʾ�������Ʒ����
#define MAX_SHOP_PAGE_ITEM_COUNT    10
// �̵����ҳ��
#define MAX_SHOP_PAGES              10

#define MAX_TRADING_BOX_SIZE        10

#define COMMON_PLAYER_OPERATION_DISTANCE    (10 * CELL_LENGTH)

#define MONEY_LIMIT 2000000000

#define GS_SETTINGS_FILENAME	"gs_settings.ini"

#pragma pack(1)
struct KMOVE_CRITICAL_PARAM
{
    BYTE byMoveState;
    int  nRunSpeed;
    int  nGravity;
    int  nJumpSpeed;
    int  nX;
    int  nY;
    int  nZ;
};
#pragma pack()

#define INVALID_SCENE_PROGRESS_ID   (-1)
#define MAX_SCENE_PROGRESS_ID       UCHAR_MAX
#define SCENE_PROGRESS_DATA_LEN     (MAX_SCENE_PROGRESS_ID / CHAR_BIT + 1)

#define MAX_ACHIEVEMENT_ID 4000 /* 2.5.2: Ghidra KAchievementInfoList::Init nID<4000 (was 2000) */

enum KACHIEVEMENT_ANNOUNCE_TYPE
{
    aatInvalid = 0,
    aatNearby,
    aatScene,
    aatGlobal,
};

enum KDESIGNATION_ANNOUNCE_TYPE
{
    datInvalid = 0,

    datNearby,
    datScene,
    datGlobal,

    datTotal,
};

#define MAX_FELLOWSHIP_ATTRACTIVE_LEVEL     6
#define MAX_FELLOWSHIP_COUNT_ACHIEVEMENT    8

enum KSKILL_EFFECT_PLAY_TYPE
{
    septInvalid = 0,

    septAniOnly,
    septSfxOnly,
    septAniAndSfx,
    septNone,

    septTotal
};

enum KITEM_EXIST_TYPE
{
    ketInvalid = -1,

    ketPermanent,        // û�д���ʱ�����Ƶĵ���
    ketOnLine,           // ����һ��ʱ����ʧ�ĵ���
    ketOffLine,          // ����һ��ʱ����ʧ�ĵ���
    ketOnlineAndOffLine, // �ܴ���һ��ʱ����ʧ�ĵ���
    ketTimeStamp
};

typedef char _CHK_enum_KITEM_EXIST_TYPE_ketTimeStamp[(ketTimeStamp == 4) ? 1 : -1];

#endif	//_GLOBAL_H_
