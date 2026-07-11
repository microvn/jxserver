# PORT MAP — SO3GameServer 2010 → v2.5 (build ~4503)

Danh sách hàm/class cần port từ source 2010 (`SO3GameServer_ours`) lên chuẩn v2.5,
lấy **binary DWARF làm ground-truth** bằng diff tầng symbol-đã-link (không grep source
để tìm thiếu — grep chỉ dùng để loại false-missing).

## Phương pháp & nguồn

- **V25 (ground-truth)**: `jx3_dwarf/SO3GameServerD` — ELF32, không strip, full DWARF.
  pyghidra: `SO3GameServerD-2d2a24` (đã xác nhận, vd `KDomesticate::Init @ 082103b4`).
- **OURS**: `172.105.112.239:/root/jx3/镜像端/extracted/root/SO3GameServer_ours` — build Jul 8, không strip.
- **Diff**: `nm` → giữ `_ZN…` định nghĩa trong `.text` (`T`/`t`) → demangle (`c++filt -n -p`)
  → `Class::method` → lọc `std::/__gnu/__cxx/_/operator/template/anon` → `LC_ALL=C sort -u` → `comm -23`.
  (Toàn bộ pipeline ép `LC_ALL=C`: nm-diff giữa macOS↔Linux sai nếu collation lệch.)
- **Chống false-missing**: mỗi hàm "missing" đối chiếu source `linux-build/src` + `include`:
  - `PRESENT_EXACT` — `Class::method` có trong source (inline/không emit) → **loại, không port**.
  - `NAME_ELSEWHERE` — tên hàm trần có ở class khác → nghi ngờ (relocate hoặc trùng tên chung).
  - `TRULY_MISSING` — tên không xuất hiện trong source → **port thật**.
- Đã xác nhận `ours` build từ chính `linux-build/src` (vd `KExterior::LoadExteriorInfoTable`).

## Con số

| chỉ số | giá trị | độ tin |
|---|---|---|
| V25 method dự án (class-scoped) | 4820 (235 class) | đếm |
| OURS method dự án | 3335 (198 class) | đếm |
| Method thiếu (raw, sau comm) | 1732 | đếm |
| — `PRESENT_EXACT` (loại) | 16 | đếm |
| — `NAME_ELSEWHERE` (nghi ngờ) | 135 | đếm |
| — **`TRULY_MISSING` (port thật)** | **1581** | **CỨNG** |
| **NEW class** (vắng hẳn khỏi ours) | **54 class / 438 method** | **CỨNG** |
| DRIFT (class có ở cả hai, thiếu method) | 54 class / 1256 truly (+22 nghi) | ước lượng cao |
| network-infra (tách riêng, xem dưới) | 9 class / 233 truly | tách |
| **Gameplay thật (trừ network-infra)** | **1348 truly** | — |

**NEW = số CỨNG** (class không tồn tại trong ours → port nguyên khối; `NAME_ELSEWHERE`
trong NEW chỉ là trùng tên `Init/UnInit/Load…` với class khác, KHÔNG phải đã có logic).
**DRIFT = ước lượng cao**: 1256 truly-missing đã trừ 16 present-exact; 22 method còn cờ
`NAME_ELSEWHERE` (có thể là bản relocate/rename — cần xác minh từng hàm khi port).

Đã kiểm: cả 54 NEW class KHÔNG do `.so` link sẵn cấp
(`libSO3ItemHouseD/libEngine_Lua5D/libSO3EnumConvertorD` chỉ export 7 class, không giao NEW)
→ tất cả phải port vào binary chính.

## Thứ tự port đề xuất (leaf/self-contained trước, hub sau)

| # | cụm | truly | nghi | NEW-cls | DRIFT-cls |
|---|---|---|---|---|---|
| 1 | movement-sync | 29 | 6 | 8 | 0 |
| 2 | pet-domesticate (驯养/宠物) | 60 | 18 | 6 | 0 |
| 3 | exterior-cosmetic (外观/发型/幻化) | 107 | 28 | 7 | 3 |
| 4 | cash-shop-currency (商城/货币/元宝) | 68 | 19 | 6 | 3 |
| 5 | regression (回归) | 20 | 4 | 2 | 0 |
| 6 | mentor (师徒) | 6 | 14 | 1 | 1 |
| 7 | anti-farmer (防工作室) | 10 | 5 | 3 | 0 |
| 8 | rank (排行榜) | 3 | 2 | 1 | 0 |
| 9 | guild-tong (帮会) | 27 | 4 | 3 | 0 |
| 10 | talent (天赋) | 12 | 2 | 1 | 0 |
| 11 | activity (活动/阵营) | 24 | 7 | 3 | 1 |
| 12 | achievement (成就) | 5 | 1 | 0 | 2 |
| 13 | quest (任务) | 9 | 0 | 0 | 2 |
| 14 | reputation (声望) | 2 | 0 | 0 | 1 |
| 15 | pk | 3 | 0 | 0 | 1 |
| 16 | gm | 4 | 0 | 0 | 1 |
| 17 | recorder-replay (录像) | 3 | 0 | 0 | 3 |
| 18 | drop-loot (掉落) | 29 | 8 | 2 | 5 |
| 19 | **skill-script (技能/Lua binding — HUB)** | 172 | 0 | 4 | 8 |
| 20 | **world-scene (场景/NPC/Doodad/AI — HUB)** | 97 | 4 | 2 | 12 |
| 21 | **item (物品 — HUB)** | 70 | 0 | 0 | 1 |
| 22 | **player-hub (KPlayer/Server/Character — HUB)** | 588 | 2 | 3 | 3 |
| 23 | network-infra (relay/log/stat/socket — TÁCH) | 233 | 11 | 2 | 7 |

**Lý do thứ tự**: #1–18 là leaf/self-contained (managers độc lập, giống KExterior đã port
thành công) — port nguyên khối, ít đụng combat/AI. #19–22 là hub (đụng KPlayer/KCharacter/
combat/Lua), để cuối vì đổi struct/offset lan rộng. #23 tách riêng.

### Cờ network-infra (KHÔNG chắc gameplay cần)

`KRelayClient`(154), `KLogClient`(41), `KStatDataServer`(22), `KLocalGSDataStat`(13),
`KCenterRemote`, `KGWServerConstList`, `KGWConstList`, `KG_SocketServerAcceptor`,
`KG_AsyncSocketStream` — plumbing mạng/telemetry. Tier network đã thông ở milestone #22;
phần lớn 233 method này là **protocol handler / RPC bổ sung**, không phải gameplay content.
Đánh giá per-method trước khi port; nhiều hàm có thể đã cover ở tầng protocol hoặc không cần.

## Bảng class chi tiết (chỉ class có việc, sắp theo `truly` giảm dần)

`v2.5` / `ours` = số method định nghĩa trong `.text`.
`truly` = port thật · `nghi` = name-elsewhere · `pe` = present-exact (loại).

| class | v2.5 | ours | tag | truly | nghi | pe | kind | cụm |
|---|---|---|---|---|---|---|---|---|
| KPlayer | 720 | 385 | DRIFT | 344 | 1 | 4 | hub | player-hub |
| KPlayerServer | 560 | 398 | DRIFT | 184 | 1 | 0 | hub | player-hub |
| KRelayClient | 374 | 240 | DRIFT | 154 | 0 | 0 | network-infra | network-infra |
| KScriptFuncList | 217 | 95 | DRIFT | 123 | 0 | 0 | hub | skill-script |
| KItemList | 161 | 94 | DRIFT | 70 | 0 | 0 | hub | item |
| KCharacter | 263 | 210 | DRIFT | 54 | 0 | 0 | hub | player-hub |
| KScene | 148 | 108 | DRIFT | 41 | 0 | 0 | hub | world-scene |
| KLogClient | 59 | 24 | DRIFT | 41 | 0 | 0 | network-infra | network-infra |
| KDomesticate | 38 | 0 | NEW | 31 | 7 | 0 | self-contained | pet-domesticate |
| KNpc | 86 | 63 | DRIFT | 26 | 0 | 0 | hub | world-scene |
| KDiamondManager | 27 | 0 | NEW | 25 | 2 | 0 | self-contained | cash-shop-currency |
| KStatDataServer | 58 | 36 | DRIFT | 22 | 0 | 0 | network-infra | network-infra |
| KExterior | 34 | 11 | DRIFT | 19 | 5 | 0 | self-contained | exterior-cosmetic |
| KHairShop | 21 | 0 | NEW | 19 | 2 | 0 | self-contained | exterior-cosmetic |
| KGMoveProcessor | 22 | 0 | NEW | 18 | 4 | 0 | self-contained | movement-sync |
| KExteriorBox | 42 | 23 | DRIFT | 17 | 1 | 10 | self-contained | exterior-cosmetic |
| KDesignation | 31 | 15 | DRIFT | 16 | 1 | 0 | self-contained | exterior-cosmetic |
| KSkillManager | 54 | 43 | DRIFT | 15 | 0 | 0 | hub | skill-script |
| KTongServer | 13 | 0 | NEW | 13 | 0 | 0 | self-contained | guild-tong |
| KLocalGSDataStat | 13 | 0 | NEW | 13 | 0 | 0 | network-infra | network-infra |
| KTongDiplomacyCache | 14 | 0 | NEW | 12 | 2 | 0 | self-contained | guild-tong |
| KTalentSkill | 14 | 0 | NEW | 12 | 2 | 0 | self-contained | talent |
| KDropCenter | 33 | 22 | DRIFT | 11 | 0 | 0 | self-contained | drop-loot |
| KFellowPetBox | 16 | 0 | NEW | 10 | 6 | 0 | self-contained | pet-domesticate |
| KMiniAvatar | 14 | 0 | NEW | 10 | 4 | 0 | self-contained | exterior-cosmetic |
| KRewardsShop | 12 | 0 | NEW | 10 | 2 | 0 | self-contained | cash-shop-currency |
| KRegressionPlayerData | 12 | 0 | NEW | 10 | 2 | 0 | self-contained | regression |
| KRegressionManager | 12 | 0 | NEW | 10 | 2 | 0 | self-contained | regression |
| KNewExtPointManager | 12 | 0 | NEW | 10 | 2 | 0 | self-contained | exterior-cosmetic |
| KActivityMgrServer | 12 | 0 | NEW | 10 | 2 | 0 | self-contained | activity |
| KCampInfo | 27 | 17 | DRIFT | 10 | 1 | 0 | self-contained | activity |
| KDomesticateCenter | 10 | 0 | NEW | 8 | 2 | 0 | self-contained | pet-domesticate |
| KSkill | 71 | 63 | DRIFT | 8 | 0 | 0 | hub | skill-script |
| KHairBox | 14 | 0 | NEW | 7 | 7 | 0 | self-contained | exterior-cosmetic |
| KManualDropList | 13 | 0 | NEW | 7 | 6 | 0 | self-contained | drop-loot |
| KScriptCenter | 34 | 27 | DRIFT | 7 | 0 | 0 | self-contained | skill-script |
| KMiniAvatarSettings | 8 | 0 | NEW | 6 | 2 | 0 | self-contained | exterior-cosmetic |
| KShopCenter | 19 | 14 | DRIFT | 6 | 1 | 0 | self-contained | cash-shop-currency |
| KShop | 20 | 14 | DRIFT | 6 | 0 | 0 | self-contained | cash-shop-currency |
| KRewardsBox | 9 | 0 | NEW | 5 | 4 | 0 | self-contained | cash-shop-currency |
| KFellowPetMgr | 8 | 0 | NEW | 5 | 3 | 0 | self-contained | pet-domesticate |
| KAntiFarmerServer | 8 | 0 | NEW | 5 | 3 | 0 | self-contained | anti-farmer |
| KRewardsShopSettings | 7 | 0 | NEW | 5 | 2 | 0 | self-contained | cash-shop-currency |
| KSkillList | 25 | 20 | DRIFT | 5 | 0 | 0 | self-contained | skill-script |
| KSO3World | 26 | 21 | DRIFT | 5 | 0 | 0 | self-contained | world-scene |
| KQuestInfoList | 13 | 8 | DRIFT | 5 | 0 | 0 | self-contained | quest |
| KNpcTemplateList | 14 | 9 | DRIFT | 5 | 0 | 0 | self-contained | world-scene |
| KDirectMentorCache | 18 | 0 | NEW | 4 | 14 | 0 | self-contained | mentor |
| KCurrency | 7 | 0 | NEW | 4 | 3 | 0 | self-contained | cash-shop-currency |
| KNpcAdronTab | 6 | 0 | NEW | 4 | 2 | 0 | self-contained | world-scene |
| KGJumpList | 6 | 0 | NEW | 4 | 2 | 0 | self-contained | movement-sync |
| KAchievement | 17 | 12 | DRIFT | 4 | 1 | 0 | self-contained | achievement |
| KTradingBox | 24 | 20 | DRIFT | 4 | 0 | 0 | self-contained | cash-shop-currency |
| KQuestList | 62 | 58 | DRIFT | 4 | 0 | 0 | self-contained | quest |
| KGMList | 6 | 3 | DRIFT | 4 | 0 | 0 | self-contained | gm |
| KDoodad | 61 | 57 | DRIFT | 4 | 0 | 0 | hub | world-scene |
| KBuffList | 40 | 36 | DRIFT | 4 | 0 | 0 | self-contained | skill-script |
| KCurrencyList | 8 | 0 | NEW | 3 | 5 | 0 | self-contained | cash-shop-currency |
| KCampActiveStat | 7 | 0 | NEW | 3 | 4 | 0 | self-contained | activity |
| KRankListServer | 5 | 0 | NEW | 3 | 2 | 0 | self-contained | rank |
| KModelInfo | 5 | 0 | NEW | 3 | 2 | 0 | self-contained | exterior-cosmetic |
| KAntiFarmerSettings | 5 | 0 | NEW | 3 | 2 | 0 | self-contained | anti-farmer |
| KWorldSettings | 7 | 2 | DRIFT | 3 | 0 | 2 | self-contained | world-scene |
| KSkillRecipeList | 20 | 17 | DRIFT | 3 | 0 | 0 | self-contained | skill-script |
| KPlayerRelatedPetAttrImportParam | 3 | 0 | NEW | 3 | 0 | 0 | self-contained | pet-domesticate |
| KPlayerRelatedPetAttrExportParam | 3 | 0 | NEW | 3 | 0 | 0 | self-contained | pet-domesticate |
| KPKController | 29 | 34 | DRIFT | 3 | 0 | 0 | self-contained | pk |
| KLootList | 34 | 32 | DRIFT | 3 | 0 | 0 | self-contained | drop-loot |
| KTongConstList | 4 | 0 | NEW | 2 | 2 | 0 | self-contained | guild-tong |
| KReputeLootBuffList | 4 | 0 | NEW | 2 | 2 | 0 | self-contained | drop-loot |
| KSkillRectangleTravFunc | 2 | 0 | NEW | 2 | 0 | 0 | self-contained | skill-script |
| KSimpThreatList | 32 | 32 | DRIFT | 2 | 0 | 0 | self-contained | world-scene |
| KRoadManager | 30 | 28 | DRIFT | 2 | 0 | 0 | self-contained | world-scene |
| KReputation | 13 | 13 | DRIFT | 2 | 0 | 0 | self-contained | reputation |
| KMoneyDropList | 7 | 5 | DRIFT | 2 | 0 | 0 | self-contained | drop-loot |
| KMentorCache | 16 | 14 | DRIFT | 2 | 0 | 0 | self-contained | mentor |
| KLevelDropList | 6 | 7 | DRIFT | 2 | 0 | 0 | self-contained | drop-loot |
| KGMoveParam | 2 | 0 | NEW | 2 | 0 | 0 | self-contained | movement-sync |
| KDropList | 8 | 6 | DRIFT | 2 | 0 | 0 | self-contained | drop-loot |
| KCharacterStealthChangedFunc | 2 | 0 | NEW | 2 | 0 | 0 | self-contained | player-hub |
| KCharacterDisarmChangedFunc | 2 | 0 | NEW | 2 | 0 | 0 | self-contained | player-hub |
| KCharacterCounterStealthChangedFunc | 2 | 0 | NEW | 2 | 0 | 0 | self-contained | player-hub |
| KBroadcastCastSkillFunc | 2 | 0 | NEW | 2 | 0 | 0 | self-contained | skill-script |
| KBandwidthOptimizeTacticsManager | 2 | 0 | NEW | 2 | 0 | 0 | self-contained | anti-farmer |
| KNpcReviveManager | 22 | 20 | DRIFT | 1 | 1 | 0 | self-contained | world-scene |
| KG_SocketServerAcceptor | 7 | 5 | DRIFT | 1 | 1 | 0 | network-infra | network-infra |
| KDoodadReviveManager | 16 | 14 | DRIFT | 1 | 1 | 0 | self-contained | world-scene |
| SYNC_SPRINT_FLAGS | 1 | 0 | NEW | 1 | 0 | 0 | self-contained | movement-sync |
| SYNC_MOVE_STATE | 1 | 0 | NEW | 1 | 0 | 0 | self-contained | movement-sync |
| SYNC_MOVE_SPEED | 1 | 0 | NEW | 1 | 0 | 0 | self-contained | movement-sync |
| SYNC_MOVE_CTRL | 1 | 0 | NEW | 1 | 0 | 0 | self-contained | movement-sync |
| KRecorderSceneLoaderReplay | 6 | 5 | DRIFT | 1 | 0 | 0 | self-contained | recorder-replay |
| KRecorderSceneLoaderRecord | 10 | 9 | DRIFT | 1 | 0 | 0 | self-contained | recorder-replay |
| KRecorderSceneLoaderNormal | 10 | 9 | DRIFT | 1 | 0 | 0 | self-contained | recorder-replay |
| KRecipeList | 13 | 12 | DRIFT | 1 | 0 | 0 | self-contained | skill-script |
| KNpcTemplate | 1 | 0 | NEW | 1 | 0 | 0 | self-contained | world-scene |
| KMoveProcessTraverseFunc | 1 | 0 | NEW | 1 | 0 | 0 | self-contained | movement-sync |
| KGetAllCopyIndexByMapIDFunc | 1 | 0 | NEW | 1 | 0 | 0 | self-contained | skill-script |
| KG_AsyncSocketStream | 12 | 11 | DRIFT | 1 | 0 | 0 | network-infra | network-infra |
| KGWConstList | 4 | 3 | DRIFT | 1 | 0 | 0 | network-infra | network-infra |
| KActivityEndNotifySceneFunc | 1 | 0 | NEW | 1 | 0 | 0 | self-contained | activity |
| KAchievementInfoList | 6 | 5 | DRIFT | 1 | 0 | 0 | self-contained | achievement |
| KAIPatrolGroupManager | 14 | 14 | DRIFT | 1 | 0 | 0 | self-contained | world-scene |
| KAIManager | 10 | 9 | DRIFT | 1 | 0 | 0 | self-contained | world-scene |
| GetSkillRecipeTraverseFunc | 1 | 0 | NEW | 1 | 0 | 0 | self-contained | skill-script |
| KCenterRemote | 11 | 4 | DRIFT | 0 | 7 | 0 | network-infra | network-infra |
| KGWServerConstList | 3 | 0 | NEW | 0 | 3 | 0 | network-infra | network-infra |
| KPendentOldDataInfoList | 2 | 0 | NEW | 0 | 2 | 0 | self-contained | exterior-cosmetic |

## Danh sách hàm chi tiết

`PORT_MAP.csv` (cùng thư mục) — 1716 dòng, cột:
`cluster, kind, class, method, signature, type(NEW/DRIFT), confidence(hard|uncertain)`.
`signature` = prototype demangle từ DWARF (đủ kiểu tham số), sắp theo thứ tự port ở trên.
Khi port từng hàm khó (hub), dùng pyghidra `SO3GameServerD-2d2a24`
(`decompile_function` + struct offset) để lấy hành vi + layout chính xác.

## Cảnh báo độ tin

- **NEW (54 cls / 438 method)** = số cứng: class vắng hẳn khỏi symbol table ours.
- **DRIFT truly (1256)** = ước lượng cao: đã trừ present-exact, nhưng vài hàm inline-only
  (không sinh symbol) có thể lọt vào "thiếu" dù logic đã có → xác minh khi port.
- **NAME_ELSEWHERE (135)** = thấp tin: tên hàm có ở nơi khác. Trong NEW là trùng tên chung
  (`Init/Load…`); trong DRIFT (22) có thể là bản đã relocate/rename.
- Số method dựa trên hàm **định nghĩa trong `.text`**; inline/template/thuần-header không tính.
