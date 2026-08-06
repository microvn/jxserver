//---------------------------------------------------------------------------
// KGWConstList : loader for settings/GameWorldConstList.ini
//
// Reconstructed 1:1 from target evidence only:
//   binary  jx3_dwarf/SO3GameServerD
//           sha256 47716c73e8de281c95759cdc4a478e70e7c61322fb46e8c4e04954e51124b94a
//   symbols _ZN12KGWConstList4InitEv                        0x082d47c4  (348 B)
//           _ZN12KGWConstList6UnInitEv                      0x082d0f5c  (6 B)
//           _ZN12KGWConstList27IsAuctionSellLeftHoursValidEi 0x082d0f62  (64 B)
//           _ZN12KGWConstList8LoadDataEP8IIniFile           0x082d0fa2  (14370 B)
//
// Statement placement follows the target DWARF line table exactly, because
// KGLOG_PROCESS_ERROR / KGLOG_CHECK_ERROR bake __LINE__ into their log text
// (target __FILE__ is "Src/KGWConstList.cpp", proven by the __assert_fail at
// line 92). Moving a statement changes observable output, so the line map is
// part of the behavioural contract, not formatting.
//
// The target build resolves KG_FUNCTION to __PRETTY_FUNCTION__: every log
// string carries the full signature, e.g. "BOOL KGWConstList::LoadData(IIniFile*)".
// The candidate tree defines KG_FUNCTION as __FUNCTION__ (kgpublic.h:21), which
// yields "LoadData". That is a cross-file macro drift outside this root:
// PORT-DEFERRED_WIRING[IMPORT] owner=Base/kgpublic.h resolution_phase=PRE_BUILD
// root_behavior_impact=NO (log text only; no control flow, state or wire).
//---------------------------------------------------------------------------
#include "stdafx.h"
#include "Global.h"
#include "KGWConstList.h"

#include <assert.h>
#include <limits.h>       // CHAR_BIT, used verbatim by the target condition at line 376
#include <stdio.h>        // snprintf
#include <string.h>       // memset, strlen

// Init() snprintf's the path into a 260-byte buffer and checks
// `nRetCode < sizeof(szFilePath)`; the size immediate is 0x104 = 260
// (0x82d47fa). The candidate tree's MAX_PATH is 1024 (Base/kstypes.h:39), so
// using MAX_PATH here would change both the buffer size and that guard.
// The literal 260 and the literal path fragments are written exactly as the
// target holds them ("settings" @0x0842ba23,
// "GameWorldConstList.ini" @0x0842ba0c); no macro name is invented for them,
// because the target gives no evidence of one.
//---------------------------------------------------------------------------
// KGWConstList::Init  (target 0x082d47c4)
//---------------------------------------------------------------------------




BOOL KGWConstList::Init()
{

    BOOL      bResult   = false;            // 0x82d47cd  movl $0x0, -0xc(%ebp)
    int       nRetCode  = 0;                // 0x82d47d4  movl $0x0, -0x8(%ebp)
    IIniFile* piIniFile = NULL;             // 0x82d47db  movl $0x0, -0x4(%ebp)
    char      szFilePath[260];

    nRetCode = snprintf(szFilePath, sizeof(szFilePath), "%s/%s", "settings", "GameWorldConstList.ini");
    KGLOG_PROCESS_ERROR(nRetCode > 0 && nRetCode < sizeof(szFilePath));

    piIniFile = g_OpenIniFile(szFilePath, false, false);   // 0x82d486d  (path, 0, 0)
    KGLOG_PROCESS_ERROR(piIniFile);

    nRetCode = LoadData(piIniFile);         // 0x82d48b6  call KGWConstList::LoadData
    KGLOG_PROCESS_ERROR(nRetCode);

    bResult = true;                         // 0x82d48f2  movl $0x1, -0xc(%ebp)
Exit0:
    KG_COM_RELEASE(piIniFile);              // 0x82d48f9  if (p) { p->Release(); p = NULL; }  vtable slot 2
    return bResult;
}
// UnInit (target 0x082d0f5c): empty body - push ebp; mov esp,ebp; leave; ret
void KGWConstList::UnInit()
{
}

BOOL KGWConstList::IsAuctionSellLeftHoursValid(int nHours)
{
    for (int i = 0; i < 3; i++)             // 0x82d0f8f  cmpl $0x2, -0x4(%ebp); jle
    {
        if (nAuctionSellLeftHours[i] == nHours)   // 0x82d0f7e  cmpl 0xc(%ebp)
            return true;                    // 0x82d0f83  movl $0x1, -0x14(%ebp)
    }

    return false;                           // 0x82d0f95  movl $0x0, -0x14(%ebp)
}
BOOL KGWConstList::LoadData(IIniFile* piIniFile)
{

    BOOL bResult  = false;                  // 0x82d0fa9  movl $0x0, -0x14(%ebp)
    int  nRetCode = 0;                      // 0x82d0fb0  movl $0x0, -0x10(%ebp)

    assert(piIniFile);

    piIniFile->GetInteger("AI", "NpcSkillCommonCD", 0, &nNpcSkillCommonCD);
    piIniFile->GetInteger("AI", "NpcCommonShortCD", 0, &nNpcCommonShortCD);
    piIniFile->GetInteger("AI", "AISearchByLifeConversionLevel", 0, &nAISearchByLifeConversionLevel);


    // [PK]
    piIniFile->GetInteger("PK", "PKFlagDoodadTemplateID", 0, &nPKFlagDoodadTemplateID);
    piIniFile->GetInteger("PK", "MinDuelLevel", 0, &nMinDuelLevel);
    piIniFile->GetInteger("PK", "MinRevengeLevel", 0, &nMinRevengeLevel);
    piIniFile->GetInteger("PK", "KillPointReduceCycle", 0, &nKillPointReduceCycle);
    piIniFile->GetInteger("PK", "ReduceKillPoint", 0, &nReduceKillPoint);


    // [MAIL]
    piIniFile->GetInteger("MAIL", "MailDelaySecondOnText", 0, &nMailDelaySecondOnText);
    piIniFile->GetInteger("MAIL", "MailDelaySecondOnMoney", 0, &nMailDelaySecondOnMoney);
    piIniFile->GetInteger("MAIL", "MailDelaySecondOnItem", 0, &nMailDelaySecondOnItem);
    piIniFile->GetInteger("MAIL", "MailSurvivalTime", 0, &nMailSurvivalTime);
    piIniFile->GetInteger("MAIL", "PayMailSurvivalTime", 0, &nPayMailSurvivalTime);


    // [AUCTION]
    piIniFile->GetInteger("AUCTION", "GameCardTaxRate", 0, &nGameCardTaxRate);
    KGLOG_PROCESS_ERROR(nGameCardTaxRate >= 0);

    piIniFile->GetMultiInteger("AUCTION", "SellLeftHours", nAuctionSellLeftHours, 3);

    // [ITEM]
    nRetCode = piIniFile->GetMultiInteger("ITEM", "CubPackageRoomRange", nCubPackageRoomRange, 2);
    KGLOG_PROCESS_ERROR(nRetCode == 2);
    KGLOG_PROCESS_ERROR(nCubPackageRoomRange[0] > 0 && nCubPackageRoomRange[1] < 32 && nCubPackageRoomRange[0] < nCubPackageRoomRange[1]);

    memset(nCubPackageRoomPrices, 0, sizeof(nCubPackageRoomPrices));
    nRetCode = piIniFile->GetMultiInteger("ITEM", "CubPackageRoomPrices", nCubPackageRoomPrices + nCubPackageRoomRange[0], nCubPackageRoomRange[1] - nCubPackageRoomRange[0]);
    KGLOG_PROCESS_ERROR(nRetCode == nCubPackageRoomRange[1] - nCubPackageRoomRange[0]);

    for (int i = 0; i < nCubPackageRoomRange[1]; i++)   // 0x82d150a  movl 0x48(%eax),%eax; cmpl -0xc(%ebp); jg
    {
        nCubPackageRoomPrices[i] *= 10000;      // 0x82d14b5  imull $0x2710
        KGLOG_PROCESS_ERROR(nCubPackageRoomPrices[i] >= 0);
    }

    nRetCode = piIniFile->GetMultiInteger("ITEM", "BankPackagePrice", nBankPackagePrice, MAX_BANK_PACKAGE_COUNT);
    KGLOG_PROCESS_ERROR(nRetCode == MAX_BANK_PACKAGE_COUNT);

    piIniFile->GetInteger("ITEM", "BookCraftID", 8, &nBookCraftID);

    for (int i = 0; i < MAX_BANK_PACKAGE_COUNT; i++)    // 0x82d1609  cmpl $0x4, -0x8(%ebp); jle
    {
        KGLOG_PROCESS_ERROR(nBankPackagePrice[i] >= 0);
    }


    // [FELLOWSHIP]
    piIniFile->GetInteger("FELLOWSHIP", "PartyAttractiveInterval", 0, &nTeamAttractiveInterval);
    piIniFile->GetInteger("FELLOWSHIP", "PartyAttractiveRange", 0, &nTeamAttractiveRange);
    piIniFile->GetInteger("FELLOWSHIP", "PartyAttractiveUpLimit", 0, &nTeamAttractiveUpLimit);
    piIniFile->GetInteger("FELLOWSHIP", "PartyAttractiveAdd", 0, &nTeamAttractiveAdd);
    piIniFile->GetInteger("FELLOWSHIP", "PartyAttractiveIntervalOnlyGroup", 0, &nTeamAttractiveIntervalOnlyGroup);
    piIniFile->GetInteger("FELLOWSHIP", "PartyAttractiveRangeOnlyGroup", 0, &nTeamAttractiveRangeOnlyGroup);
    piIniFile->GetInteger("FELLOWSHIP", "PartyAttractiveUpLimitOnlyGroup", 0, &nTeamAttractiveUpLimitOnlyGroup);
    piIniFile->GetInteger("FELLOWSHIP", "PartyAttractiveAddOnlyGroup", 0, &nTeamAttractiveAddOnlyGroup);
    piIniFile->GetInteger("FELLOWSHIP", "DuelAttractiveDailyCount", 0, &nDuelAttractiveDailyCount);
    piIniFile->GetInteger("FELLOWSHIP", "DuelAttractiveUpLimit", 0, &nDuelAttractiveUpLimit);
    piIniFile->GetInteger("FELLOWSHIP", "DuelAttractiveAdd", 0, &nDuelAttractiveAdd);

    if (nDuelAttractiveDailyCount > 15)     // 0x82d186a  cmpl $0xf; jle
    {
        KGLogPrintf(KGLOG_ERR, "[ConstList] DuelAttractiveDailyCount overflow: %d, max: %d", nDuelAttractiveDailyCount, 15);
        nDuelAttractiveDailyCount = 15;         // 0x82d189b  movl $0xf, 0x104(%eax)
    }

    piIniFile->GetInteger("FELLOWSHIP", "AssistAttractiveUpLimit", 0, &nAssistAttractiveUpLimit);
    piIniFile->GetInteger("FELLOWSHIP", "WhisperAttractiveDailyCount", 0, &nWhisperAttractiveDailyCount);
    piIniFile->GetInteger("FELLOWSHIP", "WhisperAttractiveUpLimit", 0, &nWhisperAttractiveUpLimit);
    piIniFile->GetInteger("FELLOWSHIP", "WhisperAttractiveAdd", 0, &nWhisperAttractiveAdd);

    if (nWhisperAttractiveDailyCount > 15)  // 0x82d1986  cmpl $0xf; jle
    {
        KGLogPrintf(KGLOG_ERR, "[ConstList] WhisperAttractiveDailyCount overflow: %d, max: %d", nWhisperAttractiveDailyCount, 15);
        nWhisperAttractiveDailyCount = 15;      // 0x82d19b7  movl $0xf, 0x120(%eax)
    }

    memset(nAttractiveLevel, 0, sizeof(nAttractiveLevel));
    nRetCode = piIniFile->GetMultiInteger("FELLOWSHIP", "AttractiveLevel", nAttractiveLevel, sizeof(nAttractiveLevel) / sizeof(nAttractiveLevel[0]));
    KGLOG_PROCESS_ERROR(nRetCode == sizeof(nAttractiveLevel) / sizeof(nAttractiveLevel[0]));

    memset(nAttractiveAchievement, 0, sizeof(nAttractiveAchievement));
    piIniFile->GetMultiInteger("FELLOWSHIP", "AttractiveAchievement", nAttractiveAchievement, 6);

    memset(nCountAchievement, 0, sizeof(nCountAchievement));
    piIniFile->GetMultiInteger("FELLOWSHIP", "CountAchievement", nCountAchievement, 8);

    piIniFile->GetInteger("FELLOWSHIP", "DifferentCampApplyJoinTeam", 0, &nDifferentCampApplyJoinTeam);
    piIniFile->GetInteger("FELLOWSHIP", "MaxFriendCount", 80, &nFellowshipMaxFriendCount);
    piIniFile->GetInteger("FELLOWSHIP", "MaxFoeCount", 30, &nFellowshipMaxFoeCount);
    piIniFile->GetInteger("FELLOWSHIP", "MaxBlacklistCount", 20, &nFellowshipMaxBlacklistCount);

    if (nFellowshipMaxFriendCount + nFellowshipMaxFoeCount + nFellowshipMaxBlacklistCount > 250)   // 0x82d1bf5  cmpl $0xfa; jle
    {
        KGLogPrintf(
            KGLOG_ERR,
            "[ConstList] The sum of MaxFriendCount(%d), MaxFoeCount(%d) and MaxBlacklistCount(%d) should less than 250.",
            nFellowshipMaxFriendCount, nFellowshipMaxFoeCount, nFellowshipMaxBlacklistCount);

        goto Exit0;                             // 0x82d1c37  jmp Exit0
    }

    piIniFile->GetInteger("MISC", "SaveInterval", 0, &nSaveInterval);
    piIniFile->GetInteger("MISC", "DrowningTime", 0, &nDrowningTime);
    piIniFile->GetInteger("MISC", "CorpseGenerationFrame", 0, &nCorpseGenerationFrame);
    piIniFile->GetInteger("MISC", "PlayerMaxLevel", 0, &nPlayerMaxLevel);
    piIniFile->GetInteger("MISC", "RiseFrame", 0, &nRiseFrame);
    piIniFile->GetInteger("MISC", "SkidFrame", 0, &nSkidFrame);
    piIniFile->GetInteger("MISC", "NoAddSkillExp", 0, &nNoAddSkillExp);
    piIniFile->GetInteger("MISC", "ResourceVersion", 1, &m_nResourceVersion);
    piIniFile->GetInteger("MISC", "SprintFlag", 0, &m_nSprintFlag);
    piIniFile->GetInteger("MISC", "SprintFlagV2", 0, &m_nSprintFlagV2);
    piIniFile->GetInteger("MISC", "SprintFlagV3", 0, &m_nSprintFlagV3);
    piIniFile->GetInteger("MISC", "SprintFlagV4", 0, &m_nSprintFlagV4);
    piIniFile->GetInteger("MISC", "FollowFlag", 0, &m_nFollowFlag);

    nRetCode = piIniFile->GetMultiInteger("MISC", "PlayerAndQuestDiffLevel", nPlayerAndQuestDiffLevel, PLAYER_AND_QUEST_DIFF_LEVEL_COUNT);
    KGLOG_PROCESS_ERROR(nRetCode == PLAYER_AND_QUEST_DIFF_LEVEL_COUNT);

    piIniFile->GetInteger("MISC", "AssistPresentContribution", 0, &nAssistPresentContribution);
    piIniFile->GetInteger("MISC", "NewPlayerStamina", 0, &nNewPlayerStamina);
    piIniFile->GetInteger("MISC", "NewPlayerThew", 0, &nNewPlayerThew);

    piIniFile->GetInteger("CAMP", "ReducePrestigeOnDeath", 0, &nReducePrestigeOnDeath);
    KGLOG_PROCESS_ERROR(nReducePrestigeOnDeath <= 0);

    piIniFile->GetInteger("CAMP", "SoloKill", 0, &nSoloKill);
    piIniFile->GetInteger("CAMP", "LastHit", 0, &nLastHit);
    piIniFile->GetInteger("CAMP", "CommonKill", 0, &nCommonKill);
    piIniFile->GetInteger("CAMP", "KillerPartyMember", 0, &nKillerTeamMember);

    piIniFile->GetInteger("CAMP", "SameCampKill", 0, &nSameCampKill);
    KGLOG_PROCESS_ERROR(nSameCampKill <= 0);

    piIniFile->GetInteger("CAMP", "SameCampAssistKill", 0, &nSameCampAssistKill);
    KGLOG_PROCESS_ERROR(nSameCampAssistKill <= 0);

    piIniFile->GetInteger("CAMP", "ResetKilledCountCycle", 0, &nResetKilledCountCycle);
    piIniFile->GetInteger("CAMP", "NewCampFightAddPrestige", 0, &nNewCampFightAddPrestige);
    piIniFile->GetInteger("CAMP", "NewCampFightAddTitlePoint", 0, &nNewCampFightAddTitlePoint);


    nRetCode = piIniFile->GetMultiInteger("CAMP", "KilledCountPercent", nKilledCountPercent, MAX_KILL_COUNT);
    KGLOG_PROCESS_ERROR(nRetCode == MAX_KILL_COUNT);

    // [CURRENCY]
    piIniFile->GetInteger("CURRENCY", "LogContributionThreshold", 1, &nLogCurrencyThreshold[0]);
    piIniFile->GetInteger("CURRENCY", "LogPrestigeThreshold", 1, &nLogCurrencyThreshold[1]);
    piIniFile->GetInteger("CURRENCY", "LogJusticeThreshold", 1, &nLogCurrencyThreshold[2]);
    piIniFile->GetInteger("CURRENCY", "LogExamPrintThreshold", 1, &nLogCurrencyThreshold[3]);
    piIniFile->GetInteger("CURRENCY", "LogArenaAwardThreshold", 1, &nLogCurrencyThreshold[4]);
    piIniFile->GetInteger("CURRENCY", "LogActivityAwardThreshold", 1, &nLogCurrencyThreshold[5]);

    piIniFile->GetInteger("CURRENCY", "MaxContribution", 0, &nMaxValue[0]);
    piIniFile->GetInteger("CURRENCY", "MaxPrestige", 0, &nMaxValue[1]);
    piIniFile->GetInteger("CURRENCY", "MaxJustice", 0, &nMaxValue[2]);
    piIniFile->GetInteger("CURRENCY", "MaxExamPrint", 0, &nMaxValue[3]);
    piIniFile->GetInteger("CURRENCY", "MaxArenaAward", 0, &nMaxValue[4]);
    piIniFile->GetInteger("CURRENCY", "MaxActivityAward", 0, &nMaxValue[5]);

    piIniFile->GetInteger("CURRENCY", "ContributionRemainSpace", 0, &nRemainSpace[0]);
    piIniFile->GetInteger("CURRENCY", "PrestigeRemainSpace", 0, &nRemainSpace[1]);
    piIniFile->GetInteger("CURRENCY", "JusticeRemainSpace", 0, &nRemainSpace[2]);
    piIniFile->GetInteger("CURRENCY", "ExamPrintRemainSpace", 0, &nRemainSpace[3]);
    piIniFile->GetInteger("CURRENCY", "ArenaAwardRemainSpace", 0, &nRemainSpace[4]);
    piIniFile->GetInteger("CURRENCY", "ActivityAwardRemainSpace", 0, &nRemainSpace[5]);

    piIniFile->GetInteger("CURRENCY", "ContributionRemainSpaceCanAccumulate", 0, &nRemainSpaceCanAccumulate[0]);
    piIniFile->GetInteger("CURRENCY", "PrestigeRemainSpaceCanAccumulate", 0, &nRemainSpaceCanAccumulate[1]);
    piIniFile->GetInteger("CURRENCY", "JusticeRemainSpaceCanAccumulate", 0, &nRemainSpaceCanAccumulate[2]);
    piIniFile->GetInteger("CURRENCY", "ExamPrintRemainSpaceCanAccumulate", 0, &nRemainSpaceCanAccumulate[3]);
    piIniFile->GetInteger("CURRENCY", "ArenaAwardRemainSpaceCanAccumulate", 0, &nRemainSpaceCanAccumulate[4]);
    piIniFile->GetInteger("CURRENCY", "ActivityAwardRemainSpaceCanAccumulate", 0, &nRemainSpaceCanAccumulate[5]);

    piIniFile->GetInteger("CURRENCY", "ContributionAccumulateRate", 0, &nRemainSpaceAccumulateRate[0]);
    piIniFile->GetInteger("CURRENCY", "PrestigeAccumulateRate", 0, &nRemainSpaceAccumulateRate[1]);
    piIniFile->GetInteger("CURRENCY", "JusticeAccumulateRate", 0, &nRemainSpaceAccumulateRate[2]);
    piIniFile->GetInteger("CURRENCY", "ExamPrintAccumulateRate", 0, &nRemainSpaceAccumulateRate[3]);
    piIniFile->GetInteger("CURRENCY", "ArenaAwardAccumulateRate", 0, &nRemainSpaceAccumulateRate[4]);
    piIniFile->GetInteger("CURRENCY", "ActivityAwardAccumulateRate", 0, &nRemainSpaceAccumulateRate[5]);

    piIniFile->GetInteger("CURRENCY", "RemainSpaceResetOffest", 0, &nRemainSpaceResetOffest);
    piIniFile->GetInteger("CURRENCY", "RemainSpaceResetCycle", 604800, &nRemainSpaceResetCycle);

    // [TITLE]
    piIniFile->GetInteger("TITLE", "SoloKill", 0, &nSoloKillTitlePoint);
    piIniFile->GetInteger("TITLE", "LastHit", 0, &nLastHitTitlePoint);
    piIniFile->GetInteger("TITLE", "CommonKill", 0, &nCommonKillTitlePoint);
    piIniFile->GetInteger("TITLE", "KillerPartyMember", 0, &nKillerTeamMemberTitlePoint);

    memset(nTitlePoint2Level, 0, sizeof(nTitlePoint2Level));
    memset(nTitlePointPercent2Level, 0, sizeof(nTitlePointPercent2Level));
    memset(nRank2Title, 0, sizeof(nRank2Title));
    memset(nRankPoint, 0, sizeof(nRankPoint));
    memset(nRequireRankPoint, 0, sizeof(nRequireRankPoint));
    memset(nTitleLevelDesignation[1], 0, sizeof(nTitleLevelDesignation[0]));
    memset(nTitleLevelDesignation[2], 0, sizeof(nTitleLevelDesignation[0]));
    memset(dwCampScoreLevelBuffID, 0, sizeof(dwCampScoreLevelBuffID));

    piIniFile->GetMultiInteger("TITLE", "TitlePoint2Level", nTitlePoint2Level, 7);
    piIniFile->GetMultiInteger("TITLE", "TitlePointPercent2Level", nTitlePointPercent2Level, 7);
    piIniFile->GetMultiInteger("TITLE", "Rank2Title", nRank2Title, 7);
    piIniFile->GetMultiInteger("TITLE", "RankPoint", nRankPoint, 14);
    piIniFile->GetMultiInteger("TITLE", "RequireRankPoint", nRequireRankPoint, 14);
    piIniFile->GetMultiInteger("TITLE", "GoodTitleLevelDesignation", nTitleLevelDesignation[1], 14);
    piIniFile->GetMultiInteger("TITLE", "EvilTitleLevelDesignation", nTitleLevelDesignation[2], 14);
    piIniFile->GetMultiInteger("TITLE", "CampScoreLevelBuffID", (int*)dwCampScoreLevelBuffID, 2);

    piIniFile->GetInteger("TITLE", "RankPointVersion", 0, &nRankPointVersion);
    piIniFile->GetInteger("TALK", "SceneChannelCoolDownID", 0, &nSceneChannelCoolDownID);
    piIniFile->GetInteger("TALK", "WorldChannelCoolDownID", 0, &nWorldChannelCoolDownID);
    piIniFile->GetInteger("TALK", "ForceChannelCoolDownID", 0, &nForceChannelCoolDownID);
    piIniFile->GetInteger("TALK", "CampChannelCoolDownID", 0, &nCampChannelCoolDownID);
    piIniFile->GetInteger("TALK", "MentorChannelCoolDownID", 0, &nMentorChannelCoolDownID);
    piIniFile->GetInteger("TALK", "FriendsChannelCoolDownID", 0, &nFriendsChannelCoolDownID);
    piIniFile->GetInteger("THREAT_RANK", "ApplyThreatRankListCoolDownID", 0, &nApplyThreatRankListCD);
    piIniFile->GetInteger("THREAT", "ThreatListSize", 0, &nThreatListSize);
    piIniFile->GetInteger("THREAT", "ModifyTheatenMaxCount", 0, &nModifyTheatenMaxCount);
    piIniFile->GetInteger("ITEM", "NewUseItemStyle", 0, &bNewUseItemStyle);
    piIniFile->GetInteger("CHARGE", "FreeMaxLevel", 0, &nFreeMaxLevel);
    piIniFile->GetInteger("CHARGE", "FreeDurationTime", 0, &nFreeDurationTime);
    piIniFile->GetInteger("CHARGE", "EndOfDayTimeDelay", 0, &nEndOfDayTimeDealy);
    piIniFile->GetInteger("ACHIEVEMENT", "QuestCountAchievementID", -1, &nQuestCountAchievementID);
    piIniFile->GetInteger("ACHIEVEMENT", "AssistQuestCountAchievementID", -1, &nAssistQuestCountAchievementID);
    piIniFile->GetInteger("ACHIEVEMENT", "FinishQuestAssistByOtherAchievementID", -1, &nFinishQuestAssistByOtherAchievementID);
    piIniFile->GetInteger("ACHIEVEMENT", "RemotePlayerGetAchievement", 0, &bRemotePlayerGetAchievement);
    piIniFile->GetInteger("NPC", "NpcReviveTimeReduceThreshold", 2147483647, &nNpcReviveTimeReduceThreshold);
    piIniFile->GetInteger("NPC", "NpcReviveTimeIncreaseThreshold", 2147483647, &nNpcReviveTimeIncreaseThreshold);
    piIniFile->GetInteger("NPC", "NpcReviveTimeReduceRate", 1024, &nNpcReviveTimeReduceRate);
    piIniFile->GetInteger("NPC", "NpcReviveTimeIncreaseRate", 1024, &nNpcReviveTimeIncreaseRate);
    piIniFile->GetInteger("DESIGNATION", "MaxPlayerBynameIndex", 0, &nMaxDesignationBynameIndex);

    piIniFile->GetInteger("BINDONTIMELIMITATION", "FreeForAll", 0, &bFreeForAll);
    piIniFile->GetInteger("BINDONTIMELIMITATION", "Distribute", 0, &bDistribute);
    piIniFile->GetInteger("BINDONTIMELIMITATION", "GroupLoot", 0, &bGroupLoot);

    piIniFile->GetInteger("SHOP", "RefreshCycle", 0, &nRefreshCycle);
    KGLOG_PROCESS_ERROR(nRefreshCycle > 0);
    piIniFile->GetInteger("SHOP", "BuyLimitItemCD", 0, (int*)&dwBuyLimitItemCD);

    // [HAIR_SHOP]
    piIniFile->GetInteger("HAIR_SHOP", "CloseHairShop", 0, &bCloseHairShop);

    // [FIVE_ELEMENT]
    piIniFile->GetInteger("FIVE_ELEMENT", "MaxDiamondLevel", 10, &nMaxDiamondLevel);
    piIniFile->GetInteger("FIVE_ELEMENT", "UpdateDiamondCostStamina", 3, &nUpdateDiamondCostStamina);
    piIniFile->GetInteger("FIVE_ELEMENT", "StrengthEquipCostStamina", 20, &nStrengthEquipCostStamina);
    piIniFile->GetInteger("FIVE_ELEMENT", "BreakEquipCostThew", 5, &nBreakEquipCostThew);
    piIniFile->GetInteger("FIVE_ELEMENT", "BasePriceOfOneLevelDiamond", 2500, &nBasePriceOfOneLevelDiamond);
    piIniFile->GetFloat("FIVE_ELEMENT", "ExchangeCoef", 250.0f, &fExchangeCoef);   // default bits 0x437a0000
    piIniFile->GetFloat("FIVE_ELEMENT", "UnStrengthValueReturnRateMin", 0.3f, &fUnStrengthValueReturnRateMin);   // default bits 0x3e99999a
    piIniFile->GetFloat("FIVE_ELEMENT", "UnStrengthValueReturnRateMax", 0.4f, &fUnStrengthValueReturnRateMax);   // default bits 0x3ecccccd
    piIniFile->GetFloat("FIVE_ELEMENT", "BreakValueReturnRateMin", 0.2f, &fBreakValueReturnRateMin);   // default bits 0x3e4ccccd
    piIniFile->GetFloat("FIVE_ELEMENT", "BreakValueReturnRateMax", 0.3f, &fBreakValueReturnRateMax);   // default bits 0x3e99999a

    memset(nChangeColorDiamondCostMoney, 0, sizeof(nChangeColorDiamondCostMoney));
    memset(nChangeColorDiamondCostStamina, 0, sizeof(nChangeColorDiamondCostStamina));
    memset(nUpdateColorDiamondCostMoney, 0, sizeof(nUpdateColorDiamondCostMoney));
    memset(nUpdateColorDiamondCostStamina, 0, sizeof(nUpdateColorDiamondCostStamina));

    piIniFile->GetMultiInteger("FIVE_ELEMENT", "ChangeColorDiamondCostMoney", nChangeColorDiamondCostMoney, 6);
    piIniFile->GetMultiInteger("FIVE_ELEMENT", "ChangeColorDiamondCostStamina", nChangeColorDiamondCostStamina, 6);
    piIniFile->GetMultiInteger("FIVE_ELEMENT", "UpdateColorDiamondCostMoney", nUpdateColorDiamondCostMoney, 5);
    piIniFile->GetMultiInteger("FIVE_ELEMENT", "UpdateColorDiamondCostStamina", nUpdateColorDiamondCostStamina, 5);

    piIniFile->GetInteger("FIVE_ELEMENT", "MaxChangeColorDiamondLevel", 0, &nMaxChangeColorDiamondLevel);
    KGLOG_PROCESS_ERROR(nMaxChangeColorDiamondLevel >= 0 && nMaxChangeColorDiamondLevel <= MAX_COLOR_DIAMOND_LEVEL);

    piIniFile->GetInteger("FIVE_ELEMENT", "MaxUpdateColorDiamondLevel", 0, &nMaxUpdateColorDiamondLevel);
    KGLOG_PROCESS_ERROR(nMaxUpdateColorDiamondLevel >= 0 && nMaxUpdateColorDiamondLevel < MAX_COLOR_DIAMOND_LEVEL);

    // [STAMINA_THEW_RESTRICT]
    piIniFile->GetInteger("STAMINA_THEW_RESTRICT", "RestrictCostStaminaExtPointIndex", -1, &nRestrictCostStaminaExtPointIndex);
    KGLOG_PROCESS_ERROR(nRestrictCostStaminaExtPointIndex >= -1 && nRestrictCostStaminaExtPointIndex < MAX_EXT_POINT_COUNT);

    piIniFile->GetInteger("STAMINA_THEW_RESTRICT", "RestrictCostStaminaExtPointBit", -1, &nRestrictCostStaminaExtPointBit);
    KGLOG_PROCESS_ERROR(nRestrictCostStaminaExtPointBit >= -1 && nRestrictCostStaminaExtPointBit < (int)(CHAR_BIT * sizeof(int)));

    piIniFile->GetInteger("STAMINA_THEW_RESTRICT", "RestrictCostStaminaValue", 0, &bRestrictCostStaminaValue);

    piIniFile->GetInteger("STAMINA_THEW_RESTRICT", "RestrictCostThewExtPointIndex", -1, &nRestrictCostThewExtPointIndex);
    KGLOG_PROCESS_ERROR(nRestrictCostThewExtPointIndex >= -1 && nRestrictCostThewExtPointIndex < MAX_EXT_POINT_COUNT);

    piIniFile->GetInteger("STAMINA_THEW_RESTRICT", "RestrictCostThewExtPointBit", -1, &nRestrictCostThewExtPointBit);
    KGLOG_PROCESS_ERROR(nRestrictCostThewExtPointBit >= -1 && nRestrictCostThewExtPointBit < (int)(CHAR_BIT * sizeof(int)));

    piIniFile->GetInteger("STAMINA_THEW_RESTRICT", "RestrictCostThewValue", 0, &bRestrictCostThewValue);

    // [MENTOR]
    piIniFile->GetInteger("MENTOR", "MaxUsableMentorValue", 10000, &nMaxUsableMentorValue);
    KGLOG_PROCESS_ERROR(nMaxUsableMentorValue > 0);

    piIniFile->GetInteger("LOG_CONFIG", "LootItemQualityToLog", 3, &nLootItemQualityToLog);
    piIniFile->GetInteger("LOG_CONFIG", "CraftMakeItemQualityToLog", 3, &nCraftMakeItemQualityToLog);
    piIniFile->GetInteger("LOG_CONFIG", "SellToNpcItemQualityToLog", 3, &nSellToNpcItemQualityToLog);
    piIniFile->GetInteger("LOG_CONFIG", "BuyFromNpcItemQualityToLog", 3, &nBuyFromNpcItemQualityToLog);
    piIniFile->GetInteger("LOG_CONFIG", "PlayerDestroyItemQualityToLog", 3, &nPlayerDestroyItemQualityToLog);
    piIniFile->GetInteger("LOG_CONFIG", "PlayerAddTitlePointToLog", 10, &nPlayerAddTitlePointToLog);

    m_bNormalMapIncomePunish = true;       // 0x82d3ca6  movl $0x1, 0x544(%eax)
    piIniFile->GetBool("CHEAT_PUNISH", "NormalMapIncomePunish", &m_bNormalMapIncomePunish);

    m_bBattleFieldIncomePunish = false;    // 0x82d3ce1  movl $0x0, 0x548(%eax)
    piIniFile->GetBool("CHEAT_PUNISH", "BattleFieldIncomePunish", &m_bBattleFieldIncomePunish);

    m_bDungeonIncomePunish = false;        // 0x82d3d1c  movl $0x0, 0x54c(%eax)
    piIniFile->GetBool("CHEAT_PUNISH", "DungeonIncomePunish", &m_bDungeonIncomePunish);

    m_bSalePunish = true;                  // 0x82d3d57  movl $0x1, 0x550(%eax)
    piIniFile->GetBool("CHEAT_PUNISH", "SalePunish", &m_bSalePunish);

    // [GLOBE]
    piIniFile->GetInteger("GLOBE", "SyncPlayerCount", 2, &nSyncPlayerCount);
    piIniFile->GetInteger("GLOBE", "SyncNpcCount", 2, &nSyncNpcCount);
    piIniFile->GetInteger("GLOBE", "SyncDoodadCount", 2, &nSyncDoodadCount);
    piIniFile->GetInteger("GLOBE", "DungeonOpenFlag", 0, &bDungeonOpen);

    // [BANK_PASSWORD]
    piIniFile->GetInteger("BANK_PASSWORD", "ResetTime", 0, &nBankPasswordResetTime);


    memset(nCreateCorpsCostMoney, 0, sizeof(nCreateCorpsCostMoney));
    nRetCode = piIniFile->GetMultiInteger("ARENA", "CreateCorpsCostMoney", nCreateCorpsCostMoney, sizeof(nCreateCorpsCostMoney) / sizeof(nCreateCorpsCostMoney[0]));
    KGLOG_CHECK_ERROR(nRetCode == sizeof(nCreateCorpsCostMoney) / sizeof(nCreateCorpsCostMoney[0]));

    piIniFile->GetInteger("ARENA", "ArenaPlayerMinLevel", 80, &nArenaPlayerMinLevel);

    // [TONG]
    piIniFile->GetInteger("TONG", "RepertoryRecordItemQuality", 3, &nTongRepertoryRecordItemQuality);

    // [ROAD_TRACK]
    piIniFile->GetInteger("ROAD_TRACK", "ResetRoadTrackCost", 10000, &nResetRoadTrackCost);
    KGLOG_PROCESS_ERROR(nResetRoadTrackCost >= 0);

    // [SAFE_LOCK]
    piIniFile->GetInteger("SAFE_LOCK", "DestoryItemLimitQuilty", 4, &nDestoryItemLimitQuilty);


    bUseHashMap = false;                   // 0x82d4041  movl $0x0, 0x59c(%eax)
    piIniFile->GetBool("HASH_MAP_TEST", "UseHashMap", &bUseHashMap);
    bUseStdMap = false;                    // 0x82d407c  movl $0x0, 0x5a0(%eax)
    piIniFile->GetBool("HASH_MAP_TEST", "UseStdMap", &bUseStdMap);
    bHashMapOutputLog = false;             // 0x82d40b7  movl $0x0, 0x5a4(%eax)
    piIniFile->GetBool("HASH_MAP_TEST", "OutputLog", &bHashMapOutputLog);
    bUseFileMap = false;                   // 0x82d40f2  movl $0x0, 0x5a8(%eax)
    piIniFile->GetBool("HASH_MAP_TEST", "UseFileMap", &bUseFileMap);

    if (!bUseHashMap && !bUseStdMap)        // 0x82d4133/0x82d4140  testl; jne
    {
        bUseStdMap = true;                     // 0x82d4147  movl $0x1, 0x5a0(%eax)
        KGLogPrintf(KGLOG_INFO, "[HashMapTest] Use Std Map");
    }

    // [DROP]
    piIniFile->GetInteger("DROP", "NpcTemplateDropForceCoefficient", 0, &nNpcTemplateDropForceCoefficient);
    piIniFile->GetInteger("DROP", "NpcSurpriseDropFlag", 0, &bNpcSurpriseDropFlag);

    // [EXTERIOR]
    piIniFile->GetInteger("EXTERIOR", "CloseExterior", 0, &bCloseExterior);

    // [DOMESTICATE]
    piIniFile->GetInteger("DOMESTICATE", "DomesticatePauseFlag", 0, &bDomesticatePauseFlag);
    piIniFile->GetInteger("DOMESTICATE", "AcquireEventDuration", 3600, (int*)&nAcquireEventDuration);
    KGLOG_PROCESS_ERROR(nAcquireEventDuration > 0);
    piIniFile->GetInteger("DOMESTICATE", "HorseGohungryDuration", 3600, (int*)&nHorseGohungryDuration);
    KGLOG_PROCESS_ERROR(nHorseGohungryDuration > 0);
    piIniFile->GetInteger("DOMESTICATE", "CubGohungryDuration", 3600, (int*)&nCubGohungryDuration);
    KGLOG_PROCESS_ERROR(nCubGohungryDuration > 0);
    piIniFile->GetInteger("DOMESTICATE", "HorseCostFullMeasureRateBase", 72, &nHorseCostFullMeasureRateBase);
    KGLOG_PROCESS_ERROR(nHorseCostFullMeasureRateBase > 0);
    piIniFile->GetInteger("DOMESTICATE", "CubCostFullMeasureRateBase", 24, &nCubCostFullMeasureRateBase);
    KGLOG_PROCESS_ERROR(nCubCostFullMeasureRateBase > 0);
    piIniFile->GetInteger("DOMESTICATE", "DomesticateMinPlayerLevel", 40, &nDomesticateMinPlayerLevel);

    // [CLIENT_REPORT]
    piIniFile->GetInteger("CLIENT_REPORT", "NetworkDelayReportFlag", 0, &bNetworkDelayReportFlag);
    piIniFile->GetInteger("CLIENT_REPORT", "NetworkDelayReportThreshold", 0, &nNetworkDelayReportThreshold);
    piIniFile->GetInteger("CLIENT_REPORT", "NetworkDelayReportFrequency", 600, &nNetworkDelayReportFrequency);
    KGLOG_PROCESS_ERROR(nNetworkDelayReportFrequency > 0);

    // [HUNT_REDNAME]
    piIniFile->GetInteger("HUNT_REDNAME", "NpcRefreshMinInterval", 3600, (int*)&nHuntRednameNpcRefreshMinInterval);
    KGLOG_PROCESS_ERROR(nHuntRednameNpcRefreshMinInterval >= 10);
    piIniFile->GetInteger("HUNT_REDNAME", "NpcRefreshMaxInterval", 7200, (int*)&nHuntRednameNpcRefreshMaxInterval);
    KGLOG_PROCESS_ERROR(nHuntRednameNpcRefreshMaxInterval > nHuntRednameNpcRefreshMinInterval);
    piIniFile->GetString("HUNT_REDNAME", "HunterFakeName", "Hunter", szHunterFakeName, sizeof(szHunterFakeName));
    szHunterFakeName[sizeof(szHunterFakeName) - 1] = '\0';   // 0x82d46ca  movb $0x0, 0x607(%eax)
    KGLOG_PROCESS_ERROR(strlen(szHunterFakeName) > 0);

    // [REWARDS_SHOP]
    piIniFile->GetInteger("REWARDS_SHOP", "BuyCloseFlag", 0, &bRewardsShopBuyCloseFlag);
    piIniFile->GetInteger("REWARDS_SHOP", "BuyByRewardsCloseFlag", 0, &bRewardsShopBuyByRewardsCloseFlag);
    piIniFile->GetInteger("REWARDS_SHOP", "BuyByCoinCloseFlag", 0, &bRewardsShopBuyByCoinCloseFlag);

    bResult = true;                         // 0x82d47b3  movl $0x1, -0x14(%ebp)
Exit0:
    return bResult;                         // 0x82d47ba
}
