# Feature-System Atlas — SO3GameServer 2010 → v2.5.2 (v246)

Bản đồ **cấu trúc code theo hệ thống gameplay**, dựng trên
[`README.md`](../README.md)
(diff symbol-level, ground-truth = DWARF `jx3_dwarf/SO3GameServerD`). Không lặp lại diff;
đây là lớp system-level ở trên nó.

Ký hiệu mỗi hệ thống:
- **Composition**: class thuộc hệ (từ PORT_MAP), tag NEW/DRIFT, `truly` = method port thật.
- **Completeness**: `v25` = tổng method surface v246 của các class chính (số cần coverage-diff),
  `ours` = đã port, `truly` = còn thiếu. Coverage ≈ `ours / v25`.
- **Deps**: cạnh CALLS-INTO. `⇒` hard (không chạy được nếu thiếu), `→` soft.
- **Role**: vị trí trên vòng lặp lõi (login→move→combat→kill→loot→level).

Các con số `v25/ours/truly` là **method định nghĩa trong `.text`** của class-có-việc trong hệ;
inline/template/header không tính. Tổng dự án: **v25=4080 ours=2456 truly=1581** (class-có-việc).

---

## 1. player-core (KPlayer / KPlayerServer / KCharacter) — MEGA-HUB / SPINE

**Composition** (6 class, truly **588**):

| class | v25 | ours | tag | truly | vai trò |
|---|---|---|---|---|---|
| KPlayer | 720 | 385 | DRIFT | 344 | role-object: state, save/load, packet, mọi feature gắn vào đây |
| KPlayerServer | 560 | 398 | DRIFT | 184 | player manager: sync role-data giữa GS↔center, login/logout |
| KCharacter | 263 | 210 | DRIFT | 54 | base của Player+Npc: attribute, buff, di chuyển, combat-recv |
| KCharacter{Stealth,Disarm,CounterStealth}ChangedFunc | 2×3 | 0 | NEW | 6 | callback state-change (tàng hình/tước vũ khí) |

**Completeness**: v25=1549, ours=993, truly=588 → coverage ~64%. Đây là hệ lớn nhất và
là **nút giao của mọi hệ khác**.

**Deps** (player-core gọi vào gần như tất cả): ⇒ skill, ⇒ item, ⇒ world-scene,
⇒ movement-sync, → quest, → social(guild/mentor), → economy(currency), → progression,
→ pet, → activity. Ngược lại **mọi hệ khác đều gọi vào KPlayer** (nó là role container).

**Role**: **core-loop-gating tuyệt đối** — không có KPlayer thì không có gì cả.

**SPINE (must-do-coherently)** — xem §serialization-spine cuối file. KPlayer::Save/Load
dispatch ra ~24 sub-blob `SaveXxx(uint*,uchar*,uint)` / `LoadXxx(uchar*,uint)`, mỗi blob
là một feature. Blob layout + versioning (LoadStateInfo_V0/V1/V2, LoadPendentDataV0/V2)
là phần **bắt buộc port đồng bộ**; các method còn lại (gameplay handler lẻ) port per-feature.

---

## 2. skill-script (技能 / Lua binding) — HUB

**Composition** (12 class, truly **172**):

| class | v25 | ours | tag | truly |
|---|---|---|---|---|
| KScriptFuncList | 217 | 95 | DRIFT | 123 |
| KSkillManager | 54 | 43 | DRIFT | 15 |
| KSkill | 71 | 63 | DRIFT | 8 |
| KScriptCenter | 34 | 27 | DRIFT | 7 |
| KSkillList | 25 | 20 | DRIFT | 5 |
| KBuffList | 40 | 36 | DRIFT | 4 |
| KSkillRecipeList | 20 | 17 | DRIFT | 3 |
| KBroadcastCastSkillFunc, KRecipeList, KSkillRectangleTravFunc, KGetAllCopyIndexByMapIDFunc, GetSkillRecipeTraverseFunc | — | — | mix | 7 |

**Completeness**: v25=480, ours=313, truly=172 → coverage ~65%. `KScriptFuncList` (123 truly)
là API Lua exposed cho script skill/buff — mọi skill effect chạy qua đây.

**Deps**: ⇒ player-core (KScriptFuncList nhận KCharacter*/KPlayer*/KTarget), ⇒ item
(recipe/skill trên trang bị), → world-scene (skill spawn NPC/doodad, region query),
→ drop-loot (KBroadcastCastSkillFunc), → buff (KBuffList).

**Role**: **core-loop-gating** — combat chạy hoàn toàn bằng skill+buff qua Lua. Nếu binding
nil, skill fail (đúng lỗi runtime `GetXxx nil` đã gặp). Đây là hub cần port sát vì đụng
KCharacter offset.

---

## 3. item / equipment (物品) — HUB

**Composition** (1 class chính, truly **70**): `KItemList` 161→94 DRIFT, truly 70.
(IItem interface + item subtype nằm rải trong network-infra/economy signatures.)

**Completeness**: v25=161, ours=94, truly=70 → coverage ~58%.

**Deps**: ⇒ player-core (item thuộc túi/trang bị KPlayer), → skill (recipe/diamond gắn skill),
→ economy (mua/bán tạo IItem*), → drop-loot (sinh IItem*). KItemList là factory/registry.

**Role**: **core-loop-gating** — loot→trang bị→attribute là mắt xích vòng lặp. Item struct
(KEQUIP_DESC, KEquipedDiamondInfo, WEAPON_DETAIL_TYPE) là chung, drift item = drift kinh tế + drop.

---

## 4. world-scene (场景/NPC/Doodad/AI) — HUB

**Composition** (14 class, truly **97**):

| class | v25 | ours | tag | truly |
|---|---|---|---|---|
| KScene | 148 | 108 | DRIFT | 41 |
| KNpc | 86 | 63 | DRIFT | 26 |
| KSO3World | 26 | 21 | DRIFT | 5 |
| KNpcTemplateList | 14 | 9 | DRIFT | 5 |
| KDoodad | 61 | 57 | DRIFT | 4 |
| KWorldSettings | 7 | 2 | DRIFT | 3 |
| KSimpThreatList | 32 | 32 | DRIFT | 2 |
| KRoadManager | 30 | 28 | DRIFT | 2 |
| KNpcReviveManager, KDoodadReviveManager, KAIManager, KAIPatrolGroupManager, KNpcTemplate, KNpcAdronTab | — | — | mix | 9 |

**Completeness**: v25=473, ours=377, truly=97 → coverage ~80% (đã cao). Nhưng KScene(41)+KNpc(26)
là nơi combat/AI/spawn diễn ra.

**Deps**: ⇒ player-core (character sống trong scene), ⇒ skill (NPC cast skill, threat),
⇒ movement-sync (di chuyển trong scene), → drop-loot (NPC chết → drop), → item (doodad chứa item),
→ activity (scene-level activity notify).

**Role**: **core-loop-gating** — move+combat+kill xảy ra ở đây. AI (KAIManager/threat) drive
NPC hành vi. `KNpcReviveManager`/`KDoodadReviveManager` = respawn loop.

---

## 5. combat / AI

Không có cluster riêng trong PORT_MAP — **combat là hành vi phát sinh (emergent)** từ
skill-script + world-scene(AI/threat) + player-core(KCharacter attribute/damage-recv).
Composition thực = KCharacter(54) + KSkill/KBuffList(skill-script) + KSimpThreatList(2) +
KAIManager(1)/KAIPatrolGroupManager(1). Không có "KCombat" class.

**Deps**: ⇒ skill-script ⇒ player-core ⇒ world-scene. Hard vào cả ba.
**Role**: **core-loop-gating**. Port combat = port coherent 3 hub trên, không có leaf riêng.

---

## 6. movement-sync (移动同步)

**Composition** (8 class, truly **29**): KGMoveProcessor(18 NEW), KGJumpList(4 NEW),
KGMoveParam(2 NEW), KMoveProcessTraverseFunc(1), SYNC_MOVE_{STATE,SPEED,CTRL}/SYNC_SPRINT_FLAGS(4 NEW).
Toàn bộ **NEW** (v25=35, ours=0, truly=29) — hệ vắng hẳn khỏi ours.

**Completeness**: coverage ~0%. Nhưng 2010 có move cơ bản (KCharacter tự move); đây là
**redesign server-side move validation** (KGMoveProcessor chạy thread riêng: ProcessMove,
CheckSlip, water, obstacle).

**Deps**: ⇒ world-scene (obstacle/region/waterline), ⇒ player-core (KCharacter position).
**Role**: **core-loop-gating** (move là bước 2 của vòng lõi) nhưng phiên bản 2010 đủ chạy tối
thiểu; port đầy đủ = anti-cheat/độ mượt.

---

## 7. drop-loot (掉落)

**Composition** (7 class, truly **29**): KDropCenter(11), KManualDropList(7 NEW), KLootList(3),
KMoneyDropList(2), KDropList(2), KLevelDropList(2), KReputeLootBuffList(2 NEW).

**Completeness**: v25=105, ours=72, truly=29 → coverage ~69%.

**Deps**: ⇒ item (sinh IItem*), ⇒ world-scene (NPC chết trigger), ⇒ player-core (gán loot),
→ economy (KMoneyDropList → currency), → reputation (KReputeLootBuffList).
**Role**: **core-loop-gating** — "kill→loot" là mắt xích. KDropCenter là entry.

---

## 8. quest (任务)

**Composition** (2 class, truly **9**): KQuestList(4 DRIFT), KQuestInfoList(5 DRIFT).
**Completeness**: v25=75, ours=66, truly=9 → coverage ~88% (gần đủ).
**Deps**: ⇒ player-core (quest-state lưu trong KPlayer::SaveQuestStateAndList), → item (reward),
→ npc (giao/nộp), → drop (quest-item). **Role**: **progression** (không chặn vòng lõi tối thiểu).

---

## 9. economy — cash-shop / currency / cosmetic

Gom 3 cluster liên quan chặt (mua-bán khép kín):

**9a. cash-shop-currency** (9 class, truly **68**, v25=133 ours=48 ~36%):
KDiamondManager(25 NEW), KRewardsShop(10 NEW), KShopCenter(6), KShop(6), KRewardsBox(5 NEW),
KRewardsShopSettings(5 NEW), KCurrency(4 NEW), KCurrencyList(3 NEW), KTradingBox(4).

**9b. exterior-cosmetic** (10 class, truly **107**, v25=183 ours=49 ~27%):
KExterior(19), KHairShop(19 NEW), KExteriorBox(17), KDesignation(16), KMiniAvatar(10 NEW),
KNewExtPointManager(10 NEW), KHairBox(7 NEW), KMiniAvatarSettings(6 NEW), KModelInfo(3 NEW),
KPendentOldDataInfoList(0).

**Deps**: economy ⇒ item (IItem* mua/bán/craft), ⇒ currency (KDIAMOND/元宝/KCurrency),
⇒ player-core (túi/ví/ngoại hình gắn KPlayer). cosmetic → item (幻化 dựa trang bị).
Vòng currency khép kín (đã done Tier-1/2 theo memory: designation/ranklist/currency/shop-buy).
**Role**: cash-shop-currency = **progression/QoL**; exterior-cosmetic = **cosmetic-QoL** (đã port
riêng thành công, xem `docs/exterior_port`). Không chặn vòng lõi.

---

## 10. social — guild / mentor / activity-camp / pk / rank

**10a. guild-tong** (帮会) (3 class, truly **27**, toàn NEW): KTongServer(13), KTongDiplomacyCache(12),
KTongConstList(2). Deps ⇒ player-core (KTONG_DIPLOMACY_RELATION_INFO, KTONG_WAR_KILL_INFO_SYNC
lưu/sync trong KPlayer), → economy (tong shop). Role **social**.

**10b. mentor** (师徒) (2 class, truly **6** + 14 nghi): KMentorCache(2), KDirectMentorCache(4 NEW,
14 name-elsewhere). Deps ⇒ player-core (KPlayer::Save/LoadMentorData blob). Role **social**.

**10c. activity-camp** (活动/阵营) (4 class, truly **24**): KActivityMgrServer(10 NEW), KCampInfo(10),
KCampActiveStat(3 NEW), KActivityEndNotifySceneFunc(1). Deps ⇒ player-core, → world-scene
(scene notify), → drop/reward. Role **social/progression**.

**10d. pk** (1 class, truly **3**): KPKController(29→34, ours>v25 nên gần đủ). Deps ⇒ player-core,
⇒ skill (damage). Role **social** (PvP).

**10e. rank** (排行榜) (1 class, truly **3** NEW): KRankListServer. Deps → player-core (đọc stat).
Role **social/QoL**.

---

## 11. progression — talent / achievement / reputation / regression

**11a. talent** (天赋) (1 class, truly **12** NEW): KTalentSkill. Deps ⇒ skill (talent = skill mod),
⇒ player-core. Role **progression**.

**11b. achievement** (成就) (2 class, truly **5**): KAchievement(4), KAchievementInfoList(1).
Deps → player-core (nhiều event trigger). Role **progression**.

**11c. reputation** (声望) (1 class, truly **2**): KReputation(13, gần đủ). Deps ⇒ player-core,
→ drop (KReputeLootBuffList). Role **progression**.

**11d. regression** (回归) (2 class, truly **20** NEW): KRegressionManager(10), KRegressionPlayerData(10).
Deps ⇒ player-core (KPlayer::LoadRandData/state). Role **progression/QoL** (khuyến khích quay lại).

---

## 12. pet-domesticate (宠物/驯养)

**Composition** (6 class, truly **60**, toàn NEW, v25=78 ours=0 ~0%): KDomesticate(31),
KFellowPetBox(10), KDomesticateCenter(8), KFellowPetMgr(5), KPlayerRelatedPetAttrImportParam(3),
KPlayerRelatedPetAttrExportParam(3).

**Completeness**: coverage ~0% — hệ mới hoàn toàn ở v246.
**Deps**: ⇒ player-core (KFOLLOW_TYPE, pet attr import/export vào KPlayer), ⇒ skill (pet cast),
→ item (thức ăn/đạo cụ thuần hoá), → world-scene (pet spawn).
**Role**: **progression** (pet là hệ phụ, không chặn vòng lõi). Self-contained → port nguyên khối
(giống KExterior).

---

## 13. anti-farmer (防工作室) — peripheral

**Composition** (3 class, truly **10** NEW): KAntiFarmerServer(5), KAntiFarmerSettings(3),
KBandwidthOptimizeTacticsManager(2). Deps → player-core (giám sát hành vi). Role **peripheral/QoL**.

## 14. gm / recorder-replay — peripheral tooling

gm: KGMList(4 truly). recorder-replay: KRecorderSceneLoader{Normal,Record,Replay}(1 mỗi cái, gần đủ).
Deps → player-core/world-scene. Role **peripheral** (công cụ vận hành, không gameplay).

## 15. network-infra (TÁCH — không phải gameplay content)

**Composition** (9 class, truly **233**): KRelayClient(154), KLogClient(41), KStatDataServer(22),
KLocalGSDataStat(13 NEW), KG_AsyncSocketStream(1), KG_SocketServerAcceptor(1), KGWConstList(1),
KCenterRemote(0/7 nghi), KGWServerConstList(0/3 nghi).

**Completeness**: v25=541 ours=323 truly=233. Nhưng **tầng network đã thông** (milestone #22,
xem MEMORY). Phần lớn 233 là protocol-handler/RPC bổ sung, đánh giá per-method — nhiều hàm đã
cover ở tầng protocol hoặc không cần cho gameplay.
**Deps**: mọi hệ dùng nó để sync GS↔center↔gateway, nhưng đó là **transport**, không phải logic hệ.
**Role**: **infra** (đã đủ chạy). Không xếp vào tiering gameplay.

---

## Dependency graph (adjacency list)

`⇒` = hard (chặn), `→` = soft. Ba hub lõi: **player-core, skill-script, world-scene** —
mọi cạnh cuối cùng đổ về player-core.

```
player-core   ⇒ skill-script, item, world-scene, movement-sync
              → quest, guild, mentor, currency, progression*, pet, activity
skill-script  ⇒ player-core, item        → world-scene, drop-loot, buff
item          ⇒ player-core              → skill-script, economy, drop-loot
world-scene   ⇒ player-core, skill-script, movement-sync → drop-loot, item, activity
combat(*emergent) ⇒ skill-script, player-core, world-scene
movement-sync ⇒ world-scene, player-core
drop-loot     ⇒ item, world-scene, player-core → economy(money), reputation
quest         ⇒ player-core              → item, world-scene(npc), drop-loot
economy       ⇒ item, currency, player-core
  cosmetic    ⇒ player-core, currency    → item(幻化)
guild-tong    ⇒ player-core              → economy
mentor        ⇒ player-core
activity-camp ⇒ player-core              → world-scene, drop-loot
pk            ⇒ player-core, skill-script
rank          → player-core
talent        ⇒ skill-script, player-core
achievement   → player-core
reputation    ⇒ player-core              → drop-loot
regression    ⇒ player-core
pet           ⇒ player-core, skill-script → item, world-scene
anti-farmer   → player-core
gm/recorder   → player-core, world-scene
network-infra : transport cho tất cả (không phải cạnh logic)
```

**Nút vào (in-degree) cao nhất = player-core** (mọi hệ trỏ tới). Sau đó skill-script,
item, world-scene. Đây là lý do PORT_MAP xếp 4 hub này port **cuối** (đổi struct/offset lan rộng).

---

## Playability tiering

| tier | hệ thống | truly | lý do |
|---|---|---|---|
| **core-loop-gating** | player-core | 588 | role object, không có = không có game |
| core-loop-gating | skill-script | 172 | combat chạy bằng skill/buff Lua |
| core-loop-gating | world-scene | 97 | move+combat+kill xảy ra ở scene/NPC/AI |
| core-loop-gating | item | 70 | loot→trang bị→attribute |
| core-loop-gating | drop-loot | 29 | mắt xích kill→loot |
| core-loop-gating | movement-sync | 29 | bước "move" (2010 có bản tối thiểu) |
| core-loop-gating | combat | (emergent) | = skill+scene+character, không class riêng |
| **progression** | economy(shop/currency) | 68 | mua-bán, ví |
| progression | pet-domesticate | 60 | hệ pet |
| progression | activity-camp | 24 | hoạt động/trận doanh |
| progression | regression | 20 | hệ hồi quy |
| progression | talent | 12 | nhánh thiên phú |
| progression | quest | 9 | nhiệm vụ (gần đủ, 88%) |
| progression | achievement | 5 | thành tựu |
| progression | reputation | 2 | thanh danh (gần đủ) |
| **social** | guild-tong | 27 | bang hội |
| social | mentor | 6 | sư đồ |
| social | pk | 3 | PvP (gần đủ) |
| social | rank | 3 | bảng xếp hạng |
| **cosmetic-QoL** | exterior-cosmetic | 107 | ngoại hình/tóc/称号/幻化 (đã port riêng) |
| cosmetic-QoL | anti-farmer | 10 | chống studio |
| **peripheral/infra** | network-infra | 233 | transport (đã thông) |
| peripheral | gm / recorder-replay | 4 / 3 | công cụ vận hành |

---

## Serialization spine — phần "bắt buộc port đồng bộ" (must-do-coherently)

Ba mega-hub (KPlayer 344, KPlayerServer 184, KItemList 70, KCharacter 54, KScene 41,
KScriptFuncList 123) **không port per-feature nguyên khối được** vì drift lan qua struct/offset
dùng chung. Nhưng bên trong chúng, chỉ một phần là **spine bắt buộc coherent**, phần còn lại
là handler độc lập port theo từng feature.

**Spine của KPlayer = Save/Load blob dispatch.** `KPlayer::Save(uint*,uchar*,uint)` và
`KPlayer::Load(uchar*,uint)` gọi tuần tự ~24 sub-blob, mỗi blob thuộc một feature:

```
SaveBaseInfo/LoadBaseInfo(KRoleBaseInfo*)   → player-core (khung role)
SaveStateInfo / LoadStateInfo_V0/V1/V2      → player-core (VERSIONED — bắt buộc đúng version)
SaveSkillRecipeList / LoadSkillRecipeList   → skill-script
SaveQuestStateAndList / LoadQuestData       → quest
SaveMentorData / LoadMentorData             → mentor
SaveTongExtData / LoadTongExtData           → guild-tong
SaveVisitTongInviteList / Load…             → guild-tong
SavePendentData / LoadPendentDataV0/V2      → cosmetic (pendant, VERSIONED)
SaveActivityVariables / Load…               → activity
SaveArenaData / LoadArenaData               → pk/arena
SaveHeroData / LoadHeroData                 → progression
SaveDropSurpriseData / Load…                → drop-loot
SaveSingleDungeonData / Load…               → world/dungeon
SaveRandData / LoadRandData, SaveAccount…   → player-core/account
SaveBankPasswordData, SaveDynamicPackageSize → player-core (túi/bank)
```

**Quy tắc port:**
1. **Spine (bắt buộc coherent, làm TRƯỚC khi bật feature)**: layout của blob + thứ tự dispatch +
   **versioning** (`_V0/_V1/_V2`, `LoadPendentDataV0/V2`). Sai một byte skew toàn bộ role-data
   (đúng bài học network header 3B→2B trong MEMORY). Nếu bật feature X, phải port đúng
   `SaveXData/LoadXData` của nó cùng version — không được để blob lệch với các blob khác trong cùng Save().
2. **KRoleBaseInfo / KRoleBaseData struct** = block chung; KPlayerServer::DoSyncRoleDataSection…
   sync theo section → section boundary phải khớp center. Đây là spine liên-process.
3. **Packet struct** (INTERNAL_PROTOCOL_HEADER, các KXXX_PARAM: KSHOP_BUY_ITEM_PARAM,
   KHAIRSHOP_BUY_PARAM, KEXTERIOR_COIN_BUY_PARAM…) — `#pragma pack(1)`, size cứng; đổi field
   phải grow đúng như đã làm ở tầng network. Cũng là spine (kế thừa header chung).
4. **Independent per-feature** (KHÔNG phải spine): các gameplay handler lẻ (`OnXxxRequest`,
   `DoXxx`, getter/setter) trong KPlayer/KPlayerServer/KScriptFuncList — port khi bật feature đó,
   không đụng blob/packet layout. Đây là phần lớn 344 truly của KPlayer.

**Kết luận spine**: "must-do-coherently" = (a) chuỗi Save/Load blob + version của nó,
(b) KRoleBaseInfo/section-sync boundary với center, (c) packet struct kế thừa
INTERNAL_PROTOCOL_HEADER. Ba thứ này port thành một khối. Mọi method khác của hub =
per-feature, gắn theo hệ tương ứng ở các mục trên.
