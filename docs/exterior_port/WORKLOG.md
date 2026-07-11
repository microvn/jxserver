# KExterior/KExteriorBox — RE & Port Understanding (overnight)
**Start:** 2026-07-08 03:14:26 +07

## Mục tiêu
RE code v2.5 (binary v246) của KExterior/KExteriorBox → hiểu data model + logic → thiết kế port chính xác vào build 2010.
Ràng buộc: KHÔNG xoá gì; backup trước khi chạm source; nếu cần xoá thì DỪNG hỏi.

## Nhật ký
- [2026-07-08 03:14:26 +07] Khởi động. Tạo work area. Chưa chạm source.

## [RE-1] Function map từ __PRETTY_FUNCTION__ (SO3GameServer v246)

### Data structures (structs/enums) lộ ra từ signature
- **KEXTERIOR_ITEM** — 1 món ngoại trang trong box
- **KEXTERIOR_SET_ITEM** — 1 món trong set
- **KEXTERIOR_DB_DATA** { ::KEXTERIOR_INFO } — struct lưu DB của box
- **KEXTERIOR_SET_DB_DATA** { ::KEXTERIOR_SET_INFO } — struct lưu DB của set
- **KEXTERIOR_BUY_LATEST_DB_DATA** { ::KEXTERIOR_INFO } — struct lưu list "mua gần nhất"
- **KEXTERIOR_BUY_PARAM**, **KEXTERIOR_BUY_FROM_ITEM**, **KEXTERIOR_COIN_BUY_PARAM** — tham số mua
- Enums: EXTERIOR_TIME_TYPE, EXTERIOR_BUY_SOURCE, EXTERIOR_PAY_CURRENCY_TYPE, EXTERIOR_BUY_RESPOND_CODE

### KExterior (manager tĩnh: bảng config + logic MUA) — Src/KExterior.cpp
- Init / LoadExteriorInfoTable / LoadExteriorSuitInfoTable / LoadExteriorIndexInfoTable / LoadExteriorShopPriceTable
  (khớp 4 tab: settings/Exterior/ExteriorInfo, ExteriorSuitInfo, Exteriorshop, ExteriorBuy)
- Buy chain: Buy, BuyFromShop, BuyFromItem, CanBuy, CanBuyOne, RenewExterior, CanRenew, Pay, PayCoin,
  GetAllCost, GetAllCostFromShop, IsMoneyEnough, Delivery, AddExterior, GetExteriorBuyParams, OnBuyExteriorRespond
- Lua: LuaGetExteriorShopPrice, LuaBuyExteriorFromShop, LuaRenewExterior, LuaCanAchieve,
  LuaGetSuitAchievementID, LuaGetExteriorIndex, LuaGetExteriorSuitInfo, LuaGetExteriorInfo

### KExteriorBox (instance mỗi player: tủ đồ) — Src/KExteriorBox.cpp
- Init(KPlayer*), Activate()
- Add(id,int,time_t) / _Add(→KEXTERIOR_ITEM*) / Delete / DeleteExteriorInSet / DeleteExteriorInLatestBuy
- ApplyExteriorRepresent(int), IsHaveExteriorRepresent(int)  ← MÓC vào cơ chế represent-ID
- SetExterior(size_t,int,id), SetCurrentSetID(size_t)
- AddExteriorFreeCount / SetExteriorFreeCount
- AddLatestBuy / SaveLatestBuyExterior / LoadLatestBuyExterior
- SaveExteriorSet / LoadExteriorSet
- Equip helpers: GetEquipColorIndex / GetEquipSubType / GetEquipRepresentIndex / GetEquipPosBySubType / GetEquipPos
- Persistence khối: Save / Load / SaveExteriorBox / SaveExteriorBoxForOneBlock / LoadExteriorBox

### KPlayerServer: DoSyncExteriorBoxData, DoSyncExteriorSetData (sync client)
### KLogClient: LogPlayerAddExteriorRequest/Respond

### Kiến trúc suy ra
- KExterior = singleton: nạp 4 bảng config + xử lý MUA (tiền/currency/delivery) + query Lua về info.
- KExteriorBox = gắn mỗi player (Init(KPlayer*)): sở hữu box + set + latest-buy; Add/Delete/Apply/Save/Load;
  ApplyExteriorRepresent → nối vào m_wRepresentId (cơ chế 2010 ĐÃ CÓ).

## [RE-2] Decompile 3 hàm lõi → data model

### Địa chỉ hàm (từ xref __PRETTY)
- KExteriorBox::Init = FUN_08200104 | ApplyExteriorRepresent = FUN_08200a4e | _Add = FUN_082016a6
- Save = FUN_08201100 | Load = FUN_08202498 | KExterior::AddExterior = FUN_081f9498

### KExteriorBox — LAYOUT MEMBER (từ Init)
- off 0x00 : current-set-id / count (init 0)
- off 0x04 : container STL (init FUN_08203084)  — nghi: exterior-set list
- off 0x10 : container STL (init FUN_08203bf4)  — nghi: latest-buy list
- off 0x38 : container STL (init FUN_08203010)  — **BOX map chính** (dùng trong _Add)
- off 0x44 : m_pPlayer (KPlayer*)
- off 0x48 : counter (init 0)
→ 3 container (map/list) + con trỏ player. Box chính ở 0x38.

### KEXTERIOR_ITEM (từ _Add, key = exterior ID)
- key    : exterior ID (long unsigned int)
- off+4  : flag (permanent?)
- off+8  : expire_time (time_t)
- Add(id, time_type, param4):
  * time_type 0 = vĩnh viễn (expire=0, flag=0)
  * time_type 1 = +0x93a80 (=604800s = 7 ngày)
  * time_type 2 = set expire = max(existing, param4)
- Box tối đa 0x1fff (8191) món.

### KExteriorInfo (bảng config, tra bằng ID qua FUN_081f70d0)
- off 0x0c : nSubType
- off 0x10 : representID  (giá trị đắp lên ô hiển thị)
- off 0x14 : representID2 / color

### ApplyExteriorRepresent(nSlot) — MÓC TÍCH HỢP (crux)
- nSlot ∈ [0,5) → mỗi set 5 ô.
- set = mảng ID 4-byte, lấy qua GetExteriorSet(this, this[0]=current-set).
- Với mỗi ô: tra ExteriorInfo → validate nSubType khớp → nếu còn hạn:
    representID = info[0x10], color = info[0x14]
- Kiểm bit apply: `player[0xb89c] >> nSlot & 1` → nếu bật:
    FUN_082009a2(this, representIndex, representID)  // ghi vào ô represent
    FUN_082009a2(this, colorIndex, color)
→ **Đây là chỗ nối vào cơ chế represent-ID mà 2010 ĐÃ CÓ (m_wRepresentId).**
→ Player có **apply-bitmask tại offset 0xb89c** (bit mỗi ô).

## [RE-3] Save/Load format + AddExterior + KPlayer embedding

### Save (FUN_08201100) — block-based
Format: [WORD len1][box block][WORD len2][set block]; *pOut = tổng byte.
- block1 = SaveExteriorBox (FUN_08200d72), block2 = SaveExteriorSet (FUN_08200ec4)
- (latest-buy lưu riêng qua SaveLatestBuyExterior)
### Load (FUN_08202498) — mirror
[WORD len1][box]→LoadExteriorBox(FUN_08200382); [WORD len2][set]→LoadExteriorSet(FUN_0820224a); assert leftover==0.

### AddExterior (FUN_081f9498) — entry MUA→giao hàng
Args: (pPlayer, exteriorID, timeType[0-2], payType[0-2], buySource(0,3])
- buySource 1/2: tra KExteriorInfo → check nPrice[timeType][payType]!=-1 → nếu payType==2 lấy info[+0x20 + timeType*0xc]
- buySource 3 (shop): tra ExteriorShopPrice → check nPrice → payType==2 lấy shop[+8 + timeType*0xc]
- Gọi FUN_081f7df0 (add vào box qua player) → log (FUN_081b6fe8) → sync client (FUN_081f9060)

### KExteriorInfo (bảng, tra ID qua FUN_081f70d0)
- +0x0c nSubType | +0x10 representID | +0x14 color/representID2
- nPrice[3][3] vùng ~+0x18 | +0x20+timeType*0xc: giá trị payType==2 (nghi: bind item)

### *** KEY: KPlayer EMBEDDING ***
- KExteriorBox NHÚNG trong KPlayer tại **offset 0xb850** (FUN_081ff864(player+0xb850) = lấy box)
- apply-bitmask tại **player+0xb89c** (= box+0x4c) — bit mỗi ô đang khoác
- Box size ~0x4c bytes (Init dùng tới off 0x48)
→ Port: thêm member KExteriorBox vào KPlayer, khởi tạo trong player-init, Save/Load nối vào player persistence.

## [RE-4] Latest-buy serialization
- KEXTERIOR_BUY_LATEST_DB_DATA::KEXTERIOR_INFO = 8 byte { WORD id; BYTE pad[6] }
- Save: [WORD count][count × 8-byte item]; Load mirror; leftover phải == 0
- Latest-buy sync client cap 20 (0x14) entry; sync qua player+0x44(=m_pPlayer)+0xa50
- Container box dùng iterator FUN_08202f74(list, index) → *ptr = id

## [RE-5] Slot→index mapping (móc represent CHÍNH XÁC) + box block format

### Map slot (0-4) → chỉ số vào m_wRepresentId  [từ GetEquipRepresentIndex/ColorIndex/SubType]
| Slot | RepresentIndex | ColorIndex | SubType |
|------|:--:|:--:|:--:|
| 0 | 5  | 6  | 2  |
| 1 | 2  | 3  | 3  |
| 2 | 8  | 9  | 6  |
| 3 | 14 | 15 | 9  |
| 4 | 11 | 12 | 10 |
→ Apply slot N: m_wRepresentId[RepresentIndex[N]] = representID; m_wRepresentId[ColorIndex[N]] = color.
→ 5 ô ngoại trang = 5 loại trang bị (SubType 2/3/6/9/10).

### SaveExteriorBox (FUN_08201476) — role-block
- MAX_EXTERIOR_LIST_SIZE = 0x2000 (8192)
- Chia box thành khối 0x80 (128) món; nBlock = (count>>7)+1
- Mỗi khối: **KRoleBlockHeader = 12 byte {DWORD blockType(param5); DWORD blockKey(param6); DWORD dataLen}** + data
- FUN_082012b4 = SaveExteriorBoxForOneBlock (serialize ≤128 món/khối)
→ Box lưu theo chuẩn role-block của JX3 (giống các hệ khác) — dễ nối vào ext-data player.

### Đã giải quyết follow-up: slot mapping ✓, box block format ✓
### Còn: layout đầy đủ KEXTERIOR_INFO (nPrice), packet sync, Lua binding KPlayer route

## [BACKUP] 03:23:59
- KHÔNG chỉnh source lần này (chỉ RE + viết scratchpad). Đã copy sẵn để cách li khi bắt đầu code:
  KPlayer.h/.cpp, KLuaCharacter.cpp → /private/tmp/claude-501/-Volumes-ExData-game-jx3/a3f5a210-2f89-426d-a1ab-79e564a6aecd/scratchpad/exterior_port/backup_source_20260708_032359
- Bản gốc nguyên vẹn. KHÔNG xoá gì.

## [RE-6] Giải trọn 4 follow-up (canonical: linux-build/docs)

### (1) KEXTERIOR_INFO layout — từ LoadExteriorInfoTable (FUN_081fb30c)
Bảng ExteriorInfo.tab đọc theo cột (đúng thứ tự struct, ~72B / 18 dword):
ID, ForceID, Genre, set, subtype, represetID, colorID,
[7days0, 7days1, 7daysFree], [Permanent0, Permanent1, PermanentFree],
Limittype, [Limitprice0, Limitprice1, LimitpriceFree], IconID, RepresetID1
→ nPrice[3 timeType: 7days/Permanent/Limit][3 payType: 0/1/Free]; **giá trong tab ×10000** (money).
→ Validate: nPrice[i][epctMoney] > 0.
→ File load: snprintf "%s/Exterior/%s" + g_OpenTabFile (chuẩn tab-file JX3).

### (2) PACKET SYNC — DoSyncExteriorBoxData (FUN_08059bf0) [KPlayerServer]
- Protocol ID = **0x11a (282)**; buffer @ player+0x2b68; MAX = 0x8000.
- Header 6B: {WORD protocolID=0x11a; WORD size; WORD count}
- Mỗi item 7B: {WORD id; BYTE flag; DWORD expire}   (size = count*7 + 6)
- KEXTERIOR_ITEM in-memory stride = 0xc (12B): {DWORD id@0; DWORD flag@4; DWORD expire@8}
- Gửi qua thunk_FUN_085650f3(player, connID, buf, size).

### (3) Lua routing — KPlayer::LuaApplyExterior (FUN_083157b8)
```
LuaApplyExterior(L):  // gọi không tham số từ script
  player[0xb89c] |= 0x80              // m_dwApplyExteriorFlag: bit 0x80 = master apply-on
  for slot in 0..4:
    if IsHaveExteriorRepresent(box@player+0xb850, slot):
       ApplyOneSlot(player, slot)     // FUN_0838844a → box.ApplyExteriorRepresent + sync represent
  SyncApplyExteriorFlag(player, player+0xa50, player[0xb89c])
```
→ Xác nhận: **m_dwApplyExteriorFlag @ player+0xb89c** (bit 0-4 = per-slot, bit 0x80 = master); box @ player+0xb850.
→ Có LuaApplyExterior / LuaUnApplyExterior / LuaIsApplyExterior (KPlayer).

### (4) LoadExteriorBox mirror (FUN_08201fee)
- [WORD count][count × 16B item]; assert leftover==0.
- DB item 16B: {WORD id@0; WORD flag@2; DWORD expire@3; pad→16}  (KEXTERIOR_DB_DATA::KEXTERIOR_INFO = 0x10)
- Mỗi item: lookup info → _Add(box, id, flag, expire); cuối cùng DoSyncExteriorBoxData (sync client).

### CAVEAT: client pap2 (jx3dev-master, 2.5.2 build 4550) KHÔNG có "Exterior" trong source
→ pap2 client cũ hơn server v246 → không render ngoại trang. Muốn thấy ngoại trang cần client v246/BVTLocal.
→ Packet format (0x11a) lấy từ SERVER binary là đủ để code phía server; phía nhận là client v246.

## [PORT-1] Lat #1 — enums + leaf structs (compile clean + layout verified)
**Time:** 2026-07-08 ~10:44 +07

### Artifact
- NEW: `src/SO3World/Src/KExteriorDef.h` (enums + KEXTERIOR_ITEM + KEXTERIOR_INFO + KEXTERIOR_DB_DATA + KEXTERIOR_BUY_LATEST_DB_DATA).
- No existing source modified. Backup dir: `docs/exterior_port/backup_source_20260708_104341/` (MANIFEST only; nothing to back up this slice).

### 2 layout conflicts RESOLVED by fresh decompile (were guesses/typos in RE-2/RE-6)
Decompiled FUN_081fb30c (LoadExteriorInfoTable), FUN_081f70d0 (lookup), FUN_08201fee (LoadExteriorBox).

**(A) KEXTERIOR_INFO config layout — RE-2 was right, RE-6 column-order was NOT memory order.**
- End of LoadExteriorInfoTable copies exactly 0x12 (18) dwords from `&local_78` into the map slot (FUN_081ff6e2). ID is the map KEY (`local_2c`), NOT in the struct.
- Column -> struct offset (from each GetInteger out-ptr `piVar5[4]`):
  - ForceID->+0x00, Genre->+0x04, set->+0x08, subtype->+0x0c, represetID->+0x10, colorID->+0x14,
    PermanentFree->+0x18, Permanent0->+0x1c, Permanent1->+0x20, 7daysFree->+0x24, 7days0->+0x28, 7days1->+0x2c,
    LimitpriceFree->+0x30, Limitprice0->+0x34, Limitprice1->+0x38, Limittype->+0x3c, IconID->+0x40, RepresetID1->+0x44.
  - => subtype@0xc / represetID@0x10 / colorID@0x14 CONFIRMED (matches ApplyExteriorRepresent reads). Column-order in RE-6 = tab read order, not memory order.
- nPrice[3][3] @ +0x18. Memory time-order = **Permanent(+0x18) / 7Days(+0x24) / Limit(+0x30)** (NOT "7days first" as PORT_DESIGN §3 note said). Within each group: {Free, Money, Coin}.
- Validation loop `ptr = auStack_58(+0x20) - 4 + i*0xc` = +0x1c/+0x28/+0x34 = nPrice[i][1] -> **epctMoney = index 1** (x10000, assert `nPrice[i][epctMoney] > 0`).

**(B) DB box item 16B — RE-6 was RIGHT, `expire@3` is NOT a typo (item-relative, unaligned).**
- LoadExteriorBox loop: `id = param_2[i*8+1]` (byte2, item-rel +0), `flag = param_2[i*8+2]` (byte4=+2, low byte only via `(char)`), `expire = *(DWORD*)(i*0x10 + 5 + param_2)` (byte5 = item-rel +3, unaligned).
- Item stride `uVar4 << 4` = 16. First assert `sizeof(KEXTERIOR_DB_DATA)`==2 (header WORD count); per-item assert `sizeof(KEXTERIOR_DB_DATA::KEXTERIOR_INFO)`==16.
- => `#pragma pack(1) { WORD id@0; BYTE flag@2; DWORD expire@3; BYTE pad[9] }` = 16B. Requires packing to place expire at +3.

### Enum values pinned (from assert strings + Add/AddExterior semantics)
- EXTERIOR_TIME_TYPE: ettInvalid=-1, ettPermanent=0, ett7Days=1, ettLimit=2, ettTotal=3.
- EXTERIOR_PAY_CURRENCY_TYPE: epctInvalid=-1, epctFree=0, epctMoney=1(PINNED), epctCoin=2, epctTotal=3. (epctFree/epctCoin names inferred; values pinned.)
- Deferred (not needed by slice-1 structs; members not yet pinned): EXTERIOR_BUY_SOURCE (ebsInvalid=0,ebsTotal=4 from `>ebsInvalid && <ebsTotal`, valid {1,2,3}), EXTERIOR_BUY_RESPOND_CODE -> pin in slice #8 (buy chain).
- Deferred: KEXTERIOR_SET_DB_DATA -> slice #6 (SaveExteriorSet not yet decompiled).

### Verify (net 2: binary-oracle layout)
- `scratchpad/exterior_layout_test.cpp`: models 32-bit types (DWORD/time_t=4B), includes real header, static_asserts every size+offset above. `clang++ -std=c++11` => **PASS** (0 errors). = binary-oracle layout confirmation for slice #1.
- Note: real in-tree compile happens from slice #2 on (first .cpp added). A defs-only header is not compiled standalone in the build; the static_assert test is the faithful layout proxy (types modeled to match the 32-bit build's DWORD=4B).

## [PORT-2] Lat #2 — KExterior singleton + 4 LoadTable (build + runtime verify PASS)
**Time:** 2026-07-08 ~11:05 +07

### RE bo sung (subagent + xref, ghi vao day)
- FUNC: Init=FUN_081fbe4c; LoadInfo=FUN_081fb30c; LoadSuit=FUN_081fad0a; LoadIndex=FUN_081fa8e6; LoadShop=FUN_081fa13e.
- Init: ctor 4 map (offs +0x04/+0x1c/+0x34/+0x4c, moi map 0x18B) roi goi 4 LoadTable theo thu tu Info->Suit->Index->Shop, return 1.
- FILE->FUNC (xref filename string): ExteriorInfo.tab->LoadInfo; ExteriorSuitInfo.tab->LoadSuit; **ExteriorBuy.tab->LoadIndex** (reverse-lookup, KHONG phai buy-price); Exteriorshop.tab->LoadShop.
- TABLE Suit (key SuitID, 7 dword=0x1c): +0x00 Chest,+0x04 Helm,+0x08 Waist,+0x0c Boots,+0x10 Bangle,+0x14 AchievementID(def -1, validate 0<=x<4000),+0x18 NeedPermanent(BOOL=col>0).
- TABLE Index (ExteriorBuy.tab): key = {SubType,RepresentID,ColorID,ForceID} (0x10), value = DWORD ID (reverse map appearance->ID).
- TABLE Shop (key ID, 10 dword=0x28): int nPrice[3][3] @+0x00 (time Permanent/7days/Limit, pay Free/Money/Coin, [i][epctMoney]x10000) + nLimitType @+0x24. Assert `ExteriorShopPrice.nPrice[i][epctMoney]>0`.
- SINGLETON: KExterior EMBEDDED in world object (DAT_084f67f8 = g_pSO3World/KSO3World). info-map @ world+0x92624 => KExterior base = world+0x92620. Recompiled => offset compiler-chosen; ported as member `KWorldSettings::m_Exterior` (world->m_Settings.m_Exterior), khop chain access `g_pSO3World->m_Settings.m_Xxx`.

### Artifacts
- NEW: src/SO3World/Src/KExterior.h (class + KEXTERIOR_SUIT_INFO/SHOP_PRICE/INDEX_KEY structs), KExterior.cpp (4 LoadTable + 4 Get accessors, mirror source tab-API idiom g_OpenTabFile/GetHeight/GetInteger/KG_COM_RELEASE).
- MODIFIED: KWorldSettings.h (+#include KExterior.h, + member m_Exterior), KWorldSettings.cpp (Init flag+call after m_DesignationList, UnInit x2). Backup: backup_source_20260708_104341/.

### Verify (2 luoi)
- BUILD (host Linode x86, docker jx3build): `COMPILE ok=190 fail=0`, `link exit=0 undefined refs: 0`, BINARY PRODUCED. KExterior.cpp + KWorldSettings.cpp compile clean (chi warning shim co san).
- RUNTIME (deploy + boot 45s tren data that): dat **"Load game settings ... [OK]"**; 0 dong loi exterior (khong "failed at line"/"already exist"/GetInteger-fail/achievement-assert). => ca 4 bang mo+parse+validate OK tren data that. Neu sai 1 ten cot -> GetInteger-fail -> khong co [OK]. Log-parity: khong sinh dong loi moi (center-connect fail = chua dung cluster, pre-existing, khac domain).
- Note: bang nap im lang (khong log count); thanh cong = dat [OK] voi validation dang bat.

## [PORT-3] Lat #3 — GetEquip{Represent,Color,SubType}Index + KExteriorBox skeleton (compile + oracle PASS)
**Time:** 2026-07-08 ~11:25 +07

### CRUX represent-mechanism GIAI TRON (grep -a fix)
- BAI HOC: grep coi source GBK la "binary" -> moi grep truoc do (khong -a) la AM TINH GIA. Dung `grep -a` xuyen suot.
- `m_wRepresentId[perRepresentCount]` TON TAI @ KPlayer.h:334 (+ m_dwRepresentIdLock @335 = mask player+0x95ec, + LuaGet/SetRepresentID @924/927). Dung design §0.
- v246 setter FUN_082009a2: ghi `*(WORD*)(player+0x95a8 + idx*2)`, check lock-mask (player+0x95ec), idx==2 special (player+0x9a30), sync client (FUN_0807c56e) khi doi. box+0x44=player xac nhan.

### Get*Index pinned verbatim (FUN_081ffc50/b24/bba switch tables)
- Represent: 0->5,1->2,2->8,3->14,4->11 | Color: 0->6,1->3,2->9,3->15,4->12 | SubType: 0->2,1->3,2->6,3->9,4->10.
- Map -> enum PLAYER_EQUIP_REPRESENT (SO3GlobalDef.h): 5 slot = Chest/Helm/Waist/Bangle/Boots (style+color).

### VERSION-DRIFT CHECK PASS (crucial)
- source 2010 enum ordinals: perHelmStyle=2,perHelmColor=3,perChestStyle=5,perChestColor=6,perWaistStyle=8,perWaistColor=9,perBangleStyle=11,perBangleColor=12,perBootsStyle=14,perBootsColor=15 = **GIONG HET v246**. (2010 perRepresentCount=27 vs v246=33, nhung exterior chi cham index<=15 -> an toan; m_wRepresentId[27] chua du.)
- => writing m_wRepresentId[5] = perChestStyle dung. Apply nham slot = KHONG xay ra.

### Artifacts
- NEW: KExteriorBox.h (skeleton: m_ExteriorMap<id,KEXTERIOR_ITEM> + m_pPlayer + Get*Index decls), KExteriorBox.cpp (3 Get*Index + ctor/dtor).
- Compile-time guard (C++98 negative-array _KEXT_REP_MAP_CHECK): khoa index==enum-ordinal; drift -> build fail.

### Verify (2 luoi)
- BUILD: KExteriorBox.cpp compile clean (enum-guard PASS). Full build `COMPILE ok=191 fail=0`, link 0 undefined, BINARY PRODUCED.
- Binary-oracle: Get*Index = transcription verbatim switch v246 + cross-check enum ordinal 2010==v246. (Chua wire vao KPlayer -> chua co runtime behavior; verify = compile+oracle+no-regression.)

## [PORT-4] Lat #4 — KExteriorBox Init + Add/_Add (build + logic-oracle PASS)
**Time:** 2026-07-08 ~11:40 +07
- RE: Init=FUN_08200104 (set m_pPlayer@0x44, counter@0x48=0, ctor 3 container @0x04/0x10/0x38, current-set@0x00=0). _Add=FUN_082016a6, Add=FUN_08201ee6.
- _Add(id, tt, expire): EXISTING & nFlag!=0 -> tt0:{expire=0,flag=0} / tt1:expire+=604800 / tt2:expire=max(cu,param). EXISTING & nFlag==0 (permanent): no-op. NEW: size>0x1fff->err; insert raw {id, nFlag=tt, expire}. Return item*. (LoadExteriorBox dung nhanh NEW -> luu raw flag/expire tu DB.)
- Add(id, tt, expire): id==0 -> no-op TRUE; validate GetExteriorInfo(id) ton tai (g_pSO3World->m_Settings.m_Exterior); _Add; [PORT-7 defer] DoSyncExteriorBoxData 1 item (Add chua co caller den slice #8).
- Artifacts: KExteriorBox.h (+Add/_Add/GetExteriorItem/GetExteriorCount), KExteriorBox.cpp (Init/UnInit/_Add/Add/GetExteriorItem). KExteriorDef.h +EXTERIOR_7DAYS_SECONDS.
- VERIFY: KExteriorBox.cpp compile clean; full build ok=191 link 0 undefined. Logic-oracle scratchpad/add_logic_test.cpp (mirror _Add) => **PASS 6 cases** (new-raw / 7days+=604800 / limit=max / permanent-clears / permanent-sticky / distinct-id).

## [PORT-5] Lat #5 — Apply + represent hook + m_dwApplyExteriorFlag + embed box in KPlayer (build + apply-oracle PASS)
**Time:** 2026-07-08 ~12:10 +07
### RE bo sung (subagent + local)
- Set container: box+0x04 = std::vector<KEXTERIOR_SET_INFO{DWORD dwExteriorID[5]}> stride 0x14; box+0x00 = current-set INDEX (khong phai id). GetExteriorSet(idx)=&vec[idx] neu idx<size.
- IsHaveExteriorRepresent(slot) = current-set.dwExteriorID[slot]!=0 (khong check owned/valid).
- ApplyExteriorRepresent(slot) FUN_08200a4e: id=set[slot]; validate info.nSubType==GetEquipSubType(slot); valid = (item.nFlag==0 || now<item.expire) [now = g_pSO3World->m_nCurrentTime]; neu valid -> rep=info.represetID/color=info.colorID (else 0); neu (m_dwApplyExteriorFlag>>slot&1) -> SetRepresentID(reprIdx,rep)+SetRepresentID(colorIdx,color).
- **RESOLVED "unmapped" player+0x9a30 = KPlayer::m_bHideHat**; player+0x95ec = m_dwRepresentIdLock. Setter FUN_082009a2 == KItemList::SetRepresentID(idx,val): check lock + (idx==perHelmStyle -> !m_bHideHat) + sync-if-changed + write m_wRepresentId[idx].
- Flag bits: bit0-4 per-slot applied, bit0x80 master. LuaApplyExterior: flag|=0x80, loop apply owned slots, SyncFlag(proto 0x121 payload {WORD;DWORD flag} 6B). box @ player+0xb850.
- unApply(FUN_08388252)/GetEquipPos{3,4,8,11,12}/auto-apply-on-set-change: DEFER #8 (equip-pos table chua verify chac; restore-real-item can KItemList refresh).

### Artifacts
- KExteriorBox.h/.cpp: +KEXTERIOR_SET_INFO, m_ExteriorSet vector + m_uCurrentSetID, GetExteriorSet/SetExterior/SetCurrentSetID/IsHaveExteriorRepresent/ApplyExteriorRepresent/ApplyOneSlot/ApplyAllExterior/SetRepresentID. Fields m_wRepresentId/m_dwRepresentIdLock/m_bHideHat/m_ItemList all PUBLIC (region public: @238) -> box ghi thang, dung FUN_082009a2.
- KPlayer.h: +#include KExteriorBox.h, + member m_ExteriorBox + m_dwApplyExteriorFlag. KPlayer.cpp: Init m_ExteriorBox.Init(this) (flag+cleanup x2), m_dwApplyExteriorFlag=0. Backup in backup_source_20260708_104341/.

### Verify (2 luoi)
- BUILD: KExteriorBox.cpp compile clean; full build ok=191 link 0 undefined, binary produced (embed + apply).
- Apply-logic oracle scratchpad/apply_logic_test.cpp (mirror FUN_08200a4e) => **PASS 6 cases** (applybit-off no-write / applybit-on writes rep[5]&[6] / expired->clear / not-expired->write / subtype-mismatch fail-untouched / empty-slot false).
- End-to-end runtime (apply->client appearance) can quan sat: defer #8 (can cluster+login+Lua add/set/apply).

## [PORT-6] Lat #6 — Save/Load persistence (build + roundtrip-oracle PASS)
**Time:** 2026-07-08 ~12:45 +07
### RE format pin (decompile FUN_08201100/08202498/08200d72/08200382/08200ec4/0820224a)
- KExteriorBox::Save (FUN_08201100) v246 = [WORD len1][latest-buy blk][WORD len2][set blk]; owned-items KHONG o day - luu RIENG qua scheme role-block-chunk (FUN_08201476, KRoleBlockHeader co "key" cho chunk 128).
- 2010 KRoleBlockHeader = {int nType; DWORD dwVer; DWORD dwLen} = 12B, **KHONG co key** -> scheme chunk v246 khong map -> GOP owned+set+latest vao 1 role-block (dung model 2010 1-block/subobject; item-format van pin binary).
- Item formats pin: owned 16B {WORD id@0; BYTE flag@2; DWORD expire@3; pad} (FUN_08201fee); set 18B {5 WORD id; 8B pad} + trailer [BYTE curSetId][WORD freeCounter@box+0x48] (FUN_08200ec4/0820224a); latest 8B {WORD id; 6B pad} (FUN_08200d72). -> SET_DB_DATA da pin (giai defer #1/#10).
### Artifacts
- KExteriorDef.h: +KEXTERIOR_SET_DB_DATA (nested KEXTERIOR_SET_INFO 18B). KExteriorBox.h/.cpp: +m_LatestBuy vector +m_uExteriorFreeCount + Save/Load (3 segment length-prefixed, leftover==0). Load chi _Add exterior con ton tai trong config (mirror LoadExteriorBox).
- KRoleDBDataDef.h: +rbtExteriorData (append truoc rbtTotal -> khong shift enum khac). KPlayer.cpp: SAVE_ROLE_BLOCK(m_ExteriorBox.Save,rbtExteriorData,0) sau m_Designation + case rbtExteriorData -> m_ExteriorBox.Load. Backup KRoleDBDataDef.h.
### Verify (2 luoi)
- BUILD: full ok=191 link 0 undefined, binary produced.
- Roundtrip-oracle scratchpad/saveload_test.cpp (mirror byte-format): struct sizes 16/18/8 assert; populated save(93B)->load equal moi field + leftover==0; empty box save(9B)->load OK. => **PASS**.
- Runtime DB roundtrip (save player->relog->load) can login+cluster: defer #8. Edge: box rat lon (8191x16=131KB) co the vuot MAX_EXTERNAL_PACKAGE_SIZE -> Save FALSE (fresh player 0 item -> khong anh huong; note).
### Deferred v246-compat: owned-items chunking (128/block) collapsed to 1 block (2010 header khong co key; fresh DB khong co v246-blob de load).

## [PORT-7] Lat #7 — packet sync (build + item-oracle PASS)
**Time:** 2026-07-08 ~13:15 +07
- v246: box-data protocol 0x11a (WORD-proto, hdr 6B {WORD id;WORD size;WORD count}) + item 7B {WORD id;BYTE flag;DWORD expire}; flag-sync 0x121 {WORD;DWORD}. VERSION-GAP: v246 dung WORD-proto (0x11a=282>255); 2010 dung BYTE byProtocolID -> khong map raw 0x11a. Port = 2010-NATIVE: them 2 s2c enum (s2c_sync_exterior_box_data/s2c_sync_apply_exterior_flag, append truoc gs_client_connection_end -> khong shift) mang **cung payload item 7B pin binary**.
- Struct: S2C_SYNC_EXTERIOR_BOX_DATA : UNDEFINED_SIZE_DOWNWARDS_HEADER {WORD wCount; BYTE byData[0]} (hdr 2010 = byProtocolID(1)+wSize(2)+wCount(2)=5B, item 7B; size=5+count*7). S2C_SYNC_APPLY_EXTERIOR_FLAG : DOWNWARDS_PROTOCOL_HEADER {DWORD dwApplyFlag}. KEXTERIOR_SYNC_ITEM 7B trong KExteriorDef.h.
- KPlayerServer::DoSyncExteriorBoxData(player,count,items) build vao m_byTempData + Send(connIndex) (owner-only); DoSyncApplyExteriorFlag(player,flag). Model = S2C_SYNC_BUFF_LIST var-len idiom.
- Wire: KExteriorBox::Add -> DoSyncExteriorBoxData(1 item); ApplyAllExterior -> DoSyncApplyExteriorFlag. Fire CHI khi co hanh dong exterior (client v246 trigger) -> client 2010 khong bao gio trigger -> khong dung network tier dang chay.
- VERIFY: full build ok=191 link 0. sync_item_test.cpp: KEXTERIOR_SYNC_ITEM static_assert 7B + id@0/flag@2/expire@3 == v246 => PASS. Parity 3 file protocol vs host = 0 truoc khi sua (append-only). Runtime client render: v246/BVTLocal only (2010 client version-gap, non-observable) - dung scope design.

## [PORT-8] Lat #8 — set-creation + un-apply + Lua bindings + grant (build + regression-boot PASS)
**Time:** 2026-07-08 ~13:55 +07
### RE (subagent, addresses verified xref)
- SET CREATION: KHONG co default set. AddExteriorSet=FUN_0820165c push empty {0x5}, KHONG max (script-driven). SetExterior KHONG auto-grow (require setIdx<count). LoadExteriorSet loop AddExteriorSet+write. -> them KExteriorBox::AddExteriorSet + LuaAddExteriorSetCount(n).
- UN-APPLY equip-pos: subagent cho v246 raw {3,4,8,11,12} (switch tren slot, index vao eit v246 - **enum eit DA DRIFT v246 vs 2010**). Port dung mapping 2010-NATIVE: KItemList::GetEquipPos(representIndex) -> {eitChest,eitHelm,eitWaist,eitBoots,eitBangle} = {2,3,7,10,11}, CHINH XAC semantic (giong path khoi phuc cua KItemList::SetRepresentID). KHONG transcribe {3,4,8,11,12} (sai enum). Note: day la ca port 2010-native dung hon transcribe v246 raw offset.
- Lua bindings (KPlayer): LuaApplyExterior/UnApply/IsApply (0 arg), LuaAddExterior (4 arg: id,timeType,payType,buySource -> KExterior::AddExterior grant), LuaAddExteriorSetCount (1 arg), LuaGet/SetCurrentExteriorSetID, LuaGet/SetExteriorSet (setIdx + table 5 id). Arg order pin tu decompile.
- Buy chain (FUN_081f861c Buy...): KEXTERIOR_BUY_PARAM 0xC {id,timeType,payType}, MAX_EXTERIOR_BUY_COUNT=6, EXTERIOR_BUY_SOURCE {ebsInvalid=0..ebsTotal=4}, EXTERIOR_BUY_RESPOND_CODE {1 ok,2 fail,5 force,8 free,12 coin-pending,18 disabled,19 state}. Cost=nPrice[tt][pt]; Pay: cost[0]=box bound-counter(FUN_081ffe12), cost[1]=m_ItemList money(FUN_082e3a9a), cost[2]=coin. Delivery: expire(tt0=0/tt1=now+604800/tt2=nLimitType)->box.Add.
### Artifacts
- KExteriorDef.h: +EXTERIOR_BUY_SOURCE +EXTERIOR_BUY_RESPOND_CODE (giai defer #1/#10). KExteriorBox.h/.cpp: +AddExteriorSet +GetCurrentSetID +UnApplyOneSlot +UnApplyAllExterior; SetCurrentSetID/SetExterior re-drive (giai defer #5). KExterior.h/.cpp: +AddExterior (grant, no money). KPlayer.h/KLuaPlayer.cpp: +9 LuaXxx + REGISTER_LUA_FUNC. Backup KLuaPlayer.cpp.
### VERIFY
- BUILD: full ok=191 link 0 undefined, binary produced.
- REGRESSION BOOT (data that): "Load game settings [OK]"=1, exterior-error=0, crash/assert=0 -> toan bo thay doi KPlayer/enum/protocol/binding KHONG hoi quy startup. Toi center-phase (fail = chua dung cluster, pre-existing).
- End-to-end Lua exercise (grant->set->apply->represent doi): docs/exterior_port/test_exterior.lua (chay tren player logged-in). ENV-GATED: can cluster + client login; client 2010 khong co exterior UI -> render can v246 client (version-gap). Box logic da phu boi oracle #4/#5/#6; bindings = wrapper mong tren box methods da verify.
### DEFERRED (documented, out of reach / lower value):
- Buy chain (client-initiated purchase): coin path (payType2/buySource3) = KDiamond/coin service version-gapped (absent 2010); protocol-coupled buy-request/response; UNREACHABLE without v246 client + coin subsystem. Grant path (LuaAddExterior) covers "give exterior" fully. Money-only Buy portable but unreachable (no 2010 exterior shop UI).
- LuaGetExteriorInfo/SuitInfo/Index config queries (Luna<KExterior>): convenience, not needed for core; scripts hardcode IDs. Latest-buy populate (buy chain).

## [PORT-VERIFY-DWARF] Đối chiếu KExterior port vs DWARF ground-truth (SO3GameServerD)
**Time:** 2026-07-11 · Nguồn: binary DWARF /jx3_dwarf/SO3GameServerD (superset đúng của v246, 3338/3338 hàm khớp). Decompile có tên+kiểu thật.

### KẾT LUẬN: port TRUNG THÀNH — logic khớp CHÍNH XÁC, khác biệt chỉ là TÊN + 1 lựa chọn container tương đương. KHÔNG có lỗi correctness, KHÔNG vỡ tương thích DB/packet.

### LỆCH tên field/struct/method (cosmetic, cùng offset+nghĩa, không ảnh hưởng hành vi compiled)
| Của mình | Game (DWARF thật) |
|---|---|
| KEXTERIOR_INFO.nRepresentID | **dwRepresetID** (game viết thiếu 'n', prefix dw) |
| KEXTERIOR_ITEM.nFlag | **nTimeType** |
| KEXTERIOR_ITEM.nExpireTime | **nEndTime** |
| KPlayer.m_dwApplyExteriorFlag | **m_nApplyExteriorFlag** (int) |
| struct KEXTERIOR_SET_INFO { dwExteriorID[5] } | **KEXTERIOR_SET { dwExteriorSet[5] }** |
| KExteriorBox::GetExteriorItem | **Find** |
| KExteriorBox::GetExteriorSet | **GetExteriorSetInfo** |
(khớp đúng tên: KEXTERIOR_INFO/SUIT_INFO/SHOP_PRICE/INDEX_KEY/ITEM, nSubType, nColorID, GetEquip{Represent,Color,SubType}Index, SetRepresentID, IsHaveExteriorRepresent, GetEquipPos, GetExteriorInfo, m_ExteriorInfoMap, m_uCurrentSetID.)

### LỆCH cấu trúc dữ liệu (implementation, tương đương hành vi)
- **Hộp sở hữu**: game = **std::vector<KEXTERIOR_ITEM> SẮP XẾP + std::lower_bound(ExteriorItemCompare theo dwID)** (KExteriorBox::Find @081ff9ba). Mình = **std::map<DWORD,KEXTERIOR_ITEM>**. → tương đương chức năng (đều tra/duyệt theo id tăng dần). **DB save byte-identical** vì cả hai xuất item theo thứ tự id tăng → khối [WORD count][items] giống hệt. Không phải bug.
- (config maps info/suit/shop/index: game dùng std::map — KHỚP với mình.)

### LỆCH vị trí sở hữu KExterior (architecture, không ảnh hưởng DB/packet vì offset do compiler chọn)
- Game: **g_pSO3World->m_Exterior** (KExterior là member của KSO3World). Mình: g_pSO3World->m_Settings.m_Exterior (member KWorldSettings). Singleton tương đương.

### LOGIC ApplyExteriorRepresent: KHỚP TỪNG BƯỚC với bản mình (validate slot<5 → set[slot] → represent/color/subtype index → GetExteriorInfo → Find item → subtype==match → (nTimeType==0||now<nEndTime) → nếu applyflag bit set: SetRepresentID×2). Chuẩn.

### Ý nghĩa: mọi thứ ẢNH HƯỞNG tương thích (byte DB item, packet, thứ tự lưu, enum ordinal) đều đã pin đúng ở PORT-1..8. Khác biệt còn lại = đặt tên + map-vs-sorted-vector (tương đương). Nếu muốn giống 1:1 thì đổi tên field theo bảng trên + đổi box sang sorted-vector, nhưng KHÔNG bắt buộc cho tính đúng/tương thích.
