# JX3 Data-Layer Feature Atlas

Structural survey of the server DATA tree. Source of truth:
the stock deploy tree's `settings/` directory (镜像端 = "mirror/image server").
Data dated Nov 2023. Paths/filenames are GBK; use `LC_ALL=C` + `grep -a` + `iconv -f gbk`.

The client engine parses these as `KTabFile` (tab-separated, first line = header of column names).
A loader that reads a column index no longer present, or an empty cell where it expects an
integer, throws `[settings/xxx.tab]:GetInteger(row,col) failed`. Column count/order is the
main drift axis. Where relevant below, columns are given **0-based** (matching the engine's
`GetInteger(row,col)` index).

---

## Top-level structure of `settings/`

24 subdirs + ~90 loose tables at root. System map (Chinese → English / pinyin):

| Area | Location | Serves | Rough size |
|---|---|---|---|
| **Item / equipment** | `item/` | Item attribs, sets, enchant, boxes, custom gear | 13 files, biggest `Custom_Armor.tab` 4.6MB |
| **NPC templates** | `NpcTemplate/` + `NpcTemplate*.tab` | Per-NPC stats/AI/drops/skills | master `NpcTemplate.tab` 13MB + 278 per-zone subdirs |
| **AI** | `AIType/` + `AIType*.tab` + `AIDefine.ini` | AI behavior + per-NPC AI params | `AIType.tab` 2.3MB (25417 rows), 279 subdirs |
| **Skill / buff** | `skill/` | skills, buffs, recipes, events | `skills.tab` 1.9MB, `Buff.tab` 3.1MB |
| **Drops (NPC/class)** | `DropList/` | Boss/zone/class/world/money drop tables | 110 subdirs (per-zone) |
| **Drops (doodad)** | `DoodadDropList/` | Gatherable/doodad drops | 37 subdirs |
| **Map / scene** | `map/` + `MapList.tab` + `MapDrop.tab` + `MapGroup*/MapCreate*` | Scene defs, map-level drop routing | `map/` 92 entries; `MapList.tab` 18KB |
| **Quest** | `Quests/` + `Quests*.tab` | Quest scripts + master quest table | `QuestsOrigin.tab` 11MB, 144 subdirs |
| **Shop / vendor** | `shop/` + `NpcTemplateShopInfo.tab` + `RewardsShop.tab` | Vendor inventories by faction/city/arena | `shop/` 10 subdirs; `NpcTemplateShopInfo.tab` 635KB |
| **Currency shops** | `CoinShopPreOrder/`, `CoinShopDisCoupon/`, `BlackMarket/` | Cash/coin shop, black market | small |
| **Activity / events** | `Activity.tab` + `PQ.tab` + `RandomQuestGroup.tab` | Timed activities, public quests | `Activity.tab` 14KB |
| **Doodad** | `DoodadTemplate.tab` + `DoodadClass.tab` + `doodad_classify.ini` | World interactables | `DoodadTemplate.tab` 343KB |
| **Craft / profession** | `Craft/` | Recipes, professions, collection, enchant | 8 subdirs + `craft.tab`/`profession.tab` |
| **Cosmetic / social** | `Exterior/`, `HairShop/`, `FaceLift/`, `MiniAvatar/`, `Emotion/`, `Designation*Info.tab` | 外观 appearance, hair, face, emote, titles | small dirs |
| **Pet / mount** | `Domesticate/` + `FellowPet.tab` | 驯养 taming, pets | small |
| **Guild** | `Tong*.tab`/`TongConstList.ini`/`NpcOrder/` | 帮会 guild ops, templates | small |
| **Faction / camp** | `RelationForce.tab`, `RelationCamp.tab`, `CampLevelParam.tab`, `ReputationLimit.tab` | Force/camp relations, reputation | `RelationForce.tab` 22KB |
| **Battle / arena** | `BattleField.tab`, `ArenaMap.tab`, `TongBattleField.tab`, `DungeonQueue.tab` | PvP maps + queues | small |
| **Levelup / progression** | `LevelUpData/`, `Achievement*.tab`, `RegressionReward.tab`, `RegionList.ini` | Level curves, achievements, comeback rewards | small |
| **Misc control** | `TextFilter.txt`, `URLFilter.txt`, `AntiFarmer*`, `gm.tab`, `gmip.tab`, `MPak/` | Chat filter, anti-bot, GM whitelist | small |
| **Transport / road** | `TransmissionList.tab`, `Road/`, `JumpParam.tab`, `JumpFrameParam.tab` | Teleport, pathing, jump physics | small |

---

## `item/Set.tab` — LIVE DRIFT (equipment set bonuses)

**Location:** `settings/item/Set.tab` (52KB, 616 lines = header + 615 data rows, IDs 0–614).
**Purpose:** equipment set-bonus definitions. One row per gear set; grants bonus skills as more
set pieces are equipped.

**Shape:** 49 columns (constant across all rows — column count itself is NOT drifted).

```
col 0  ID
col 1  Name        (GBK set name)
col 2  UiID
col 3  2_1   col 4  2_2      <- 2-piece bonus (part 1 / part 2)
col 5  3_1   col 6  3_2      <- 3-piece
col 7  4_1   col 8  4_2      <- 4-piece
col 9  5_1   col 10 5_2      <- 5-piece
col 11 6_1   col 12 6_2      <- 6-piece   *** the failing column ***
col 13 7_1 ... up to col 47 24_1 col 48 24_2  <- tiers for 7..24 pieces
```

Pairs are `N_1`/`N_2` = the set-bonus granted when N pieces are worn. `_1` holds a
**set-skill / represent ID** (e.g. row ID 2 has `2_1=14221`, `4_1=7455`, `6_1=5172`); `_2` is
almost always blank in this data.

**The drift — `GetInteger(605, 11) failed (135×)`:**
- Column 11 (0-based) = **`6_1`**, the 6-piece set-bonus skill/represent ID.
- Real sets rarely reach 6 pieces, so `6_1` is left **empty** in 139 of 615 rows (matches the
  ~135× count). The loader iterates the full N-piece rectangle and calls `GetInteger` on every
  `N_1` cell; an empty string is not a valid integer → warning.
- This is a **tolerance drift, not a value drift**: the cell should be treated as "no bonus"
  (skip / default 0), not parsed as a required integer. The 2010 loader reads the range too
  eagerly for the way 2.5.2 data leaves higher tiers blank.
- Row 605: 1-based data row 606 = ID 604; the neighborhood (IDs 604/605) are sets with no
  6-piece tier, so `6_1` is blank there. Not a missing/renamed column — the value expected in
  col 11 is a set-skill ID and it is simply absent for short sets.
- (For reference, `5_2` at col 10 is empty in 612/615 rows — the `_2` half is essentially
  unused in this dataset, so any loader that also reads `_2` as a required int would flood too.)

**Fix direction:** make the Set loader treat empty `N_1`/`N_2` cells as absent (0/skip) rather
than calling `GetInteger` unconditionally. No column remap needed.

---

## NPC templates & AIType

**Master table:** `settings/NpcTemplate.tab` (13MB). **Per-zone overrides:** `NpcTemplate/<zone>/sNpcTemplate.tab`
(278 subdirs). **Defaults:** `NpcTemplateDefault.tab`. **List:** `NpcTemplateList.tab`.
`NpcTemplatePlus.tab` (88KB) and `NpcTemplateShopInfo.tab` (635KB, vendor stock) extend it.

**Column count: 250** — identical between `NpcTemplateDefault.tab` and the per-zone
`sNpcTemplate.tab` (schema parity confirmed, no column drift here).

Key columns (1-based, as printed by the header):

```
1  ID           5  Kind        7  ForceID     9  Species
10 DropClassID  <- drop routing (-> DropList / drop-class system)
11 Level        12 AdjustLevel
46 NpcDialogID
47 AIType             <- AI behavior selector
48 AIParamTemplateID  <- AI parameter-set key
49-52 Melee/Range weapon damage base/rand
53+  SkillID1/SkillLevel1/SkillInterval1/... (8-field skill blocks, repeating)
```

So **AIType lives at field 47, the drop hook at field 10 (`DropClassID`), AI params at field 48.**

### AI tables (two distinct "AIType" roles)

- `AIType.tab` (2.3MB, 25417 rows). Column 0 = `AIType`, then 21 behavior params
  (`ScriptFile`, `IsPositive`, `AlertRange`, `SearchPlayerDistance`, `AttackRange`, escape/
  wander/patrol fields, `NpcSceneType`, `DialogIdleTime` …). Two populations of col-0 IDs:
  - **293 small IDs (0–300)** = the actual **AI behavior definitions** with a `ScriptFile`.
    These are what `NpcTemplate.AIType` (field 47) selects.
  - **25123 large/negative IDs** = per-NPC-template **AI parameter instances** (template-hash
    keyed), referenced by `NpcTemplate.AIParamTemplateID` (field 48). This is why the row
    count (~25417) matches the runtime's "AI 25414 loaded".
- `AITypeDefault.tab` — single default row (`AIType 0` = `scripts/ai/StandardAI.lua`), 22 cols.
- `AITypeList.tab` — 264 `FilePath` entries: the master list of `sAIType.tab` files to load
  (`settings\AIType\<zone>\sAIType.tab`), one per behavior subdir under `AIType/` (279 dirs).
- `AIDefine.ini` — 19 named AI params with descriptions/defaults (editor metadata).

### AIType drift check (referenced vs defined)

Sampled `NpcTemplate.tab` field 47 usage: `AIType 6` dominates (12998 NPCs → StandardAI.lua),
then blank (5037, → default 0), then `0, 4, 22, 8, 5, 3, 9, 20, 16, 11, 1, 50, 28 … max 200`.
Verified programmatically: **every numeric AIType used by NpcTemplate.tab is defined in
AIType.tab col 0** ("ALL used AITypes are defined"). Max used = 200, well inside the 293 small
IDs. Field 48 (`AIParamTemplateID`) sample value 22 also resolves in AIType.tab.
**No AIType out-of-range / undefined drift** on the current master template data.

---

## Skill / Buff

**Location:** `settings/skill/`.
- `skills.tab` (1.9MB) — master skill table. Header cols 1-based: `1 SkillName, 2 SkillID,
  3 Design_Belong, 4 Design_Effect, 5 MaxLevel, 6 KindType, 7 FunctionType, 8 UIType,
  9 BelongKungfu, 10 BelongSchool, 11 CastMode, 12 WeaponRequest …` then a long tail of
  Is* flags and `SkillEventMask*`. (Skill loading passes at runtime per the recovery notes;
  NPC skills are referenced from NpcTemplate fields 53+.)
- `Buff.tab` (3.1MB), `BuffRecipe.tab`, `SkillEvent.tab` (63KB), `SkillRealization.tab`,
  `recipeSkill.tab`, `SkillLearning.tab` + `SkillLearning/`, `DynamicSkillGroup.tab`,
  `MainKungfuInfo.tab`, `KungFuExp.tab`, `DefenceCoefficient.tab`, `DecayType.tab`,
  `WeaponMapSkill.tab`, `PendentSkill.tab`.

---

## DropList (NPC / zone / class drops) — the "1,3,7 missing" situation

**Location:** `settings/DropList/` — 110 entries, mostly **per-zone / per-instance subdirs**
(e.g. `10人单BOSS` "10-man solo BOSS", `纯阳` Chunyang, `成都` Chengdu, `帮会` guild, `节日`
festival), plus structural dirs:
- `ClassDrop/` — `Beast/` (per-creature-type, e.g. `Fish_1.tab`), `WorldDrop/` (per-item, e.g.
  `biaoju.tab`), `MoneyDrop.tab`.
- `MapDrop/` — per-zone gear-drop tables (`chunyang_gear_MapDrop.tab`, `jinshui_MapDrop.tab` …),
  routed by the top-level `MapDrop.tab`.

Leaf drop files are **named**, not numeric — e.g. `gear_tianwangdian.tab`, `craft_boss.tab`,
`badge.tab`, or Chinese `1号.tab`/`3号.tab` (号 = "No.") inside a few zone dirs.

**Regarding files reported missing as `1`, `3`, `7`:** confirmed there are **no bare-numeric
files** (`1`, `3`, `7`, `1.tab`, `3.tab`, `7.tab`) anywhere under `DropList/`
(`find` returned empty). The runtime "missing" reports are therefore a **reference/version
drift**, not a corrupted extraction: the 2010 loader resolves a drop by a numeric ID (from
`NpcTemplate.DropClassID` field 10, or a drop-class index) and expects a file/row keyed by that
integer, but the 2.5.2 data reorganized drops into **named per-zone `.tab` files** with no
integer-keyed file. So low IDs like 1/3/7 have no target and log as missing. This is the same
family as the map-drop redesign noted in project memory (`MapList.MapDrop=ID → MapDrop.tab →
slots`, where the 2010 source treated the field as a filename). Treatment so far has been
tolerant-skip, not a data fix.

---

## Map / scene

**Location:** `settings/map/` (92 entries, per-zone scene resource dirs, GBK-named:
`长安` Chang'an, `成都` Chengdu, `纯阳` Chunyang, `恶人谷` Evil Valley, instances like
`10人英雄荻花圣殿` "10-man heroic Dihua Shrine"). One stray `111.tar.gz` archive present.

**Master tables at root:**
- `MapList.tab` (18KB) — scene registry. Cols 1-based: `1 ID, 2 Name, 3 DisplayName, 4 Perform,
  6 MaxLootRange, 7 bCanPK, 8 bCanDuel, 9 Type, 10 MaxCopyCount, 11/12 Min/MaxPlayerCount,
  18 MapDrop (-> MapDrop.tab ID), 26 ResourcePath, 30 QuestCountAchID, 31 CampType,
  32 FightList …` (40+ cols).
- `MapDrop.tab` — cols `ID, MapDrop1, DropType1 … MapDrop8, DropType8`; `MapDropN` = path into
  `DropList/MapDrop/*.tab`, `DropTypeN` = numeric drop type.
- `MapCreate.tab` / `MapCreate_12.tab` / `MapCreate_MJN.tab`, `MapGroup*.tab` (several regional
  variants + a GBK `MapGroup-简体.tab`), `MapQueue.tab`, `ArenaMap.tab`.

---

## Activity / Quest / Shop (survey depth)

- **Activity:** `Activity.tab` cols `ID, Activity, ParentID1..5, StartTimeRule, Cycle, Duration,
  ShowUI, ServerScript, CenterScript` — timed activities with a parent hierarchy + server/center
  Lua hooks. `PQ.tab` public quests, `RandomQuestGroup.tab`.
- **Quest:** `Quests.tab` (5.4MB) + `QuestsOrigin.tab` (11MB, raw) + `QuestsDefault.tab` +
  `QuestsList.tab`, with `Quests/` holding 144 per-quest-group script subdirs.
- **Shop:** `shop/` split by channel — `Arena, City, Contribution, Craft, Currency, Festival,
  Force, Justice, Newbie` + `NpcShopList.tab`. Vendor stock joined via
  `NpcTemplateShopInfo.tab` (635KB) keyed off NPC template.

---

## Drift summary

| Table | Drift | Status |
|---|---|---|
| `item/Set.tab` | col 11 (`6_1`) empty in ~135 rows → `GetInteger` on blank; also `_2` halves blank. Tolerance, not schema/value. | **live**, fix = skip empty cells |
| `DropList` numeric refs (1/3/7…) | 2010 loader wants integer-keyed drop files; 2.5.2 uses named per-zone `.tab`. No bare-numeric files exist. | reference drift, tolerant-skipped |
| `MapList.MapDrop` | field is ID→`MapDrop.tab`→slots in 2.5.2; 2010 treated as filename. | known, tolerant-skipped |
| NpcTemplate schema | 250 cols, Default == per-zone sNpcTemplate; no column drift. | clean |
| AIType (field 47) | all used values defined in AIType.tab (max 200 ≤ 293 defined). | clean |
