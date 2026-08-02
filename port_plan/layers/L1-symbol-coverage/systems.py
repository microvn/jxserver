#!/usr/bin/env python3
"""Single source of truth: class -> gameplay system, and system -> playability tier.
Shared by the measurement harness (cpp_coverage, lua_coverage). Extend the map
when a new class appears; keep it the ONLY place the mapping lives.
Derived from port_plan/layers/inventory/generate_method_inventory.py CLUSTER + FEATURE_ATLAS tiers."""

CLUSTER = {
 "KDomesticate":"pet-domesticate","KDomesticateCenter":"pet-domesticate","KFellowPetBox":"pet-domesticate",
 "KFellowPetMgr":"pet-domesticate","KPlayerRelatedPetAttrImportParam":"pet-domesticate","KPlayerRelatedPetAttrExportParam":"pet-domesticate",
 "KDiamondManager":"cash-shop-currency","KCurrency":"cash-shop-currency","KCurrencyList":"cash-shop-currency",
 "KRewardsShop":"cash-shop-currency","KRewardsBox":"cash-shop-currency","KRewardsShopSettings":"cash-shop-currency",
 "KShop":"cash-shop-currency","KShopCenter":"cash-shop-currency","KTradingBox":"cash-shop-currency",
 "KExterior":"exterior-cosmetic","KExteriorBox":"exterior-cosmetic","KHairShop":"exterior-cosmetic","KHairBox":"exterior-cosmetic",
 "KMiniAvatar":"exterior-cosmetic","KMiniAvatarSettings":"exterior-cosmetic","KModelInfo":"exterior-cosmetic",
 "KNewExtPointManager":"exterior-cosmetic","KPendentOldDataInfoList":"exterior-cosmetic","KDesignation":"exterior-cosmetic",
 "KTongServer":"guild-tong","KTongDiplomacyCache":"guild-tong","KTongConstList":"guild-tong",
 "KTalentSkill":"talent",
 "KRegressionManager":"regression","KRegressionPlayerData":"regression",
 "KDirectMentorCache":"mentor","KMentorCache":"mentor",
 "KActivityMgrServer":"activity","KActivityEndNotifySceneFunc":"activity","KCampActiveStat":"activity","KCampInfo":"activity",
 "KAntiFarmerServer":"anti-farmer","KAntiFarmerSettings":"anti-farmer","KBandwidthOptimizeTacticsManager":"anti-farmer",
 "KGMoveProcessor":"movement-sync","KGMoveParam":"movement-sync","KMoveProcessTraverseFunc":"movement-sync",
 "SYNC_MOVE_CTRL":"movement-sync","SYNC_MOVE_SPEED":"movement-sync","SYNC_MOVE_STATE":"movement-sync",
 "SYNC_SPRINT_FLAGS":"movement-sync","KGJumpList":"movement-sync",
 "KManualDropList":"drop-loot","KDropCenter":"drop-loot","KMoneyDropList":"drop-loot","KLevelDropList":"drop-loot",
 "KDropList":"drop-loot","KLootList":"drop-loot","KReputeLootBuffList":"drop-loot",
 "KRankListServer":"rank",
 "KScriptFuncList":"skill-script","KScriptCenter":"skill-script","KSkill":"skill-script","KSkillManager":"skill-script",
 "KSkillList":"skill-script","KSkillRecipeList":"skill-script","KRecipeList":"skill-script","KBroadcastCastSkillFunc":"skill-script",
 "KSkillRectangleTravFunc":"skill-script","KGetAllCopyIndexByMapIDFunc":"skill-script","GetSkillRecipeTraverseFunc":"skill-script","KBuffList":"skill-script",
 "KAILogic":"skill-script","KAIAction":"skill-script","KAIState":"skill-script","KAIVM":"skill-script",
 "KPlayer":"player-hub","KPlayerServer":"player-hub","KCharacter":"player-hub",
 "KCharacterStealthChangedFunc":"player-hub","KCharacterDisarmChangedFunc":"player-hub","KCharacterCounterStealthChangedFunc":"player-hub",
 "KScene":"world-scene","KNpc":"world-scene","KDoodad":"world-scene","KNpcTemplate":"world-scene","KNpcTemplateList":"world-scene",
 "KNpcAdronTab":"world-scene","KNpcReviveManager":"world-scene","KDoodadReviveManager":"world-scene","KRoadManager":"world-scene",
 "KAIManager":"world-scene","KAIPatrolGroupManager":"world-scene","KSimpThreatList":"world-scene","KSO3World":"world-scene","KWorldSettings":"world-scene",
 "KRelayClient":"network-infra","KLogClient":"network-infra","KStatDataServer":"network-infra","KLocalGSDataStat":"network-infra",
 "KG_SocketServerAcceptor":"network-infra","KG_AsyncSocketStream":"network-infra","KGWServerConstList":"network-infra","KGWConstList":"network-infra","KCenterRemote":"network-infra",
 "KAchievement":"achievement","KAchievementInfoList":"achievement",
 "KQuestList":"quest","KQuestInfoList":"quest",
 "KReputation":"reputation",
 "KItemList":"item",
 "KPKController":"pk",
 "KRecorderSceneLoaderReplay":"recorder-replay","KRecorderSceneLoaderRecord":"recorder-replay","KRecorderSceneLoaderNormal":"recorder-replay",
 "KGMList":"gm",
}

# playability tier per FEATURE_ATLAS: C=core-loop, P=progression, S=social, Q=cosmetic-QoL, I=infra, X=peripheral
SYSTEM_TIER = {
 "player-hub":"C","skill-script":"C","world-scene":"C","item":"C","drop-loot":"C","movement-sync":"C",
 "quest":"P","talent":"P","achievement":"P","reputation":"P","cash-shop-currency":"P","pet-domesticate":"P","regression":"P",
 "guild-tong":"S","mentor":"S","activity":"S","pk":"S","rank":"S",
 "exterior-cosmetic":"Q","anti-farmer":"Q",
 "network-infra":"I","gm":"X","recorder-replay":"X","other":"X",
}

def system_of(cls):
    return CLUSTER.get(cls, "other")
