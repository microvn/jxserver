#include "stdafx.h"
#include "Global.h"
#include "Luna.h"
#include "KCharacter.h"
#include "KItemLib.h"
#include "KQuestList.h"
#include "KLootList.h"
#include "KGatewayClient.h"
#include "KPlayer.h"
#include "KAIAction.h"
#include "KAIVM.h"
#include "KTongDef.h"
#include "KGameCardDef.h"
#include "KMailDef.h"
#include "KAuctionDef.h"
#include "KMentorDef.h"

#define REGISTER_CONST(__VALUE__)	{#__VALUE__, __VALUE__},
#define REGISTER_CONST_LIST(__VALUE__)	{#__VALUE__, LUA_CONST_##__VALUE__},

KLuaConst LUA_CONST_GLOBAL[] =
{
	{"GAME_FPS",			    GAME_FPS},
    {"DIRECTION_COUNT",         DIRECTION_COUNT},
	{"CELL_LENGTH",			    CELL_LENGTH},
	{"LOGICAL_CELL_CM_LENGTH",	LOGICAL_CELL_CM_LENGTH},
    {"INVALID_PARTY_ID",        ERROR_ID},
    {"INVALID_SKILL_ID",        INVALID_SKILL_ID},
    {"MOVE_DEST_RANGE",         MOVE_DEST_RANGE},

	{"CURRENT_ITEM_VERSION",    CURRENT_ITEM_VERSION},
	{"MAX_BANK_PACKAGE_COUNT",  MAX_BANK_PACKAGE_COUNT},

	{"START_QUEST_DELAY",		START_ESCORT_QUEST_DELAY}, // 小队任务的延迟时间

    {"AI_USER_EVENT",           KAI_USER_EVENT},
    {"AI_USER_ACTION",          KAI_USER_ACTION},
	{NULL,						0}
};

//道具的种类
KLuaConst LUA_CONST_ITEM_GENRE[] =
{
    {"EQUIPMENT",               igEquipment},   // 装备
    {"POTION",                  igPotion},      // 药品
    {"TASK_ITEM",               igTaskItem},    // 任务道具
    {"MATERIAL",                igMaterial},    // 材料
	{"BOOK",			        igBook},        // 书
    {"DESIGNATION",             igDesignation}, // 称号道具
    {"MOUNT_ITEM",              igMountItem},
	{"ENCHANT_ITEM",            igEnchantItem},
    {"BOX",                     igBox},
    {"BOX_KEY",                 igBoxKey},
    {"DIAMOND",                 igDiamond},
    {"COLOR_DIAMOND",           igColorDiamond},
    {"CUB",                     igCub},
    {"FODDER",                  igFodder},

    {"TOTAL",                   igTotal},

	{NULL,			            0}
};

// 装备的大致分类
KLuaConst LUA_CONST_EQUIPMENT_SUB[] = 
{
    {"MELEE_WEAPON",            estMeleeWeapon},    // 近战武器
    {"RANGE_WEAPON",            estRangeWeapon},    // 远程武器
    {"CHEST",                   estChest},		    // 上衣
    {"HELM",	                estHelm},			// 头部
    {"AMULET",                  estAmulet},		    // 项链
    {"RING",		            estRing},			// 戒指
    {"WAIST",                   estWaist},		    // 腰带
    {"PENDANT",	                estPendant},		// 腰缀
    {"PANTS",                   estPants},		    // 裤子
    {"BOOTS",                   estBoots},		    // 鞋子
    {"BANGLE",                  estBangle},		    // 护臂
    {"WAIST_EXTEND",            estWaistExtend},    // 腰部扩展类装备，酒壶之类的
    {"PACKAGE",	                estPackage},		// 包裹
	{"ARROW",					estArrow},			// 暗器
    {"BACK_EXTEND",             estBackExtend},     // 背部扩展
    {"HORSE",                   estHorse},
    {"TOTAL",                   estTotal},

	{NULL,				        0}
};

//武器的具体分类
KLuaConst LUA_CONST_WEAPON_DETAIL[] =
{
    {"WAND",                    wdtWand},
    {"SPEAR",                   wdtSpear},
    {"SWORD",                   wdtSword},
    {"FIST",                    wdtFist},
    {"DOUBLE_WEAPON",           wdtDoubleWeapon},
    {"PEN",                     wdtPen},
    {"SLING_SHOT",              wdtSlingShot},
    {"DART",                    wdtDart},
    {"MACH_DART",               wdtMachDart},
    {"BIG_SWORD",               wdtBigSword},
    {"FLUTE",                   wdtFlute},
    {"KNIFE",                   wdtKnife},
    {"TOTAL",                   wdtTotal},


	{NULL,				        0}
};

//装备的
KLuaConst LUA_CONST_EQUIPMENT_INVENTORY[] = 
{
    {"MELEE_WEAPON",            eitMeleeWeapon},
    {"RANGE_WEAPON",            eitRangeWeapon},
    {"CHEST",                   eitChest},
    {"HELM",                    eitHelm},
    {"AMULET",                  eitAmulet},
    {"LEFT_RING",               eitLeftRing},
    {"RIGHT_RING",              eitRightRing},
    {"WAIST",                   eitWaist},
    {"PENDANT",                 eitPendant},
    {"PANTS",                   eitPants},
    {"BOOTS",                   eitBoots},
    {"BANGLE",                  eitBangle},
    {"WAIST_EXTEND",            eitWaistExtend},
    {"PACKAGE1",                eitPackage1},
    {"PACKAGE2",                eitPackage2},
    {"PACKAGE3",                eitPackage3},
    {"PACKAGE4",                eitPackage4},
    {"BANK_PACKAGE1",			eitBankPackage1},
	{"BANK_PACKAGE2",			eitBankPackage2},
	{"BANK_PACKAGE3",			eitBankPackage3},
	{"BANK_PACKAGE4",			eitBankPackage4},
	{"BANK_PACKAGE5",			eitBankPackage5},
	{"ARROW",					eitArrow},
    {"BACK_EXTEND",             eitBackExtend},
    {"HORSE",                   eitHorse},
    {"TOTAL",                   eitTotal},
    
	{NULL,				        0}
};

KLuaConst LUA_CONST_EQUIPMENT_REPRESENT[] = 
{
    {"FACE_STYLE",              perFaceStyle},
    {"HAIR_STYLE",              perHairStyle},
    {"HELM_STYLE",              perHelmStyle},
    {"HELM_COLOR",              perHelmColor},
    {"HELM_ENCHANT",            perHelmEnchant},
    {"CHEST_STYLE",             perChestStyle},
    {"CHEST_COLOR",             perChestColor},
    {"CHEST_ENCHANT",           perChestEnchant},
    {"WAIST_STYLE",             perWaistStyle},
    {"WAIST_COLOR",             perWaistColor},
    {"WAIST_ENCHANT",           perWaistEnchant},
    {"BANGLE_STYLE",            perBangleStyle},
    {"BANGLE_COLOR",            perBangleColor},
    {"BANGLE_ENCHANT",          perBangleEnchant},
    {"BOOTS_STYLE",             perBootsStyle},
    {"BOOTS_COLOR",			    perBootsColor},
	{"WEAPON_STYLE",		    perWeaponStyle},
    {"WEAPON_ENCHANT1",		    perWeaponEnchant1},
	{"WEAPON_ENCHANT2",		    perWeaponEnchant2},
	{"BACK_EXTEND",			    perBackExtend},
	{"WAIST_EXTEND",		    perWaistExtend},
    {"HORSE_STYLE",		        perHorseStyle},
    {"HORSE_ADORNMENT1",        perHorseAdornment1},
	{"HORSE_ADORNMENT2",        perHorseAdornment2},
	{"HORSE_ADORNMENT3",        perHorseAdornment3},
	{"HORSE_ADORNMENT4",        perHorseAdornment4},
    {"RESERVED",                perReserved},
    {"TOTAL",                   perRepresentCount},
    
	{NULL,				        0}
};

KLuaConst LUA_CONST_INVENTORY_TYPE[] =
{
    {"EQUIPMENT",               ivtEquipment},
    {"PACKAGE",                 ivtPackage},
    {"BANK",                    ivtBank},
    {"SLOT",                    ivtSlot},
    {"SOLD_LIST",               ivtSoldlist},
    {"TOTAL",                   ivtTotal},

    {NULL,                      0}
};

//绑定类型
KLuaConst LUA_CONST_ITEM_BIND[] =
{
    {"INVALID",                 ibtInvalid},
    {"NEVER_BIND",              ibtNeverBind},
    {"BIND_ON_EQUIPPED",        ibtBindOnEquipped},
    {"BIND_ON_PICKED",          ibtBindOnPicked},
    {"BIND_ON_TIME_LIMITATION", ibtBindOnTimeLimitation},

	{NULL,				        0}
};

KLuaConst LUA_CONST_ITEM_USE_TARGET_TYPE[] =
{
    {"INVALID",             iuttInvalid},
    {"NO_TARGET",           iuttNoTarget},
    {"SELF",                iuttSelf},
    {"OTHER_PLAYER",        iuttOtherPlayer},
    {"NPC",                 iuttNpc},
    {"COORDINATES",         iuttCoordinates},
    {"Doodad",              iuttDoodad},
    {"ITEM",                iuttItem},
    {"TOTAL",               iuttTotal},

	{NULL,				    0}
};


KLuaConst LUA_CONST_ITEM_INFO_TYPE[] =
{
    {"INVALID",                 iitInvaild},
    {"CUSTEQUIP_INFO",          iitCustEquipInfo},
    {"OTHER_INFO",              iitOtherInfo},

	{NULL,				        0}
};

KLuaConst LUA_CONST_ITEM_RESULT_CODE[] = 
{
    {"INVALID",                             ircInvalid},                 
    {"SUCCESS",                             ircSuccess},
    {"FAILED",                              ircFailed},

    {"ITEM_NOT_EXIST",                      ircItemNotExist},
    {"NOT_ENOUGH_DURABILITY",               ircNotEnoughDurability},
    {"ERROR_EQUIP_PLACE",                   ircErrorEquipPlace},
    {"FORCE_ERROR",                         ircForceError},
    {"TOO_LOW_AGILITY",                     ircTooLowAgility},
    {"TOO_LOW_STRENGTH",                    ircTooLowStrength},
    {"TOO_LOW_SPIRIT",                      ircTooLowSpirit},
    {"TOO_LOW_VITALITY",                    ircTooLowVitality},
    {"TOO_LOW_LEVEL",                       ircTooLowLevel},
    {"TOO_LOW_DURABILITY",                  ircTooLowDurability},
    {"CANNOT_EQUIP",                        ircCannotEquip},
    {"CANNOT_PUT_THAT_PLACE",               ircCannotPutToThatPlace},
    {"CANNOT_DISABLE_PACKAGE",              ircCannotDisablePackage},
    {"PACKAGE_NOT_ENOUGH_SIZE",             ircPackageNotEnoughSize},
	{"PLAYER_IS_DEAD" ,						ircPlayerIsDead},
	{"BANK_PACKAGE_DISABLED",				ircBankPackageDisabled},
    {"EQUIPED_PACKAGE_CANNOT_DESTROY",		ircEquipedPackageCannotDestroy},
    {"ITEM_BINDED",                         ircBinded},
    {"CANNOT_DESTROY",                      ircCannotDestroy},
    {"GENDER_ERROR",                        ircGenderError},
    {"PLAYER_IN_FIGHT",                     ircPlayerInFight},
    {"ON_HORSE",                            ircOnHorse},
    {"CAMP_CAN_NOT_EQUIP",                  ircCampCannotEquip},
    {"REQUIRE_PROFESSION",                  ircRequireProfession},
    {"REQUIRE_PROFESSION_BRANCH",           ircRequireProfessionBranch},
    {"PROFESSION_LEVEL_TOO_LOW",            ircProfessionLevelTooLow},
    {"TOTAL",                               ircTotal},

	{NULL,				                    0}
};

KLuaConst LUA_CONST_USE_ITEM_RESULT_CODE[] = 
{
    {"INVALID",                             uircInvalid},                 
    {"SUCCESS",                             uircSuccess},
    {"FAILED",                              uircFailed},

    {"ITEM_NOT_EXIST",                      uircItemNotExist},             
    {"CANNOT_USE_ITEM",                     uircCannotUseItem},
    {"NOT_ENOUGH_DURABILITY",               uircNotEnoughDurability},
    {"NOT_READY",                           uircNotReady},
    {"USE_IN_PACKAGE",                      uircUseInPackage},
    {"USE_WHEN_EQUIPPED",                   uircUseWhenEquipped},
    {"TOO_LOW_LEVEL",                       uircTooLowLevel},
    {"REQUIRE_PROFESSION",                  uircRequireProfession},
    {"REQUIRE_PROFESSION_BRANCH",           uircRequireProfessionBranch},
    {"PROFESSION_LEVEL_TOO_LOW",            uircProfessionLevelTooLow},
    {"ON_HORSE",                            uircOnHorse},
    {"IN_FIGHT",                            uircInFight},
    {"REQUIRE_MALE",                        uircRequireMale},
    {"REQUIRE_FEMALE",                      uircRequireFeMale},
    {"CAMP_CANNOT_USE",                     uircCampCannotUse},
    {"CANNOT_CAST_SKILL",                   uircCannotCastSkill},
    {"CAST_SKILL_FAILED",                   uircCastSkillFailed},
    {"COST_ITEM_DURABILITY_FAILED",         uircCostItemDurabilityFailed},
    {"FORCE_ERROR",                         uircForceError},
    {"ALEARD_HAVE_DESIGNATION",             uircAleardHaveDesignation},

    {NULL,				                    0}
};

KLuaConst LUA_CONST_LOOT_ITEM_RESULT_CODE[] = 
{
	{"INVALID",						lircInvalid},
	{"SUCCESS",						lircSuccess},
	{"FAILED",						lircFailed},

	{"INVENTORY_IS_FULL",			lircInventoryIsFull},
	{"NOT_EXIST_LOOT_ITEM",			lircNotExistLootItem},
	{"ADD_LOOT_ITEM_FAILED",		lircAddLootItemFailed},
	{"NO_LOOT_TARGET",				lircNoLootTarget},
	{"TOO_FAR_TO_LOOT",				lircTooFarToLoot},
	{"OVER_ITEM_LIMIT",				lircOverItemLimit},
	{"TOTAL",						lircTotal},

	{NULL,							0}
};

KLuaConst LUA_CONST_CRAFT_RESULT_CODE[] = 
{
	{"INVALID",						crcInvalid},
	{"SUCCESS",						crcSuccess},
	{"FAILED",						crcFailed},

	{"NOT_ENOUGH_STAMINA",			crcNotEnoughStamina},
    {"NOT_ENOUGH_THEW",			    crcNotEnoughThew},
	{"NOT_ENOUGH_ITEM",				crcNotEnoughItem},
	{"SKILL_NOT_READY",				crcSkillNotReady},
	{"WEAPON_ERROR",				crcWeaponError},
	{"TOO_LOW_PROFESSION_LEVEL",	crcTooLowProfessionLevel},
    {"TOO_LOW_EXT_PROFESSION_LEVEL", crcTooLowExtProfessionLevel},
	{"ADD_ITEM_FAILED",				crcAddItemFailed},
	{"INVENTORY_IS_FULL",			crcInventoryIsFull},
	{"PROFESSION_NOT_LEARNED",		crcProfessionNotLearned},
    {"EXT_PROFESSION_NOT_LEARNED",	crcExtProfessionNotLearned},
	{"ERROR_TOOL",					crcErrorTool},
    {"BOOK_IS_ALREADY_MEMORIZED",   crcBookIsAlreadyMemorized},
	{"REQUIRE_DOODAD",				crcRequireDoodad},
    {"ITEM_TYPE_ERROR",			    crcItemTypeError},
    {"BOOK_CANNOT_BE_COPY",         crcBookCannotBeCopy},
    {"DOING_OTACTION",              crcDoingOTAction},
    {"MOVE_STATE_ERROR",            crcMoveStateError},
    {"TOO_LOW_LEVEL",               crcTooLowLevel},

	{NULL,			0}
};


KLuaConst LUA_CONST_QUEST_RESULT_CODE[] = 
{
	{"INVALID",								qrcInvalid},
	{"SUCCESS",								qrcSuccess},
	{"FAILED",								qrcFailed},

	{"ERROR_QUEST_INDEX",					qrcErrorQuestIndex},
	{"ERROR_QUEST_ID",						qrcErrorQuestID},
	{"QUESTLIST_FULL",						qrcQuestListFull},
	{"ERROR_QUEST_STATE",					qrcErrorQuestState},
	{"ALREADY_ACCEPTED",					qrcAlreadyAcceptQuest},
	{"ALREADY_FINISHED",					qrcAlreadyFinishedQuest},
	{"CANNOT_FIND_QUEST",					qrcCannotFindQuest},
	{"TOO_LOW_LEVEL",						qrcTooLowLevel},
	{"TOO_HIGH_LEVEL",						qrcTooHighLevel},
	{"ERROR_GENDER",						qrcErrorGender},
	{"ERROR_ROLETYPE",						qrcErrorRoleType},
	{"ERROR_FORCE_ID",						qrcErrorForceID},
	{"COOLDOWN",							qrcCooldown},
	{"PREQUEST_UNFINISHED",					qrcPrequestUnfinished},
	{"ERROR_ITEM_COUNT",					qrcErrorItemCount},
	{"ERROR_KILL_NPC_COUNT",				qrcErrorKillNpcCount},
	{"TOO_LONG_TIME",						qrcTooLongTime},
	{"ERROR_TASK_VALUE",					qrcErrorTaskValue},
	{"ASSIST_QUEST_LIST_FULL",				qrcAssistQuestListFull},
	{"ERROR_ASSIST_QUEST_INDEX",			qrcErrorAssistQuestIndex},
	{"ERROR_START_NPC_TARGET",				qrcErrorStartNpcTarget},
	{"ERROR_START_DOODAD_TARGET",			qrcErrorStartDoodadTarget},
	{"ERROR_END_NPC_TARGET",				qrcErrorEndNpcTarget},
	{"ERROR_END_DOODAD_TARGET",				qrcErrorEndDoodadTarget},
	{"NO_START_QUEST_ITEM",					qrcNoStartQuestItem},
	{"NO_END_QUEST_ITEM",					qrcNoEndQuestItem},
	{"TOO_EARLY_TIME",						qrcTooEarlyTime},
	{"TOO_LATE_TIME",						qrcTooLateTime},
	{"TOO_LOW_SKILL_LEVEL",					qrcTooLowSkillLevel},
	{"TOO_LOW_LIFE_LEVEL",					qrcTooLowLifeLevel},
	{"ERROR_GENERATE_ITEM",					qrcErrorGenerateItem},
	{"ADD_ITEM_FAILED",						qrcAddItemFailed},
	{"ADD_MONEY_FAILED",					qrcAddMoneyFailed},
	{"ADD_REPUTATION_FAILED",				qrcAddReputationFailed},
	{"NOT_ENOUGH_START_ITEM",				qrcNotEnoughStartItem},
	{"NOT_ENOUGH_FREE_ROOM",				qrcNotEnoughFreeRoom},
	{"ERROR_MAP",							qrcErrorMap},
	{"UNFINISHED_QUEST",					qrcUnfinishedQuest},
	{"CANNOT_FIND_QUEST_IN_DEST_PLAYER",	qrcCannotFindQuestInDestPlayer},
	{"NEED_ACCEPT",							qrcNeedAccept},
	{"NO_NEED_ACCEPT",						qrcNoNeedAccept},
	{"QUEST_FAILED",						qrcQuestFailed},
	{"ADD_SKILL_FAILED",					qrcAddSkillFailed},
	{"NOT_ENOUGH_MONEY",					qrcNotEnoughMoney},
    {"MONEY_LIMIT",                         qrcMoneyLimit},
    {"ERROR_REPUTE",                        qrcErrorRepute},
    {"ADD_PRESENT_ITEM_FAILED",             qrcAddPresentItemFailed},
    {"DAILY_QUEST_FULL",                    qrcDailyQuestFull},
    {"ESCORT_QUEST_IS_RUNNING",             qrcEscortQuestIsRunning},
    {"ERROR_CAMP",                          qrcErrorCamp},
    {"QUEST_END_BOOK_NOT_MEMORIZED",        qrcQuestEndBookNotMemorized},
    {"CHARGE_LIMIT",                        qrcChargeLimit},
    {"REQUIRE_APPRENTICE",                  qrcRequireApprentice},
    {"TOO_MANY_QUEST_OFFER_ITEM",           qrcTooManyQuestOfferItem},

	{NULL,									0}
};


KLuaConst LUA_CONST_LOOT_ITEM[] =
{
	{"MAX_LOOT_SIZE",			MAX_LOOT_SIZE},

	{NULL,						0}
};

KLuaConst LUA_CONST_QUEST_COUNT[] =
{
	{"MAX_ACCEPT_QUEST_COUNT",	MAX_ACCEPT_QUEST_COUNT},
	{"QUEST_PARAM_COUNT",		QUEST_PARAM_COUNT},

	{NULL,						0}
};

KLuaConst LUA_CONST_QUEST_STATE[] =
{
	{"UNFINISHED",			qsUnfinished},
	{"FINISHED",			qsFinished},

	{NULL,					0}
};

KLuaConst LUA_CONST_QUEST_EVENT_TYPE[] = 
{
	{"UPDATE_ALL",			qetUpdateAll},
	{"KILL_NPC",			qetKillNpc},
	{"GET_ITEM",			qetGetItem},
	{"DELETE_ITEM",			qetDeleteItem},
	{"SET_QUEST_VALUE",		qetSetQuestValue},

	{NULL,					0}
};

KLuaConst LUA_CONST_QUEST_DIFFICULTY_LEVEL[] =
{
    {"INVALID",         qdlInvalid},
    {"PROPER_LEVEL",    qdlProperLevel},
    {"HIGH_LEVEL",      qdlHighLevel},
    {"HIGHER_LEVEL",    qdlHigherLevel},
    {"LOW_LEVEL",       qdlLowLevel},
    {"LOWER_LEVEL",     qdlLowerLevel},

    {NULL,              0}
};

KLuaConst LUA_CONST_PATROL_REPEAT_STYLE[] =
{
	{"NO_REPEAT",		prsNoRepeat},
	{"CIRCLE",			prsCircle},
	{"LINE",			prsLine},

	{NULL,				0}
};

KLuaConst LUA_CONST_TARGET_TYPE[] =
{
    {"NO_TARGET",		ttNoTarget},
    {"COORDINATION",	ttCoordination},
    {"NPC",			    ttNpc},
    {"DOODAD",		    ttDoodad},
    {"PLAYER",		    ttPlayer},
    {"ITEM",            ttItem},

    {NULL,			    0}
};

KLuaConst LUA_CONST_ITEM_TABLE_TYPE[] =
{
	{"ATTRIB",			ittAttrib},
	{"OTHER",			ittOther},
	{"CUST_WEAPON",		ittCustWeapon},
	{"CUST_ARMOR",		ittCustArmor},
	{"CUST_TRINKET",	ittCustTrinket},
	{"SET",				ittSet},

	{NULL,              0}
};

KLuaConst LUA_CONST_ALL_CRAFT_TYPE[] =
{
    {"INVALID",			actInvalid},

    {"COLLECTION",      actCollection},
    {"PRODUCE",			actProduce},
    {"READ",			actRead},
    {"ENCHANT",			actEnchant},
	{"RADAR",			actRadar},
    {"COPY",            actCopy},

    {"TOTAL",			actTotal},

    {NULL,				0}
};

KLuaConst LUA_CONST_MINI_RADAR_TYPE[]=
{
	{"INVALID",				actInvalid},

	{"NO_RADAR",			mrtNoRadar},
	{"FIND_NPC",			mrtFindNpc},
	{"FIND_CRAFT_DOODAD",	mrtFindCraftDoodad},

	{"TOTAL",				actTotal},

	{NULL,					0}
};

KLuaConst LUA_CONST_INVENTORY_INDEX[] = 
{
	{"INVALID",			ibInvalid},
	{"EQUIP",			ibEquip},
	{"PACKAGE",			ibPackage},
	{"PACKAGE1",		ibPackage1},
	{"PACKAGE2",		ibPackage2},
	{"PACKAGE3",		ibPackage3},
	{"PACKAGE4",		ibPackage4},
	{"BANK",			ibBank},
	{"BANK_PACKAGE1",	ibBankPackage1},
	{"BANK_PACKAGE2",	ibBankPackage2},
	{"BANK_PACKAGE3",	ibBankPackage3},
	{"BANK_PACKAGE4",	ibBankPackage4},
	{"BANK_PACKAGE5",	ibBankPackage5},
	{"SOLD_LIST",		ibSoldList},
	{"TOTAL",			ibTotal},

	{NULL,				0}
};

KLuaConst LUA_ATTRIBUTE_TYPE[] =
{
    /* [drift 2.5.2] regenerated 451 entries from exe LUA_ATTRIBUTE_TYPE @0x084ca1a0
       (tools/extract_lua_attr.py, pyghidra) - authoritative name->value, no transcription */
    {"RUN_SPEED_BASE",                                      1},
    {"MOVE_SPEED_PERCENT",                                  2},
    {"MAX_JUMP_COUNT",                                      3},
    {"GRAVITY_BASE",                                        4},
    {"GRAVITY_PERCENT",                                     5},
    {"JUMP_SPEED_BASE",                                     6},
    {"JUMP_SPEED_PERCENT",                                  7},
    {"DROP_DEFENCE",                                        8},
    {"DIVING_FRAME_BASE",                                   9},
    {"DIVING_FRAME_PERCENT",                                10},
    {"WATER_FLY_ABILITY",                                   11},
    {"STRENGTH_BASE",                                       12},
    {"STRENGTH_BASE_PERCENT_ADD",                           13},
    {"AGILITY_BASE",                                        14},
    {"AGILITY_BASE_PERCENT_ADD",                            15},
    {"VITALITY_BASE",                                       16},
    {"VITALITY_BASE_PERCENT_ADD",                           17},
    {"SPIRIT_BASE",                                         18},
    {"SPIRIT_BASE_PERCENT_ADD",                             19},
    {"SPUNK_BASE",                                          20},
    {"SPUNK_BASE_PERCENT_ADD",                              21},
    {"TRANSMIT_TRAIN",                                      22},
    {"MAX_TRAIN_VALUE",                                     23},
    {"BASE_VENATION_COF",                                   24},
    {"PHYSICS_VENATION_COF",                                25},
    {"MAGIC_VENATION_COF",                                  26},
    {"ASSIST_VENATION_COF",                                 27},
    {"BASE_POTENTIAL_ADD",                                  28},
    {"CURRENT_LIFE",                                        29},
    {"MAX_LIFE_BASE",                                       30},
    {"MAX_LIFE_PERCENT_ADD",                                31},
    {"MAX_LIFE_ADDITIONAL",                                 32},
    {"LIFE_REPLENISH",                                      33},
    {"LIFE_REPLENISH_PERCENT",                              34},
    {"LIFE_REPLENISH_COEFFICIENT",                          35},
    {"LIFE_REPLENISH_EXT",                                  36},
    {"CURRENT_MANA",                                        37},
    {"MAX_MANA_BASE",                                       38},
    {"MAX_MANA_PERCENT_ADD",                                39},
    {"MAX_MANA_ADDITIONAL",                                 40},
    {"MANA_PERCENT_ADD",                                    41},
    {"MANA_REPLENISH",                                      42},
    {"MANA_REPLENISH_PERCENT",                              43},
    {"MANA_REPLENISH_COEFFICIENT",                          44},
    {"MANA_REPLENISH_EXT",                                  45},
    {"CURRENT_RAGE",                                        46},
    {"MAX_RAGE",                                            47},
    {"RAGE_REPLENISH",                                      48},
    {"CURRENT_ENERGY",                                      49},
    {"MAX_ENERGY",                                          50},
    {"ENERGY_REPLENISH",                                    51},
    {"SUN_POWER_VALUE",                                     52},
    {"MOON_POWER_VALUE",                                    53},
    {"CURRENT_SUN_ENERGY",                                  54},
    {"MAX_SUN_ENERGY",                                      55},
    {"SUN_ENERGY_ADD_PERCENT",                              56},
    {"SUN_ENERGY_REPLENISH",                                57},
    {"SUN_ENERGY_FALL_OFF",                                 58},
    {"CURRENT_MOON_ENERGY",                                 59},
    {"MAX_MOON_ENERGY",                                     60},
    {"MOON_ENERGY_ADD_PERCENT",                             61},
    {"MOON_ENERGY_REPLENISH",                               62},
    {"MOON_ENERGY_FALL_OFF",                                63},
    {"CURRENT_STAMINA",                                     64},
    {"CURRENT_THEW",                                        65},
    {"ACTIVE_THREAT_COEFFICIENT",                           66},
    {"PASSIVE_THREAT_COEFFICIENT",                          67},
    {"DODGE",                                               68},
    {"DODGE_BASE_RATE",                                     69},
    {"PARRY_BASE_RATE",                                     70},
    {"PARRY_BASE",                                          71},
    {"PARRY_PERCENT",                                       72},
    {"PARRYVALUE_BASE",                                     73},
    {"PARRYVALUE_PERCENT",                                  74},
    {"SENSE",                                               75},
    {"STRAIN_BASE",                                         76},
    {"STRAIN_PERCENT",                                      77},
    {"TOUGHNESS_BASE_RATE",                                 78},
    {"TOUGHNESS_BASE",                                      79},
    {"TOUGHNESS_PERCENT",                                   80},
    {"DECRITICAL_DAMAGE_POWER_BASE_KILONUM_RATE",           81},
    {"DECRITICAL_DAMAGE_POWER_BASE",                        82},
    {"DECRITICAL_DAMAGE_POWER_PERCENT",                     83},
    {"POSITIVE_SHIELD",                                     84},
    {"NEGATIVE_SHIELD",                                     85},
    {"GLOBAL_RESIST_PERCENT",                               86},
    {"GLOBAL_BLOCK",                                        87},
    {"GLOBAL_THERAPY_ABSORB",                               88},
    {"GLOBAL_DAMAGE_ABSORB",                                89},
    {"GLOBAL_DAMAGE_ABSORB_BY_SELF_MAX_LIFE",               90},
    {"GLOBAL_DAMAGE_ABSORB_BY_SELF_SPIRIT",                 91},
    {"GLOBAL_DAMAGE_ABSORB_BY_SELF_AGILITY",                92},
    {"GLOBAL_DAMAGE_ABSORB_BY_SELF_VITALITY",               93},
    {"GLOBAL_DAMAGE_MANA_SHIELD",                           94},
    {"DAMAGE_TO_LIFE_FOR_SELF",                             95},
    {"DAMAGE_TO_MANA_FOR_SELF",                             96},
    {"BEAT_BACK_RATE",                                      97},
    {"KNOCKED_BACK_RATE",                                   98},
    {"KNOCKED_OFF_RATE",                                    99},
    {"KNOCKED_DOWN_RATE",                                   100},
    {"PULL_RATE",                                           101},
    {"REPULSED_RATE",                                       102},
    {"IMMORTAL",                                            103},
    {"MOUNTING_RATE",                                       104},
    {"MELEE_WEAPON_DAMAGE_BASE",                            105},
    {"MELEE_WEAPON_DAMAGE_PERCENT",                         106},
    {"MELEE_WEAPON_DAMAGE_RAND",                            107},
    {"MELEE_WEAPON_DAMAGE_RAND_PERCENT",                    108},
    {"RANGE_WEAPON_DAMAGE_BASE",                            109},
    {"RANGE_WEAPON_DAMAGE_PERCENT",                         110},
    {"RANGE_WEAPON_DAMAGE_RAND",                            111},
    {"RANGE_WEAPON_DAMAGE_RAND_PERCENT",                    112},
    {"SKILL_PHYSICS_DAMAGE",                                113},
    {"SKILL_PHYSICS_DAMAGE_RAND",                           114},
    {"SKILL_PHYSICS_DAMAGE_PERCENT",                        115},
    {"PHYSICS_DAMAGE_COEFFICIENT",                          116},
    {"PHYSICS_ATTACK_POWER_BASE",                           117},
    {"PHYSICS_ATTACK_POWER_PERCENT",                        118},
    {"PHYSICS_HIT_BASE_RATE",                               119},
    {"PHYSICS_HIT_VALUE",                                   120},
    {"PHYSICS_CRITICAL_STRIKE_BASE_RATE",                   121},
    {"PHYSICS_CRITICAL_STRIKE",                             122},
    {"PHYSICS_CRITICAL_DAMAGE_POWER_BASE_KILONUM_RATE",     123},
    {"PHYSICS_CRITICAL_DAMAGE_POWER_BASE",                  124},
    {"PHYSICS_CRITICAL_DAMAGE_POWER_PERCENT",               125},
    {"PHYSICS_OVERCOME_BASE",                               126},
    {"PHYSICS_OVERCOME_PERCENT",                            127},
    {"PHYSICS_RESIST_PERCENT",                              128},
    {"PHYSICS_DEFENCE_ADD",                                 129},
    {"PHYSICS_DEFENCE_MAX",                                 130},
    {"PHYSICS_SHIELD_BASE",                                 131},
    {"PHYSICS_SHIELD_PERCENT",                              132},
    {"PHYSICS_SHIELD_ADDITIONAL",                           133},
    {"PHYSICS_REFLECTION",                                  134},
    {"PHYSICS_REFLECTION_PERCENT",                          135},
    {"PHYSICS_BLOCK",                                       136},
    {"PHYSICS_DAMAGE_ABSORB",                               137},
    {"PHYSICS_DAMAGE_MANA_SHIELD",                          138},
    {"ALL_TYPE_HIT_VALUE",                                  139},
    {"ALL_TYPE_CRITICAL_STRIKE",                            140},
    {"ALL_TYPE_CRITICAL_DAMAGE_POWER_BASE",                 141},
    {"SKILL_MAGIC_DAMAGE",                                  142},
    {"MAGIC_ATTACK_POWER_BASE",                             143},
    {"MAGIC_ATTACK_POWER_PERCENT",                          144},
    {"MAGIC_CRITICAL_STRIKE",                               145},
    {"MAGIC_SHIELD",                                        146},
    {"MAGIC_CRITICAL_DAMAGE_POWER_BASE_KILONUM_RATE",       147},
    {"MAGIC_CRITICAL_DAMAGE_POWER_BASE",                    148},
    {"MAGIC_CRITICAL_DAMAGE_POWER_PERCENT",                 149},
    {"MAGIC_OVERCOME",                                      150},
    {"MAGIC_HIT_BASE_RATE",                                 151},
    {"MAGIC_HIT_VALUE",                                     152},
    {"SKILL_SOLAR_DAMAGE",                                  153},
    {"SKILL_SOLAR_DAMAGE_RAND",                             154},
    {"SKILL_SOLAR_DAMAGE_PERCENT",                          155},
    {"SOLAR_DAMAGE_COEFFICIENT",                            156},
    {"SOLAR_ATTACK_POWER_BASE",                             157},
    {"SOLAR_ATTACK_POWER_PERCENT",                          158},
    {"SOLAR_HIT_BASE_RATE",                                 159},
    {"SOLAR_HIT_VALUE",                                     160},
    {"SOLAR_CRITICAL_STRIKE_BASE_RATE",                     161},
    {"SOLAR_CRITICAL_STRIKE",                               162},
    {"SOLAR_CRITICAL_DAMAGE_POWER_BASE_KILONUM_RATE",       163},
    {"SOLAR_CRITICAL_DAMAGE_POWER_BASE",                    164},
    {"SOLAR_CRITICAL_DAMAGE_POWER_PERCENT",                 165},
    {"SOLAR_OVERCOME_BASE",                                 166},
    {"SOLAR_OVERCOME_PERCENT",                              167},
    {"SOLAR_MAGIC_RESIST_PERCENT",                          168},
    {"SOLAR_MAGIC_SHIELD_BASE",                             169},
    {"SOLAR_MAGIC_SHIELD_PERCENT",                          170},
    {"SOLAR_MAGIC_REFLECTION",                              171},
    {"SOLAR_MAGIC_REFLECTION_PERCENT",                      172},
    {"SOLAR_MAGIC_BLOCK",                                   173},
    {"SOLAR_DAMAGE_ABSORB",                                 174},
    {"SOLAR_DAMAGE_MANA_SHIELD",                            175},
    {"SKILL_NEUTRAL_DAMAGE",                                176},
    {"SKILL_NEUTRAL_DAMAGE_RAND",                           177},
    {"SKILL_NEUTRAL_DAMAGE_PERCENT",                        178},
    {"NEUTRAL_DAMAGE_COEFFICIENT",                          179},
    {"NEUTRAL_ATTACK_POWER_BASE",                           180},
    {"NEUTRAL_ATTACK_POWER_PERCNET",                        181},
    {"NEUTRAL_HIT_BASE_RATE",                               182},
    {"NEUTRAL_HIT_VALUE",                                   183},
    {"NEUTRAL_CRITICAL_STRIKE_BASE_RATE",                   184},
    {"NEUTRAL_CRITICAL_STRIKE",                             185},
    {"NEUTRAL_CRITICAL_DAMAGE_POWER_BASE_KILONUM_RATE",     186},
    {"NEUTRAL_CRITICAL_DAMAGE_POWER_BASE",                  187},
    {"NEUTRAL_CRITICAL_DAMAGE_POWER_PERCENT",               188},
    {"NEUTRAL_OVERCOME_BASE",                               189},
    {"NEUTRAL_OVERCOME_PERCENT",                            190},
    {"NEUTRAL_MAGIC_RESIST_PERCENT",                        191},
    {"NEUTRAL_MAGIC_SHIELD_BASE",                           192},
    {"NEUTRAL_MAGIC_SHIELD_PERCENT",                        193},
    {"NEUTRAL_MAGIC_REFLECTION",                            194},
    {"NEUTRAL_MAGIC_REFLECTION_PERCENT",                    195},
    {"NEUTRAL_MAGIC_BLOCK",                                 196},
    {"NEUTRAL_DAMAGE_ABSORB",                               197},
    {"NEUTRAL_DAMAGE_MANA_SHIELD",                          198},
    {"SKILL_LUNAR_DAMAGE",                                  199},
    {"SKILL_LUNAR_DAMAGE_RAND",                             200},
    {"SKILL_LUNAR_DAMAGE_PERCENT",                          201},
    {"LUNAR_DAMAGE_COEFFICIENT",                            202},
    {"LUNAR_ATTACK_POWER_BASE",                             203},
    {"LUNAR_ATTACK_POWER_PERCENT",                          204},
    {"LUNAR_HIT_BASE_RATE",                                 205},
    {"LUNAR_HIT_VALUE",                                     206},
    {"LUNAR_CRITICAL_STRIKE_BASE_RATE",                     207},
    {"LUNAR_CRITICAL_STRIKE",                               208},
    {"LUNAR_CRITICAL_DAMAGE_POWER_BASE_KILONUM_RATE",       209},
    {"LUNAR_CRITICAL_DAMAGE_POWER_BASE",                    210},
    {"LUNAR_CRITICAL_DAMAGE_POWER_PERCENT",                 211},
    {"LUNAR_OVERCOME_BASE",                                 212},
    {"LUNAR_OVERCOME_PERCENT",                              213},
    {"LUNAR_MAGIC_RESIST_PERCENT",                          214},
    {"LUNAR_MAGIC_SHIELD_BASE",                             215},
    {"LUNAR_MAGIC_SHIELD_PERCENT",                          216},
    {"LUNAR_MAGIC_REFLECTION",                              217},
    {"LUNAR_MAGIC_REFLECTION_PERCENT",                      218},
    {"LUNAR_MAGIC_BLOCK",                                   219},
    {"LUNAR_DAMAGE_ABSORB",                                 220},
    {"LUNAR_DAMAGE_MANA_SHIELD",                            221},
    {"SKILL_POISON_DAMAGE",                                 222},
    {"SKILL_POISON_DAMAGE_RAND",                            223},
    {"SKILL_POISON_DAMAGE_PERCENT",                         224},
    {"POISON_DAMAGE_COEFFICIENT",                           225},
    {"POISON_ATTACK_POWER_BASE",                            226},
    {"POISON_ATTACK_POWER_PERCENT",                         227},
    {"POISON_HIT_BASE_RATE",                                228},
    {"POISON_HIT_VALUE",                                    229},
    {"POISON_CRITICAL_STRIKE_BASE_RATE",                    230},
    {"POISON_CRITICAL_STRIKE",                              231},
    {"POISON_CRITICAL_DAMAGE_POWER_BASE_KILONUM_RATE",      232},
    {"POISON_CRITICAL_DAMAGE_POWER_BASE",                   233},
    {"POISON_CRITICAL_DAMAGE_POWER_PERCENT",                234},
    {"POISON_OVERCOME_BASE",                                235},
    {"POISON_OVERCOME_PERCENT",                             236},
    {"POISON_MAGIC_RESIST_PERCENT",                         237},
    {"POISON_MAGIC_SHIELD_BASE",                            238},
    {"POISON_MAGIC_SHIELD_PERCENT",                         239},
    {"POISON_MAGIC_REFLECTION",                             240},
    {"POISON_MAGIC_REFLECTION_PERCENT",                     241},
    {"POISON_MAGIC_BLOCK",                                  242},
    {"POISON_DAMAGE_ABSORB",                                243},
    {"POISON_DAMAGE_MANA_SHIELD",                           244},
    {"THERAPY_POWER_BASE",                                  245},
    {"THERAPY_POWER_PERCENT",                               246},
    {"SKILL_THERAPY",                                       247},
    {"SKILL_THERAPY_RAND",                                  248},
    {"SKILL_THERAPY_PERCENT",                               249},
    {"THERAPY_COEFFICIENT",                                 250},
    {"BE_THERAPY_COEFFICIENT",                              251},
    {"SOLAR_AND_LUNAR_ATTACK_POWER_BASE",                   252},
    {"SOLAR_AND_LUNAR_HIT_VALUE",                           253},
    {"SOLAR_AND_LUNAR_CRITICAL_STRIKE",                     254},
    {"SOLAR_AND_LUNAR_CRITICAL_DAMAGE_POWER_BASE",          255},
    {"SOLAR_AND_LUNAR_OVERCOME_BASE",                       256},
    {"NONE_WEAPON_ATTACK_SPEED_BASE",                       257},
    {"MELEE_WEAPON_ATTACK_SPEED_BASE",                      258},
    {"MELEE_WEAPON_ATTACK_SPEED_PERCENT_ADD",               259},
    {"MELEE_WEAPON_ATTACK_SPEED_ADDITIONAL",                260},
    {"RANGE_WEAPON_ATTACK_SPEED_BASE",                      261},
    {"RANGE_WEAPON_ATTACK_SPEED_PERCENT_ADD",               262},
    {"RANGE_WEAPON_ATTACK_SPEED_ADDITIONAL",                263},
    {"IGNORE_OTHER_ALARM_RANGE",                            264},
    {"SELF_ALARM_RANGE_REVISE",                             265},
    {"OTHER_ALARM_RANGE_REVISE",                            266},
    {"DASH",                                                268},
    {"DASH_TO_POINT",                                       269},
    {"DASH_FORWARD",                                        270},
    {"DASH_BACKWARD",                                       271},
    {"DASH_LEFT",                                           272},
    {"DASH_RIGHT",                                          273},
    {"DASH_TO_DST_BACK",                                    274},
    {"HALT",                                                275},
    {"FREEZE",                                              276},
    {"ENTRAP",                                              277},
    {"FEAR",                                                278},
    {"REVIVE",                                              279},
    {"JUMP",                                                280},
    {"SIT",                                                 281},
    {"DISARM",                                              282},
    {"DISABLE_MOVE_CTRL",                                   283},
    {"CALL_KNOCKED_BACK_EXHALE",                            284},
    {"CALL_KNOCKED_BACK_CONVERGENCE",                       285},
    {"CALL_KNOCKED_BACK_RANDOM",                            286},
    {"CALL_KNOCKED_BACK_SRC_FACE",                          287},
    {"CALL_KNOCKED_OFF_PARABOLA",                           288},
    {"CALL_KNOCKED_DOWN",                                   289},
    {"CALL_REPULSED",                                       290},
    {"CALL_THERAPY",                                        291},
    {"CALL_PHYSICS_DAMAGE",                                 292},
    {"CALL_SOLAR_DAMAGE",                                   293},
    {"CALL_NEUTRAL_DAMAGE",                                 294},
    {"CALL_LUNAR_DAMAGE",                                   295},
    {"CALL_POISON_DAMAGE",                                  296},
    {"CAST_SKILL_TARGET_SRC",                               297},
    {"CAST_SKILL_TARGET_DST",                               298},
    {"SKILL_EVENT_HANDLER",                                 299},
    {"PRACTICE_KUNG_FU",                                    300},
    {"ACCUMULATE",                                          301},
    {"CALL_BUFF",                                           302},
    {"CALL_BUFF_STACK_2",                                   303},
    {"CALL_BUFF_STACK_3",                                   304},
    {"CALL_BUFF_STACK_4",                                   305},
    {"CALL_BUFF_STACK_5",                                   306},
    {"CALL_BUFF_STACK_6",                                   307},
    {"CALL_BUFF_STACK_7",                                   308},
    {"CALL_BUFF_STACK_8",                                   309},
    {"CALL_BUFF_ACCORDING_TO_ATTRACTION",                   310},
    {"DETACH_SINGLE_BUFF",                                  311},
    {"DETACH_MULTI_GROUP_BUFF",                             312},
    {"DEL_SINGLE_BUFF_BY_FUNCTIONTYPE",                     313},
    {"DEL_MULTI_GROUP_BUFF_BY_FUNCTIONTYPE",                314},
    {"DEL_SINGLE_BUFF_BY_ID_AND_LEVEL",                     315},
    {"DEL_SINGLE_GROUP_BUFF_BY_ID_AND_LEVEL",               316},
    {"MODIFY_THREAT",                                       317},
    {"MODIFY_ALL_THREATEN",                                 318},
    {"MODIFY_THREAT_PERCENT",                               319},
    {"MODIFY_ALL_THREATEN_PERCENT",                         320},
    {"SPOOF",                                               321},
    {"SILENCE",                                             322},
    {"SILENCE_ALL",                                         323},
    {"IMMUNITY",                                            324},
    {"ADD_WEAK",                                            325},
    {"SHIFT_WEAK",                                          326},
    {"ADD_WEAK_SHIELD",                                     327},
    {"MODIFY_SKILL_CAST_FLAG",                              328},
    {"EXECUTE_SCRIPT",                                      329},
    {"PRINK_REPRESENT_ID",                                  330},
    {"PRINK_MODEL",                                         331},
    {"PRINK_FORCE",                                         332},
    {"RIDE_HORSE",                                          333},
    {"DO_ACTION",                                           334},
    {"ADD_EXP_PERCENT",                                     335},
    {"ADD_EXP_PERCENT_BY_QUEST",                            336},
    {"ADD_EXP_PERCENT_BY_SCRIPT",                           337},
    {"ADD_REPUTATION_PERCENT",                              338},
    {"ADD_JUSTICE_PERCENT",                                 339},
    {"ADD_PRESTIGE_PERCENT",                                340},
    {"ADD_TRAIN_PERCENT",                                   341},
    {"DISABLE_ACQUIRE_TITLE_POINT",                         342},
    {"ADD_CONTRIBUTION_PERCENT_FOR_ALL",                    343},
    {"ADD_PRESTIGE_PERCENT_FOR_ALL",                        344},
    {"EMPLOYER_CAST_SKILL_TARGET_SELF",                     345},
    {"EMPLOYER_CAST_SKILL_TARGET_DEST",                     346},
    {"CLEAR_COOL_DOWN",                                     347},
    {"RESET_COOL_DOWN",                                     348},
    {"TRANSFER_DAMAGE_TO_MANA_PERCENT",                     349},
    {"MODIFY_COST_MANA_PERCENT",                            350},
    {"MODIFY_COST_RAGE_PERCENT",                            351},
    {"MODIFY_COST_ENERGY_PERCENT",                          352},
    {"DIRECT_CAST_MASK",                                    353},
    {"PULL",                                                354},
    {"TURN_TO_SRC",                                         355},
    {"CONSUME_BUFF",                                        356},
    {"TOP_THREAT",                                          357},
    {"ZERO_THREAT",                                         358},
    {"FORMATION_EFFECT",                                    359},
    {"ADD_DAMAGE_BY_DST_MOVE_STATE",                        360},
    {"TRANSFER_DAMAGE",                                     361},
    {"TRANSFER_THERAPY",                                    362},
    {"TRANSFER_THREAT",                                     363},
    {"ADD_FELLOWSHIP_ATTRACTION",                           364},
    {"CALL_HELP",                                           365},
    {"CALL_REQUEST",                                        366},
    {"CATCH_CHARACTER",                                     367},
    {"TRANSMISSION",                                        368},
    {"VITALITY_TO_PARRY_VALUE_COF",                         369},
    {"VITALITY_TO_MAX_LIFE_COF",                            370},
    {"VITALITY_TO_PHYSICS_ATTACK_POWER_COF",                371},
    {"VITALITY_TO_SOLAR_ATTACK_POWER_COF",                  372},
    {"VITALITY_TO_LUNAR_ATTACK_POWER_COF",                  373},
    {"VITALITY_TO_NEUTRAL_ATTACK_POWER_COF",                374},
    {"VITALITY_TO_POISON_ATTACK_POWER_COF",                 375},
    {"VITALITY_TO_THERAPY_POWER_COF",                       376},
    {"VITALITY_TO_PHYSICS_SHIELD_COF",                      377},
    {"SPIRIT_TO_MAX_MANA_COF",                              378},
    {"SPIRIT_TO_MANA_REPLENISH_COF",                        379},
    {"SPIRIT_TO_THERAPY_POWER_COF",                         380},
    {"SPIRIT_TO_PHYSICS_ATTACK_POWER_COF",                  381},
    {"SPIRIT_TO_SOLAR_ATTACK_POWER_COF",                    382},
    {"SPIRIT_TO_LUNAR_ATTACK_POWER_COF",                    383},
    {"SPIRIT_TO_NEUTRAL_ATTACK_POWER_COF",                  384},
    {"SPIRIT_TO_POISON_ATTACK_POWER_COF",                   385},
    {"SPIRIT_TO_PHYSICS_CRITICAL_STRIKE_COF",               386},
    {"SPIRIT_TO_SOLAR_CRITICAL_STRIKE_COF",                 387},
    {"SPIRIT_TO_LUNAR_CRITICAL_STRIKE_COF",                 388},
    {"SPIRIT_TO_NEUTRAL_CRITICAL_STRIKE_COF",               389},
    {"SPIRIT_TO_POISON_CRITICAL_STRIKE_COF",                390},
    {"SPIRIT_TO_PHYSICS_CRITICAL_DAMAGE_POWER_COF",         391},
    {"SPIRIT_TO_SOLAR_CRITICAL_DAMAGE_POWER_COF",           392},
    {"SPIRIT_TO_LUNAR_CRITICAL_DAMAGE_POWER_COF",           393},
    {"SPIRIT_TO_NEUTRAL_CRITICAL_DAMAGE_POWER_COF",         394},
    {"SPIRIT_TO_POISON_CRITICAL_DAMAGE_POWER_COF",          395},
    {"SPIRIT_TO_SOLAR_MAGIC_SHIELD_COF",                    396},
    {"SPIRIT_TO_LUNAR_MAGIC_SHIELD_COF",                    397},
    {"SPIRIT_TO_NEUTRAL_MAGIC_SHIELD_COF",                  398},
    {"SPIRIT_TO_POISON_MAGIC_SHIELD_COF",                   399},
    {"SPUNK_TO_MANA_REPLENISH_COF",                         400},
    {"SPUNK_TO_PHYSICS_ATTACK_POWER_COF",                   401},
    {"SPUNK_TO_SOLAR_ATTACK_POWER_COF",                     402},
    {"SPUNK_TO_LUNAR_ATTACK_POWER_COF",                     403},
    {"SPUNK_TO_NEUTRAL_ATTACK_POWER_COF",                   404},
    {"SPUNK_TO_POISON_ATTACK_POWER_COF",                    405},
    {"SPUNK_TO_SOLAR_AND_LUNAR_ATTACK_POWER_COF",           406},
    {"STRENGTH_TO_PHYSICS_CRITICAL_STRIKE_COF",             407},
    {"STRENGTH_TO_PHYSICS_ATTACK_POWER_COF",                408},
    {"STRENGTH_TO_SOLAR_ATTACK_POWER_COF",                  409},
    {"STRENGTH_TO_LUNAR_ATTACK_POWER_COF",                  410},
    {"STRENGTH_TO_NEUTRAL_ATTACK_POWER_COF",                411},
    {"STRENGTH_TO_POISON_ATTACK_POWER_COF",                 412},
    {"AGILITY_TO_PHYSICS_CRITICAL_STRIKE_COF",              413},
    {"AGILITY_TO_POISON_CRITICAL_STRIKE_COF",               414},
    {"AGILITY_TO_PHYSICS_ATTACK_POWER_COF",                 415},
    {"AGILITY_TO_SOLAR_AND_LUNAR_ATTACK_POWER_COF",         416},
    {"AGILITY_TO_DODGE_COF",                                417},
    {"STEALTH",                                             418},
    {"COUNTER_STEALTH",                                     419},
    {"COUNTER_STEALTH_RANGE",                               420},
    {"TRANSPARENCY_VALUE",                                  421},
    {"AUTO_SELECT_TARGET",                                  422},
    {"CALL_MENTOR_BUFF",                                    423},
    {"MENTOR_FORMATION_EFFECT",                             424},
    {"ADJUST_PROFESSION_LEVEL",                             425},
    {"SET_USE_BIG_SWORD_FLAG",                              426},
    {"CHANGE_CARRIER_STATE",                                427},
    {"SET_INCOME_LIMIT_FLAG",                               428},
    {"SET_SALE_LIMIT_FLAG",                                 429},
    {"SET_DISABLE_SKILL_CAST_ON_ENTRAP_FLAG",               430},
    {"SET_EQUIPMENT_RECIPE",                                431},
    {"SET_TALENT_RECIPE",                                   432},
    {"TRANSFER_LIFE",                                       433},
    {"TRANSFER_MANA",                                       434},
    {"COPY_EFFECTIVE_THERAPY",                              435},
    {"AUTO_SELECT_TARGET_BY_LIFELESS",                      436},
    {"CAST_SUB_SKILL",                                      437},
    {"CAST_SKILL",                                          438},
    {"SET_DISABLE_TITLEPOINT_PRODUCE_FLAG",                 439},
    {"ADD_SPRINT_POWER_MAX",                                440},
    {"ADD_SPRINT_POWER_COST",                               441},
    {"ADD_SPRINT_POWER_REVIVE",                             442},
    {"ADD_SPRINT_POWER_COST_ON_WALL",                       443},
    {"ADD_SPRINT_POWER_COST_RUN_ON_WALL_EXTRA",             445},
    {"ADD_SPRINT_POWER_COST_STAND_ON_WALL",                 446},
    {"DISABLE_SPRINT_FLAG",                                 447},
    {"ADD_HORSE_SPRINT_POWER_MAX",                          448},
    {"ADD_HORSE_SPRINT_POWER_COST",                         449},
    {"ADD_HORSE_SPRINT_POWER_REVIVE",                       450},
    {"ENABLE_DOUBLE_RIDE",                                  451},
    {"CHANGE_SKILL_ICON",                                   452},
    {"ADD_SCALE",                                           453},

    {NULL,                                  0}
};

KLuaConst LUA_ATTRIBUTE_EFFECT_MODE[] =
{
    {"EFFECT_TO_SELF_AND_ROLLBACK",      aemEffectToSelfAndRollback},
    {"EFFECT_TO_SELF_NOT_ROLLBACK",      aemEffectToSelfNotRollback},    
    {"EFFECT_TO_DEST_AND_ROLLBACK",      aemEffectToDestAndRollback},
    {"EFFECT_TO_DEST_NOT_ROLLBACK",      aemEffectToDestNotRollback},
    {"EFFECT_TO_DEST_ON_MINOR_WEAK",     aemEffectToDestOnMinorWeak},
    {"EFFECT_TO_DEST_ON_MAJOR_WEAK",     aemEffectToDestOnMajorWeak},
    {"EFFECT_TO_DEST_ON_MAX_WEAK",       aemEffectToDestOnMaxWeak},
    {NULL,                               0}
};

KLuaConst LUA_SKILL_EFFECT_TYPE[] = 
{
    {"INVALID",     setInvalid},
    {"SKILL",       setSkill},
    {"BUFF",        setBuff},
    {"TOTAL",       setTotal},
    {NULL,          0}
};

KLuaConst LUA_SKILL_CAST_EFFECT_TYPE[] =
{
    {"INVALID",     scetInvalid},
    {"HARMFUL",     scetHarmful},
    {"NERTUAL",     scetNeutral},
    {"BENEFICIAL",  scetBeneficial},
    {NULL,          0}
};

KLuaConst LUA_ADD_ITEM_RESULT_CODE[] =
{
    {"INVALID",                         aircInvalid},
    {"SUCCESS",                         aircSuccess},
    {"FAILED",                          aircFailed},
    
    {"NOT_ENOUGH_FREE_ROOM",            aircNotEnoughFreeRoom},
    {"ITEM_AMOUNT_LIMITED",             aircItemAmountLimited},
    {"CANNOT_PUT_THAT_PLACE",           aircCannotPutThatPlace},

    {NULL,                              0}
};

KLuaConst LUA_SKILL_RESULT_CODE[] =
{
    {"SUCCESS",                         srcSuccess},
    {"FAILED",                          srcFailed},
    {"INVALID_CAST_MODE",               srcInvalidCastMode},    
    {"NOT_ENOUGH_LIFE",                 srcNotEnoughLife},
    {"NOT_ENOUGH_MANA",                 srcNotEnoughMana},
    {"NOT_ENOUGH_RAGE",                 srcNotEnoughRage},
    {"NOT_ENOUGH_STAMINA",              srcNotEnoughStamina},
    {"NOT_ENOUGH_ITEM",                 srcNotEnoughItem},
    {"NOT_ENOUGH_AMMO",                 srcNotEnoughAmmo},
    {"SKILL_NOT_READY",                 srcSkillNotReady},
    {"INVALID_SKILL",                   srcInvalidSkill},
    {"INVALID_TARGET",                  srcInvalidTarget},
    {"NO_TARGET",                       srcNoTarget},
    {"NEED_TARGET",                     srcNeedTarget},
    {"TOO_CLOSE_TARGET",                srcTooCloseTarget},
    {"TOO_FAR_TARGET",                  srcTooFarTarget},
    {"OUT_OF_ANGLE",                    srcOutOfAngle},
    {"TARGET_INVISIBLE",                srcTargetInvisible},
    {"WEAPON_ERROR",                    srcWeaponError},
    {"WEAPON_DESTROY",                  srcWeaponDestroy},
    {"AMMO_ERROR",                      srcAmmoError},
    {"NOT_EQUIT_AMMO",                  srcNotEquitAmmo},
    {"MOUNT_ERROR",                     srcMountError},
    {"IN_OTACTION",                     srcInOTAction},
    {"ON_SILENCE",                      srcOnSilence},
    {"NOT_FORMATION_LEADER",            srcNotFormationLeader},
    {"NOT_ENOUGH_MEMBER",               srcNotEnoughMember},
    {"NOT_START_ACCUMULATE",            srcNotStartAccumulate},
    {"BUFF_ERROR",                      srcBuffError},
    {"NOT_IN_FIGHT",                    srcNotInFight},
    {"MOVE_STATE_ERROR",                srcMoveStateError},
    {"ERROR_BY_HORSE",                  srcErrorByHorse},
    {"BUFF_INVALID",                    srcBuffInvalid},
    {"FORCE_EFFECT",                    srcForceEffect},
    {"BUFF_IMMUNITY",                   srcBuffImmunity},
    {"TARGET_LIFE_ERROR",               srcTargetLifeError},
    {"SELF_LIFE_ERROR",                 srcSelfLifeError},
    {"DST_MOVE_STATE_ERROR",            srcDstMoveStateError},
    {"NOT_PARTY",                       srcNotTeam},
    {"MAP_BAN",                         srcMapBan},

    {NULL,                              0}
};

KLuaConst LUA_CONST_SKILL_CAST_MODE[] =
{
    {"SECTOR",              scmSector},
    {"CASTER_AREA",         scmCasterArea},
    {"TARGET_AREA",         scmTargetArea},
    {"POINT_AREA",          scmPointArea},
    {"CASTER_SINGLE",       scmCasterSingle},
    {"TARGET_SINGLE",       scmTargetSingle},
    {"TARGET_CHAIN",        scmTargetChain},
    {"POINT",               scmPoint},
	{"ITEM",				scmItem},
    {"TARGET_LEADER",       scmTargetLeader},
    {"PARTY_AREA",          scmTeamArea},

    {NULL,                  0}
};

KLuaConst LUA_CONST_SKILL_FUNCTION_TYPE[] =
{

    {"NORMAL",              sftNormal},
    {"SLOW",                sftSlow},
    {"FEAR",                sftFear},
    {"HALT",                sftHalt},
    {"SILENCE",             sftSilence},
    {"CHAOS",               sftChaos},
    {"CHARM",               sftCharm},
    {"STUN",                sftStun},
    {"ENMITY",              sftEnmity},
    {"BLOODING",            sftBoolding},
    {"DAZE",                sftDaze},

    {"DAMAGE",              sftDamage},

	{NULL,                  0}
};


KLuaConst LUA_SKILL_KIND_TYPE[] =
{
    {"PHYSICS",             sktPhysics},     
    {"SOLAR_MAGIC",         sktSolarMagic},  
    {"NEUTRAL_MAGIC",       sktNeutralMagic},
    {"LUNAR_MAGIC",         sktLunarMagic},  
    {"POISON",              sktPoison},  
    {"LEAP",                sktLeap},  
    {"NONE",                sktNone},  

    {NULL,                  0}
};

KLuaConst LUA_SKILL_RESULT_TYPE[] = 
{
    {"PHYSICS_DAMAGE",          serPhysicsDamage},
    {"SOLAR_MAGIC_DAMAGE",      serSolarMagicDamage},
    {"NEUTRAL_MAGIC_DAMAGE",    serNeutralMagicDamage},
    {"LUNAR_MAGIC_DAMAGE",      serLunarMagicDamage},
    {"POISON_DAMAGE",           serPoisonDamage},
    {"REFLECTIED_DAMAGE",       serReflectiedDamage},   

    {"THERAPY",                 serTherapy}, 
    {"STEAL_LIFE",              serStealLife}, 
    {"ABSORB_DAMAGE",           serAbsorbDamage}, 
    {"SHIELD_DAMAGE",           serShieldDamage}, 
    {"PARRY_DAMAGE",            serParryDamage}, 
    {"INSIGHT_DAMAGE",          serInsightDamage}, 
    
    {NULL,                      0}
};

KLuaConst LUA_BUFF_COMPARE_FLAG[] =
{
    {"EQUAL",                   bcfEqual},
    {"GREATER_EQUAL",           bcfGreaterEqual},

    {NULL,                   0}
};

KLuaConst LUA_ROLE_TYPE[] =
{
    {"STANDARD_MALE",    	  rtStandardMale},    
    {"STANDARD_FEMALE",       rtStandardFemale},  
    {"STRONG_MALE",           rtStrongMale},      
    {"SEXY_FEMALE",           rtSexyFemale},      
    {"LITTLE_BOY",            rtLittleBoy},       
    {"LITTLE_GIRL",           rtLittleGirl},
  
    {NULL,                   0}
};

KLuaConst LUA_CONST_CHARACTER_ACTION_TYPE[] =
{
	{"DIALOGUE",	catDialogue},
	{"HOWL",		catHowl},

	{NULL,			0}
};

KLuaConst LUA_CONST_CHARACTER_OTACTION_TYPE[] =
{
	{"ACTION_IDLE",				otActionIdle},
	{"ACTION_SKILL_PREPARE",	otActionSkillPrepare},
	{"ACTION_SKILL_CHANNEL",	otActionSkillChannel},
	{"ACTION_RECIPE_PREPARE",	otActionRecipePrepare},
	{"ACTION_PICK_PREPARE",		otActionPickPrepare},
	{"ACTION_PICKING",			otActionPicking},

	{NULL,                      0}
};

KLuaConst LUA_CONST_PARTY_NOTIFY_CODE[] = 
{
    {"PNC_PLAYER_INVITE_NOT_EXIST",           epncPlayerInviteNotExist},
    {"PNC_PLAYER_APPLY_NOT_EXIST",            epncPlayerApplyNotExist},
    {"PNC_PLAYER_ALREAD_IN_YOUR_PARTY",       epncPlayerAlreadInYourTeam},
    {"PNC_PLAYER_ALREAD_IN_OTHER_PARTY",      epncPlayerAlreadInOtherTeam},
    {"PNC_YOU_ALREAD_IN_PARTY_STATE",         epncYouAlreadInTeamState},
    {"PNC_PLAYER_IS_BUSY",                    epncPlayerIsBusy},
    {"PNC_INVITATION_DENIED",                 epncInvitationDenied},
    {"PNC_APPLICATION_DENIED",                epncApplicationDenied},
    {"PNC_PLAYER_NOT_ONLINE",                 epncPlayerNotOnline},
    {"PNC_DEST_PARTY_IS_FULL",                epncDestTeamIsFull},
    {"PNC_YOUR_PARTY_IS_FULL",                epncYourTeamIsFull},
    {"PNC_INVITATION_OUT_OF_DATE",            epncInvitationOutOfDate},
    {"PNC_APPLICATION_OUT_OF_DATE",           epncApplicationOutOfDate},
    {"PNC_PARTY_CREATED",                     epncTeamCreated},
    {"PNC_PARTY_JOINED",                      epncTeamJoined},
    {"PNC_INVITATION_DONE",                   epncInvitationDone},
    {"PNC_APPLICATION_DONE",                  epncApplicationDone},
    {"PNC_INVITATION_REJECT",                 epncInvitationReject},  
    {"PNC_APPLICATION_REJECT",                epncApplicationReject}, 
    {"PNC_CAMP_ERROR",                        epncCampError},

	{NULL,                                      0}
};

KLuaConst LUA_CONST_TEAM_AUTHORITY_TYPE[] =
{
    {"LEADER",                      tatLeader},
    {"DISTRIBUTE",                  tatDistribute},
    {"MARK",                        tatMark},

    {NULL,                          0}
};


KLuaConst LUA_CONST_ITEM_LOOT_MODE[] = 
{

    {"INVALID",             ilmInvalid},
    {"FREE_FOR_ALL",        ilmFreeForAll},
    {"DISTRIBUTE",          ilmDistribute},
    {"GROUP_LOOT",          ilmGroupLoot},
    
    {NULL,					0}
};

KLuaConst LUA_CONST_DOODAD_KIND[] =
{
    {"INVALID",           dkInvalid},   
    {"NORMAL",            dkNormal},
    {"CORPSE",            dkCorpse},
    {"QUEST",             dkQuest},
	{"READ",			  dkRead},
	{"DIALOG",			  dkDialog},
	{"ACCEPT_QUEST",	  dkAcceptQuest},
    {"TREASURE",          dkTreasure},
    {"ORNAMENT",		  dkOrnament},
	{"CRAFT_TARGET",	  dkCraftTarget},
	{"CLIENT_ONLY",		  dkClientOnly},
	{"CHAIR",			  dkChair},
    {"GUIDE",             dkGuide},
    {"DOOR",              dkDoor},
    {"NPCDROP",           dkNpcDrop},

	{NULL,                0}
};

KLuaConst LUA_CONST_CHARACTER_KIND[] =
{
	{"INVALID",           ckInvalid},   
	{"PLAYER",            ckPlayer},
	{"FIGHT_NPC",         ckFightNpc},
	{"DIALOG_NPC",        ckDialogNpc},
	{"CANNOT_OPERATE_NPC",ckCannotOperateNpc},
	{"CLIENT_NPC",		  ckClientNpc},

	{NULL,               0}
};

KLuaConst LUA_CONST_RELATION_TYPE[] = 
{
	{"INVALID",		sortInvalid},
	{"NONE",		sortNone},
	{"SELF",		sortSelf},
	{"ALLY",		sortAlly},
	{"ENEMY",		sortEnemy},
	{"NEUTRALITY",	sortNeutrality},
	{"PARTY",		sortParty},
	{"ALL",			sortAll},

	{NULL,			0}
};

KLuaConst LUA_CONST_CAMP[] = 
{
	{"NEUTRAL",		cNeutral},
	{"GOOD",		cGood},
	{"EVIL",	    cEvil},

	{NULL,			0}
};

KLuaConst LUA_CONST_SHOP_SYSTEM_RESPOND_CODE[] =
{
    {"INVALID",                 ssrcInvalid},
    {"SELL_SUCCESS",            ssrcSellSuccess},
    {"BUY_SUCCESS",             ssrcBuySuccess},
    {"REPAIR_SUCCESS",          ssrcRepairSuccess},
    {"SELL_FAILED",             ssrcSellFailed},
    {"BUY_FAILED",              ssrcBuyFailed},
    {"REPAIR_FAILED",           ssrcRepairFailed},

    {"TOO_FAR_AWAY",            ssrcTooFarAway},
    {"NOT_ENOUGH_MONEY",        ssrcNotEnoughMoney},
    {"NOT_ENOUGH_PRESTIGE",     ssrcNotEnoughPrestige},
    {"NOT_ENOUGH_CONTRIBUTION", ssrcNotEnoughContribution},
    {"ACHIEVEMENT_RECORD_ERROR", ssrcAchievementRecordError},
    {"NOT_ENOUGH_ACHIEVEMENT_POINT", ssrcNotEnoughAchievementPoint},
    {"NOT_ENOUGH_REPATE",       ssrcNotEnoughRepate},
    {"NOT_ENOUGH_RANK",         ssrcNotEnoughRank},
    {"ITEM_SOLD_OUT",           ssrcItemSoldOut},
    {"BAG_FULL",                ssrcBagFull},
    {"ITEM_EXIST_LIMIT",        ssrcItemExistLimit},
    {"ITEM_NOT_NEED_REPAIR",    ssrcItemNotNeedRepair},
    {"NONE_ITEM_NEED_REPAIR",   ssrcNoneItemNeedRepair},
    {"CAN_NOT_SELL",            ssrcCanNotSell},
    {"ITEM_HAD_LIMIT",          ssrcItemHadLimit},
    {"ITEM_MODIFIED_WHILEBUY",  ssrcItemModifiedWhileBuy},
    {"YOU_DEATH",               ssrcYouDeath},
    {"ITEM_BROKEN",             ssrcItemBroken},
    {"ITEM_NOT_IN_PACKAGE",     ssrcItemNotInPackage},
    {"NOT_ENOUGH_ITEM",         ssrcNotEnoughItem},

    {"TOTAL",                   ssrcTotal},

    {NULL,                      0}
};

KLuaConst LUA_CONST_TRADING_RESPOND_CODE[] =
{
    {"INVALID",                     trcInvalid},
    {"SUCCESS",                     trcSuccess},
    {"FAILED",                      trcFailed},
    
    {"INVALID_INVITE",              trcInvalidInvite},
    {"INVALID_TARGET",              trcInvalidTarget},
    {"REFUSE_INVITE",               trcRefuseInvite},
    {"TARGET_NOT_IN_GAME",          trcTargetNotInGame},
    {"TARGET_BUSY",                 trcTargetBusy},
    {"YOU_BUSY",                    trcYouBusy},
    {"TOO_FAR",                     trcTooFar},
    {"TRADING_CANCEL",              trcTradingCancel},
    {"TRADING_CANCELED",            trcTradingCanceled},
    {"ITEM_BE_BOUND",               trcItemBeBound},
    {"NOT_ENOUGH_MONEY",            trcNotEnoughMoney},
    {"NOT_ENOUGH_FREE_ROOM",        trcNotEnoughFreeRoom},
    {"ITEM_EXIST_AMOUNT_LIMIT",     trcItemExistAmountLimit},
    {"YOU_DEAD",                    trcYouDead},
    {"ITEM_NOT_IN_PACKAGE",         trcItemNotInPackage},
    {"TARGET_DEAD",                 trcTargetDead},
    
    {"Total",                       trcTotal},
    {NULL,                          0}
};
#ifdef _SERVER

KLuaConst LUA_CONST_AI_EVENT_CONDITION[] = 
{
	{"SELF_HP_BELOW",		aecSelfHPBelow},
	{"TARGET_HP_BELOW",		aecTargetHPBelow},
	{"FRAME_INTERVAL",		aecFrameInterval},
	{"RANDOM",				aecRandom},

	{"TURN_TO_FIGHT",		aecTurnToFight},
	{"TURN_TO_IDLE",		aecTurnToIdle},
	{"RETURN_TO_ORIGIN",	aecReturnToOrigin},
	{"SELF_DEAD",			aecSelfDead},
	{"TARGET_DEAD",			aecTargetDead},
	
	{NULL,					0}
};

KLuaConst LUA_CONST_AI_EVENT_ACTION[] = 
{
	{"ESCAPE",				aeaEscape},
	{"ESCAPE_FOR_HELP",		aeaEscapeForHelp},
	{"CALL_HELP",			aeaCallHelp},
	{"DEATH",				aeaDeath},
	{"RUN_SCRIPT",			aeaRunScript},

	{NULL,					0}
};

#endif //_SERVER

KLuaConst LUA_CONST_REVIVE_TYPE[] =
{    
	{"IN_SITU",				    drcReviveInSitu},
	{"BY_PLAYER",				drcReviveByPlayer},
	{"IN_ALTAR",				drcReviveInAltar},
    {"CANCEL_BY_PLAYER",        drcCancelReviveByPlayer},
	{NULL,                      0}
};

KLuaConst LUA_CONST_BANISH_CODE[] =
{    
	{"MAP_REFRESH",				    bpcMapRefresh},
    {"MAP_UNLOAD",                  bpcMapUnload},
	{"NOT_IN_MAP_OWNER_PARTY",	    bpcNotInMapOwnerTeam},
	{"CANCEL_BANISH",				bpcCancelBanish},
	{NULL,                          0}
};

#if defined(_CLIENT)
KLuaConst LUA_CONST_LOGIN[] = 
{
	{ _T("UNABLE_TO_CONNECT_SERVER"),				LPUM_UNABLE_TO_CONNECT_SERVER },
	{ _T("MISS_CONNECTION"),						LPUM_MISS_CONNECTION },
	{ _T("SYSTEM_MAINTENANCE"),						LPUM_SYSTEM_MAINTENANCE },
	{ _T("UNMATCHED_PROTOCOL_VERSION"),				LPUM_UNMATCHED_PROTOCOL_VERSION },

    { _T("HANDSHAKE_SUCCESS"),                      LPUM_HANDSHAKE_SUCCESS},
    { _T("HANDSHAKE_ACCOUNT_SYSTEM_LOST"),          LPUM_HANDSHAKE_ACCOUNT_SYSTEM_LOST },

	{ _T("VERIFY_ALREADY_IN_GATEWAY"),				LPUM_LOGIN_VERIFY_ALREADY_IN_GATEWAY },
	{ _T("VERIFY_IN_GATEWAY_BLACK_LIST"),			LPUM_LOGIN_VERIFY_IN_GATEWAY_BLACK_LIST },
	{ _T("VERIFY_SUCCESS"),							LPUM_LOGIN_VERIFY_SUCCESS },
    { _T("NEED_MATRIX_PASSWORD"),                   LPUM_LOGIN_NEED_MATRIX_PASSWORD},
    { _T("NEED_TOKEN_PASSWORD"),                    LPUM_LOGIN_NEED_TOKEN_PASSWORD},
    { _T("MIBAO_SYSTEM_ERROR"),                     LPUM_LOGIN_MIBAO_SYSTEM},
    { _T("MATRIX_FAILED"),                          LPUM_LOGIN_MATRIX_FAILED},
    { _T("MATRIX_CARDINVALID"),                     LPUM_LOGIN_MATRIX_CARDINVALID},
    { _T("MATRIX_NOTFOUND"),                        LPUM_LOGIN_MATRIX_NOTFOUND},
    { _T("VERIFY_IN_GAME"),							LPUM_LOGIN_VERIFY_IN_GAME },
	{ _T("VERIFY_ACC_PSW_ERROR"),					LPUM_LOGIN_VERIFY_ACC_PSW_ERROR },
	{ _T("VERIFY_NO_MONEY"),						LPUM_LOGIN_VERIFY_NO_MONEY },
	{ _T("VERIFY_NOT_ACTIVE"),						LPUM_LOGIN_VERIFY_NOT_ACTIVE },
	{ _T("VERIFY_ACTIVATE_CODE_ERR"),				LPUM_LOGIN_VERIFY_ACTIVATE_CODE_ERR },
	{ _T("VERIFY_IN_OTHER_GROUP"),					LPUM_LOGIN_VERIFY_IN_OTHER_GROUP },
	{ _T("VERIFY_ACC_FREEZED"),						LPUM_LOGIN_VERIFY_ACC_FREEZED },
	{ _T("VERIFY_PAYSYS_BLACK_LIST"),				LPUM_LOGIN_VERIFY_PAYSYS_BLACK_LIST },
    { _T("VERIFY_LIMIT_ACCOUNT"),                   LPUM_LOGIN_VERIFY_LIMIT_ACCOUNT },
    { _T("VERIFY_ACC_SMS_LOCK"),                    LPUM_LOGIN_VERIFY_ACC_SMS_LOCK},
	{ _T("VERIFY_UNKNOWN_ERROR"),					LPUM_LOGIN_VERIFY_UNKNOWN_ERROR },

	{ _T("GET_ROLE_LIST_SUCCESS"),					LPUM_GET_ROLE_LIST_SUCCESS },
    { _T("GET_ALL_ROLE_LIST_SUCCESS"),              LPUM_GET_ALL_ROLE_LIST_SUCCESS },
	{ _T("UPDATE_HOMETOWN_LIST"),					LPUM_UPDATE_HOMETOWN_LIST },

	{ _T("CREATE_ROLE_SUCCESS"),					LPUM_CREATE_ROLE_SUCCESS },
    { _T("CREATE_ROLE_NAME_EXIST"),                 LPUM_CREATE_ROLE_NAME_EXIST },
    { _T("CREATE_ROLE_INVALID_NAME"),               LPUM_CREATE_ROLE_INVALID_NAME },
    { _T("CREATE_ROLE_NAME_TOO_LONG"),              LPUM_CREATE_ROLE_NAME_TOO_LONG },
    { _T("CREATE_ROLE_NAME_TOO_SHORT"),             LPUM_CREATE_ROLE_NAME_TOO_SHORT },
    { _T("CREATE_ROLE_UNABLE_TO_CREATE"),           LPUM_CREATE_ROLE_UNABLE_TO_CREATE},

	{ _T("REQUEST_LOGIN_GAME_SUCCESS"),				LPUM_REQUEST_LOGIN_GAME_SUCCESS },
	{ _T("REQUEST_LOGIN_GAME_OVERLOAD"),			LPUM_REQUEST_LOGIN_GAME_OVERLOAD },
	{ _T("REQUEST_LOGIN_GAME_MAINTENANCE"),			LPUM_REQUEST_LOGIN_GAME_MAINTENANCE },
    { _T("REQUEST_LOGIN_GAME_ROLEFREEZE"),          LPUM_REQUEST_LOGIN_GAME_ROLEFREEZE},
	{ _T("REQUEST_LOGIN_GAME_UNKNOWN_ERROR"),		LPUM_REQUEST_LOGIN_GAME_UNKNOWN_ERROR },

	{ _T("DELETE_ROLE_SUCCESS"),					LPUM_DELETE_ROLE_SUCCESS },
	{ _T("DELETE_ROLE_DELAY"),				        LPUM_DELETE_ROLE_DELAY },
	{ _T("DELETE_ROLE_TONG_MASTER"),				LPUM_DELETE_ROLE_TONG_MASTER },
    { _T("DELETE_ROLE_FREEZE_ROLE"),				LPUM_DELETE_ROLE_FREEZE_ROLE },
	{ _T("DELETE_ROLE_UNKNOWN_ERROR"),				LPUM_DELETE_ROLE_UNKNOWN_ERROR },

    { _T("GIVEUP_QUEUE_SUCCESS"),                   LPUM_GIVEUP_QUEUE_SUCCESS },
    { _T("GIVEUP_QUEUE_ERROR"),                     LPUM_GIVEUP_QUEUE_ERROR },

    { _T("RENAME_SUCCESS"),                         LPUM_RENAME_SUCCESS},
    { _T("RENAME_NAME_ALREADY_EXIST"),              LPUM_RENAME_NAME_ALREADY_EXIST},
    { _T("RENAME_NAME_TOO_LONG"),                   LPUM_RENAME_NAME_TOO_LONG},
    { _T("RENAME_NAME_TOO_SHORT"),                  LPUM_RENAME_NAME_TOO_SHORT},
    { _T("RENAME_NEW_NAME_ERROR"),                  LPUM_RENAME_NEW_NAME_ERROR},
    { _T("RENAME_ERROR"),                           LPUM_RENAME_ERROR},
    
    { _T("SNDA_VERIFY_SUCCESS"),                    LPUM_SNDA_VERIFY_SUCCESS},
    { _T("SNDA_VERIFY_FAILED"),                     LPUM_SNDA_VERIFY_FAILED},

	{ NULL,											NULL },
};
#endif	//_CLIENT

KLuaConst LUA_CONST_MOVE_STATE[] = 
{
    {"INVALID",		    cmsInvalid},
	{"ON_STAND",        cmsOnStand},
	{"ON_WALK",	        cmsOnWalk},
	{"ON_RUN",		    cmsOnRun},
	{"ON_JUMP",         cmsOnJump},
    {"ON_SWIM_JUMP",    cmsOnSwimJump},
    {"ON_SWIM",		    cmsOnSwim},
    {"ON_FLOAT",        cmsOnFloat},
	{"ON_SIT",	        cmsOnSit},

	{"ON_KNOCKED_DOWN", cmsOnKnockedDown},
	{"ON_KNOCKED_BACK", cmsOnKnockedBack},
	{"ON_KNOCKED_OFF",	cmsOnKnockedOff},
	{"ON_HALT",		    cmsOnHalt},
    {"ON_FREEZE",       cmsOnFreeze},
    {"ON_ENTRAP",       cmsOnEntrap},
    {"ON_AUTO_FLY",     cmsOnAutoFly},

	{"ON_DEATH",	    cmsOnDeath},
    {"ON_DASH",	        cmsOnDash},
    {"ON_PULL",         cmsOnPull},
    {"ON_REPULSED",     cmsOnRepulsed},

    {"ON_RISE",         cmsOnRise},
    {"ON_SKID",         cmsOnSkid},

	{NULL,			0}
};

KLuaConst LUA_CONST_PLAYER_TALK_CHANNEL[] =
{
    {"INVALID",         trInvalid},

    {"NEARBY",          trNearby},
    {"TEAM",            trTeam},
    {"PARTY",           trRaid},
    {"BATTLE_FIELD",    trBattleField},
    {"SENCE",           trSence},
    {"WHISPER",         trWhisper},
    {"FACE",            trFace},

    {"LOCAL_SYS",       trLocalSys},
    {"GLOBAL_SYS",      trGlobalSys},
    {"GM_MESSAGE",      trGmMessage},

    {"NPC_NEARBY",      trNpcNearby},
    {"NPC_PARTY",       trNpcTeam},
    {"NPC_SENCE",       trNpcSence},
    {"NPC_WHISPER",     trNpcWhisper},
    {"NPC_SAY_TO",      trNpcSayTo},
    {"NPC_YELL_TO",     trNpcYellTo},
    {"NPC_FACE",        trNpcFace},
    {"NPC_SAY_TO_ID",   trNpcSayToTargetUseSentenceID},
    {"NPC_SAY_TO_CAMP", trNpcSayToCamp},

    {"TONG",            trTong},
    {"WORLD",           trWorld},
    {"FORCE",           trForce},
    {"CAMP",            trCamp},
    {"MENTOR",          trMentor},
    {"FRIENDS",         trFriends},

    {"CHANNEL1",        trCustomCh1},
    {"CHANNEL2",        trCustomCh2},
    {"CHANNEL3",        trCustomCh3},
    {"CHANNEL4",        trCustomCh4},
    {"CHANNEL5",        trCustomCh5},
    {"CHANNEL6",        trCustomCh6},
    {"CHANNEL7",        trCustomCh7},
    {"CHANNEL8",        trCustomCh8},

    {"DEBUG_THREAT",    trDebugThreat},

    {NULL,              0}
};

KLuaConst LUA_CONST_PLAYER_TALK_ERROR[] =
{
    {"PLAYER_NOT_FOUND",            TALK_ERROR_E_PLAYER_NOT_FOUND}, 
    {"NOT_IN_PARTY",                TALK_ERROR_E_NOT_IN_PARTY}, 
    {"NOT_IN_SENCE",                TALK_ERROR_E_NOT_IN_SENCE}, 
    {"PLAYER_OFFLINE",              TALK_ERROR_E_PLAYER_OFFLINE}, 
    {"YOU_BLACKLIST_TARGET",        TALK_ERROR_E_YOU_BLACKLIST_TARGET}, 
    {"TARGET_BLACKLIST_YOU",        TALK_ERROR_E_TARGET_BLACKLIST_YOU}, 
    {"BAN",                         TALK_ERROR_E_BAN}, 
    {"SCENE_CD",                    TALK_ERROR_E_CD}, 
    {"NOT_IN_TONG",                 TALK_ERROR_E_NOT_IN_TONG}, 
    {"TONG_CAN_NOT_SPEAK",          TALK_ERROR_E_TONG_CAN_NOT_SPEAK}, 
    {"DAILY_LIMIT",                 TALK_ERROR_E_DAILY_LIMIT}, 
    {"NOT_IN_FORCE",                TALK_ERROR_E_NOT_IN_FORCE},
    {"CHARGE_LIMIT",                TALK_ERROR_E_CHARGE_LIMIT},

    {NULL,                          0}
};

KLuaConst LUA_CONST_MAIL_RESPOND_CODE[] = 
{
    {"SUCCEED",                     mrcSucceed},
    {"FAILED",                      mrcFailed},

    {"SYSTEM_BUSY",                 mrcSystemBusy},
    {"DST_NOT_EXIST",               mrcDstNotExist},
    {"NOT_ENOUGH_MONEY",            mrcNotEnoughMoney},
    {"ITEM_AMOUNT_LIMIT",           mrcItemAmountLimit},
    {"NOT_ENOUGH_ROOM",             mrcNotEnoughRoom},
    {"MONEY_LIMIT",                 mrcMoneyLimit},
    {"MAIL_NOT_FOUND",              mrcMailNotFound},
    {"MAIL_BOX_FULL",               mrcMailBoxFull},
    {"RETURN_MAIL_FAILED",          mrcReturnMailFailed},
    {"ITEM_BE_BIND",                mrcItemBeBind},
    {"TIME_LIMIT_ITEM",             mrcTimeLimitItem},
    {"ITEM_NOT_IN_PACKAGE",         mrcItemNotInPackage},
    {"DST_NOT_SELF",                mrcDstNotSelf},
    {"DELETE_REFUSED",              mrcDeleteRefused},
    {"SELF_MAIL_BOX_FULL",          mrcSelfMailBoxFull},
    {"TOO_FAR_AWAY",                mrcTooFarAway},

    {"TOTAL",                       mrcTotal},
    {NULL,                          0}
};

KLuaConst LUA_CONST_PLAYER_FELLOWSHIP_RESPOND[] =
{
    {"INVALID",     rrcInvalid},

    {"SUCCESS",                     rrcSuccess},
    {"SUCCESS_ADD",                 rrcSuccessAdd},
    {"SUCCESS_ADD_FOE",             rrcSuccessAddFoe},
    {"SUCCESS_ADD_BLACK_LIST",      rrcSuccessAddBlackList},

    {"ERROR",                       rrcFailed},

    {"ERROR_INVALID_NAME",          rrcInvalidName},
    {"ERROR_ADD_SELF",              rrcAddSelf},
    {"ERROR_LIST_FULL",             rrcFellowshipListFull},
    {"ERROR_EXISTS",                rrcFellowshipExists},
    {"ERROR_NOT_FOUND",             rrcFellowshipNotFound},

    {"ERROR_FOE_LIST_FULL",         rrcFoeListFull},
    {"ERROR_FOE_EXISTS",            rrcFoeExists},
    {"ERROR_FOE_NOT_FOUND",         rrcFoeNotFound},

    {"ERROR_BLACK_LIST_FULL",       rrcBlackListFull},
    {"ERROR_BLACK_LIST_EXISTS",     rrcBlackListExists},
    {"ERROR_BLACK_LIST_NOT_FOUND",  rrcBlackListNotFound},

    {"ERROR_INVALID_GROUP_NAME",    rrcInvalidGroupName},
    {"ERROR_INVALID_REMARK",        rrcInvalidRemark},
    {"ERROR_GROUP_FULL",            rrcFellowshipGroupFull},
    {"ERROR_GROUP_NOT_FOUND",       rrcFellowshipGroupNotFound},

    {"ERROR_IN_FIGHT",              rrcInFight},

    {"SUCCESS_BE_ADD_FRIEND",       rrcBeAddFriend},
    {"SUCCESS_BE_ADD_FOE",          rrcBeAddFoe},
    {"SUCCESS_BE_ADD_BLACK_LIST",   rrcBeAddBlackList},
    
    {NULL,          0}
};

KLuaConst LUA_CONST_DATE_FORMAT[] =
{
	{"SECOND",			dfSecond},
	{"MINUTE",			dfMinute},
	{"HOUR",			dfHour},
	{"DAY",				dfDay},
	{"MONTH",			dfMonth},
	{"YEAR",			dfYear},
	{"WEEK",			dfWeek},
    {NULL,              0}
};

KLuaConst LUA_CONST_SKILL_ATTACK_WEAK_POS[] =
{
    {"UPPER",           swpUpper},
    {"MIDDLE",          swpMiddle},
    {"LOWER",           swpLower},
    {NULL,              0}
};

KLuaConst LUA_CONST_ERROR_CODE_TYPE[] =
{
	{"INVALID",					ectInvalid},

	{"SKILL_ERROR_CODE",		ectSkillErrorCode},
	{"LOGIN_ERROR_CODE",		ectLoginErrorCode},
	{"QUEST_ERROR_CODE",		ectQuestErrorCode},
	{"FELLOW_SHIP_ERROR_CODE",	ectFellowshipErrorCode},
	{"ITEM_ERROR_CODE",			ectItemErrorCode},
	{"TRADING_ERROR_CODE",		ectTradingErrorCode},
	{"TALK_ERROR_CODE",			ectTalkErrorCode},
	{"CRAFT_ERROR_CODE",		ectCraftErrorCode},
	{"LOOT_ITEM_ERROR_CODE",	ectLootItemErrorCode},
	{"MAIL_ERROR_CODE",			ectMailErrorCode},
    {"USE_ITEM_ERROR_CODE",     ectUseItemErrorCode},
    {"ADD_ITEM_ERROR_CODE",     ectAddItemErrorCode},
    {"SHARE_QUEST_RESULT_CODE", ectShareQuestErrorCode},

	{NULL,						0}
};

KLuaConst LUA_CONST_PEEK_OTHER_PLAYER_RESPOND[] = 
{
    {"INVALID",                 prcInvalid},

    {"SUCCESS",                 prcSuccess},
    {"FAILED",                  prcFailed},
    {"CAN_NOT_FIND_PLAYER",     prcCanNotFindPlayer}, 
    {"TOO_FAR",                 prcTooFar},

    {NULL,                      0}
};

KLuaConst LUA_CONST_MAIL_TYPE[] = 
{
    {"PLAYER",					eMailType_Player},
    {"SYSTEM",                  eMailType_System},
    {"AUCTION",					eMailType_Auction}, 
    {"GM_MESSAGE",              eMailType_GmMessage},
	{"PLAYER_MESSAGE",			eMailType_PlayerMessage},

    {NULL,                      0}
};

KLuaConst LUA_CONST_PK_STATE[] =
{
	{"INVALID",                 pksInvalid},

	{"IDLE",					pksIdle},
	{"WAIT_FOR_DUEL_RESPOND",	pksWaitForDuelRespond},
	{"CONFIRM_DUEL",			pksConfirmDuel},
	{"PREPARE_DUEL",	        pksPrepareDuel},
	{"DUELING", 				pksDueling},
	{"DUEL_OUTRANGE",   		pksDuelOutOfRange},
	
    {"START_SLAY",				pksStartSlay},
    {"SLAYING",					pksSlaying},
	{"EXITSLAY",			    pksExitSlay},

	{NULL,						0}
};

KLuaConst LUA_CONST_PK_RESPOND[] =
{
	{"INVALID",					pkrcInvalid},

	{"SUCCESS",					pkrcSuccess},
	{"FAILED",					pkrcFailed},
	{"APPLY_DUEL_FAILED",		pkrcApplyDuelFailed},
    {"TARGET_IS_DUELING",       pkrcTargetIsDueling},
	{"APPLY_SLAY_FAILED",   	pkrcApplySlayFailed},
    {"LEVEL_TOO_LOW",           pkrcLevelTooLow},
    {"PK_NOT_READY",            pkrcPKNotReady},
    {"IN_SAME_PARTY",           pkrcInSameTeam},

	{NULL, 0}
};

KLuaConst LUA_CONST_PK_ENDCODE[] =
{
    {"KICK_OUT",            pkecBeat},
    {"LOGOUT",              pkecOffline},
    {"UNEXPECTED_DIE",      pkecUnexpectedDie},
    {"SWITCH_MAP",          pkecSwitchMap},
    {"OUT_OF_RANGE",        pkecOutofRangeTimeUp},

    {NULL, 0}
};

KLuaConst LUA_PREEMPTIVE_ATTACK[] =
{
    {"INVALID",             paInvalid},

    {"SELF",                paSelf},
    {"TARGET",              paTarget},

    {"TOTAL",               paTotal},
    {NULL, 0}
};

KLuaConst LUA_AI_THREAT_TYPE[] =
{
    {"INVALID",                         thtInvalid},

    {"MAIN_THREAT",                     thtMainThreat},
    {"BASE_THREAT",                     thtBaseThreat},
    {"DAMAGE_THREAT",                   thtDamageThreat},
    {"THERAPY_THREAT",                  thtTherapyThreat},
    {"SPECIAL_THREAT",                  thtSpecialThreat},

    {"TOTAL",                           thtTotal},
    {NULL, 0}
};

KLuaConst LUA_AI_TARGET_TYPE[] =
{
    {"INVALID",                         aittInvalid},

    {"SELECT_TARGET",                   aittSelectTarget},
    {"SKILL_TARGET",                    aittSkillTarget},

    {"LAST_SELECT_TARGET",              aittLastSelectTarget},
    {"LAST_SKILL_TARGET",               aittLastSkillTarget},
    {"ATTACK_TARGET",                   aittAttackTarget},

    {"CUSTOM_TARGET_1",                 aittCustomTarget1},
    {"CUSTOM_TARGET_2",                 aittCustomTarget2},
    {"CUSTOM_TARGET_3",                 aittCustomTarget3},
    {"CUSTOM_TARGET_4",                 aittCustomTarget4},

    {"TOTAL",                           aittTotal},
    {NULL, 0}
};

KLuaConst LUA_AI_FILTER_TYPE[] =
{
    {"INVALID",                         airtInvalid},

    {"BASE",                            airtBaseTarget},
    {"RANGE",                           airtRangeTarget},
    {"NEAREST",                         airtNearestTarget},

    {"TOTAL",                           airtTotal},
    {NULL, 0}
};

KLuaConst LUA_AI_ACTION_KEY[] = 
{
    {"INVALID",                                 eakInvalid},

    {"SET_STATE",                               eakSetState},
    {"SET_PRIMARY_TIMER",                       eakSetPrimaryTimer},
    {"SET_SECONDARY_TIMER",                     eakSetSecondaryTimer},
    {"RANDOM_BI_BRANCH",                        eakRandomBiBranch},
    {"RANDOM_TRI_BRANCH",                       eakRandomTriBranch},
    {"STAND",                                   eakStand},
     
    {"SEARCH_ENEMY",                            eakSearchEnemy},
    {"ADD_TARGET_TO_THREAT_LIST",               eakAddTargetToThreatList},
    {"SELECT_FIRST_THREAT",                     eakSelectFirstThreat},
    {"IS_IN_FIGHT",                             eakIsInFight},
    {"KEEP_RANGE",                              eakKeepRange},
    {"CAST_SKILL",                              eakCastSkill},
    {"SET_SYSTEM_SHEILD",                       eakSetSystemShield},
    {"RECORD_RETURN_POSITION",                  eakRecordReturnPosition},
    {"RETURN",                                  eakReturn},
    {"ENTER_PATROL_PATH",                       eakEnterPatrolPath},
    {"LEAVE_PATROL_PATH",                       eakLeavePatrolPath},
    {"DO_PATROL",                               eakPatrol},
    {"RECORD_ORIGIN_POSITION",                  eakRecordOriginPosition},
    {"KEEP_ORIGIN_DIRECTION",                   eakKeepOriginDirection},
    {"FACE_TARGET",                             eakFaceTarget},
    {"WANDER",                                  eakWander},
    {"SEARCH_PLAYER",                           eakSearchPlayer},
    {"SELECT_EVENT_SRC",                        eakSelectEventSrc},
    {"NPC_OPEN_DIALOG_WINDOW",                  eakNpcOpenDialogWindow},
    {"NPC_SAY_TO_TARGET",                       eakNpcSayToTarget},
    {"CHECK_LIFE",                              eakCheckLife},
    {"NPC_INIT_SKILL_CD",                       eakNpcInitSkillCD},
    {"NPC_STANDARD_SKILL_SELECTOR",             eakNpcStandardSkillSelector},
    {"NPC_KEEP_SKILL_CAST_RANGE",               eakNpcKeepSkillCastRange},
    {"NPC_CAST_SELECT_SKILL",                   eakNpcCastSelectSkill},
    {"DO_ANIMATION",                            eakDoAnimation},
    {"GROUP_REVIVE",                            eakGroupRevive},
    {"IS_TARGET_IN_RANGE",                      eakIsTargetInRange},
    {"CLEAN_BUFF_BY_FUNCTION",                  eakCleanBuffByFunction},
    {"NEED_ESCAPE",                             eakNeedEscape},
    {"PARAM_COMPARE",                           eakParamCompare},
    {"SELECT_STATE",                            eakSelectState},
    {"RANDOM_RUN",                              eakRandomRun},
    {"CLEAN_DEBUFF",                            eakCleanDebuff},
    {"SEARCH_MULTI_PLAYER_ADD_THREAT_LIST",     eakSearchMultiPlayerAddThreatList},
    {"CHECK_SKILL_FAILED_COUNTER",              eakCheckSkillFailedCounter},
    {"NPC_SET_SEC_TIMER_BY_SKILL_ANI",          eakNpcSetSecTimerBySkillAni},
    {"NPC_SET_SEC_TIMER_BY_SKILL_REST",         eakNpcSetSecTimerBySkillRest},   
    {"IS_TARGET_EXIST",                         eakIsTargetExist},
    {"IS_TARGE_IN_WATER",                       eakIsTargeInWater},
    {"NPC_CLEAR_SELECT_SKILL_CD",               eakNpcClearSelectSkillCD},
    {"NPC_RESET_SELECT_SKILL_CD",               eakNpcResetSelectSkillCD},
    {"IS_NEED_KEEP_RANGE",                      eakIsNeedKeepRange},
    {"SET_TERTIARY_TIMER",                      eakSetTertiaryTimer},
    {"SEARCH_PLAYER_WITHOUT_ANY_AFFECT",        eakSearchPlayerWithoutAnyAffect},
    {"CLEAR_THREAT_FOR_TARGET",                 eakClearThreatForTarget},
    {"SELECT_NTH_THREAT",                       eakSelectNthThreat},
    {"FILTER_THREAT",                           eakFilterThreat},
    {"CHANGE_TARGET",                           eakChangeTarget},
    {"SET_TARGET",                              eakSetTarget},
    {"CLEAR_TOTAL_DAMAGE_VALUE",                eakClearTotalDamageValue},
    {"TOTAL_DAMAGE_VALUE_COMPARE",              eakTotalDamageValueCompare},
    {"SEARCH_BUSTUP_PLAYER_ADD_THREAT_LIST",    eakSearchBustupPlayerAddThreatList},
    {"COPY_THREATLIST_TO_TARGETGROUP",          eakCopyThreatListToTargetGroup},
    {"EXTRACTION_TARGETGROUP",                  eakExtractionTargetGroup},
    {"RETURN_POSITION_DISTANCE_COMPARE",        eakReturnPositionDistanceCompare},
    {"IS_OTACTION_STATE",                       eakIsOTActionState},

    {"TOTAL",                                   eakTotal},
    {NULL, 0}
};

KLuaConst LUA_AI_EVENT[] =
{
    {"INVALID",                         aevInvalid},

    {"ON_PRIMARY_TIMER",                aevOnPrimaryTimer},
    {"ON_SECONDARY_TIMER",              aevOnSecondaryTimer},
    {"ON_TERTIARY_TIMER",               aevOnTertiaryTimer},
    {"ON_PATH_SUCCESS",                 aevOnPathSuccess},
    {"ON_PATH_FAILED",                  aevOnPathFailed},
    {"ON_DIALOG",                       aevOnDialog},
    {"ON_ACTION",                       aevOnAction},
    {"ON_ATTACKED",                     aevOnAttacked},
    {"ON_SELF_KILLED",                  aevOnSelfKilled},
    {"ON_TARGET_KILLED",                aevOnTargetKilled},
    {"ON_IN_FEAR",                      aevOnInFear},
    {"ON_OUT_FEAR",                     aevOnOutFear},
    {"ON_BEGIN_PREPARE",                aevOnBeginPrepare},
    {"ON_BREAK_PREPARE",                aevOnBreakPerpare},
    {"ON_CAST_SKILL",                   aevOnCastSkill},
    {"ON_CAST_SKILL_FAILED",            aevOnCastSkillFailed},
    {"ON_OUT_OF_BATTLE_ZONE",           aevOnOutOfBattleZone},

    {"TOTAL",                           aevTotal},
    {NULL, 0}
};

KLuaConst LUA_SWITCH_MAP[] = 
{
    {"INVALID",					        smrcInvalid},

	{"SUCCESS",					        smrcSuccess},
	{"FAILED",					        smrcFailed},
	{"MAP_CREATING",    		        smrcMapCreating},
    {"ENTER_NEW_COPY_TOO_MANY_TIMES",   smrcEnterNewCopyTooManyTimes},
    {"GAME_MAINTENANCE",                smrcGameMaintenance},
	{"GAME_OVERLOADED",	                smrcGameOverLoaded},
	{"CREATE_MAP_FINISHED",   	        smrcCreateMapFinished},
    {"MAPCOPY_COUNT_LIMIT",             smrcMapCopyCountLimit},
    
	{NULL, 0}
};

KLuaConst LUA_BATTLE_FIELD_NOTIFY_TYPE[] =
{
    {"INVALID",                     bfntInvalid},

    {"QUEUE_INFO",                  bfntQueueInfo},
    {"JOIN_BATTLE_FIELD",           bfntJoinBattleField},
    {"LEAVE_BATTLE_FIELD",          bfntLeaveBattleField},
    {"IN_BLACK_LIST",               bfntInBlackList},
    {"LEAVE_BLACK_LIST",            bfntLeaveBlackList},

    {NULL, 0}
};

KLuaConst LUA_BATTLE_FIELD_RESULT_CODE[] =
{
    {"INVALID",                     bfrcInvalid},
    {"SUCCESS",                     bfrcSuccess},
    {"FAILED",                      bfrcFailed},
    {"IN_BLACK_LIST",               bfrcInBlackList},
    {"LEVEL_ERROR",                 bfrcLevelError},
    {"FORCE_ERROR",                 bfrcForceError},
    {"TEAM_MEMBER_ERROR",           bfrcTeamMemberError},
    {"TEAM_SIZE_ERROR",             bfrcTeamSizeError},
    {"TOO_MANY_JOIN",               bfrcTooManyJoin},
    {"NOT_IN_SAME_MAP",             bfrcNotInSameMap},
    {"CAMP_ERROR",                  bfrcCampError},
    {"TIME_ERROR",                  bfrcTimeError},

    {NULL, 0}
};

KLuaConst LUA_SHARE_QUEST_RESULT_CODE[] =
{
    {"INVALID",                     sqrcInvalid},
    {"SUCCESS",                     sqrcSuccess},
    {"FAILED",                      sqrcFailed},
    
    {"TOO_FAR",                     sqrcTooFar},
    {"QUEST_CAN_NOT_SHARE",         sqrcQuestCannotShare},
    {"NOT_IN_PARTY",                sqrcNotInTeam},
    {"ALREADY_ACCEPT_QUEST",        sqrcAlreadyAcceptQuest},
    {"ALREADY_FINISHED_QUEST",      sqrcAlreadyFinishedQuest},
    {"QUEST_LIST_FULL",             sqrcQuestListFull},
    {"ACCEPT_QUEST",                sqrcAcceptQuest},
    {"REFUSE_QUEST",                sqrcRefuseQuest},

    {NULL,                          0}
};

KLuaConst LUA_SKILL_RECIPE_RESULT_CODE[] =
{
    {"INVALID",                     srrInvalid},

    {"SUCCESS",                     srrSuccess},
    {"FAILED",                      srrFailed},
    {"ERROR_IN_FIGHT",              srrErrorInFight},

    {NULL,                          0}
};

KLuaConst LUA_PQ_STATISTICS_INDEX[] =
{
    {"KILL_COUNT",                  psiKillCount},
    {"DECAPITATE_COUNT",            psiDecapitateCount},
    {"SOLO_COUNT",                  psiSoloCount},
    {"HARM_OUTPUT",                 psiHarmOutput},
    {"TREAT_OUTPUT",                psiTreatOutput},
    {"INJURY",                      psiInjury},
    {"DEATH_COUNT",                 psiDeathCount},
    {"THREAT_OUTPUT",               psiThreatOutput},
    {"SKILL_MARK",                  psiSkillMark},
    {"SPECIAL_OP_1",                psiSpecialOp1},
    {"SPECIAL_OP_2",                psiSpecialOp2},
    {"SPECIAL_OP_3",                psiSpecialOp3},
    {"SPECIAL_OP_4",                psiSpecialOp4},
    {"FINAL_MARK",                  psiFinalMark},
    {"AWARD_MONEY",                 psiAwardMoney},
    {"AWARD_EXP",                   psiAwardExp},
    {"AWARD_1",                     psiAward1},
    {"AWARD_2",                     psiAward2},
    {"AWARD_3",                     psiAward3},

    {NULL,                          0}
};

#if defined(_CLIENT)
KLuaConst LUA_TONG_OPERATION_INDEX[] =
{
    {"SPEAK",                       totSpeak},
    {"JOIN_MANAGER_CHANNEL",        totJoinManagerChannel},
    {"SPEAK_IN_MANAGER_CHANNEL",    totSpeakInManagerChannel},
    {"CHECK_OPERATION_RECORD",      totCheckOperationRecord},
    {"MODIFY_ANNOUNCEMENT",         totModifyAnnouncement},
    {"MODIFY_ONLINE_MESSAGE",       totModifyOnlineMessage},
    {"MODIFY_INTRODUCTION",         totModifyIntroduction},
    {"MODIFY_RULES",                totModifyRules},
    {"MODIFY_MEMORABILIA",          totModifyMemorabilia},
    {"DEVELOP_TECHNOLOGY",          totDevelopTechnology},
    {"PRODUCT_ITEM",                totProductItem},

    {"PUT_REPOSITORY_PAGE2",        totPutRepositoryPage2},
    {"PUT_REPOSITORY_PAGE3",        totPutRepositoryPage3},
    {"PUT_REPOSITORY_PAGE4",        totPutRepositoryPage4},
    {"PUT_REPOSITORY_PAGE5",        totPutRepositoryPage5},
    {"PUT_REPOSITORY_PAGE6",        totPutRepositoryPage6},
    {"PUT_REPOSITORY_PAGE7",        totPutRepositoryPage7},
    {"PUT_REPOSITORY_PAGE8",        totPutRepositoryPage8},
    {"PUT_REPOSITORY_PAGE9",        totPutRepositoryPage9},
    {"PUT_REPOSITORY_PAGE10",       totPutRepositoryPage10},

    {"GET_FROM_REPOSITORY_PAGE2",   totGetFromRepositoryPage2},
    {"GET_FROM_REPOSITORY_PAGE3",   totGetFromRepositoryPage3},
    {"GET_FROM_REPOSITORY_PAGE4",   totGetFromRepositoryPage4},
    {"GET_FROM_REPOSITORY_PAGE5",   totGetFromRepositoryPage5},
    {"GET_FROM_REPOSITORY_PAGE6",   totGetFromRepositoryPage6},
    {"GET_FROM_REPOSITORY_PAGE7",   totGetFromRepositoryPage7},
    {"GET_FROM_REPOSITORY_PAGE8",   totGetFromRepositoryPage8},
    {"GET_FROM_REPOSITORY_PAGE9",   totGetFromRepositoryPage9},
    {"GET_FROM_REPOSITORY_PAGE10",  totGetFromRepositoryPage10},

    {"MODIFY_CALENDAR",             totModifyCalendar},
    {"MANAGE_TASK",                 totManageTask},
    {"MANAGE_VOTE",                 totManageVote},
    {"ADD_TO_GROUP",                totAddToGroup},
    {"MOVE_FROM_GROUP",             totMoveFromGroup},
    {"MODIFY_WAGE",                 totModifyWage},
    {"PAY_SALARY",                  totPaySalary},
    {"MODIFY_MEMBER_REMARK",        totModifyMemberRemark},
    {"MODIFY_GROUP_NAME",           totModifyGroupName},

    {NULL,                          0}
};

KLuaConst LUA_TONG_EVENT_CODE[] =
{
    {"INVITE_SUCCESS",                                      tenInviteSuccess},
    {"INVITE_NAME_NOT_FOUND_ERROR",                         tenInviteNameNotFoundError},
    {"INVITE_TARGET_ALREADY_JOIN_TONG_ERROR",               tenInviteTargetAlreadyJoinTongError},
    {"INVITE_TARGET_BUSY_ERROR",                            tenInviteTargetBusyError},
    {"INVITE_TARGET_OFFLINE_ERROR",                         tenInviteTargetOfflineError},
    {"INVITE_NO_PERMISSION_ERROR",                          tenInviteNoPermissionError}, 
    {"INVITE_INVAILD_ERROR",                                tenInviteInvaildError},
    {"INVITE_TIME_OUT_ERROR",                               tenInviteTimeOutError},
    {"INVITE_TARGET_REFUSE",                                tenInviteTargetRefuse},
    {"INVITE_TARGET_CHARGE_LIMIT",                          tenInviteTargetChargeLimit},

    {"INVITE_MEMBER_NUMBER_LIMIT_ERROR",                    tenInviteMemberNumberLimitError},

    {"KICK_OUT_SUCCESS",                                    tenKickOutSuccess},
    {"KICK_OUT_TARGET_NOT_FOUND_ERROR",                     tenKickOutTargetNotFoundError},
    {"KICK_OUT_TARGET_NO_PERMISSION_ERROR",                 tenKickOutTargetNoPermissionError},

    {"MODIFY_TONG_NAME_SUCCESS",                            tenModifyTongNameSuccess},
    {"MODIFY_TONG_NAME_ILLEGAL_ERROR",                      tenModifyTongNameIllegalError},
    {"MODIFY_TONG_NAME_NO_PERMISSION_ERROR",                tenModifyTongNameNoPermissionError},

    {"MODIFY_ANNOUNCEMENT_SUCCESS",                         tenModifyAnnouncementSuccess},
    {"MODIFY_ANNOUNCEMENT_NOPERMISSIONERROR",               tenModifyAnnouncementNoPermissionError},

    {"MODIFY_ONLINE_MESSAGE_SUCCESS",                       tenModifyOnlineMessageSuccess},
    {"MODIFY_ONLINE_MESSAGE_NO_PERMISSION_ERROR",           tenModifyOnlineMessageNoPermissionError},

    {"MODIFY_INTRODUCTION_SUCCESS",                         tenModifyIntroductionSuccess},
    {"MODIFY_INTRODUCTION_NO_PERMISSION_ERROR",             tenModifyIntroductionNoPermissionError},

    {"MODIFY_RULES_SUCCESS",                                tenModifyRulesSuccess},
    {"MODIFY_RULES_NO_PERMISSION_ERROR",                    tenModifyRulesNoPermissionError},

    {"MODIFY_GROUP_NAME_SUCCESS",                           tenModifyGroupNameSuccess},
    {"MODIFY_GROUP_NAME_NO_PERMISSION_ERROR",               tenModifyGroupNameNoPermissionError},

    {"MODIFY_BASE_OPERATION_MASK_SUCCESS",                  tenModifyBaseOperationMaskSuccess},
    {"MODIFY_BASE_OPERATION_MASK_NO_PERMISSION_ERROR",      tenModifyBaseOperationMaskNoPermissionError},

    {"MODIFY_ADVANCE_OPERATION_MASK_SUCCESS",               tenModifyAdvanceOperationMaskSuccess},
    {"MODIFY_ADVANCE_OPERATION_MASK_NO_PERMISSION_ERROR",   tenModifyAdvanceOperationMaskNoPermissionError},

    {"MODIFY_GROUP_WAGE_SUCCESS",                           tenModifyGroupWageSuccess},
    {"MODIFY_GROUP_WAGE_NO_PERMISSION_ERROR",               tenModifyGroupWageNoPermissionError},

    {"MODIFY_MEMORABILIA_SUCCESS",                          tenModifyMemorabiliaSuccess},
    {"MODIFY_MEMORABILIA_ERROR",                            tenModifyMemorabiliaError},

    {"QUIT_NOT_IN_TONG_ERROR",                              tenQuitNotInTongError},
    {"QUIT_IS_MASTER_ERROR",                                tenQuitIsMasterError},

    {"CHANGE_MEMBER_GROUP_SUCCESS",                         tenChangeMemberGroupSuccess},
    {"CHANGE_MEMBER_GROUP_ERROR",                           tenChangeMemberGroupError},
     
    {"CHANGE_MASTER_SUCCESS",                               tenChangeMasterSuccess},
    {"CHANGE_MASTER_ERROR",                                 tenChangeMasterError},
     
    {"CHANGE_MEMBER_REMARK_SUCCESS",                        tenChangeMemberRemarkSuccess},
    {"CHANGE_MEMBER_REMARK_ERROR",                          tenChangeMemberRemarkError},
        
    {"INVITE_TARGET_IN_RIVAL_CAMP_ERROR",                   tenInviteTargetInRivalCampError},
    {"TARGET_TONG_IS_RIVAL_CAMP_ERROR",                     tenTargetTongIsRivalCampError},

    {"CHANGE_CAMP_PERMISSION_ERROR",                        tenChangeCampPermissionError},
    {"CHANGE_CAMP_LIMIT_ERROR",                             tenChangeCampLimitError},

    {"CREATE_TONG_NAME_ILLEGAL_ERROR",                      tenCreateTongNameIllegalError},
    {"CREATE_TONG_ALREADY_IN_TONG_ERROR",                   tenCreateTongAlreadyInTongError},
    {"CREATE_TONG_NAME_CONFLICT_ERROR",                     tenCreateTongNameConflictError},

    {"REPERTORY_GRID_FILLED_ERROR",                         tenRepertoryGridFilledError},
    {"REPERTORY_PAGE_FULL_ERROR",                           tenRepertoryPageFullError},
    {"PUT_ITEM_IN_REPERTORY_SUCCESS",                       tenPutItemInRepertorySuccess},
    {"ITEM_NOT_IN_REPERTORY_ERROR",                         tenItemNotInRepertoryError},
    {"TAKE_ITEM_FROM_REPERTORY_SUCCESS",                    tenTakeItemFromRepertorySuccess},
    {"EXCHANGE_REPERTORY_ITEM_SUCCESS",                     tenExchangeRepertoryItemSuccess},

    {"TAKE_REPERTORY_ITEM_PERMISSION_DENY_ERROR",           tenTakeRepertoryItemPermissionDenyError},
    {"PUT_ITEM_TO_REPERTORY_PERMISSION_DENY_ERROR",         tenPutItemToRepertoryPermissionDenyError},
    {"STACK_ITEM_TO_REPERTORY_FAIL_ERROR",                  tenStackItemToRepertoryFailError},

    {"SAVE_MONEY_SUCCESS",                                  tenSaveMoneySuccess},
    {"SAVE_MONEY_TOO_MUSH_ERROR",                           tenSaveMoneyTooMushError},

    {"PAY_SALARY_SUCCESS",                                  tenPaySalarySuccess},
    {"PAY_SALARY_FAILED_ERROR",                             tenPaySalaryFailedError},

    {"RENAME_SUCCESS",                                      tenRenameSuccess},
    {"RENAME_NO_RIGHT_ERROR",                               tenRenameNoRightError},
    {"RENAME_UNNECESSARY_ERROR",                            tenRenameUnnecessaryError},
    {"RENAME_CONFLICT_ERROR",                               tenRenameConflictError},
    {"RENAME_ILLEGAL_ERROR",                                tenRenameIllegalError},

    {NULL,                                                  0}
};

KLuaConst LUA_AUCTION_SALE_STATE[] =
{
    {"IGNORE",              assIgnore},
    {"SOMEONE_BID",         assSomeoneBid},
    {"NO_ONE_BID",          assNoOneBid},

    {NULL,                  0}
};

KLuaConst LUA_AUCTION_ORDER_TYPE[] =
{
    {"QUALITY",             aotQuality},
    {"LEVEL",               aotLevel},
    {"LEFT_TIME",           aotLeftTime},
    {"PRICE",               aotPrice},
    {"BUY_IT_NOW_PRICE",    aotBuyItNowPrice},

    {NULL,                  0}
};

KLuaConst LUA_AUCTION_RESPOND_CODE[] =
{
    {"SUCCEED",             arcSucceed},
    {"ITEM_NOT_EXIST",      arcItemNotExist},
    {"PRICE_TOO_LOW",       arcPriceTooLow},
    {"CANNOT_CANCEL",       arcCannotCancel},
    {"SERVER_BUSY",         arcServerBusy},
    {"MAILBOX_FULL",        arcMailBoxFull},
    {"UNKNOWN_ERROR",       arcUnknownError},

    {NULL,                  0}
};

KLuaConst LUA_AUCTION_MESSAGE_CODE[] =
{
    {"ACQUIRE_ITEM",        amcAcquireItem},
    {"BID_LOST",            amcBidLost},
    {"SOMEONE_BID",         amcSomeoneBid},
    {"ITEM_SOLD",           amcItemSold},
    {"TIME_OVER",           amcTimeOver},

    {NULL,                  0}
};

KLuaConst LUA_GAME_CARD_TYPE[] =
{
    {"MONTH_CARD",  gctMonthCard},
    {"POINT_CARD",  gctPointCard},
    {"DAY_CARD",    gctDayCard},

    {NULL, 0}
};

KLuaConst LUA_GAME_CARD_RESPOND_CODE[] =
{
    {"SUCCEED",                 gorSucceed},
    {"FREEZE_COIN_FAILED",      gorFreezeCoinFailed},
    {"EXCHANGE_FAILED",         gorExchangeFailed},
    {"UNFREEZE_COIN_FAILED",    gorUnFreezeCoinFailed},
    {"SYSTEM_ERROR",            gorSystemError},
    {"UNKNOWN_ERROR",           gorUnknownError},

    {NULL, 0}
};

KLuaConst LUA_GAME_CARD_ORDER_TYPE[] =
{
    {"BY_GAME_TIME",    gotByGameTime},
    {"BY_END_TIME",     gotByEndTime},
    {"BY_PRICE",        gotByPrice},

    {NULL, 0}
};
#endif // _CLIENT

KLuaConst LUA_CAMP_RESULT_CODE[] =
{
    {"FAILD",               crtFaild},
    {"SUCCEED",             crtSuccess},
    {"TONG_CONFLICT",       crtTongConflict},
    {"IN_PARTY",            crtInTeam},

    {NULL,                  0}
};

KLuaConst LUA_CHARGE_LIMIT_CODE[] =
{
    {"INVALID",             cltInvalid},
    {"TALK_WORLD",          cltTalkWorld},
    {"TALK_CAMP",           cltTalkCamp},
    {"YOU_TRADING",         cltYouTrading},
    {"TARGET_TRADING",      cltTargetTrading},
    {"MAIL",                cltMail},
    {"AUCTION_BID",         cltAuctionBid},
    {"AUCTION_SELL",        cltAuctionSell},
    {"YOU_JOIN_TONG",       cltYouJoinTong},
    {"TARGET_JOIN_TONG",    cltTargetJoinTong},
    {"LOGIN_MESSAGE",       cltLoginMessage},

    {NULL,                  0}
};

KLuaConst LUA_TONG_STATE[] =
{
    {"TRIAL",               tsTrialTong},
    {"NORMAL",              tsNormalTong},
    {"DISBAND",             tsDisbandTong},

    {NULL,                  0}
};

KLuaConst LUA_TONG_CHANGE_REASON[] =
{
    {"CREATE",   ctrCreate},
    {"DISBAND",  ctrDisband},
    {"JOIN",     ctrJoin},
    {"QUIT",     ctrQuit},
    {"FIRED",    ctrFired},

    {NULL,          0}
};

KLuaConst LUA_MAP_TYPE[] =
{
    {"NORMAL_MAP",          emtNormalMap},
    {"DUNGEON",             emtDungeon},
    {"BATTLE_FIELD",        emtBattleField},
    {"BIRTH_MAP",           emtBirthMap},

    {NULL,                  0}
};

KLuaConst LUA_MAP_CAMP_TYPE[] = 
{
    {"ALL_PROTECT",     emctAllProtect},
    {"PROTECT_GOOD",    emctProtectGood},
    {"PROTECT_EVIL",    emctProtectEvil},
    {"NEUTRAL",         emctNeutral},    
    {"FIGHT",           emctFight},

    {NULL,              0}
};

KLuaConst LUA_ACHIEVEMENT_ANNOUNCE_TYPE[] =
{
    {"NEARBY",  aatNearby},
    {"SCENE",   aatScene},
    {"GLOBAL",  aatGlobal},

    {NULL,  0}
};

KLuaConst LUA_DESIGNATION_ANNOUNCE_TYPE[] =
{
    {"NEARBY", datNearby},
    {"SCENE",  datScene},
    {"GLOBAL", datGlobal},

    {NULL, 0}
};

KLuaConst LUA_ENCHANT_RESULT_CODE[] =
{
    {"SUCCESS",             ercSuccess},
    {"PACKAGE_IS_FULL",     ercPackageIsFull},
    {"CAN_NOT_UNMOUNT",     ercCannotUnMount},
    {"FAILED",              ercFailed},

    {NULL, 0}
};

KLuaConst LUA_ENCHANT_INDEX[] =
{
    {"PERMANENT_ENCHANT",             eiPermanentEnchant},
    {"TEMPORARY_ENCHANT",             eiTemporaryEnchant},
    {"MOUNT1",                        eiMount1},
    {"MOUNT2",                        eiMount2},
    {"MOUNT3",                        eiMount3},
    {"MOUNT4",                        eiMount4},

    {NULL, 0}
};

KLuaConst LUA_TONG_HISTORY_TYPE[] = 
{
    {"MEMBER_MANAGE",   ohtMemberManage},
    {"FUND",            ohtFund},
    {"TECH_TREE",       ohtTechTree},
    {"ACTIVITY",        ohtActivity},
    {"REPERTORY",       ohtRepertory},

    {NULL, 0}
};

KLuaConst LUA_TONG_HISTORY_RECORD_TYPE[] = 
{
    {"MEMBER_MANAGE_BEGIN",         hrtMemberManageBegin},

    {"CHANGE_GROUP_BASE_RIGHT",     hrtChangeGroupBaseRight},
    {"CHANGE_GROUP_ADVANCE_RIGHT",  hrtChangeGroupAdvanceRight},
    {"CHANGE_MEMBER_GROUP",         hrtChangeMemberGroup},
    {"JOIN",                        hrtJoin},
    {"QUIT",                        hrtQuit},
    {"KICK_OUT",                    hrtKickOut},

    {"MEMBER_MANAGE_END",           hrtMemberManageEnd},
// ------------------------
    {"FUND_BEGIN",                  hrtFundBegin},

    {"CHANGE_GROUP_WAGE",           hrtChangeGroupWage},
    {"PAY_SALARY",                  hrtPaySalary},
    {"SAVE_FUND",                   hrtSaveFund},

    {"FUND_END",                    hrtFundEnd},
// ------------------------
    {"TECH_TREE_BEGIN",             hrtTechTreeBegin},

    {"ACTIVATE_TECHTREE",           hrtActivateTechtree},
    {"PRODUCT_ITEM",                hrtProductItem},

    {"TECH_TREE_END",               hrtTechTreeEnd},
// ------------------------
    {"ACTIVITY_BEGIN",              hrtActivityBegin},

    {"MISSION_START",               hrtMissionStart},
    {"MISSION_COMPLETE",            hrtMissionComplete},
    {"MISSION_FAILED",              hrtMissionFailed},
    {"WAR_START",                   hrtWarStart},
    {"WAR_WIN",                     hrtWarWin},
    {"WAR_LOSE",                    hrtWarLose},
    {"ACHIEVEMENT",                 hrtAchievement},

    {"ACTIVITY_END",                hrtActivityEnd},
// ------------------------
    {"REPERTORY_BEGIN",             hrtRepertoryBegin},

    {"REPERTORY_TAKE_ITEM",         hrtRepertoryTakeItem},
    {"REPERTORY_PUT_ITEM",          hrtRepertoryPutItem},

    {"REPERTORY_END",               hrtRepertoryEnd},
// ------------------------
    {NULL,      0}
};

KLuaConst LUA_MENTOR_RECORD_STATE[] = 
{
    {"INVALID",              emrsInvalid},
    {"NORMAL",               emrsNormal},
    {"MENTOR_BREAK",         emrsMBreak},
    {"APPRENTICE_BREAK",     emrsABreak},
    {"BROKEN",               emrsBroken},
    {"GRADUATING",           emrsGraduating},
    {"GRADUATED",            emrsGraduated},

    {NULL, 0}
};

KLuaConst LUA_CHARACTER_GENDER[] = 
{
    {"MALE",            cgMale},
    {"FEMALE",          cgFemale},
    {"GAY",             cgGay},

    {NULL,              0}
};

KLuaConst LUA_ITEM_EXIST_TYPE[] = 
{
    {"INVALID",             ketInvalid},

    {"PERMANENT",           ketPermanent},
    {"OFFLINE",             ketOffLine},
    {"ONLINE",              ketOnLine},
    {"ONLINEANDOFFLINE",    ketOnlineAndOffLine},

    {NULL,                  0}
};

/* [drift 2.5.2] DIAMOND_SUB_TYPE (from exe LUA array @0x084c9500) */
KLuaConst LUA_DIAMOND_SUB_TYPE[] =
{
    {"INVALID",     -1},
    {"GOLD",         0},
    {"WOOD",         1},
    {"WATER",        2},
    {"FIRE",         3},
    {"EARTH",        4},
    {"TOTAL",        5},
    {NULL,           0}
};

#ifdef _SERVER
KLuaConst LUA_ACTIVITY_STATE[] =
{
    {"NORMAL_OFF", 0},
    {"DELAY_OFF",  1},
    {"NORMAL_ON",  2},
    {"RECOVER_ON", 3},
    {NULL,          0}
};
#endif

KLuaConstList g_LuaConstList[] =
{
    {"GLOBAL",                      LUA_CONST_GLOBAL},
    {"PATROL",				        LUA_CONST_PATROL_REPEAT_STYLE},
    {"TARGET",				        LUA_CONST_TARGET_TYPE},
    {"ATTRIBUTE_TYPE",              LUA_ATTRIBUTE_TYPE},
    {"DIAMOND_SUB_TYPE",            LUA_DIAMOND_SUB_TYPE},  /* [drift 2.5.2] */
    {"ATTRIBUTE_EFFECT_MODE",       LUA_ATTRIBUTE_EFFECT_MODE},
	{"ITEM_TABLE_TYPE",			    LUA_CONST_ITEM_TABLE_TYPE},
    {"SKILL_EFFECT_TYPE",           LUA_SKILL_EFFECT_TYPE},
    {"SKILL_CAST_EFFECT_TYPE",      LUA_SKILL_CAST_EFFECT_TYPE},
    {"ADD_ITEM_RESULT_CODE",        LUA_ADD_ITEM_RESULT_CODE},
    {"SKILL_RESULT_CODE",           LUA_SKILL_RESULT_CODE},
	{"SKILL_FUNCTION_TYPE",         LUA_CONST_SKILL_FUNCTION_TYPE},
    {"SKILL_KIND_TYPE",             LUA_SKILL_KIND_TYPE},
    {"SKILL_RESULT_TYPE",           LUA_SKILL_RESULT_TYPE},
    {"BUFF_COMPARE_FLAG",           LUA_BUFF_COMPARE_FLAG},
    {"ROLE_TYPE",                   LUA_ROLE_TYPE},
	{"QUEST_STATE",				    LUA_CONST_QUEST_STATE},
	{"QUEST_RESULT",			    LUA_CONST_QUEST_RESULT_CODE},
	{"QUEST_EVENT_TYPE",		    LUA_CONST_QUEST_EVENT_TYPE},
    {"QUEST_DIFFICULTY_LEVEL",      LUA_CONST_QUEST_DIFFICULTY_LEVEL},
	{"QUEST_COUNT",				    LUA_CONST_QUEST_COUNT},
    {"ALL_CRAFT_TYPE",              LUA_CONST_ALL_CRAFT_TYPE},
	{"MINI_RADAR_TYPE",				LUA_CONST_MINI_RADAR_TYPE},
	{"INVENTORY_INDEX",			    LUA_CONST_INVENTORY_INDEX},
	{"LOOT_ITEM",				    LUA_CONST_LOOT_ITEM},
	{"CRAFT_RESULT_CODE",		    LUA_CONST_CRAFT_RESULT_CODE},
	{"CHARACTER_ACTION_TYPE",	    LUA_CONST_CHARACTER_ACTION_TYPE},
	{"CHARACTER_OTACTION_TYPE",     LUA_CONST_CHARACTER_OTACTION_TYPE},
#ifdef _SERVER
	{"AI_EVENT_CONDITION",		    LUA_CONST_AI_EVENT_CONDITION},
	{"AI_EVENT_ACTION",			    LUA_CONST_AI_EVENT_ACTION},
#endif //_SERVER

#ifdef _SERVER
    {"ACTIVITY_STATE",               LUA_ACTIVITY_STATE},
#endif

    {"ITEM_GENRE",                  LUA_CONST_ITEM_GENRE},
    {"EQUIPMENT_SUB",               LUA_CONST_EQUIPMENT_SUB},
    {"WEAPON_DETAIL",               LUA_CONST_WEAPON_DETAIL},
    {"INVENTORY_TYPE",              LUA_CONST_INVENTORY_TYPE},
    {"ITEM_BIND",                   LUA_CONST_ITEM_BIND},
    {"ITEM_USE_TARGET_TYPE",        LUA_CONST_ITEM_USE_TARGET_TYPE},
    {"EQUIPMENT_INVENTORY",         LUA_CONST_EQUIPMENT_INVENTORY},
    {"EQUIPMENT_REPRESENT",         LUA_CONST_EQUIPMENT_REPRESENT},
    {"ITEM_RESULT_CODE",            LUA_CONST_ITEM_RESULT_CODE},
    {"USE_ITEM_RESULT_CODE",        LUA_CONST_USE_ITEM_RESULT_CODE},
    {"SKILL_CAST_MODE",             LUA_CONST_SKILL_CAST_MODE},
    {"PARTY_NOTIFY_CODE",           LUA_CONST_PARTY_NOTIFY_CODE},
    {"TEAM_AUTHORITY_TYPE",         LUA_CONST_TEAM_AUTHORITY_TYPE},
    {"PARTY_LOOT_MODE",             LUA_CONST_ITEM_LOOT_MODE},
    {"DOODAD_KIND",                 LUA_CONST_DOODAD_KIND},
	{"CHARACTER_KIND",			    LUA_CONST_CHARACTER_KIND},
	
	{"RELATION_TYPE",			    LUA_CONST_RELATION_TYPE},
	{"CAMP",			            LUA_CONST_CAMP},
    {"SHOP_SYSTEM_RESPOND_CODE",    LUA_CONST_SHOP_SYSTEM_RESPOND_CODE},
    {"TRADING_RESPOND_CODE",        LUA_CONST_TRADING_RESPOND_CODE},
	{"LOOT_ITEM_RESULT_CODE",	    LUA_CONST_LOOT_ITEM_RESULT_CODE},

#if defined(_CLIENT)
    {"LOGIN",                       LUA_CONST_LOGIN},
#endif	//_CLIENT

    {"MOVE_STATE",                  LUA_CONST_MOVE_STATE},
	{"REVIVE_TYPE",                 LUA_CONST_REVIVE_TYPE},

    {"PLAYER_TALK_CHANNEL",         LUA_CONST_PLAYER_TALK_CHANNEL},
    {"PLAYER_TALK_ERROR",           LUA_CONST_PLAYER_TALK_ERROR},
    {"MAIL_RESPOND_CODE",           LUA_CONST_MAIL_RESPOND_CODE},
    {"PLAYER_FELLOWSHIP_RESPOND",   LUA_CONST_PLAYER_FELLOWSHIP_RESPOND},
	{"DATE_FORMAT",				    LUA_CONST_DATE_FORMAT},
    {"SKILL_ATTACK_WEAK_POS",       LUA_CONST_SKILL_ATTACK_WEAK_POS},
	{"ERROR_CODE_TYPE",				LUA_CONST_ERROR_CODE_TYPE},
    {"PEEK_OTHER_PLAYER_RESPOND",   LUA_CONST_PEEK_OTHER_PLAYER_RESPOND},
	{"PK_STATE",					LUA_CONST_PK_STATE},
	{"PK_RESPOND",					LUA_CONST_PK_RESPOND},
    {"PK_ENDCODE",                  LUA_CONST_PK_ENDCODE},

    {"AI_ACTION",                   LUA_AI_ACTION_KEY},
    {"AI_EVENT",                    LUA_AI_EVENT},
    {"AI_THREAT_TYPE",              LUA_AI_THREAT_TYPE},
    {"AI_TARGET_TYPE",              LUA_AI_TARGET_TYPE},
    {"AI_FILTER_TYPE",              LUA_AI_FILTER_TYPE},
    {"PREEMPTIVE_ATTACK",           LUA_PREEMPTIVE_ATTACK},
    
    {"SWITCH_MAP",                  LUA_SWITCH_MAP},
    {"BATTLE_FIELD_NOTIFY_TYPE",    LUA_BATTLE_FIELD_NOTIFY_TYPE},
    {"BATTLE_FIELD_RESULT_CODE",    LUA_BATTLE_FIELD_RESULT_CODE},
    {"SHARE_QUEST",                 LUA_SHARE_QUEST_RESULT_CODE},
    {"SKILL_RECIPE_RESULT_CODE",    LUA_SKILL_RECIPE_RESULT_CODE},
    {"BANISH_CODE",                 LUA_CONST_BANISH_CODE},
    {"ITEM_INFO_TYPE",              LUA_CONST_ITEM_INFO_TYPE},
    {"PQ_STATISTICS_INDEX",         LUA_PQ_STATISTICS_INDEX},

#if defined(_CLIENT)
    {"TONG_OPERATION_INDEX",        LUA_TONG_OPERATION_INDEX},
    {"TONG_EVENT_CODE",             LUA_TONG_EVENT_CODE},
    {"TONG_STATE",                  LUA_TONG_STATE},

    {"AUCTION_SALE_STATE",          LUA_AUCTION_SALE_STATE},
    {"AUCTION_ORDER_TYPE",          LUA_AUCTION_ORDER_TYPE},
    {"AUCTION_RESPOND_CODE",        LUA_AUCTION_RESPOND_CODE},
    {"AUCTION_MESSAGE_CODE",        LUA_AUCTION_MESSAGE_CODE},

    {"GAME_CARD_TYPE",              LUA_GAME_CARD_TYPE},
    {"GAME_CARD_RESPOND_CODE",      LUA_GAME_CARD_RESPOND_CODE},
    {"GAME_CARD_ORDER_TYPE",        LUA_GAME_CARD_ORDER_TYPE},
#endif
    {"CAMP_RESULT_CODE",            LUA_CAMP_RESULT_CODE},
    {"CHARGE_LIMIT_CODE",           LUA_CHARGE_LIMIT_CODE},
    {"TONG_CHANGE_REASON",          LUA_TONG_CHANGE_REASON},
    
    {"MAP_TYPE",                    LUA_MAP_TYPE},
    {"MAP_CAMP_TYPE",               LUA_MAP_CAMP_TYPE},
    {"ACHIEVEMENT_ANNOUNCE_TYPE",   LUA_ACHIEVEMENT_ANNOUNCE_TYPE},
    {"DESIGNATION_ANNOUNCE_TYPE",   LUA_DESIGNATION_ANNOUNCE_TYPE},
    {"ENCHANT_RESULT_CODE",         LUA_ENCHANT_RESULT_CODE},
    {"ENCHANT_INDEX",               LUA_ENCHANT_INDEX},
    {"TONG_HISTORY_TYPE",           LUA_TONG_HISTORY_TYPE},
    {"TONG_HISTORY_RECORD_TYPE",    LUA_TONG_HISTORY_RECORD_TYPE},

	{"MAIL_TYPE",					LUA_CONST_MAIL_TYPE},

    {"MENTOR_RECORD_STATE",         LUA_MENTOR_RECORD_STATE},
    {"CHARACTER_GENDER",            LUA_CHARACTER_GENDER},

    {"ITEM_EXIST_TYPE",             LUA_ITEM_EXIST_TYPE},

    {NULL,                          0}
};
