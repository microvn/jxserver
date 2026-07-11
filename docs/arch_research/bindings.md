# JX3 SO3GameServer — BẢN ĐỒ LIÊN KẾT (binding + connections)

> Nguồn: source 2010 `linux-build/src` + `include`; cross-check DWARF `jx3_dwarf/`.
> READ-ONLY research. Mọi dòng kèm `file:line`. Mục tiêu: port subsystem mới KHÔNG BỎ SÓT
> điểm móc nào (như vụ KExterior quên register Lua). **Mục 4 (CHECKLIST) là output chính.**

Đếm nhanh:
- **Luna<T> classes register**: 37 (26 chung + 5 `_SERVER` + 6 `_CLIENT`) — server build = 31.
- **base-func `GetXxx`/free-func** trong `KBaseFuncList.cpp`: 179 entry bảng (≈155 free-func Lua*, phần còn lại là `Luna<>::Register` loader).
- **REGISTER_LUA_FUNC** (method KPlayer/KCharacter/KSceneObject): 407 trong `KLuaPlayer.cpp`.
- **ROLE_DATA_BLOCK_TYPE** (role-block persistence): 23 block hợp lệ (rbtSkillList..rbtHairBoxData) + rbtInvalid + rbtTotal.

---

## 1. C++ ↔ Lua binding — 3 kiểu

Cơ chế nạp tổng thể: `KScriptCenter::CreateScriptHolder()` (`KScriptCenter.cpp:709`) tạo mỗi
lua_State rồi làm 3 việc: (a) `RegisterFunctions(pFunList, nFuncNum)` với bảng base-func
(`:721-722`); (b) `RegisterConstList(g_LuaConstList)` (`:724`); (c) một loạt `Luna<T>::Register`
(`:729-780`). Đây là 3 kiểu binding.

### (a) Luna<T> — object binding (class C++ thành "userdata" Lua có metatable)

Cơ chế (`include/Include/Luna.h`):
- Macro `DECLARE_LUA_CLASS(T)` (`Luna.h:13-39`) đặt trong khai báo class, sinh 3 thứ:
  `static const char szClassName[]`, `static Luna<T>::KLuaData LuaInterface[]`, và method
  `int LuaGetObj(lua_State* L)`. `LuaGetObj` làm: `lua_newuserdata(L, sizeof(KObjData))`,
  gán `pObjData->pObj = this`, rồi `luaL_getmetatable(L, szClassName)` + `lua_setmetatable`.
  → tức là push chính object C++ (con trỏ `this`) lên stack Lua dưới dạng userdata có metatable.
- Macro `DEFINE_LUA_CLASS_BEGIN(T)` / `..._END(T)` (`Luna.h:112-114, 216`) định nghĩa
  `szClassName` = tên class và mở/đóng mảng `LuaInterface[]`. Giữa 2 macro là các
  `REGISTER_LUA_*` (INTEGER/DWORD/STRING/ENUM/BOOL/TIME/FUNC + biến thể `_READONLY`,
  `Luna.h:119-211`) — mỗi dòng = 1 property/method expose sang Lua kèm getter/setter và
  `LUA_DATA_TYPE`.
- `Luna<T>::Register(L)` (`Luna.h:300-350`): tạo 1 table tra cứu (name→KLuaData) đặt vào
  global theo `szClassName`, rồi `luaL_newmetatable(szClassName)` gắn `__index`=`index`,
  `__newindex`=`newindex`, `__tostring`, `__gc`. `index`/`newindex` (`Luna.h:369/461`) dùng
  property-name để tra `LuaInterface`, ép kiểu getter/setter đúng `eDataType`, gọi member.
  Method (`ldtFunc`) push closure `Dispatcher<T>` (`Luna.h:282-297`, có specialization riêng
  cho KPlayer/KNpc/KDoodad).

**Danh sách class đăng ký `Luna<T>::Register` — `KScriptCenter.cpp:729-780`:**

Chung (client+server), `:729-756,780` (26):
KScene, KCell, KPlayer, KNpc, KNpcTemplate, KDoodad, KDoodadTemplate, KItem, KSkill,
KQuestInfo, KShop, KProfession, KCraftRecipe, KCraftEnchant, KCraftCollection, KCraftRead,
KCraftCopy, KROUTE_NODE, KCustomEquipInfo, KOtherItemInfo, KAILogic, KAIState, KAIAction,
KCampInfo, KGPQ, KTarget.

`#ifdef _SERVER` `:758-760, 774-779` (5):
KBUFF_LIST_NODE, KScriptServer, KGlobalSystemValueCache, KMentorCache, **KHairShop** (`:778`).

`#ifdef _CLIENT` `:762-772` (6):
KTeamClient, KAuctionClient, KTongClient, KGameCardClient, KMailClient, KMailInfo.

**Các class dùng `DEFINE_LUA_CLASS_BEGIN` (40 nơi) — `grep -rn DEFINE_LUA_CLASS_BEGIN src/SO3World/Src`:**
KAIAction, KAILogic, KAIState, KBUFF_LIST_NODE, KBuff, KGlobalSystemValueCache,
KCustomEquipInfo, KOtherItemInfo, KGPQ, KAIEvent, KGameCardClient, KMentorCache,
KAuctionClient, KCell, KQuestInfo, KShop, **KHairShop** (`KLuaHairShop.cpp:110`),
KDoodadTemplate, KNpc, KNpcTemplate, KMailClient, KMailInfo, KProfession, KCraftCollection,
KCraftRecipe, KCraftEnchant, KCraftRead, KCraftCopy, KSkill, KTongClient, KCampInfo, KDoodad,
KItem, KPlayer (`KLuaPlayer.cpp:11000`), KScene, KTarget, KROUTE_NODE, KScriptServer,
KTeamClient.
(Lệch nhẹ: KBuff/KAIEvent có DEFINE nhưng KHÔNG ở list Register — dùng qua đường khác;
ngược lại các loader chỉ có trong base-func table không cần DEFINE.)

**Khi nào dùng Luna<T>**: khi Lua cần *cầm nguyên object* và gọi nhiều method/đọc field trên
nó. 2 mẫu: (1) **per-instance** như KPlayer/KNpc/KItem — object có nhiều instance, lấy qua
`GetPlayer(id)`; (2) **global singleton** như KMentorCache/KHairShop/KGlobalSystemValueCache
— 1 instance sống trong world/settings, lấy qua base-func `GetXxx` (mục b).

### (b) base-func `GetXxx` — free-function toàn cục (không gắn với object nào)

Bảng `KLuaFunc BaseFuncList[]` (`KBaseFuncList.cpp:4833-5117`) — cặp `{"TênLua", LuaCFunc}`.
`GetBaseFuncList` (`:5120-5124`) trả con trỏ + `sizeof/sizeof`. Được nạp ở
`KScriptCenter.cpp:721-722`.

Free-func `LuaXxx(Lua_State* L)` đọc arg bằng `Lua_GetTopIndex` + `lua_toXxx`, làm việc, push
kết quả. Với **singleton exposé qua Luna** thì free-func chỉ gọi `.LuaGetObj(L)`:
- `LuaGetMentorCache` (`:4170`): `g_pSO3World->m_MentorCache.LuaGetObj(L)`.
- `LuaGetHairShop` (`:4183`): `g_pSO3World->m_Settings.m_HairShop.LuaGetObj(L)` — **đây là cầu
  nối kiểu (a)↔(b): register metatable ở (a), phát object ra Lua ở (b).**
- `LuaGetGlobalSystemValueCache` (`:4525`).

Đăng ký tương ứng trong bảng `#ifdef _SERVER`:
`{"GetMentorCache", LuaGetMentorCache}` (`:5046`), `{"GetHairShop", LuaGetHairShop}` (`:5047`),
`{"GetGlobalSystemValueCache", ...}` (`:5087`). Loader `{"LoadXxx", Luna<T>::Register}` cũng
nằm chung bảng (`:4841-4872`).

Ví dụ free-func điển hình (không object): GetPlayer/GetNpc/GetScene/GetSkill/GetItem
(`:4881-4885`), GetRelation/IsEnemy/IsAlly (`:4899-4904`), Random (`:4905`), SendSystemMail
(`:4938`), GSCommand/GCCommand (`:5010-5011`).

**Khi nào dùng (b)**: hàm toàn cục không thuộc object (query/tiện ích/factory-getter), HOẶC
cổng phát 1 singleton Luna ra Lua.

### (c) KPlayer/KCharacter method binding — `REGISTER_LUA_FUNC`

Method của nhân vật KHÔNG đăng ký lẻ; chúng nằm trong `LuaInterface[]` của **KPlayer**:
`DEFINE_LUA_CLASS_BEGIN(KPlayer)` tại `KLuaPlayer.cpp:11000`, 407 dòng `REGISTER_LUA_FUNC`.
Vì KPlayer kế thừa KCharacter/KSceneObject, bảng gộp method của cả 3 lớp:
- `REGISTER_LUA_FUNC(KSceneObject, GetAbsoluteCoordinate)` `:11007`
- `REGISTER_LUA_FUNC(KCharacter, AutoFly)` / `SetModelID` `:11340-11341`
- `REGISTER_LUA_FUNC(KPlayer, Stop)` `:11343`
- `REGISTER_LUA_FUNC(KCharacter, WalkTo)` `:11346`

`KLuaCharacter.cpp` (2124 dòng) chứa **thân hàm** `KCharacter::LuaXxx(lua_State*)` nhưng **0**
`REGISTER_LUA_FUNC` — đăng ký sống ở bảng KPlayer. Macro (`Luna.h:207-211`) ánh xạ
`"FuncName"` → `&T::LuaFuncName`, kiểu `ldtFunc`, gọi qua `Dispatcher<T>`.

**Khi nào dùng (c)**: thêm hành vi lên nhân vật (player/npc) mà script gọi `player.DoXxx(...)`.
Đây là cách KExterior đã port (`KLuaPlayer.cpp`, 9 binding Apply/AddExterior...) — khác KHair
(dùng singleton Luna kiểu a/b). Chọn (a/b) khi tính năng có "cửa hàng/kho" toàn cục; chọn (c)
khi thao tác gắn thẳng vào 1 player.

---

## 2. Persistence (role-block save/load)

### enum ROLE_DATA_BLOCK_TYPE — `include/Include/KRoleDBDataDef.h:14-44` (đầy đủ)
```
rbtInvalid=0, rbtSkillList, rbtItemList, rbtQuestList, rbtProfessionList, rbtStateInfo,
rbtBuffList, rbtRepute, rbtUserPreferences, rbtBookState, rbtSkillRecipeList, rbtRecipeList,
rbtCoolDownTimer, rbtRoadOpenList, rbtCustomData, rbtVisitedMap, rbtPQList, rbtHeroData,
rbtAchievementData, rbtDesignationData, rbtRandData, rbtAntiFarmerData, rbtMentorData,
rbtExteriorData, rbtHairBoxData,   rbtTotal
```
Lưu ý: bản `src/SO3GameServer/Test/.../Robot/KRoleDBDataDef.h` là COPY CŨ — bỏ qua, chỉ dùng
`include/Include/`. Append block MỚI ngay trước `rbtTotal` (ordinal self-consistent, không cần
khớp v246 vì đọc/ghi cùng 1 build).

### SAVE (KPlayer::Save) — `KPlayer.cpp:2321-2388`
Macro `SAVE_ROLE_BLOCK(Func, BlockType, Version)` (`:2303-2319`): ghi `KRoleBlockHeader`
{nType, dwVer, dwLen} rồi gọi `Func(&uDataLen, pbyOffset, remain)`. Chuỗi 23 lời gọi
`:2344-2367`, ví dụ `SAVE_ROLE_BLOCK(m_ExteriorBox.Save, rbtExteriorData, 0)` (`:2364`),
`SAVE_ROLE_BLOCK(m_HairBox.Save, rbtHairBoxData, 0)` (`:2365`). Bao ngoài là
`KRoleDataHeader` {dwVer, dwCRC=CRC32(...), dwLen} (`:2340-2373`). `#undef` tại `:2390`.
Chữ ký callee: `Save(size_t* puUsed, BYTE* pbyBuf, size_t uBufSize)`.

### LOAD (KPlayer::LoadExtRoleData) — `KPlayer.cpp:1903-~2070`
Kiểm `KRoleDataHeader` + CRC32 (`:1914-1924`), rồi vòng `while(uLeftSize>0)` đọc từng
`KRoleBlockHeader` và `switch(pBlock->nType)` (`:1938`) — mỗi case gọi `m_Xxx.Load(pbyOffset,
pBlock->dwLen[, dwVer])`. Các case: rbtSkillList `:1940` … rbtExteriorData `:2035`,
rbtHairBoxData `:2040`, rbtMentorData `:2055`. Chữ ký callee: `Load(BYTE* p, size_t len)`.
**QUAN TRỌNG**: SAVE và LOAD-switch là 2 nơi PHẢI sửa song song khi thêm block; thứ tự save
không cần khớp thứ tự enum (load dispatch theo nType).

### KRelayClient — vai trò DB proxy
KRelayClient (`KRelayClient.cpp`) là client tới **center/relay** (`Connect` `:253`,
`DoHandshakeRequest` `:271`, "Connect to center server" `:284`). Đây là kênh GS↔Center; dữ
liệu role thực tế đi qua đường relay/center để tới DB. Blob role-data (Save/Load ở trên) là
payload nội bộ GS, gói vào protocol S2R/R2S để center lưu/nạp. (Chi tiết net = mục 3.)

Luồng: login → center/relay đẩy blob về GS → `LoadExtRoleData` dựng lại player → chơi →
`KPlayer::Save` đóng gói lại → đẩy ngược center → DB.

---

## 3. Network / process connections

### Các process
- **SO3GameServer** (`src/SO3GameServer/`, entry `KSO3GameServer.cpp` + `Main.cpp`) — world/GS.
- **SO3GameCenter** — điều phối cluster, cầu tới DB (binary `jx3_dwarf/SO3GameCenterD`).
- **SO3Gateway** — cổng client vào (binary `jx3_dwarf/SO3GatewayD`).
- **Relay** — tầng chuyển tiếp GS↔Center.
- **Paysys/Bishop, LogServer** — ngoài leak / phụ trợ (xem protocol headers).

### File protocol + nhóm enum
| File | Nhóm enum | Cặp |
|---|---|---|
| `include/Include/GatewayClientProtocol.h` | `C2G_PROTOCOL` (:8), `G2C_PROTOCOL` (:26) | client ↔ gateway |
| `include/Include/GatewayRelayProtocol.h` | `G2R_PROTOCOL` (:6), `R2G_PROTOCOL` (:27) | gateway ↔ relay |
| `include/Include/Relay_GS_Protocol.h` | `KS2R_PROTOCOL` (:24), `KR2S_PROTOCOL` (:202) | GS ↔ relay/center |
| `include/Include/GS_Client_Protocol.h` | `GS_CLIENT_PROTOCOL` (:25), `CLIENT_GS_PROTOCOL` (:336) | GS ↔ client (game) |
| `include/Include/GE_Protocol.h` | `EDITOR_2_GS_PROTOCOL` (:16), `GS_2_EDITOR_PROTOCOL` (:30) | editor ↔ GS |
| `include/Base/protocol/*` | Bishop/Paysys/Log/Goddess/Spreader | dịch vụ ngoài |

Struct payload dùng header có `#pragma pack(1)`: `UNDEFINED_SIZE_*_HEADER` (GS_Client),
`INTERNAL_PROTOCOL_HEADER` (Relay_GS — lưu ý drift 3B→2B đã fix, xem MEMORY §R10).

### Ai kết nối ai + handshake/connect code
- **client → gateway**: `KGatewayClient` (`KGatewayClient.cpp`). `ConnectGateway` `:399`
  (`Connector.ConnectSecurity(m_szLoginIP,...)` `:407`), `DoHandshakeRequest` `:490`,
  `OnHandshakeRespond` `:862`. Sau login gateway trỏ client sang GS: `g_PlayerClient.Connect`
  `:1304` + `DoHandshakeRequest(roleID, guid)` `:1307`.
- **GS → center/relay**: `KRelayClient` (`KRelayClient.cpp`). `Init` `:221`, `Connect` `:253`,
  `DoHandshakeRequest` `:271`. Đăng ký handler kiểu
  `REGISTER_INTERNAL_FUNC(r2s_handshake_respond, &KRelayClient::OnHandshakeRespond, 18)` `:39`
  (số cuối = size gói, khớp binary — bẫy drift). `KCenterRemote.cpp` = mặt điều khiển
  center-side.
- **GS → gateway** (đẩy trạng thái player): `KGatewayClient` phía GS.
- **Đăng ký handler net (mẫu binding cần khi thêm protocol)**:
  - client-side: `REGISTER_G2C_FUNCTION(g2c_handshake_respond, OnHandshakeRespond,
    G2C_HANDSHAKE_RESPOND)` (`KGatewayClient.cpp:43`).
  - internal GS↔relay: `REGISTER_INTERNAL_FUNC(name, &Class::Handler, size)`
    (`KRelayClient.cpp:39`).

### Emit gói đồng bộ (server→client) — mẫu "Do*"
`KPlayerServer::DoSyncXxx` phát broadcast. VD KExterior: `DoSyncExteriorBoxData` (protocol
0x11a) — thêm enum vào `GS_CLIENT_PROTOCOL` + viết hàm `Do*` emit + gọi tại điểm dữ liệu đổi.
KHair: `DoSyncEquipRepresent` (`KPlayerServer.cpp:3400`) tự chạy khi `SetRepresentID` đổi giá
trị (`KItemList.cpp:3373`).

---

## 4. SUBSYSTEM INTEGRATION CHECKLIST (output chính — copy để dùng)

Khi port 1 subsystem MỚI (kiểu KExterior/KHair/KDesignation), duyệt HẾT các mục dưới. Mỗi mục
= *nơi phải sửa* + *pattern*. Bỏ sót 1 mục = binding treo (nil ở runtime, mất data, hoặc client
không sync). Đánh dấu N/A có lý do, đừng để trống.

**A. Config table (nạp bảng .tab từ `settings/`)**
1. Viết `KXxx::Init()` nạp bảng qua `g_OpenTabFile` + vtbl `KTabFile`. Mẫu:
   `KExterior::Init` / `KHairShop::Init`.
2. Nhúng singleton config vào `KWorldSettings` làm member (`KWorldSettings` header) và gọi
   `bRetCode = m_Xxx.Init();` trong `KWorldSettings::Init()` (`KWorldSettings.cpp:8`, cạnh
   `m_Exterior.Init()` `:117` / `m_HairShop.Init()` `:121`). Thêm `m_Xxx.UnInit()` vào nhánh
   lỗi (`:140`) và `UnInit()` (`:269`). — **Quên bước này ⇒ bảng không nạp, "Load game settings"
   fail.**

**B. Per-player state (nếu có dữ liệu theo từng nhân vật)**
3. Thêm member vào `KPlayer.h` (VD `KHairBox m_HairBox;` / `KExteriorBox m_ExteriorBox;`).
   KHÔNG hardcode offset — để compiler layout; box giữ back-ptr `m_pPlayer`.
4. Khởi tạo member trong `KPlayer::Init`/ctor nếu box cần `SetPlayer(this)`.

**C. Persistence (role-block) — nếu B có, thường phải có**
5. Thêm enum `rbtXxxData` vào `ROLE_DATA_BLOCK_TYPE` **ngay trước `rbtTotal`**
   (`include/Include/KRoleDBDataDef.h:41-43`). (KHÔNG sửa bản copy trong Test/Robot.)
6. Thêm `SAVE_ROLE_BLOCK(m_Xxx.Save, rbtXxxData, 0);` vào `KPlayer::Save`
   (`KPlayer.cpp:2344-2367`, cạnh dòng Exterior/Hair).
7. Thêm `case rbtXxxData:` vào switch trong `KPlayer::LoadExtRoleData`
   (`KPlayer.cpp:1938`, cạnh `case rbtHairBoxData:` `:2040`).
8. Định nghĩa struct DB-item trong `KRoleDBDataDef.h` với `#pragma pack(1)` + `static_assert`
   size khớp binary (bẫy: WORD id vs DWORD, trailing count). Callee chữ ký
   `Save(size_t*,BYTE*,size_t)` / `Load(BYTE*,size_t)`.

**D. Lua binding — CHỌN 1 trong 2 (đây là mục hay-bị-sót nhất, vụ KExterior)**
9a. **Singleton (kiểu KHair/KMentor)** — 3 nơi:
   - `KScriptCenter.cpp` khối `#ifdef _SERVER` (`:774-779`): thêm `Luna<KXxx>::Register(pLuaState);`.
   - `KBaseFuncList.cpp` (~`:4183`): viết free-func `LuaGetXxx` → `...m_Xxx.LuaGetObj(L)`.
   - `KBaseFuncList.cpp` bảng `#ifdef _SERVER` (`:5046-5047`): thêm `{"GetXxx", LuaGetXxx}`.
   - Trong class: `DECLARE_LUA_CLASS(KXxx)` (header) + `DEFINE_LUA_CLASS_BEGIN/END` + các
     `REGISTER_LUA_*` (file `KLuaXxx.cpp`, mẫu `KLuaHairShop.cpp:110`).
9b. **Method trên player (kiểu KExterior)** — thêm `REGISTER_LUA_FUNC(KPlayer, DoXxx)` vào
   bảng KPlayer (`KLuaPlayer.cpp:11000+`) và viết thân `KPlayer::LuaDoXxx(lua_State*)`.
   → **Kiểm tra**: sau build, gọi thử `GetXxx()`/`player.DoXxx()` trong Lua phải KHÁC nil.

**E. Packet sync (server→client) — nếu client cần thấy thay đổi**
10. Thêm ID vào `GS_CLIENT_PROTOCOL` (`GS_Client_Protocol.h:25`); struct payload `#pragma
    pack(1)` khớp binary.
11. Viết `KPlayerServer::DoSyncXxx(...)` (mẫu `DoSyncExteriorBoxData` / `DoSyncEquipRepresent`
    `KPlayerServer.cpp:3400`) và GỌI nó tại mọi điểm dữ liệu đổi (setter). Nếu tái dùng
    represent, `SetRepresentID` tự broadcast (`KItemList.cpp:3373`) — có thể khỏi viết mới.
12. Nếu là protocol nội bộ GS↔center: thêm vào `KS2R_/KR2S_PROTOCOL`
    (`Relay_GS_Protocol.h`) + `REGISTER_INTERNAL_FUNC(name,&Class::Handler,SIZE)` với SIZE
    khớp binary (bẫy pack/drift).

**F. Log (tuỳ chọn, cho GM/audit)**
13. Khai báo `LogPlayerXxx(...)` trong `KLogClient.h` (mẫu `:48-80`) + impl; gọi tại
    request/respond. Có thể stub nếu chưa cần.

**G. Version-gap note (BẮT BUỘC ghi lại)**
14. Ghi rõ client-side: subsystem mới thường CHỈ render trên client v246/BVTLocal, KHÔNG có
    trong PAP2/client 2010 (UI Lua nằm trong PAK, không phải C++). Server-emit vẫn đúng nhưng
    client 2010 sẽ bỏ qua gói. Note vào PORT_DESIGN §1.

**H. Trình tự & kỷ luật (từ exterior_port README/PORT_DESIGN)**
15. Backup TRƯỚC khi chạm (`backup_source_YYYYMMDD_HHMMSS/` giữ nguyên cây), KHÔNG xoá gì.
16. Port **bé→to** (leaf: enum/struct → singleton/box → apply/hook → persistence → packet →
    Lua binding → buy/grant). Compile sạch + verify từng bước, tránh TODO treo.
17. Mọi offset/struct/enum lấy từ Ghidra/objdump — không đoán. Fix source + build, KHÔNG patch
    exe.

Thứ tự khuyến nghị áp checklist: A → B → C → D → E → F → G, xen H xuyên suốt.
