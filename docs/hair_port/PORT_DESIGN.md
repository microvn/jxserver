# PORT_DESIGN — KHairShop + KHairBox (发型/hair)

Nguồn RE: `WORKLOG.md` [RE-1..3] + `INTEGRATION.md`. Ground-truth = DWARF `SO3GameServerD`
(layout) + decompile stripped `-3c8199` theo địa chỉ nm (logic). Khuôn = KExterior/KExteriorBox
(đã port, `docs/exterior_port/`). Mục tiêu pilot: **grant-path đầy đủ + verify**, hoãn buy chain.

## §1 LINKED / RELEVANT (dependency + hook)

```
KHairShop (singleton)  --embeds/uses-->  KHairBox (per-player, KPlayer+0xb8a0)
KHairShop.AddHair      --grant-->        KHairBox.Add --> KHairBox.ChangeHair (APPLY)
KHairBox.ChangeHair    --writes-->       KPlayer face/hair short[2] (v246 @0x95a8) + broadcast
KHairShop.Buy/Pay      --needs-->        currency/coin/paysys  [NEW → HOÃN]
Lua                    --Luna<KHairShop>--> KHairShop methods (self=global singleton)
KHairBox.Save/Load     --role-block-->    KPlayer::Save/Load  (rbtHairBoxData, mới)
```

Hook ra file KHÁC KHair* (bảng A INTEGRATION):
- `KPlayer.h` — embed `KHairBox m_HairBox` (offset compiler tự lo).
- role-block enum (định nghĩa: **RE-4 cần định vị** — dùng ở `KPlayer.cpp:2032/2356`) — append `rbtHairBoxData` trước `rbtTotal`.
- `KPlayer.cpp` — `SAVE_ROLE_BLOCK(m_HairBox.Save, rbtHairBoxData, 0)` + `case rbtHairBoxData:` (khuôn rbtExteriorData).
- Lua register — `Luna<KHairShop>` (KHÁC KExterior; KHÔNG vào KLuaPlayer method table). Định vị nơi register `Luna<>` cho global object.
- apply-hook — face/hair field 2010: **RE-4 cần định vị** (KHÔNG transcribe offset v246 0x95a8).

## §2 Data structures (pinned từ DWARF — PHẢI khớp)

Config (KHairShop.h): KHAIR_INDEX_INFO 0xc {dwHeadformID@0,dwBangID@4,dwPlaitID@8};
KHAIR_PRICE_INFO 0xc {nPrice@0,nCoin@4,nNeedFreeCount@8}; KHAIR_PRICE_KEY 0xc {nRoleType@0,nType@4,dwID@8};
KHAIRSHOP_BUY_PARAM 0x10 {dwID[2]@0,bUseFreeCount[2]@8}; KHAIRSHOP_COIN_BUY_PARAM 0x18 {nPrice@0,nFreeCount@4,dwID[2]@8,bUseFreeCount[2]@0x10}.

Class KHairShop 0x34: m_bCloseHairShop@0, m_HairIndexInfoMap@0x4 (map HeadID→INDEX_INFO),
m_HairPriceInfoMap@0x1c (map PRICE_KEY→PRICE_INFO).
Class KHairBox 0x20: m_nFreeCount@0(int), m_HairList[2]@0x4 (vector<DWORD>[2], face=0/hair=1), m_pPlayer@0x1c.
In-mem item = DWORD id thuần, sorted+unique, cap **1024**.

## §3 Persistence (DB byte-layout — PHẢI byte-exact)

`KHAIR_DB_DATA::KHAIR_INFO` = **8B** {WORD wID@0; BYTE reserved[6]@2} (memset 0).
```
list0: [WORD blockSize0][WORD wCount0][ wCount0 × 8B ]
list1: [WORD blockSize1][WORD wCount1][ wCount1 × 8B ]      (luôn ghi cả 2 dù rỗng)
trailer: [WORD freeCount]                                   (int in-mem → WORD on-disk)
blockSizeN = 2 + wCountN*8
```
Load assert leftover==2 ở cuối. `#pragma pack(1)`. 2010 KRoleBlockHeader không chunk key → 1 block.

## §4 Drift phải khoá (bảng D — negative-array guard `typedef char _CHK[(COND)?1:-1]`)

- **wID = WORD** → hair id ≤ 65535 (in-mem DWORD, chỉ 16-bit thấp vào DB). Guard giá trị khi Add.
- KHAIR_INFO stride == 8; sizeof static-assert.
- freeCount WORD on-disk; cap 60000 (< 65535) — an toàn.
- face/hair field offset + role-block enum ordinal: **cross-check 2010, không dùng số v246**.

## §5 PORT ORDER (bé→to, mỗi slice có verify)

- [x] **S1. enums+structs + config load** — tạo `KHairShopDef.h/KHairShop.h/KHairShop.cpp`. ✅ DONE: oracle-mirror layout PASS; **build host ok=192 fail=0, link 0 undefined**. Bảng data khớp cột 100% (Price=0/Coin=0). Chưa wire Init vào boot (làm ở S5 cùng embed).
- [x] **S2. KHairBox skeleton + _Add/Add/Find/GetHairList/FreeCount** — tạo `KHairBox.h/.cpp`. **Verify: oracle-mirror logic** (dedupe+sort+cap 1024; freecount saturating 60000).
- [x] **S3. Save/Load role-block** — `KHAIR_INFO` 8B pack(1) + Save/Load/SaveHairList/LoadHairList + append `rbtHairBoxData` + wire KPlayer::Save/Load. **Verify: oracle-mirror roundtrip byte-exact + leftover==2.** (cần RE-4 enum def location)
- [x] **S4. apply-hook ChangeHair + embed KHairBox trong KPlayer** — ghi face/hair field 2010 + broadcast. **Verify: cross-check field 2010 (RE-4) + logic oracle + regression boot.** RỦI RO CAO — làm sau khi RE-4 xong.
- [x] **S5. AddHair grant + ChangePlayerHair + Lua bindings (Luna<KHairShop>) + register** — grant path đầy đủ. **Verify: regression boot + test_hair.lua.**
- [x] **S6. coverage-diff (step-8 bắt buộc)** — `comm -13` ported-names vs binary surface; gap == defer list.

## §6 Defer (documented — không port lần này)
Buy/Pay/CanBuy/IsMoneyEnough/GetCost/GetTotalCost/OnHairShopBuyHairRespond/LuaBuyHair
= buy chain depend currency/coin/paysys (NEW). Grant path (AddHair/LuaChangePlayerHair) thay thế cho pilot.

## §7 Remaining RE — ✅ RE-4 RESOLVED (2026-07-12)
1. **enum role-block**: `ROLE_DATA_BLOCK_TYPE` @ `include/Include/KRoleDBDataDef.h` (KHÔNG phải bản Test/Robot). `rbtExteriorData` dòng 40, `rbtTotal` dòng 42 → append `rbtHairBoxData` GIỮA (khe KExterior đã dùng).
2. **apply-hook — TÁI DÙNG 2010, không transcribe offset v246.** v246 short[2]@KPlayer+0x95a8 = `KPlayer::m_wRepresentId[]` (WORD[], `KPlayer.h:338`). Index qua enum `PLAYER_EQUIP_REPRESENT` (`SO3GlobalDef.h:116`): `perFaceStyle=0`, `perHairStyle=1`. Setter = `KItemList::SetRepresentID(int,int)` (`KItemList.cpp:3361`); broadcast tự động `KPlayerServer::DoSyncEquipRepresent`. → ChangeHair = `SetRepresentID(perFaceStyle,face)` / `SetRepresentID(perHairStyle,hair)`. Drift-lock GỠ.
3. **Luna register (self=singleton, khuôn KMentorCache)** — 3 điểm: (a) `Luna<KHairShop>::Register(pLuaState)` trong `#ifdef _SERVER` `KScriptCenter.cpp:774`; (b) free-func `LuaGetHairShop` → `g_pSO3World->m_HairShop.LuaGetObj(L)` theo khuôn `KBaseFuncList.cpp:4170`; (c) đăng ký `{"GetHairShop", LuaGetHairShop}` `KBaseFuncList.cpp:5033`. Script gọi `GetHairShop()`.
4. **Đặt singleton**: `m_HairShop` = member trực tiếp của `g_pSO3World` (như KMentorCache; v246 dùng g_HairShop global đơn), KHÔNG trong m_Settings như KExterior.
