# SO3GameServer — Build / Init map

READ-ONLY research. Nguồn: `src/` (2010 leak) + `include/Include/SO3World/`. Mọi file:line dưới đây bám vào cây source thật, không phải bản `Test/Robot/` (bản test là stub, bỏ qua).

Mục tiêu: nhìn vào hiểu ngay cái gì khởi tạo cái gì, subsystem nào thuộc ai.

---

## 1. Entry → Init chain

Process khởi động ở `main()`, dựng hạ tầng (log/memory), rồi bàn giao toàn bộ cho object global `KSO3GameServer`. `KSO3GameServer::Init` mới là nơi tạo `g_pSO3World` (con trỏ global tới `KSO3World` — trái tim game logic) và các client mạng (Relay/Player/Log/Eyes).

```
main()                                             src/SO3GameServer/Main.cpp:94
├─ fork()+setsid()  (daemon hoá, __GNUC__)                              :110
├─ tzset() / getcwd()                                                   :122
├─ KGLogInit()                     hạ tầng log                          :133
├─ KG_MemoryInit() / KMemory::Initialize("game_server.memory")          :137
├─ g_SO3GameServer.Init()   ← object global (Main.cpp:92)               :148
│    └─ KSO3GameServer::Init()             src/SO3GameServer/KSO3GameServer.cpp:16
│         ├─ LoadConfig()                    đọc locale + run-mode        :30
│         ├─ setlocale(LC_ALL, m_szLocale)                               :33
│         ├─ CreateRecorderFactoryInterface(m_nRunMode)  record/replay   :38
│         ├─ piRecorderFactory->CreateToolsInterface()                   :41
│         ├─ g_pSO3World = new KSO3World                                 :44
│         ├─ g_pSO3World->Init(piRecorderFactory)  ★ GAME WORLD          :47
│         ├─ g_RelayClient.Init(piRecorderFactory) kết nối relay/center  :51
│         ├─ g_PlayerServer.Init(piRecorderFactory) listener client      :55
│         ├─ m_Eyes.Init()                    giám sát/ping              :59
│         ├─ g_LogClient.Init()                                          :63
│         └─ g_pSO3World->m_bRunFlag = true                              :68
├─ g_SO3GameServer.Run()   ← vòng lặp game (GAME_FPS)                    :162
│    └─ KSO3GameServer::Run()              src/SO3GameServer/KSO3GameServer.cpp:124
│         while (m_bRunFlag):
│           ├─ g_RelayClient.ProcessPackage()   nhận gói từ relay        :142
│           ├─ g_PlayerServer.Breathe()         nhận gói từ client       :143
│           ├─ (throttle theo GAME_FPS)                                  :146
│           ├─ m_Eyes.Activate()                                         :157
│           ├─ g_RelayClient.Activate() / g_LogClient.Activate()         :159
│           ├─ g_pSO3World->Activate()   ★ 1 frame game                  :161
│           └─ g_PlayerServer.FlushSend()  (mỗi GAME_FPS/4)              :167
└─ g_SO3GameServer.UnInit()                                             :164
```

Thứ tự huỷ trong `KSO3GameServer::Init` khi lỗi (Exit0, `KSO3GameServer.cpp:71-102`) và trong `UnInit` (`:106`) đều đảo ngược: Eyes → PlayerServer → RelayClient → World.

### KSO3World::Init — thứ tự init subsystem lõi

`src/SO3World/Src/KSO3World.cpp:71` (`BOOL KSO3World::Init(IRecorderFactory*)`). Chỉ liệt kê nhánh `_SERVER`:

```
KSO3World::Init(piFactory)                         src/SO3World/Src/KSO3World.cpp:71
├─ reset clock fields (m_eWorldMode=wmNormal, m_nGameLoop=0…)            :100
├─ m_piRecorderTools = piFactory->CreateToolsInterface()                :124
├─ m_nCurrentTime / m_dwStartTime = tools clock                         :126
├─ m_piSceneLoader = piFactory->CreateSceneLoader()   [_SERVER]         :131
├─ g_OpenIniFile(GS_SETTINGS_FILENAME)  đọc FastBootMode + [TalkRec]    :134
├─ g_RandomSeed / srand(m_dwStartTime)                                  :171
├─ m_TeamServer.Init()            [_SERVER]  (client: m_TeamClient)     :176
├─ m_FellowshipMgr.Init()         quan hệ bạn bè                        :185
├─ m_Settings.Init()          ★ KWorldSettings — toàn bộ config/table   :189  → xem §2
├─ m_ScriptCenter.Init()          engine Lua/script                     :193
├─ InitAttributeFunctions()       bảng hàm thuộc tính                   :197
├─ m_ItemManager.Init()                                                 :200
├─ m_ShopCenter.Init()                                                  :204
├─ m_ProfessionManager.Init()     môn phái/nghề                         :208
├─ m_RoadManager.Init()           tầm đường (pathfinding data)          :212
├─ m_AIManager.Init()                                                   :216
├─ m_BuffManager.Init()           (KGLOG_CHECK — non-fatal)             :220
├─ m_SkillManager.Init()                                                :224
├─ m_DropCenter.Init()            [_SERVER] rơi đồ                       :229
├─ m_CampInfo.Init()              trận doanh                            :234
├─ m_PQManager.Init()             chiến trường / public quest           :238
├─ m_TransmissionList.Init()      [_SERVER] điểm dịch chuyển            :253
├─ m_StatDataServer.Init()        [_SERVER] thống kê                    :257
├─ m_NpcSet.SetPrefix(NPC_ID_PREFIX)   object index                     :262
├─ m_CenterRemote.Setup()   [_SERVER]  (client: m_ScriptClient.Setup)   :289
├─ m_ScriptServer.Setup()   [_SERVER]                                   :290
└─ lzo_init()                     nén dữ liệu mạng                       :299
```

Quan sát: `m_Settings` (tables) init trước `m_ScriptCenter`, và cả hai trước Item/Skill/AI managers. Set object (`m_NpcSet`…) chỉ gán prefix, không "Init". `CenterRemote/ScriptServer` chỉ `Setup()` (không phải Init) — đăng ký handler mạng chứ chưa connect.

---

## 2. KWorldSettings — bảng "config / table managers"

Header: `include/Include/SO3World/KWorldSettings.h`. Impl Init: `src/SO3World/Src/KWorldSettings.cpp:8`. Đây là túi chứa 25 manager kiểu-bảng (đọc file tab/txt trong `settings/`). Tất cả embed by-value (không con trỏ).

| # | Member | Kiểu | Init ở dòng (KWorldSettings.cpp) | Vai trò |
|---|--------|------|----------------------------------|---------|
| 1 | m_ConstList | KGWConstList | 32 | hằng số game (GWConst) |
| 2 | m_NpcTemplateList | KNpcTemplateList | 36 | template NPC |
| 3 | m_DoodadTemplateList | KDoodadTemplateList | 40 | template doodad |
| 4 | m_MapListFile | KMapListFile | 44 | danh sách map (init thứ 4 dù khai báo #2) |
| 5 | m_QuestInfoList | KQuestInfoList | 47 | nhiệm vụ |
| 6 | m_NpcClassList | KNpcClassList | 51 | class NPC |
| 7 | m_DoodadClassList | KDoodadClassList | 55 | class doodad |
| 8 | m_PatrolPathList | KPatrolPathList | 59 | đường tuần tra |
| 9 | m_LevelUpList | KLevelUpList | 63 | bảng thăng cấp |
| 10 | m_CoolDownList | KCoolDownList | 67 | cooldown |
| 11 | m_WeaponTypeList | KWeaponTypeList | 71 | loại vũ khí |
| 12 | m_RelationCampList | KRelationList | 75 | quan hệ trận doanh (RELATION_CAMP_FILE) |
| 13 | m_RelationForceList | KRelationList | 78 | quan hệ môn phái (RELATION_FORCE_FILE) |
| 14 | m_ForceDefaultList | KFightFlagList | 81 | cờ chiến đấu mặc định |
| 15 | m_ReputeLimit | KReputationLimit | 84 | giới hạn danh vọng |
| 16 | m_CharacterActionList | KCharacterActionList | 88 | hệ động tác nhân vật |
| 17 | m_SmartDialogList | KSmartDialogList | 92 | thoại NPC |
| 18 | m_GMList | KGMList | 97 `Load()` [_SERVER] | danh sách GM |
| 19 | m_OrderManager | KNpcOrderManager | 101 | lệnh NPC |
| 20 | m_NpcTeamList | KNpcTeamList | 105 | tổ đội NPC |
| 21 | m_AchievementInfoList | KAchievementInfoList | 109 | thành tựu (成就) |
| 22 | m_DesignationList | KDesignationList | 113 | danh hiệu/xưng hào (称号) |
| 23 | m_Exterior | KExterior | 117 | ngoại quan/wardrobe (外观) |
| 24 | m_HairShop | KHairShop | 121 | phát hình/kiểu tóc (发型) |
| 25 | m_GameCardInfoList | KGameCardInfoList | 124 (tolerant, không PROCESS_ERROR) | thẻ trò chơi |

Thứ tự Init (theo cpp, KHÔNG hẳn theo thứ tự khai báo trong .h):
ConstList → NpcTemplate → DoodadTemplate → **MapListFile** → QuestInfo → NpcClass → DoodadClass → PatrolPath → LevelUp → CoolDown → WeaponType → RelationCamp → RelationForce → ForceDefault → ReputeLimit → CharacterAction → SmartDialog → GMList → OrderManager → NpcTeam → Achievement → Designation → **Exterior → HairShop** → GameCard.

`m_Exterior` và `m_HairShop` (2.5.2 port) là 2 manager cuối trước GameCard. UnInit đảo ngược đầy đủ theo cờ `bXxxInitFlag` (`KWorldSettings.cpp:130-247`).

---

## 3. KPlayer — "per-player state" (composition)

Header: `src/SO3World/Src/KPlayer.h`. `class KPlayer : public KCharacter` (`:223`). KPlayer nhồi các sub-object trạng thái riêng của từng người chơi (embed by-value). Các sub-object nghiệp vụ chính:

| Member | Kiểu | Dòng | Vai trò |
|--------|------|------|---------|
| m_CustomData | KCustomData<128> | 262 | dữ liệu tuỳ biến/persist |
| m_ScriptTimerList | KScriptTimerList<KPlayer,16> | 264 | timer script gắn player |
| m_QuestList | KQuestList | 287 | nhiệm vụ đang làm |
| m_PQList | KGPQList | 288 | public quest / chiến trường |
| m_SkillList | KSkillList | 289 | kỹ năng đã học |
| m_ProfessionList | KProfessionList | 291 | môn phái/nghề |
| m_RecipeList | KRecipeList | 292 | công thức chế tạo |
| m_TimerList | KCDTimerList | 295 | cooldown timers |
| m_ItemList | KItemList | 297 | túi + trang bị + ngân hàng |
| m_ReputeList | KReputation | 300 | danh vọng |
| m_PK | KPKController | 302 | trạng thái PK |
| m_SelectList | KWindowSelectList | 304 | cửa sổ chọn |
| m_UserPreferences | KUserPreferences | 306 | thiết lập người dùng |
| m_BookList | KBookList | 308 | sách/đọc |
| m_Designation | KDesignation | 310 | danh hiệu (称号) — per-player, ánh xạ tới m_DesignationList |
| m_ExteriorBox | KExteriorBox | 312 | tủ ngoại quan (外观) — per-player, tới m_Exterior |
| m_HairBox | KHairBox | 313 | kho kiểu tóc (发型) — per-player, tới m_HairShop |
| m_SkillRecipeList | KSkillRecipeList | 348 | công thức kỹ năng |
| m_Achievement | KAchievement | 359 | thành tựu đã đạt (成就) — tới m_AchievementInfoList |
| m_AntiFarmer | KAntiFarmer | 799 | chống cày thuê/bot |

Sub-object POD/hệ thống khác (không phải manager nghiệp vụ, nêu để đủ): `m_LastEntry`/`m_SavePosition`/`m_DelayedSwitchMapParam` (KROLE_POSITION), `m_MoveCtrl`, `m_ViewPoint`, `m_Player/Npc/DoodadSyncTable`+`SyncQueue` (KVIEW_SYNC_* — bảng đồng bộ tầm nhìn tới client, `:461-468`), `m_PositionRecord[]`, `m_ReviveCtrl`, `m_eCamp`, `m_QuestRand`/`m_VenationRand` (KProbability), `m_SceneVisitRecord`/`m_HeroMapCopyOpenFlag` (KCustomData).

Ghi chú cặp settings↔player: mỗi thứ có "definition table" toàn cục trong `KWorldSettings` và "instance state" per-player trong `KPlayer` — Designation↔DesignationList, ExteriorBox↔Exterior, HairBox↔HairShop, Achievement↔AchievementInfoList, SkillList↔SkillManager, RecipeList/ProfessionList↔ProfessionManager.

Kế thừa từ `KCharacter` (dùng cho cả Player lẫn Npc): `m_BuffList` (KBuffList, `KCharacter.h:1335`), `m_ThreatList`/`m_SimpThreatList` (`:930/:934`), `m_SelectTarget`/`m_SkillTarget` (KTarget, `:1074/:1075`), `m_AIVM` (KAIVM, `:1377`), `m_SkillDebug` (`:1370`), `m_SkillEventVector` (`:1340`).

---

## 4. Taxonomy các class lõi

Kế thừa (từ grep decl trong `src/SO3World/Src`):

```
KBaseObject
├─ KItem                              (KItem.h:47  : public KBaseObject)
└─ KSceneObject                       (KSceneObject : public KBaseObject)
   ├─ KCharacter                      (KCharacter.h:194 : public KSceneObject)
   │  ├─ KNpc                         (KNpc.h:34  : public KCharacter)
   │  └─ KPlayer                      (KPlayer.h:223 : public KCharacter)
   └─ KDoodad                         (KDoodad.h:35 : public KSceneObject)
```

| Class | Vai trò | Loại |
|-------|---------|------|
| **KSO3World** (`g_pSO3World`) | Root game world. Sở hữu mọi manager + mọi object index. Chủ vòng `Activate()`. | **Core / root** |
| **KWorldSettings** (`m_Settings`) | Túi 25 bảng config đọc từ `settings/`. Read-only sau Init. | **Sub-core: config/table** |
| **KScriptCenter** (`m_ScriptCenter`) | Engine Lua, `Setup`+`Activate` mỗi frame. | **Sub-core: script** |
| KItemManager / KShopCenter / KProfessionManager / KRoadManager / KSkillManager / KBuffManager / KAIManager / KGPQManager / KDropCenter / KCampInfo | Manager toàn cục, mỗi cái Init từ KSO3World::Init. | **Sub-core: manager (global singleton-per-world)** |
| KTeamServer / KFellowshipMgr / KCenterRemote / KScriptServer / KTransmissionList / KStatDataServer | Dịch vụ toàn cục (đội/bạn/center/thống kê). | **Sub-core: service** |
| KObjectIndex<KScene/KPlayer/KNpc/KDoodad/KItem> (`m_SceneSet`…) | Registry ID→object toàn cục, chỉ `SetPrefix`. | **Sub-core: registry** |
| **KScene** | 1 bản đồ (map/copy). Chứa lưới `m_RegionList[W][H]` (`KScene.h:258`); KRegion → cell → object. | **Per-scene container** |
| **KCharacter** | Base cho thực thể sống (Buff/Threat/Target/AIVM). | Base thực thể |
| **KNpc** | NPC (dùng AI + template). | Per-scene object |
| **KPlayer** | Người chơi; nhồi ~20 sub-object state (§3). | **Per-player state** |
| **KDoodad** | Vật thể tương tác tĩnh. | Per-scene object |
| **KItem** | Vật phẩm (không ở scene, thuộc KItemList). | Object |
| KScriptFuncList / KBaseFuncList | Bảng hàm bind cho Lua (không phải object runtime). | Binding table |

Quan hệ chứa: `KSO3World` chứa managers + `KObjectIndex` (registry phẳng theo ID) **và** các `KScene`; mỗi `KScene` chứa lưới `KRegion` → object nằm trong region. Object vừa nằm trong registry toàn cục (tra theo ID) vừa nằm trong 1 region (tra theo vị trí).

---

## 5. Quy tắc thứ tự Init (rút ra từ chain)

1. **Hạ tầng trước tất cả**: log → memory pool → recorder tools/clock. (`Main.cpp`, đầu `KSO3World::Init`).
2. **Settings/tables trước mọi thứ dùng chúng**: `m_Settings.Init()` (KWorldSettings) chạy trước ScriptCenter, Item/Skill/AI/Drop managers — vì template NPC/Doodad, const list, cooldown, weapon type là dữ liệu các manager kia đọc.
3. **ScriptCenter trước InitAttributeFunctions trước ItemManager**: attribute-function table cần script engine sẵn sàng; item/skill dùng attribute.
4. **Manager toàn cục trước object**: mọi manager (Item/Shop/Profession/Road/AI/Buff/Skill/Drop/Camp/PQ) Init xong rồi mới tới object index `SetPrefix`; NPC/Doodad/Player được tạo runtime (`NewNpc/NewPlayer`) sau khi world đã Init.
5. **Setup mạng (CenterRemote/ScriptServer) cuối cùng trong world Init**, còn connect thật (RelayClient/PlayerServer) do `KSO3GameServer::Init` gọi SAU `g_pSO3World->Init` — tức toàn bộ data-load xong mới mở mạng.
6. **Scene trước Npc/Doodad/Player**: object phải `AddNpc/AddPlayer` vào một `KScene` đã tồn tại (chữ ký `AddNpc(KNpc*, KScene*, x,y,z)` — `KSO3World.h:108`); scene load qua `m_piSceneLoader`/`ProcessLoadedScene` (`KSO3World.cpp:790`).
7. **UnInit luôn đảo ngược Init**, canh theo cờ `bXxxInitFlag` ở block `Exit0` (thấy rõ ở cả `KSO3GameServer::Init`, `KSO3World::Init`, `KWorldSettings::Init`).

Runtime sau Init: `KSO3GameServer::Run` gọi `g_pSO3World->Activate()` mỗi frame (`KSO3World.cpp:539`) — tăng `m_nGameLoop`, traverse toàn bộ Player rồi Scene, rồi `ShopCenter.Activate` → `ScriptCenter.Activate` → `StatDataServer.Activate`.
