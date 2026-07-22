/************************************************************************/
/* Skill								                                */
/* Copyright : Kingsoft 2005										    */
/* Author	 : Zhu Jianqiu												*/
/* History	 :															*/
/*		2005.01.14	Create												*/
/************************************************************************/

#ifndef _KSKILL_H_
#define _KSKILL_H_

#include "Global.h"
#include "KBaseObject.h"
#include "SO3Result.h"
#include "list"
#include "KTarget.h"
#include "KMath.h"
#include "KBuff.h"

class   KAttribute;
class   KRegion;
struct  KCell;
class   ITabFile;
struct  KTarget;
class   KSceneObject;
class   KCharacter;
class   KNpc;
class   KDoodad;
class   KPlayer;
class   KSkill;

#define MAX_SKILL_COOL_DOWN_TIMER       3
#define MAX_SKILL_CHECKONLY_COOL_DOWN_TIMER  3   /* [drift 2.5.2] check-only cooldown slots (binary: nCoolDownIndex < 3) */
#define MAX_SUN_POWER_VALUE   1   /* [drift 2.5.2] sun subsection slots (binary: i <= 1) */
#define MAX_MOON_POWER_VALUE  1   /* [drift 2.5.2] moon subsection slots */
#define MAX_SKILL_BIND_BUFF_COUNT       4
#define PHYSICS_AP_TO_DPS               10
#define MAGIC_AP_TO_DPS                 12
#define THERAPY_AP_TO_DPS               12

#define WEAK_MINOR                      40
#define WEAK_MAJOR                      80
#define WEAK_MAX                        100

#define MAX_SKILL_REICPE_COUNT          4
#define MAX_BUFF_REICPE_COUNT           1

#define MAX_ACCUMULATE_VALUE            10                          // ��������С��Ԫʱ��Ϊ0.5��
#define MAX_ACCUMULATE_SUB_COUNT        MAX_ACCUMULATE_VALUE + 1    // ������10 + 1(�����Ժ�)����Ԫ

#define MAX_CHAIN_TARGET                21

#define SAME_SCHOOL_KUNGFU_CD_INDEX     0
#define OTHER_SCHOOL_KUNGFU_CD_INDEX    1


// ��ö�������˼��ܵ�����Ч������
enum KSKILL_EFFECT_TYPE
{
    setInvalid,
    setSkill,
    setBuff,
    setTotal
};

// ���ܷ��࣬������Ӧ�ã�ApplyType
enum KSKILL_APPLY_TYPE
{
    satInvalid = 0,

    satPlayerSkill,     // ���Ǽ���
    satNpcSkill,        // Npc����
    satGearSkill,       // װ������
    satItemSkill,       // Item����
    satSystemSkill,     // ϵͳ����
    satMissionSkill,    // ������ؼ���
    satTestSkill,       // ���Լ���

    satTotal
};

// ���ܷ��࣬���������ͣ�KindType
enum KSKILL_KIND_TYPE
{
    sktInvalid = 0,

    sktPhysics,         // �⹦����
    sktSolarMagic,      // �����ڹ�����
    sktNeutralMagic,    // �����ڹ�����
    sktLunarMagic,      // �����ڹ�����
    sktPoison,          // ������
    sktLeap,            // �Ṧ����
    sktNone,            // ������

    sktTotal
};

// ���ܷ��࣬�����ܹ������ʣ�FunctionType
enum KSKILL_FUNCTION_TYPE
{
    sftInvalid = 0,

    sftNormal,          // ��ͨ
    sftSlow,            // �ٻ�
    sftFear,            // �־�
    sftHalt,            // ����
    sftSilence,         // ����
    sftChaos,           // ����
    sftCharm,           // �Ȼ�
    sftStun,            // ����
    sftEnmity,          // ���
    sftBoolding,        // ��Ѫ
    sftDaze,            // ѣ��

    sftDamage,          // �˺�

    sftFly,
    sftDisarm,

    sftTotal
};

typedef char _CHK_enum_KSKILL_FUNCTION_TYPE_sftTotal[(sftTotal == 15) ? 1 : -1];

// ���ܷ��࣬���������ɣ�GenreType
enum KSILL_GENRE_TYPE
{
    sgtInvalid = 0,

    sgtTotal
};

// ���ܶ�����״̬��Ҫ��
enum SKILL_HORSE_STATE_CONDITION
{
    shcInvalid = 0,
    shcAny,
    shcHorse,
    shcNotHorse,
    shcIgnore,                  /* [drift 2.5.2] append */
    shcEquipSpecialHorse,
    shcTotal
};


// ���ܵ��ͷŷ�ʽ
enum KSKILL_CAST_MODE
{
    scmInvalid,
    scmSector,          // ���η�Χ,���Ĺ̶����Լ�
    scmTargetAngleSector,       /* [drift 2.5.2] interleave: shifts scmCasterArea..scmTeamArea +3 */
    scmRectangle,
    scmTargetAngleRectangle,
    scmCasterArea,      // ���Լ�Ϊ���ĵ�Բ������
    scmTargetArea,      // ��Ŀ��Ϊ���ĵ�Բ������
    scmPointArea,
    scmCasterSingle,    // �Ե������(�����Լ�)ʩ��
    scmTargetSingle,    // �Ե������(ָ��Ŀ��)ʩ��
    scmPoint,           // �������ʩ��
	scmItem,			// �Ե����ͷ�
    scmTargetChain,     // ��Ŀ���ͷ���״����
    scmTargetLeader,    // �������ͷż���
    scmTeamArea,       // С�ӵķ�Χ����
    scmTotal
};

enum KSKILL_WEAPON_REQUEST_TYPE
{
    swrtInvalid = 0,

    swrtMeleeWeapon,
    swrtRangeWeapon,

    swrtTotal
};

enum KSKILL_WEAK_POS
{
    swpInvalid = 0,

    swpUpper,
    swpMiddle,
    swpLower,
    swpNone,

    swpTotal
};

enum KSKILL_WEAK_LEVEL
{
    swlNone = 0,
    swlMinor,
    selMajor,
    selMax
};

enum KSKILL_COMMON_ACTIVE_MODE
{
    scamInvalid = 0,

    scamNone,
    scamMelee,
    scamRange,

    scamTotal
};

enum KSKILL_CAST_EFFECT_TYPE
{
    scetInvalid = 0,

    scetHarmful,
    scetNeutral,
    scetBeneficial,

    scetTotal
};

#define WEAK_VALUE_TO_LEVEL(WeakLevel, WeakValue)                       \
    do                                                                  \
    {                                                                   \
        if ((WeakValue) == 0)                                           \
            (WeakLevel) = swlNone;                                      \
        else if ((WeakValue) > 0 && (WeakValue) <= WEAK_MINOR)          \
            (WeakLevel) = swlMinor;                                     \
        else if ((WeakValue) > WEAK_MINOR && (WeakValue) <= WEAK_MAJOR) \
            (WeakLevel) = selMajor;                                     \
        else if ((WeakValue) > WEAK_MAJOR)                              \
            (WeakLevel) = selMax;                                       \
    } while(false)

//////////////////////////////////////////////////////////////////////////
#define MAKE_RECIPE_KEY(recipekey, recipeid, recipelevel)               \
    do                                                                  \
    {                                                                   \
        DWORD dwID    = (DWORD)(recipeid);                              \
        DWORD dwLevel = (DWORD)(recipelevel);                           \
        assert(dwID    <= USHRT_MAX);                                   \
        assert(dwLevel <= USHRT_MAX);                                   \
        (recipekey) =                                                   \
            (dwID << (sizeof(unsigned short) * CHAR_BIT)) |             \
            (dwLevel & USHRT_MAX);                                      \
    } while(false)

#define GET_RECIPE_ID_OF_KEY(recipeid, recipekey)                       \
    do                                                                  \
    {                                                                   \
        DWORD dwKey = (recipekey);                                      \
        (recipeid) = (dwKey >> (sizeof(unsigned short) * CHAR_BIT));    \
    } while(false)

#define GET_RECIPE_LEVEL_OF_KEY(recipelevel, recipekey)                 \
    do                                                                  \
    {                                                                   \
        DWORD dwKey = (recipekey);                                      \
        (recipelevel) = (dwKey & USHRT_MAX);                            \
    } while(false)


struct KSKILL_RECIPE_KEY
{
    DWORD   dwSkillID;
    DWORD   dwSkillLevel;
    DWORD   dwRecipeKey[MAX_SKILL_REICPE_COUNT];
};

bool operator<(const KSKILL_RECIPE_KEY& lhs, const KSKILL_RECIPE_KEY& rhs);
bool operator==(const KSKILL_RECIPE_KEY& lhs, const KSKILL_RECIPE_KEY& rhs);

enum KSKILL_RECIPE_RESULT_CODE 
{
    srrInvalid = 0,

    srrSuccess,
    srrFailed,
    srrErrorInFight,

    srrTotal
};

//////////////////////////////////////////////////////////////////////////

struct KSKILL_RECIPE_BASE_INFO
{
    BOOL        bIsSystemRecipe;

    DWORD       dwSkillID;
    DWORD       dwSkillLevel;

    int         nSkillRecipeType;

    int         nPrepareFramesAdd;
    int         nPrepareFramesPercent;

    int         nCoolDownAdd[MAX_SKILL_COOL_DOWN_TIMER];

    int         nMinRadiusAdd;
    int         nMaxRadiusAdd;

    int         nCostLifeAddPercent;
    int         nCostManaAddPercent;
    int         nCostStaminaAdd;

    int         nDamageAddPercent;

    char	    szScriptFile[MAX_PATH];	// ���ŶԼ��ܵ�Ӱ��ű��ļ����߻������ڽű�������ħ�����Եȵȡ�
                                        // ���������Žű��е�ħ����������������������ǰ�ˡ�
};

//////////////////////////////////////////////////////////////////////////

// Buff�������
enum KBUFF_COMPARE_FLAG
{
    bcfInvailed,

    bcfEqual,
    bcfGreaterEqual,
    bcfLess,

    bcfTotal
};

typedef char _CHK_enum_KBUFF_COMPARE_FLAG_bcfTotal[(bcfTotal == 4) ? 1 : -1];

struct KSKILL_REQUIRE_BUFF_NODE
{
    DWORD               dwBuffID;
    int                 nStackNum;
    KBUFF_COMPARE_FLAG  eStackCompareFlag;
    int                 nBuffLevel;
    KBUFF_COMPARE_FLAG  eLevelCompareFlag;
};

// �������Ӽ����ͷ���Ϣ

struct KSKILL_ACCUMULATE
{
    DWORD   dwSubSkillID;
    DWORD   dwSubSkillLevel;
};

// ��״������Ŀ��Ľṹ
struct KSKILL_CHAIN_TARGET
{
    KSKILL_CHAIN_TARGET() {};
    KSKILL_CHAIN_TARGET(KCharacter* pTargetCharacter, int nTargetDistance) 
                    : pCharacter(pTargetCharacter), nDistance(nTargetDistance) {};

    KCharacter* pCharacter;
    int         nDistance;
};

// 1. �ӵ��Ǽ�¼�ڷ����������ϵ�
// 2. һ���ӵ��������ö����
// 3. ���������Լ��������ӵ��������
// 4. ���������Լ�����ͼʱҪ��������ӵ�

struct KUSER_SKILL_EVENT;

class KSkillRecipePointer 
{
public:
    KSkill*             SetRecipeKey(const KSKILL_RECIPE_KEY& crRecipeKey);
    KSKILL_RECIPE_KEY&  GetRecipeKey();
    void                ClearRecipeKey();
    KSkill*             GetPointer();

private:
    KSkill*             m_pSkill;
    DWORD               m_dwSkillSN;
    KSKILL_RECIPE_KEY   m_SkillRecipeKey;
};

struct KSKILL_BULLET
{
    DWORD       dwBulletID;             // �ӵ�ID�������ӵ���ʱ��ϵͳΪÿ���ӵ�����һ��Ψһ��ID

    KSkillRecipePointer SkillRecipePointer;   // ����ָ�����
                                              // ʹ��ǰ�����ȱȽϽṹ���м�¼��SN��pSkill��ָ�����SNֻ��ƥ�䣬
                                              // �粻ƥ����˵��ָ���Ѿ���Ч��������SkillRecipeKey����GetSkill��
    
    KCharacter* pSkillSrc;
    DWORD       dwSkillSrcID;           // �����ͷ��ߵ�ID
    int         nSkillSrcLevel;
    KTarget     Target;                 // ֻ�����Ǽ������: NpcPointer, PlayerPointer, DoodadPointer, Coordination
    DWORD       dwTargetID;             // ��Target��¼Ϊָ���ʱ����Ҫ��֤ID
    int         nEndFrame;      

    int         nPhysicsHitValue;
    int         nSolarHitValue;
    int         nLunarHitValue;
    int         nNeutralHitValue;
    int         nPoisonHitValue;

    int         nCriticalStrike;        // �ػ���
    int         nCriticalStrikePower;

    std::vector<KUSER_SKILL_EVENT> SkillEventVector; // �����ͷ������ϼ����¼��Ŀ���
    
    int         nPhysicsDamage;         // �⹦�˺�, �������ܱ������˺���������������������ɵĸ����˺�
    int         nSolarDamage;      
    int         nNeutralDamage;    
    int         nLunarDamage;      
    int         nPoisonDamage; 
    int         nTherapy;	    
    
    int         nDotPhysicsAP;          // ���ӵ�Dot�ϵ��⹦�˺�, ������Dot�������˺�
    int         nDotSolarAP;      
    int         nDotNeutralAP;    
    int         nDotLunarAP;      
    int         nDotPoisonAP; 
    int         nDotTherapyAP;	       

    int         nDamageToLifeForSelf;     // ���˺�תΪ�Լ�������
    int         nDamageToManaForSelf;     // ���˺�תΪ�Լ�������

    int         nBaseThreatCoefficient;
    int         nDamageThreatCoefficient;
    int         nTherapyThreatCoefficient;

    DWORD       dwAddDamageByDstMoveStateMask;
    int         nAddDamagePercentByDstMoveState;

    struct KSKILL_BULLET* pNext;
};


// ע��,�书��ʵ����Ҳ��һ������ļ���,���һ��Skill��dwBelongKungfuΪ0,�Ǿͱ�ʾ��ʵ�����Ǹ��书

struct KSKILL_BASE_INFO
{
	char	szSkillName[_NAME_LEN];
    DWORD	dwSkillID;
    int     nSkillMark;             // PQ���׶�ͳ���õ�ʩ������
	DWORD	dwMaxLevel;				// ���ܵȼ�����,�����ȼ��Ϸ���Χ [1, dwMaxLevel]
    
    int     nApplyType;             // ���ܷ��࣬������Ӧ�ã�ApplyType
    int     nKindType;              // ���ܷ��࣬���������ͣ�KindType 
    int     nRecipeType;            // ��������ƥ�����
    // int     nFunctionType;          // ���ܷ��࣬�����ܹ������ʣ�FunctionType
    int     nGenreType;             // ���ܷ��࣬���������ɣ�

    int     nUIType;                // ��ʾ�ü��������ڹ������⹦��������...

    DWORD   dwBelongKungfu;         // �����书
    DWORD   dwBelongSchool;         // ��������

    int     nCastMode;              // KSKILL_CAST_MODE, �ͷŷ�ʽ

    DWORD   dwWeaponRequest;
    BOOL    bCostAmmo;              // �Ƿ����ĵ�ҩ������ĵ�ҩ
    int     nCostItemType;          // ��������ָ����Ʒ���ڵ����ñ�
    int     nCostItemIndex;         // ��������ָ����Ʒ�������ñ��������ֵ(Index)
    DWORD   dwMountRequestType;     // ������Ҫ���ڹ����ͣ�ֻ���ڲ���Ҫ�ڹ���ʱ��ſ�ʼ�ж�������ͣ�һ��������й�
    DWORD   dwMountRequestDetail;   // ������Ҫ���ڹ���ָ��һ���ڹ�
    DWORD   dwMountRequestDetailLevel;  // ������Ҫ���ڹ��ȼ�
    DWORD   dwMountType;

    BOOL    bIsMountable;
	BOOL    bIsPassiveSkill;
    BOOL    bIsChannelSkill;
    BOOL    bIsExpSkill;
    BOOL    bIsExactHit;            // �Ƿ����(��������������)
    BOOL    bIsInstinct;            // �Ƿ���(������ѧ�����õļ���)
    BOOL    bIsAutoTurn;            // �Ƿ���Ҫ�Զ�����
    
    int     nEffectType;            // ����Ч������Ϊ���棬�к�������
    BOOL    bCheckBindBuff;

    BOOL    bCauseAbradeEquipment;  // �������к�,�Ƿ�Լ����ͷ��ߵ������ͼ���Ŀ��װ�����ĥ��
    BOOL    bCauseBeatBreak;        // �����Ƿ���԰ѱ��˵�ͨ�������
    BOOL    bCauseBeatBack;         // �����Ƿ���԰ѱ��˵�����/ͨ������
    BOOL    bHasCriticalStrike;     // �����Ƿ����漴�ػ�

    DWORD   dwSkillEventMask1;
    DWORD   dwSkillEventMask2;
    DWORD   dwSkillCastFlag;
    DWORD   dwCastMask;             // �����ͷŷ��࣬����ĳ���༼���Ƿ������������ض�����Ӱ��

    BOOL    bUseCastScript;
    BOOL    bUse3DObstacle;

    BOOL    bIgnorePositiveShield;
    BOOL    bIgnoreNegativeShield;

    BOOL    bNeedOutOfFight;
    int     nSelfMoveStateMask;
    int     nSelfBackupMoveStateMask;
    BOOL    bSelfOnFear;

    int     nSelfHorseStateRequest;
    
    BOOL    bTargetTypePlayer;
    BOOL    bTargetTypeNpc;
    int     nTargetRelationMask;
    BOOL    bTargetRelationMentor;
    BOOL    bTargetRelationApprentice;

    BOOL    bBindCombatTag;         // �Ƿ��ðѪ�Ķ�����ǩ�����߼��޹ء����ڱ���

    int     nTargetMoveStateMask;
    BOOL    bTargetOnFear;

    int     nTargetHorseStateRequest;

    int     nEffectPlayType;

    int     nCommonSkillActiveMode;
    BOOL    bAutoSelectTarget;
    int     nNPCWeaponType;
    DWORD   dwMapBanMask;
	char	szScriptFile[MAX_PATH];	// ���ܽű��ļ�
};

struct KSKILL_DISPLAY_INFO
{
    int     nCastTime;
    int     nCoolDown[MAX_SKILL_COOL_DOWN_TIMER];
    int     nAreaRadius;
    int     nMinRadius;
    int     nMaxRadius;
    int     nCostMana;
	int		nCostLife;
    int     nCostRage;

    int     nMinPhysicsDamage;
    int     nMaxPhysicsDamage;
    int     nPhysicsDamagePercent;

    int     nMinSolarDamage;
    int     nMaxSolarDamage;
    int     nSolarDamagePercent;

    int     nMinNeutralDamage;
    int     nMaxNeutralDamage;
    int     nNeutralDamagePercent;

    int     nMinLunarDamage;
    int     nMaxLunarDamage;
    int     nLunarDamagePercent;

    int     nMinPoisonDamage;
    int     nMaxPoisonDamage;
    int     nPoisonDamagePercent;

    int     nMinTherapy;
    int     nMaxTherapy;
    int     nTherapyPercent;

    int     nBuffCount[MAX_SKILL_BIND_BUFF_COUNT];
    int     nBuffIntervalFrame[MAX_SKILL_BIND_BUFF_COUNT];
    int     nBuffDamage[MAX_SKILL_BIND_BUFF_COUNT];
    int     nBuffTherapy[MAX_SKILL_BIND_BUFF_COUNT];
};

class KSkill
{
public:
    KSkill();
    ~KSkill();

    BOOL Init(KSKILL_BASE_INFO *pBaseInfo, DWORD dwLevel);
    void UnInit();

    BOOL CallInitializeScript();
    BOOL CallLevelUpScript(KPlayer* pPlayer);

	SKILL_RESULT_CODE CanCast(KCharacter *pSrcCharacter, KTarget &rTarget);
    SKILL_RESULT_CODE CanCastOnPrepare(KCharacter *pSrcCharacter, KTarget &rTarget);
	SKILL_RESULT_CODE Cast(KCharacter* pDisplayCaster, KCharacter* pLogicCaster, KTarget& rTarget, const KSKILL_RECIPE_KEY& crRecipeKey);


public:
    KSKILL_BASE_INFO*   m_pBaseInfo;			// ����������Ϣ

    DWORD               m_dwSerialNumber;       // ���ܶ����ʼ��ʱȷ����һ��Ψһ��ʶ��

	DWORD				m_dwLevel;		        // ���ܵȼ�

    // --------- �������� ------------------------------>
	DWORD				m_dwLevelUpExp;
    int 				m_nPlayerLevelLimit;    // ѧ�øü��ܵȼ����� 
    int                 m_nExpAddOdds;          // ������������������

    // --------- �������� ------------------------------>
	int	                m_nCostLife;
	int	                m_nCostMana;
    int                 m_nCostManaBasePercent;  /* [drift 2.5.2] DWARF offset 0x20 */
    int                 m_nCostRage;
	int                 m_nCostEnergy;           /* [drift 2.5.2] DWARF offset 0x28 */
	int					m_nCostStamina;	
    int                 m_nCostTrain;            /* [wave1b P1] */

    // <--------- �������� ------------------------------

	int					m_nPrepareFrames;		// ����֡��

	int					m_nThreatRevisePercent;	// ��������ٷֱ�

	int					m_nMinRadius;		    // ��С����    
    int					m_nMaxRadius;		    // ������
    int                 m_nProtectRadius;        /* [drift 2.5.2] DWARF offset 0x44 */
    // m_nAreaRadius��ʲô����?
    // ���������������񵯵ļ���,m_nMaxRadius��ʾ���������Զ�Զ,��m_nAreaRadius���ʾ�����񵯵ı�ը�뾶
    int                 m_nAreaRadius;         
    int                 m_nHeight;               /* [drift 2.5.2] DWARF offset 0x4c */
    int                 m_nRectWidth;            /* [drift 2.5.2] DWARF offset 0x50 */

	int					m_nAngleRange;		    // ��Χ�����������˺��Ƕȷ�Χ

    int                 m_nBulletVelocity;      // �ӵ��ٶ�

    int                 m_nWeaponDamagePercent; // �������˺�ֵ�ļӳ�
    
    int                 m_nBreakRate;           // ��ϵĸ���
	int					m_nBrokenRate;		    // ����ϵĸ���(����1024)

    int                 m_nBeatBackRate;        // ���˸���
    
    int                 m_nChannelFrame;        // ͨ�����ܳ���ʱ��
    int                 m_nChannelInterval;     // ͨ�������ʱ��

    int                 m_nTargetCountLimit;    // ��������Ŀ���������ƣ���Ҫ���ڷ�Χ��; (С��0 ����Ŀ������������)

    int                 m_nAttackWeakPos;

    int                 m_nDamageAddPercent;

    // �����
    BOOL                m_bIsFormationSkill;            // ���Ϊtrue���˼���ֻ�������ۣ��ӳ����ͷţ�������Ϊfalseʱ����������������������
    int                 m_nFormationRange;              // ����ķ�Χ��������ΪԲ�ġ�
    int                 m_nLeastFormationPopulation;    // ����ķ�Χ�����ٶ�Ա���������ӳ���

    int                 m_nDismountingRate;             // ����������

    int                 m_nBaseThreat;                  // ���ܲ����Ļ������

    // ���������
    BOOL                m_bIsAccumulate;
    BOOL                m_bIsSunMoonPower;       /* [wave1b P1] */
    KSKILL_ACCUMULATE   m_SubsectionSkill[MAX_ACCUMULATE_SUB_COUNT];

    // ��״�������
    int                 m_nChainDepth;
    int                 m_nChainBranch;
    
    int                 m_nDamageToLifeForParty;    // ���˺�תΪС�ӵ�����
    int                 m_nDamageToManaForParty;    // ���˺�תΪС�ӵ�����

    int                 m_nTargetLifePercentMax;    // ��Ŀ��Ѫ�������󣬼�������Ŀ��Ѫ�����������
    int                 m_nTargetLifePercentMin;    // ��Ŀ��Ѫ�������󣬼�������Ŀ��Ѫ�����������

    int                 m_nSelfLifePercentMax;      // ������Ѫ�������󣬼�����������Ѫ�����������
    int                 m_nSelfLifePercentMin;      // ������Ѫ�������󣬼�����������Ѫ�����������

	KAttribute*		    m_pAttrbuteEffectToSelfAndRollback;
	KAttribute*		    m_pAttrbuteEffectToSelfNotRollback;
	KAttribute*		    m_pAttrbuteEffectToDestAndRollback;
    KAttribute*		    m_pAttrbuteEffectToDestNotRollback;
    KAttribute*		    m_pAttrbuteEffectToDestOnMinorWeak;
    KAttribute*		    m_pAttrbuteEffectToDestOnMajorWeak;
    KAttribute*		    m_pAttrbuteEffectToDestOnMaxWeak;

    KBuffRecipePointer  m_BindBuffs[MAX_SKILL_BIND_BUFF_COUNT];

    typedef std::vector<KSKILL_REQUIRE_BUFF_NODE> KSKILL_REQUIRE_BUFF_VECTOR;
    KSKILL_REQUIRE_BUFF_VECTOR m_SelfRequireBuffVector;
    KSKILL_REQUIRE_BUFF_VECTOR m_DestRequireBuffVector;
    KSKILL_REQUIRE_BUFF_VECTOR m_SelfOwnRequireBuffVector;  /* [drift 2.5.2] own-buff variants */
    KSKILL_REQUIRE_BUFF_VECTOR m_DestOwnRequireBuffVector;

    DWORD               m_dwPublicCoolDownID;
    DWORD               m_dwCoolDownID[MAX_SKILL_COOL_DOWN_TIMER];
    int                 m_nCoolDownAlter[MAX_SKILL_COOL_DOWN_TIMER];
    DWORD               m_dwCheckCoolDownID[MAX_SKILL_CHECKONLY_COOL_DOWN_TIMER];  /* [drift 2.5.2] */

public:

    BOOL IsAutoTurn();

    SKILL_RESULT_CODE CheckWeaponRequest(KPlayer* pCaster);
    SKILL_RESULT_CODE CheckMountRequest(KPlayer* pCaster);

    SKILL_RESULT_CODE CheckCoolDown(KPlayer* pCaster);
    BOOL              ResetPublicCooldown(KPlayer* pCaster);
    BOOL              ClearPublicCooldown(KPlayer* pCaster);
    BOOL              ResetNormalCooldown(KPlayer* pCaster);

	SKILL_RESULT_CODE CheckCost(KCharacter* pCaster);

#ifdef _SERVER
    void              DoCost(KCharacter* pCaster);
#endif

    SKILL_RESULT_CODE CheckSelfBuff(KCharacter* pCaster);
    SKILL_RESULT_CODE CheckTargetBuff(KTarget& rTarget);
    SKILL_RESULT_CODE CheckTargetType(KCharacter* pCaster, KTarget& rTarget);
    SKILL_RESULT_CODE CheckFormationRequire(KCharacter* pCaster);
    SKILL_RESULT_CODE CheckSelfState(KCharacter* pCaster);
    SKILL_RESULT_CODE CheckTargetState(KTarget& rTarget);
	SKILL_RESULT_CODE CheckTargetRange(KCharacter* pCaster, KTarget& rTarget, int nFixPlayerCastSkillRange = 0, int nFixNpcCastSkillRange = 0);
    SKILL_RESULT_CODE CheckTargetBuffList(KCharacter* pCaster, KTarget& rTarget);


    SKILL_RESULT_CODE CallCastScript(KCharacter* pCaster, SKILL_RESULT_CODE nPreResult);

    // �Զ�ѡ��һ���ʺ��ͷŸü��ܵ�Ŀ��
    BOOL AutoSelectTarget(KCharacter* pCaster); 

public:
#ifdef _SERVER

	SKILL_RESULT_CODE CastOnSector(KCharacter* pDisplayCaster, KCharacter* pLogicCaster, const KSKILL_RECIPE_KEY& crRecipeKey);
    SKILL_RESULT_CODE CastOnCasterArea(KCharacter* pDisplayCaster, KCharacter* pLogicCaster, const KSKILL_RECIPE_KEY& crRecipeKey);
    SKILL_RESULT_CODE CastOnTargetArea(KCharacter* pDisplayCaster, KCharacter* pLogicCaster, KTarget& rTarget, const KSKILL_RECIPE_KEY& crRecipeKey);
    SKILL_RESULT_CODE CastOnPointArea(KCharacter* pDisplayCaster, KCharacter* pLogicCaster, KTarget& rTarget, const KSKILL_RECIPE_KEY& crRecipeKey);
    SKILL_RESULT_CODE CastOnCasterSingle(KCharacter* pDisplayCaster, KCharacter* pLogicCaster, const KSKILL_RECIPE_KEY& crRecipeKey);
    SKILL_RESULT_CODE CastOnTargetSingle(KCharacter* pDisplayCaster, KCharacter* pLogicCaster, KTarget& rTarget, const KSKILL_RECIPE_KEY& crRecipeKey);
    SKILL_RESULT_CODE CastOnPoint(KCharacter* pDisplayCaster, KCharacter* pLogicCaster, KTarget& rTarget, const KSKILL_RECIPE_KEY& crRecipeKey);
    SKILL_RESULT_CODE CastOnTargetChain(KCharacter* pDisplayCaster, KCharacter* pLogicCaster, KTarget& rTarget, const KSKILL_RECIPE_KEY& crRecipeKey);
    SKILL_RESULT_CODE CastOnTargetLeader(KCharacter* pDisplayCaster, KCharacter* pLogicCaster, const KSKILL_RECIPE_KEY& crRecipeKey);
    SKILL_RESULT_CODE CastOnTeamArea(KCharacter* pDisplayCaster, KCharacter* pLogicCaster, const KSKILL_RECIPE_KEY& crRecipeKey);

    BOOL SetupBullet(KSKILL_BULLET *pBullet, KCharacter* pCaster, KTarget &rTarget, const KSKILL_RECIPE_KEY& crRecipeKey);

    SKILL_RESULT_CODE ApplyOnSector(KSKILL_BULLET* pBullet);
    SKILL_RESULT_CODE ApplyOnArea(KSKILL_BULLET* pBullet, int nX, int nY);
    SKILL_RESULT_CODE ApplyOnSingle(KSKILL_BULLET* pBullet);
    SKILL_RESULT_CODE ApplyOnPoint(KSKILL_BULLET* pBullet);
    SKILL_RESULT_CODE ApplyOnChain(KSKILL_BULLET* pBullet);
    
#endif

public:
#ifdef _CLIENT
    int GetSkillDisplayInfo(KSKILL_DISPLAY_INFO* pSkillDisplayInfo);
#endif


public:
    //�ű��ӿ�
    DECLARE_LUA_CLASS(KSkill);

    DECLARE_LUA_STRUCT_STRING(SkillName,             sizeof(m_pBaseInfo->szSkillName),  (char *)m_pBaseInfo->szSkillName);
    DECLARE_LUA_STRUCT_DWORD(SkillID,                m_pBaseInfo->dwSkillID);
    DECLARE_LUA_STRUCT_DWORD(MaxLevel,               m_pBaseInfo->dwMaxLevel);
	//DECLARE_LUA_STRUCT_INTEGER(FunctionType,         m_pBaseInfo->nFunctionType);
    DECLARE_LUA_STRUCT_INTEGER(UIType,               m_pBaseInfo->nUIType);
    DECLARE_LUA_STRUCT_DWORD(BelongKungfu,           m_pBaseInfo->dwBelongKungfu);
    DECLARE_LUA_STRUCT_DWORD(BelongSchool,           m_pBaseInfo->dwBelongSchool);
    DECLARE_LUA_STRUCT_INTEGER(CastMode,             m_pBaseInfo->nCastMode);
    DECLARE_LUA_STRUCT_DWORD(WeaponRequest,          m_pBaseInfo->dwWeaponRequest);
    DECLARE_LUA_STRUCT_INTEGER(CostItemType,         m_pBaseInfo->nCostItemType);
    DECLARE_LUA_STRUCT_INTEGER(CostItemIndex,        m_pBaseInfo->nCostItemIndex);
    DECLARE_LUA_STRUCT_DWORD(MountRequestType,       m_pBaseInfo->dwMountRequestType);
    DECLARE_LUA_STRUCT_DWORD(MountRequestDetail,     m_pBaseInfo->dwMountRequestDetail);
    DECLARE_LUA_STRUCT_DWORD(MountRequestDetailLevel, m_pBaseInfo->dwMountRequestDetailLevel);
    DECLARE_LUA_STRUCT_DWORD(MountType,              m_pBaseInfo->dwMountType);
    DECLARE_LUA_STRUCT_BOOL(IsMountAble,             m_pBaseInfo->bIsMountable);
    DECLARE_LUA_STRUCT_BOOL(IsPassiveSkill,          m_pBaseInfo->bIsPassiveSkill);
    DECLARE_LUA_STRUCT_BOOL(IsChannelSkill,          m_pBaseInfo->bIsChannelSkill);
    DECLARE_LUA_STRUCT_BOOL(IsExpSkill,              m_pBaseInfo->bIsExpSkill);
    DECLARE_LUA_STRUCT_BOOL(IsExactHit,              m_pBaseInfo->bIsExactHit);
    DECLARE_LUA_STRUCT_BOOL(IsInstinct,              m_pBaseInfo->bIsInstinct);
    DECLARE_LUA_STRUCT_BOOL(IsAutoTurn,              m_pBaseInfo->bIsAutoTurn);
    DECLARE_LUA_STRUCT_INTEGER(EffectType,           m_pBaseInfo->nEffectType);
    DECLARE_LUA_STRUCT_BOOL(CauseAbradeEquipment,    m_pBaseInfo->bCauseAbradeEquipment);
    DECLARE_LUA_STRUCT_BOOL(UseCastScript,           m_pBaseInfo->bUseCastScript);
    
    DECLARE_LUA_DWORD(Level);
    DECLARE_LUA_DWORD(LevelUpExp);
    DECLARE_LUA_INTEGER(PlayerLevelLimit);
    DECLARE_LUA_INTEGER(ExpAddOdds);

    DECLARE_LUA_INTEGER(CostLife);
    DECLARE_LUA_INTEGER(CostMana);
    DECLARE_LUA_INTEGER(CostRage);
    DECLARE_LUA_INTEGER(CostStamina);
    DECLARE_LUA_INTEGER(CostTrain);              /* [wave1b P1] */

    DECLARE_LUA_INTEGER(PrepareFrames);	

    DECLARE_LUA_INTEGER(ThreatRevisePercent);

    DECLARE_LUA_INTEGER(MinRadius);	
    DECLARE_LUA_INTEGER(MaxRadius);	
    DECLARE_LUA_INTEGER(AreaRadius);	

    DECLARE_LUA_INTEGER(AngleRange);		

    DECLARE_LUA_INTEGER(BulletVelocity);  
    DECLARE_LUA_INTEGER(WeaponDamagePercent);
    
    DECLARE_LUA_INTEGER(BreakRate);	
    DECLARE_LUA_INTEGER(BrokenRate);

    DECLARE_LUA_INTEGER(BeatBackRate);

    DECLARE_LUA_INTEGER(ChannelFrame);		
    DECLARE_LUA_INTEGER(ChannelInterval);	

    DECLARE_LUA_INTEGER(TargetCountLimit);

    DECLARE_LUA_INTEGER(AttackWeakPos);

    DECLARE_LUA_BOOL(IsFormationSkill);
    DECLARE_LUA_INTEGER(FormationRange);
    DECLARE_LUA_INTEGER(LeastFormationPopulation);

    DECLARE_LUA_BOOL(IsAccumulate);
    DECLARE_LUA_BOOL(IsSunMoonPower);            /* [wave1b P1] */

    DECLARE_LUA_INTEGER(DismountingRate);
    DECLARE_LUA_INTEGER(BaseThreat);
    DECLARE_LUA_INTEGER(Height);              /* [drift 2.5.2] */
    DECLARE_LUA_INTEGER(RectWidth);
    DECLARE_LUA_INTEGER(ProtectRadius);
    DECLARE_LUA_INTEGER(CostManaBasePercent);
    DECLARE_LUA_INTEGER(CostEnergy);

    DECLARE_LUA_INTEGER(ChainBranch);
    DECLARE_LUA_INTEGER(ChainDepth);

    DECLARE_LUA_INTEGER(DamageToLifeForParty);
    DECLARE_LUA_INTEGER(DamageToManaForParty);

    DECLARE_LUA_INTEGER(TargetLifePercentMax);
    DECLARE_LUA_INTEGER(TargetLifePercentMin);

    DECLARE_LUA_INTEGER(SelfLifePercentMax);
    DECLARE_LUA_INTEGER(SelfLifePercentMin);

    int LuaAddAttribute(Lua_State* L);
    int LuaGetAttribute(Lua_State* L);

    int LuaAddSlowCheckSelfBuff(Lua_State* L);
    int LuaAddSlowCheckDestBuff(Lua_State* L);

    int LuaBindBuff(Lua_State* L);

    int LuaGetBuff(Lua_State* L);
    int LuaGetDebuff(Lua_State* L);
    int LuaGetDot(Lua_State* L);
    int LuaGetHot(Lua_State* L);

    int LuaSetBuffRecipe(Lua_State* L);
    int LuaSetDebuffRecipe(Lua_State* L);
    int LuaSetDotRecipe(Lua_State* L);
    int LuaSetHotRecipe(Lua_State* L);

    int LuaSetPublicCoolDown(Lua_State* L);
    int LuaSetNormalCoolDown(Lua_State* L);
    int LuaGetNormalCooldownCount(Lua_State* L);
    int LuaGetNormalCooldownID(Lua_State* L);
    int LuaSetCheckCoolDown(Lua_State* L);        /* [drift 2.5.2] */
    int LuaGetCheckCoolDownCount(Lua_State* L);
    int LuaGetCheckCoolDownID(Lua_State* L);
    int LuaAddSlowCheckSelfOwnBuff(Lua_State* L);
    int LuaAddSlowCheckDestOwnBuff(Lua_State* L);

    int LuaSetSubsectionSkill(Lua_State* L);
    int LuaSetSunSubsectionSkill(Lua_State* L);   /* [drift 2.5.2] */
    int LuaSetMoonSubsectionSkill(Lua_State* L);

#ifdef _CLIENT
    int LuaUITestCast(Lua_State* L);

    int LuaCheckWeaponRequest(Lua_State* L);
    int LuaCheckMountRequest(Lua_State* L);
    int LuaCheckCostItemRequest(Lua_State* L);
    int LuaCheckDistance(Lua_State* L);
    int LuaCheckAngle(Lua_State* L);
    int LuaCheckCasterState(Lua_State* L);
    int LuaCheckTargetState(Lua_State* L);

    int _LuaCheckCostItemRequest(KPlayer* pPlayer);
    int _LuaCheckDistance(KCharacter* pCaster, KTarget* pTarget);
    int _LuaCheckAngle(KCharacter* pCaster, KTarget* pTarget);
#endif
};

struct KSkillSectorTravFunc
{	
    KSKILL_BULLET* pBullet;
    int nLeftCount;
    BOOL CanApply(KTarget &rTarget);
    BOOL operator()(KCharacter *pCharacter);
	BOOL operator()(KDoodad *pDoodad);
};


struct KSkillAreaTravFunc
{
    KSKILL_BULLET*  pBullet;
    int             nLeftCount;
    BOOL            bTargetArea;
    int             nTargetType;  // ������¼TargetArea��ʼ���õ�Ŀ������
    DWORD           dwTargetID;   // ������¼TargetArea��ʼ���õ�Ŀ��ID
    BOOL            CanApply(KTarget &rTarget);
    BOOL            operator()(KCharacter *pCharacter);
	BOOL            operator()(KDoodad *pDoodad);
};

// --------------------- �����¼� -------------------------------->

enum KSKILL_EVENT_TYPE
{
    seInvalid,
    seCast,
    seBeCast,
    seHit,
    seBeHit,
    seDodge,
    seBeDodge,
    seCriticalStrike,
    seBeCriticalStrike,
    seBlock,
    seBeBlock,
    seAccumulate,
    seHitOTAction,
    seBeHitOTAction,
    seKill,
    seBeKill,
    seOverHeal,
    seBeOverHeal,
    seMiss,
    seBeMiss,
    seParry,                    /* [drift 2.5.2] append =20 */
    seBeParry,
    seTotal
};

typedef char _CHK_enum_KSKILL_EVENT_TYPE_seTotal[(seTotal == 22) ? 1 : -1];

// �����¼��Ĵ�������ʵ���Ͼ��ǰ���Ԥ��Ĳ�����������һ������
// ����A->Bʩ�ż��ܵĹ����д��������¼�X
// �������ö�ٶ��ڼ����¼�����������˵,esttCasterָ�������Ӧ����AΪĿ��,esttTargetָ�������Ӧ����BΪĿ��
enum KEVENT_SKILL_CASTER_TARGET_TYPE
{
    escttInvalid,
    escttCaster,
    escttTarget,
    escttTotal
};

struct KSKILL_EVENT
{
    int     nEventType;
    int     nOdds;  // ��������,�ٷ���, 1 / 1024.
    DWORD   dwEventMask1;
    DWORD   dwEventMask2;
    DWORD   dwSkillID;
    DWORD   dwSkillLevel;
    int     nCasterType;
    int     nTargetType;
};

struct KUSER_SKILL_EVENT
{
    DWORD           dwEventID;
    int             nRefCount;
    KSKILL_EVENT    SkillEvent;
};

// Ϊ���� AI ��ϣ����ܵľ����ж�����Բ����ķ�ʽ�����������塣
BOOL SkillInRange(int nSrcX, int nSrcY, int nSrcZ, int nDstX, int nDstY, int nDstZ, int nRange);
IN_RANGE_RESULT SkillInRange(int nSrcX, int nSrcY, int nSrcZ, int nDstX, int nDstY, int nDstZ, int nMinRange, int nMaxRange);

// <--------------------- �����¼� --------------------------------

#endif	//_KSKILL_H_

