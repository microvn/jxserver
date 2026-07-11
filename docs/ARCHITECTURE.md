# JX3 server ARCHITECTURE — bản đồ build & liên kết (2010 source / v2.5 binary)

Tài liệu tổng quan: **nhìn vào hiểu ngay một GameServer/GameCenter/Gateway được build & nối với
nhau thế nào**, và **mọi điểm binding một subsystem phải móc vào** (để port không bỏ sót như vụ
KExterior thiếu register Lua). Nguồn: source 2010 `linux-build/src`+`include`, DWARF
`jx3_dwarf/`, tài liệu thiết kế gốc dev `source/JX3-AIO/Document/`. Chi tiết + file:line ở
`docs/arch_research/{devdocs,build_init,bindings}.md`. Case study port: `docs/{exterior,hair}_port/`.

---

## 1. Bốn tiến trình & luồng kết nối

```
                 (account+mật khẩu)
   Client  ───────────────────────────►  Bishop / Gateway ──► Paysys (点券/元宝)
     │                                         │ auth OK, trỏ client sang GS
     │   gameplay (C2G/G2C protocol)           ▼
     └──────────────────────────────►  GameServer (KSO3World)  ◄──── mỗi GS giữ 1 mảnh bản đồ
                                              │  ▲                    (nhiều GS = nhiều scene)
                            (KS2R/KR2S)       ▼  │ (blob role-data qua KRelayClient)
                                        Relay / GameCenter  ──►  Database (MySQL)
                                        总控+中转: data global player/map-instance/
                                        friend/bang/task-var; điều phối login + cross-server
                                              │
                                        RelayServer / LogClient
```

- **Gateway (Bishop)**: xác thực account+mật khẩu. Sau khi OK, client **reconnect thẳng vào GS**.
  Gateway không relay từng gói gameplay. (Trong leak: gateway binary-only, xem [[jx3-auth-paysys]].)
- **GameServer** = process chính chứa `KSO3World`, chạy toàn bộ logic gameplay của các scene nó giữ.
  Client ↔ GS đi thẳng (protocol `GS_CLIENT`/`CLIENT_GS`).
- **GameCenter / Relay** = 总控 (tổng-control) + 中转 (trung chuyển): giữ state global (player online,
  map-instance, bạn bè, bang, biến nhiệm vụ), điều phối **login** và **cross-server**, và là **kênh
  DB proxy** — blob role-data (`Save`/`Load`) gói vào protocol `KS2R/KR2S` gửi về center → MySQL.
  GS **không** nói chuyện trực tiếp với DB; mọi persistence đi qua `KRelayClient`.
- Handshake chain: `KGatewayClient::ConnectGateway/DoHandshakeRequest` (client→gateway),
  gateway trỏ client→GS, `KRelayClient::Connect/DoHandshakeRequest` (GS→center). Xem
  [[jx3-lan-multiplayer-blocker]] cho bẫy IP loopback khi chơi LAN.

---

## 2. SO3GameServer — init chain (cái gì dựng cái gì)

```
main()  (Main.cpp:94)
  └─ log → memory pool → clock
  └─ g_SO3GameServer.Init()                         (KSO3GameServer.cpp:16)
       ├─ g_pSO3World = new KSO3World
       ├─ g_pSO3World->Init(piFactory)              (KSO3World.cpp:71) ── TRÁI TIM data-load
       │     ├─ recorder/clock → đọc gs_settings.ini → TeamServer → FellowshipMgr
       │     ├─ m_Settings (KWorldSettings::Init)    ◄── 25 config-table manager (mục 3)
       │     ├─ KScriptCenter (Lua env) + InitAttributeFunctions
       │     ├─ Item / Shop / Profession / Road / AI / Buff / Skill / Drop / Camp / PQ managers
       │     ├─ TransmissionList → StatDataServer → object-index SetPrefix
       │     └─ CenterRemote/ScriptServer Setup → lzo_init
       └─ RỒI mới mở mạng: g_RelayClient → g_PlayerServer → m_Eyes → g_LogClient
  └─ Run() loop → g_pSO3World->Activate() mỗi frame (traverse Player rồi Scene)
```

**Quy tắc init (bất biến):** tables/settings **trước** managers → managers **trước** object →
scene **trước** npc/player → **data-load xong hết mới mở mạng**. UnInit luôn đảo ngược theo cờ
`bXxxInitFlag`. Milestone boot của cả pipeline = log **`Load game settings ... [OK]`** (mọi port
phải giữ mốc này = no-regression).

### Taxonomy (Core / Sub-core / per-player / per-scene)
- **Core (root)**: `KSO3World` (`g_pSO3World`) — sở hữu mọi manager + registry `KObjectIndex`
  (ID→object) + tất cả `KScene`.
- **Sub-core**: `KWorldSettings` (config-table), `KScriptCenter` (Lua env), các `*Manager` global
  (AI/Buff/Skill/Drop/Shop/Road...). Init trong chuỗi trên.
- **Cây kế thừa object**:
  `KBaseObject → { KItem, KSceneObject → { KCharacter → { KNpc, KPlayer }, KDoodad } }`.
  `KCharacter` = base có state-machine + combat state (m_BuffList, m_ThreatList, m_AIVM, target).
- **per-player**: `KPlayer` (mục 3). **per-scene**: `KNpc`/`KDoodad` sống trong `KScene`
  (lưới `KRegion`/`KCell`).

---

## 3. Hai "trục" gắn subsystem: KWorldSettings & KPlayer

Đây là 2 nơi phần lớn subsystem cắm vào. Khi port một tính năng, gần như luôn đụng một/cả hai.

### 3a. KWorldSettings — 25 config-table manager (dữ liệu tĩnh, load từ .tab)
Embed by-value, init tuần tự trong `KWorldSettings::Init` (`KWorldSettings.cpp:8`), từ `m_ConstList`
(:32) đến `m_GameCardInfoList` (:124). Mỗi manager = "đọc .tab → map tra cứu". Ví dụ đã port:
`m_Exterior` (:117, KExterior), `m_HairShop` (:121, KHairShop). **Thêm subsystem config mới = thêm
member vào đây + gọi `m_Xxx.Init()`.**

### 3b. KPlayer — ~20 sub-object nghiệp vụ (state per-player, serialize vào DB)
`class KPlayer : public KCharacter`. Sub-object nổi bật: `m_ItemList`, `m_SkillList`, `m_QuestList`,
`m_Designation`, `m_ExteriorBox`, `m_HairBox`, `m_Achievement`, `m_PK`, `m_AntiFarmer`... + state kế
thừa từ `KCharacter` (`m_BuffList`, `m_ThreatList`, `m_AIVM`, `m_SelectTarget`). **Thêm state
per-player = embed member vào KPlayer.h + Init(this) + (nếu lưu DB) một role-block.**

---

## 4. Biên C++ ↔ Lua (binding boundary)

Lua (nội dung, sửa không cần biên dịch) GỌI C++ wrapper `LuaXxx` → unpack Lua stack → gọi method
C++ thật. C++ giữ data/network/DB/hot-path/security. **Binding thiếu = lỗi runtime `GetXxx nil`**
(script có cửa, C++ thiếu cửa). Có **3 kiểu** — chọn đúng kiểu là điểm KExterior từng làm sai:

| kiểu | cơ chế | dùng khi | đăng ký ở đâu |
|---|---|---|---|
| **(a) `Luna<T>` object** | `DECLARE_LUA_CLASS(T)` sinh `LuaGetObj`+`szClassName`+`LuaInterface[]`; `Register` tạo metatable `__index/__newindex`. Script: `GetT():Method()` | object cầm-nguyên: per-instance (KPlayer, KItem) HOẶC **singleton** (KHairShop, KMentorCache) | `Luna<T>::Register` @ `KScriptCenter.cpp:729-780` (`#ifdef _SERVER` cho server-only) + free-func `LuaGetT` + `{"GetT",...}` @ `KBaseFuncList.cpp` |
| **(b) free-func `GetXxx`** | hàm toàn cục trong bảng base-func | hàm global HOẶC cổng phát singleton ra Lua | bảng `KBaseFuncList.cpp:4833-5117` (179 entry) |
| **(c) `REGISTER_LUA_FUNC(KPlayer/KCharacter, X)`** | method gắn vào player | thao tác trên player hiện tại | `KLuaPlayer.cpp` (407 register; gộp cả method KCharacter/KSceneObject) |

**Con số (server build):** 37 class `Luna<T>` register (31 ở server build), 179 entry base-func,
407 `REGISTER_LUA_FUNC` trên KPlayer, 23 role-block. **Cách nhận kiểu từ binary:** nếu v246
`LuaGetObj` làm `lua_newuserdata + setmetatable "KClass"` ⇒ kiểu (a) `Luna<KClass>` — KHÔNG nhét
thành method KPlayer. (KExterior nhét nhầm (a)→(c) ⇒ 6 query binding nil, xem TODO §1.)

---

## 5. Persistence — role-block + KRelayClient

- State per-player lưu thành **role-block**: enum `ROLE_DATA_BLOCK_TYPE`
  (`include/Include/KRoleDBDataDef.h:14-44`, 23 block hợp lệ `rbtSkillList..rbtHairBoxData`).
- Save: `SAVE_ROLE_BLOCK(m_Xxx.Save, rbtXxx, 0)` trong `KPlayer::Save` (`KPlayer.cpp:2364`).
  Load: `case rbtXxx:` trong switch (`KPlayer.cpp:2040`) gọi `m_Xxx.Load`.
- **Append `rbtXxx` mới TRƯỚC `rbtTotal`, không chèn giữa** (giá trị = tag DB; dịch sẽ hỏng blob của
  mọi hệ khác). 2010 `KRoleBlockHeader` không có chunk-key → gộp thành 1 block (v246 chunk riêng).
- Struct DB `#pragma pack(1)` + pin byte-layout từ DWARF + verify roundtrip. Blob đi qua
  **`KRelayClient`** (KS2R/R2S) → center → MySQL. GS không chạm DB trực tiếp.

---

## 6. Network — 6 nhóm protocol

`C2G/G2C` (client↔gateway), `GS_CLIENT/CLIENT_GS` (client↔GS gameplay), `G2R/R2G` (gateway↔relay),
`KS2R/KR2S` (GS↔center, gồm blob DB), `EDITOR_2_GS/GS_2_EDITOR`, + Bishop/Paysys/Log.
- Client-facing packet: ID vào `GS_CLIENT_PROTOCOL`, emit qua `KPlayerServer::DoSyncXxx`
  (`KPlayerServer.cpp:3400`) gọi ở **mọi setter** thay đổi state cần đồng bộ.
- Internal packet: `KS2R/KR2S_PROTOCOL` + `REGISTER_INTERNAL_FUNC(name, handler, SIZE)` — SIZE là
  kích thước struct cố định (sai byte = reject handshake; xem saga #22 [[jx3-linux-build-recovery]]).

---

## 7. Vòng đời player: login, cross-server, sync

- **Connection state (client)**: `gsInvalid → gsWaitForGUID → gsSyncData → gsPlaying`
  (+ `gsSearchMap/gsTransferData` khi đổi map). GameCenter dùng state `rs*` (rsOffline/rsOnline +
  rsLG_*/rsCG_* cho login+cross-server).
- **Login**: Bishop auth → Relay `psLogin` → GS phê duyệt → client reconnect thẳng GS → sync →
  `psOnline`.
- **Cross-server**: 2 ca — trong-GS (nhẹ) và **跨GS** (transfer đầy đủ nhân vật Src-GS→Relay→Dest-GS,
  client re-connect Dest với GUID mới).
- **Sync strategy** (剑三同步策略): đồng bộ **9-Region** quanh player (Region 16m, ô 1m×1m), mục tiêu
  ≤2s. Tách 逻辑数据 (toạ độ/tốc độ/state/%máu — sync liên tục) vs 显示数据 (ít đổi). Di chuyển =
  **client-làm-chủ + server校验修正** với lịch sử vị trí + rollback theo frame ("帧数以服务端为准") →
  chống kéo-về/dịch-chuyển-hack. Object mới: broadcast + **forced-sync 2s** cho player thụ động.

---

## 8. ✅ SUBSYSTEM INTEGRATION CHECKLIST — port không sót binding

Mọi điểm một subsystem MỚI phải móc vào. Bỏ qua điểm nào = tính năng câm lặng (compile/boot vẫn
qua). Đối chiếu với `docs/hair_port/INTEGRATION.md` (ví dụ đã điền đủ).

**A. Config (nếu có .tab tĩnh)**
1. `KXxx::Init()` nạp .tab → thêm member vào `KWorldSettings` + gọi `m_Xxx.Init()`
   (`KWorldSettings.cpp:~117`) + `UnInit()` đối xứng.

**B. State per-player (nếu lưu theo player)**
2. Embed member vào `KPlayer.h` (KHÔNG hardcode offset — compiler tự layout).
3. `m_Xxx.Init(this)` cạnh sibling trong `KPlayer` init + set back-ptr player.

**C. Persistence (nếu lưu DB)**
4. Enum `rbtXxxData` TRƯỚC `rbtTotal` (`KRoleDBDataDef.h`).
5. `SAVE_ROLE_BLOCK(m_Xxx.Save, rbtXxxData, 0)` (`KPlayer.cpp:2364`).
6. `case rbtXxxData:` trong load-switch (`KPlayer.cpp:2040`) gọi `m_Xxx.Load`.
7. Struct DB `#pragma pack(1)` + static_assert size + roundtrip Save↔Load byte-exact.

**D. Lua binding — CHỌN 1 kiểu (mục KExterior từng SÓT)**
8. Xác định kiểu từ binary (`Luna<T>` object vs KPlayer method).
9. Nếu (a) singleton `Luna<T>`: `DECLARE_LUA_CLASS(T)` trong .h (`#ifdef _SERVER`+`#include "Luna.h"`);
   `DEFINE_LUA_CLASS_BEGIN(T) REGISTER_LUA_FUNC(T,M)... END` trong `KLuaT.cpp`;
   `Luna<T>::Register` @ `KScriptCenter.cpp`; free-func `LuaGetT` + `{"GetT",...}` @ `KBaseFuncList.cpp`.
10. Nếu (c) player method: `LuaXxx` trên KPlayer + `REGISTER_LUA_FUNC(KPlayer, Xxx)` @ `KLuaPlayer.cpp`.
11. **Verify binding sống**: boot không nil + (nếu được) `test_xxx.lua`.

**E. Packet sync (nếu client cần thấy thay đổi)**
12. Client-facing: ID vào `GS_CLIENT_PROTOCOL` + `DoSyncXxx` gọi ở mọi setter.
13. Internal: `KS2R/KR2S` + `REGISTER_INTERNAL_FUNC(name, handler, SIZE)` (SIZE chính xác).

**F. Log (tuỳ chọn)**
14. `KLogClient::LogPlayerXxx` (có thể stub).

**G. Apply/hook vào cơ chế sẵn có**
15. Tái dùng cơ chế 2010 thay vì transcribe offset v246 (vd ngoại hình = `KItemList::SetRepresentID`
    + enum `PLAYER_EQUIP_REPRESENT`, KHÔNG ghi thẳng `KPlayer+0x95a8`). Khoá drift bằng
    `typedef char _CHK[(COND)?1:-1]`.

**H. Kỷ luật chung**
16. Backup trước khi sửa; port bé→to; compile + boot từng slice; coverage-diff cuối (gap == defer list).
17. Ghi version-gap: subsystem mới thường chỉ render trên client v246, không PAP2/2010 → packet
    có thể defer.

---

## 9. Con trỏ chi tiết
- `docs/arch_research/devdocs.md` — tài liệu thiết kế gốc dev (SO3World taxonomy, sync, login/cross-server, combat/AI/quest/enchant/magic-attr).
- `docs/arch_research/build_init.md` — init chain + 25 KWorldSettings manager + KPlayer sub-objects + taxonomy, có file:line.
- `docs/arch_research/bindings.md` — 3 kiểu Lua binding + counts + network + checklist gốc, có file:line.
- `docs/port_map/PORT_MAP.md` + `PORT_STRATEGY.md` — 1581 method cần port + thứ tự cụm.
- `docs/{exterior,hair}_port/` — 2 case study port hoàn chỉnh (hair = model DWARF-based).
