//---------------------------------------------------------------------------
// KGWConstList : settings/GameWorldConstList.ini
//
// Reconstructed 1:1 from target evidence only (jx3_dwarf/SO3GameServerD):
//   struct KGWConstList  DIE 0x0002a4bb  sizeof = 0x614  183 members
//   member/method placement follows DW_AT_decl_line exactly.
//---------------------------------------------------------------------------
#ifndef _KGWCONSTLIST_H_
#define _KGWCONSTLIST_H_

#include "Global.h"

#define PLAYER_AND_QUEST_DIFF_LEVEL_COUNT   4   // target: GetMultiInteger count 4 @line 216, condition string @0xd9
#define MAX_KILL_COUNT                      11  // target: GetMultiInteger count 0xb @line 242, condition string @0xf3
#define MAX_COLOR_DIAMOND_LEVEL             6   // target: cmp $0x6/jle @line 366, cmp $0x5/jle @line 369
class IIniFile;   // PORT-DEFERRED_WIRING[IMPORT] target: Base/Include/Engine/FileTypeBase.h:20 (class IIniFile, byte_size 4, 28 pure-virtual slots) is absent from the candidate include tree
struct KGWConstList
{

    int       nNpcSkillCommonCD;                                   // 0x000
    int       nNpcCommonShortCD;                                   // 0x004
    int       nAISearchByLifeConversionLevel;                      // 0x008

    // [PK]
    int       nPKFlagDoodadTemplateID;                             // 0x00c
    int       nMinDuelLevel;                                       // 0x010
    int       nMinRevengeLevel;                                    // 0x014
    int       nKillPointReduceCycle;                               // 0x018
    int       nReduceKillPoint;                                    // 0x01c

    // [MAIL]
    int       nMailDelaySecondOnText;                              // 0x020
    int       nMailDelaySecondOnMoney;                             // 0x024
    int       nMailDelaySecondOnItem;                              // 0x028
    int       nMailSurvivalTime;                                   // 0x02c
    int       nPayMailSurvivalTime;                                // 0x030

    // [AUCTION]
    int       nGameCardTaxRate;                                    // 0x034
    int       nAuctionSellLeftHours[3];                            // 0x038

    // [ITEM]
    int       nCubPackageRoomRange[2];                             // 0x044
    int       nCubPackageRoomPrices[32];                           // 0x04c
    int       nBankPackagePrice[5];                                // 0x0cc
    int       nBookCraftID;                                        // 0x0e0

    // [FELLOWSHIP]
    int       nTeamAttractiveInterval;                             // 0x0e4
    int       nTeamAttractiveRange;                                // 0x0e8
    int       nTeamAttractiveUpLimit;                              // 0x0ec
    int       nTeamAttractiveAdd;                                  // 0x0f0

    int       nTeamAttractiveIntervalOnlyGroup;                    // 0x0f4
    int       nTeamAttractiveRangeOnlyGroup;                       // 0x0f8
    int       nTeamAttractiveUpLimitOnlyGroup;                     // 0x0fc
    int       nTeamAttractiveAddOnlyGroup;                         // 0x100

    int       nDuelAttractiveDailyCount;                           // 0x104
    int       nDuelAttractiveUpLimit;                              // 0x108
    int       nDuelAttractiveAdd;                                  // 0x10c

    int       nAssistAttractiveUpLimit;                            // 0x110
    int       nAssistPresentContribution;                          // 0x114
    int       nNewPlayerStamina;                                   // 0x118
    int       nNewPlayerThew;                                      // 0x11c

    int       nWhisperAttractiveDailyCount;                        // 0x120
    int       nWhisperAttractiveUpLimit;                           // 0x124
    int       nWhisperAttractiveAdd;                               // 0x128

    int       nAttractiveLevel[6];                                 // 0x12c
    int       nAttractiveAchievement[6];                           // 0x144
    int       nCountAchievement[8];                                // 0x15c

    int       nDifferentCampApplyJoinTeam;                         // 0x17c

    int       nFellowshipMaxFriendCount;                           // 0x180
    int       nFellowshipMaxFoeCount;                              // 0x184
    int       nFellowshipMaxBlacklistCount;                        // 0x188

    // [MISC]
    int       nDrowningTime;                                       // 0x18c
    int       nSaveInterval;                                       // 0x190

    // [MISC]
    int       nCorpseGenerationFrame;                              // 0x194

    // [MISC]
    int       nPlayerMaxLevel;                                     // 0x198

    // [MISC]
    int       nRiseFrame;                                          // 0x19c
    int       nSkidFrame;                                          // 0x1a0
    int       nNoAddSkillExp;                                      // 0x1a4

    // [TALK]
    int       nSceneChannelCoolDownID;                             // 0x1a8
    int       nWorldChannelCoolDownID;                             // 0x1ac
    int       nForceChannelCoolDownID;                             // 0x1b0
    int       nCampChannelCoolDownID;                              // 0x1b4
    int       nMentorChannelCoolDownID;                            // 0x1b8
    int       nFriendsChannelCoolDownID;                           // 0x1bc

    // [THREAT_RANK]
    int       nApplyThreatRankListCD;                              // 0x1c0

    // [THREAT]
    int       nThreatListSize;                                     // 0x1c4
    int       nModifyTheatenMaxCount;                              // 0x1c8

    int       nPlayerAndQuestDiffLevel[4];                         // 0x1cc

    // [CAMP]
    int       nReducePrestigeOnDeath;                              // 0x1dc
    int       nSoloKill;                                           // 0x1e0
    int       nLastHit;                                            // 0x1e4
    int       nCommonKill;                                         // 0x1e8
    int       nKillerTeamMember;                                   // 0x1ec
    int       nSameCampKill;                                       // 0x1f0
    int       nSameCampAssistKill;                                 // 0x1f4
    int       nKilledCountPercent[11];                             // 0x1f8
    int       nResetKilledCountCycle;                              // 0x224
    int       nNewCampFightAddPrestige;                            // 0x228
    int       nNewCampFightAddTitlePoint;                          // 0x22c

    // [CURRENCY]
    int       nLogCurrencyThreshold[6];                            // 0x230
    int       nMaxValue[6];                                        // 0x248
    int       nRemainSpace[6];                                     // 0x260
    int       nRemainSpaceCanAccumulate[6];                        // 0x278
    int       nRemainSpaceAccumulateRate[6];                       // 0x290
    int       nRemainSpaceResetOffest;                             // 0x2a8
    int       nRemainSpaceResetCycle;                              // 0x2ac

    // [TITLE]
    int       nSoloKillTitlePoint;                                 // 0x2b0
    int       nLastHitTitlePoint;                                  // 0x2b4
    int       nCommonKillTitlePoint;                               // 0x2b8
    int       nKillerTeamMemberTitlePoint;                         // 0x2bc
    int       nTitlePoint2Level[7];                                // 0x2c0
    int       nTitlePointPercent2Level[7];                         // 0x2dc
    int       nRank2Title[7];                                      // 0x2f8
    int       nRankPoint[14];                                      // 0x314
    int       nRequireRankPoint[14];                               // 0x34c

    int       nTitleLevelDesignation[3][14];                       // 0x384
    int       nRankPointVersion;                                   // 0x42c
    DWORD     dwCampScoreLevelBuffID[2];                           // 0x430

    // [ITEM]
    BOOL      bNewUseItemStyle;                                    // 0x438

    // [CHARGE]
    int       nFreeMaxLevel;                                       // 0x43c
    int       nFreeDurationTime;                                   // 0x440
    int       nEndOfDayTimeDealy;                                  // 0x444

    // [ACHIEVEMENT]
    int       nQuestCountAchievementID;                            // 0x448
    int       nAssistQuestCountAchievementID;                      // 0x44c
    int       nFinishQuestAssistByOtherAchievementID;              // 0x450
    BOOL      bRemotePlayerGetAchievement;                         // 0x454

    int       nNpcReviveTimeReduceThreshold;                       // 0x458
    int       nNpcReviveTimeIncreaseThreshold;                     // 0x45c
    int       nNpcReviveTimeReduceRate;                            // 0x460
    int       nNpcReviveTimeIncreaseRate;                          // 0x464

    int       nMaxDesignationBynameIndex;                          // 0x468

    // [SHOP]
    int       nRefreshCycle;                                       // 0x46c
    DWORD     dwBuyLimitItemCD;                                    // 0x470

    BOOL      bFreeForAll;                                         // 0x474
    BOOL      bDistribute;                                         // 0x478
    BOOL      bGroupLoot;                                          // 0x47c

    // [HAIR_SHOP]
    BOOL      bCloseHairShop;                                      // 0x480

    // [LOG_CONFIG]
    int       nLootItemQualityToLog;                               // 0x484
    int       nCraftMakeItemQualityToLog;                          // 0x488
    int       nSellToNpcItemQualityToLog;                          // 0x48c
    int       nBuyFromNpcItemQualityToLog;                         // 0x490
    int       nPlayerDestroyItemQualityToLog;                      // 0x494
    int       nPlayerAddTitlePointToLog;                           // 0x498

    // [FIVE_ELEMENT]
    int       nMaxDiamondLevel;                                    // 0x49c
    int       nMaxEquipStrengthLevel;                              // 0x4a0
    int       nUpdateDiamondCostStamina;                           // 0x4a4
    int       nStrengthEquipCostStamina;                           // 0x4a8
    int       nBreakEquipCostThew;                                 // 0x4ac
    int       nBasePriceOfOneLevelDiamond;                         // 0x4b0
    float     fExchangeCoef;                                       // 0x4b4
    float     fUnStrengthValueReturnRateMin;                       // 0x4b8
    float     fUnStrengthValueReturnRateMax;                       // 0x4bc
    float     fBreakValueReturnRateMin;                            // 0x4c0
    float     fBreakValueReturnRateMax;                            // 0x4c4
    int       nChangeColorDiamondCostMoney[6];                     // 0x4c8
    int       nChangeColorDiamondCostStamina[6];                   // 0x4e0
    int       nUpdateColorDiamondCostMoney[5];                     // 0x4f8
    int       nUpdateColorDiamondCostStamina[5];                   // 0x50c
    int       nMaxChangeColorDiamondLevel;                         // 0x520
    int       nMaxUpdateColorDiamondLevel;                         // 0x524

    // [STAMINA_THEW_RESTRICT]
    int       nRestrictCostStaminaExtPointIndex;                   // 0x528
    int       nRestrictCostStaminaExtPointBit;                     // 0x52c
    BOOL      bRestrictCostStaminaValue;                           // 0x530
    int       nRestrictCostThewExtPointIndex;                      // 0x534
    int       nRestrictCostThewExtPointBit;                        // 0x538
    BOOL      bRestrictCostThewValue;                              // 0x53c

    // [MENTOR]
    int       nMaxUsableMentorValue;                               // 0x540

    // [CHEAT_PUNISH]
    BOOL      m_bNormalMapIncomePunish;                            // 0x544
    BOOL      m_bBattleFieldIncomePunish;                          // 0x548
    BOOL      m_bDungeonIncomePunish;                              // 0x54c
    BOOL      m_bSalePunish;                                       // 0x550

    // [GLOBE]
    int       nSyncPlayerCount;                                    // 0x554
    int       nSyncNpcCount;                                       // 0x558
    int       nSyncDoodadCount;                                    // 0x55c
    BOOL      bDungeonOpen;                                        // 0x560

    // [BANK_PASSWORD]
    int       nBankPasswordResetTime;                              // 0x564

    // [ARENA]
    int       nCreateCorpsCostMoney[3];                            // 0x568
    int       nArenaPlayerMinLevel;                                // 0x574

    // [TONG]
    int       nTongRepertoryRecordItemQuality;                     // 0x578

    // [ROAD_TRACK]
    int       nResetRoadTrackCost;                                 // 0x57c

    // [SAFE_LOCK]
    int       nDestoryItemLimitQuilty;                             // 0x580

    int       m_nResourceVersion;                                  // 0x584
    int       m_nSprintFlag;                                       // 0x588
    int       m_nSprintFlagV2;                                     // 0x58c
    int       m_nSprintFlagV3;                                     // 0x590
    int       m_nSprintFlagV4;                                     // 0x594
    int       m_nFollowFlag;                                       // 0x598

    // [HASH_MAP_TEST]
    BOOL      bUseHashMap;                                         // 0x59c
    BOOL      bUseStdMap;                                          // 0x5a0
    BOOL      bHashMapOutputLog;                                   // 0x5a4
    BOOL      bUseFileMap;                                         // 0x5a8

    // [DROP]
    int       nNpcTemplateDropForceCoefficient;                    // 0x5ac
    BOOL      bNpcSurpriseDropFlag;                                // 0x5b0

    // [EXTERIOR]
    BOOL      bCloseExterior;                                      // 0x5b4

    // [DOMESTICATE]
    BOOL      bDomesticatePauseFlag;                               // 0x5b8
    time_t    nAcquireEventDuration;                               // 0x5bc
    time_t    nHorseGohungryDuration;                              // 0x5c0
    time_t    nCubGohungryDuration;                                // 0x5c4
    int       nHorseCostFullMeasureRateBase;                       // 0x5c8
    int       nCubCostFullMeasureRateBase;                         // 0x5cc
    int       nDomesticateMinPlayerLevel;                          // 0x5d0

    // [CLIENT_REPORT]
    BOOL      bNetworkDelayReportFlag;                             // 0x5d4
    int       nNetworkDelayReportFrequency;                        // 0x5d8
    int       nNetworkDelayReportThreshold;                        // 0x5dc

    // [HUNT_REDNAME]
    time_t    nHuntRednameNpcRefreshMinInterval;                   // 0x5e0
    time_t    nHuntRednameNpcRefreshMaxInterval;                   // 0x5e4
    char      szHunterFakeName[32];                                // 0x5e8

    // [REWARDS_SHOP]
    BOOL      bRewardsShopBuyCloseFlag;                            // 0x608
    BOOL      bRewardsShopBuyByRewardsCloseFlag;                   // 0x60c
    BOOL      bRewardsShopBuyByCoinCloseFlag;                      // 0x610
public:
    BOOL Init();
    void UnInit();

    BOOL IsAuctionSellLeftHoursValid(int nHours);

private:
    BOOL LoadData(IIniFile* piIniFile);
};

#endif // _KGWCONSTLIST_H_
