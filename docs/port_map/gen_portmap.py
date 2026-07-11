#!/usr/bin/env python3
import csv, os
SP="/private/tmp/claude-501/-Volumes-ExData-game-jx3/4aecc4f7-3d4e-4ee1-ac60-0bdf28a2a840/scratchpad"
OUT="/Volumes/ExData/game/jx3/linux-build/docs/port_map"
os.makedirs(OUT, exist_ok=True)

# cluster, kind(hub/self/infra), and a base priority order for the cluster
CLUSTER = {
 # class : (cluster)
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
HUB={"KPlayer","KPlayerServer","KCharacter","KItemList","KScriptFuncList","KNpc","KScene","KSkill","KSkillManager","KDoodad"}
# cluster port order: leaf/self-contained first, hubs last
CLUSTER_ORDER=["movement-sync","pet-domesticate","exterior-cosmetic","cash-shop-currency","regression","mentor",
 "anti-farmer","rank","guild-tong","talent","activity","achievement","quest","reputation","pk","gm","recorder-replay",
 "drop-loot","skill-script","world-scene","item","player-hub","network-infra"]

def kind(cls):
    if CLUSTER.get(cls)=="network-infra": return "network-infra"
    if cls in HUB: return "hub"
    return "self-contained"

# read per_class_full.tsv
classes={}
with open(f"{SP}/per_class_full.tsv") as f:
    for line in f:
        c,v25,ours,truly,nel,pe,tag=line.rstrip("\n").split("\t")
        if int(truly)+int(nel)==0: continue  # nothing meaningful to port
        classes[c]=dict(v25=int(v25),ours=int(ours),truly=int(truly),nel=int(nel),pe=int(pe),tag=tag,
                        cluster=CLUSTER.get(c,"other"),kind=kind(c))

# sig map
sigs={}
with open(f"{SP}/v25_sig_map.tsv") as f:
    for line in f:
        k,s=line.rstrip("\n").split("\t",1); sigs.setdefault(k,s)

# member-level: read missing_full.tsv, keep TRULY_MISSING + NAME_ELSEWHERE
members=[]
with open(f"{SP}/missing_full.tsv") as f:
    for line in f:
        cat,tag,cls,m=line.rstrip("\n").split("\t")
        if cat=="PRESENT_EXACT": continue
        if cls not in classes: continue
        members.append(dict(cls=cls,method=m.split("::",1)[1],sig=sigs.get(m,m+"()"),
                            tag=tag,cat=cat,cluster=classes[cls]["cluster"],kind=classes[cls]["kind"]))

# write member CSV
with open(f"{OUT}/PORT_MAP.csv","w",newline="") as f:
    w=csv.writer(f); w.writerow(["cluster","kind","class","method","signature","type","confidence"])
    prio={c:i for i,c in enumerate(CLUSTER_ORDER)}
    members.sort(key=lambda x:(prio.get(x["cluster"],99),x["cls"],x["method"]))
    for x in members:
        conf="hard" if x["cat"]=="TRULY_MISSING" else "uncertain(name-elsewhere)"
        w.writerow([x["cluster"],x["kind"],x["cls"],x["method"],x["sig"],x["tag"],conf])
print("members written:",len(members))

# cluster summary
from collections import defaultdict
cl=defaultdict(lambda:dict(truly=0,nel=0,new_cls=0,drift_cls=0,classes=[]))
for c,d in classes.items():
    k=cl[d["cluster"]]; k["truly"]+=d["truly"]; k["nel"]+=d["nel"]
    if d["tag"]=="NEW": k["new_cls"]+=1
    else: k["drift_cls"]+=1
    k["classes"].append(c)
import json
with open(f"{SP}/cluster_summary.json","w") as f: json.dump(cl,f,indent=1,default=dict)
with open(f"{SP}/classes.json","w") as f: json.dump(classes,f,indent=1)
# print ordered cluster table
print("\ncluster\ttruly\tname_el\tNEWcls\tDRIFTcls")
for c in CLUSTER_ORDER+[x for x in cl if x not in CLUSTER_ORDER]:
    if c not in cl: continue
    k=cl[c]; print(f"{c}\t{k['truly']}\t{k['nel']}\t{k['new_cls']}\t{k['drift_cls']}")
