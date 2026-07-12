# PORT_DESIGN — KMiniAvatar + KMiniAvatarSettings (小玩伴 acquire)

Nguồn RE: `WORKLOG.md` [RE-1..4] + `INTEGRATION.md`. Ground-truth = DWARF `SO3GameServerD`
(layout+sig) + decompile `-3c8199` theo địa chỉ (logic). Khuôn = KHairShop/KHairBox
(`docs/hair_port/`). NEW subsystem. **Đơn giản hơn hair**: 1 set thay vì 2 vector, KHÔNG buy chain,
KHÔNG apply-representID (chỉ track + worn-id).

## §1 LINKED (dependency + hook)

```
KMiniAvatarSettings (config singleton)  --GetMiniAvatarInfo/AcquireAllFree-->  KMiniAvatar (per-player box)
KMiniAvatar.Acquire  --CanAcquire(force gate)-->  set.insert  --> notify (DEFER packet)
KMiniAvatar (box)    --Luna<KMiniAvatar>-->  4 Lua methods (self = box)
KPlayer.LuaGetMiniAvatarMgr  --push box userdata-->  Lua script
KPlayer.LuaSetMiniAvatar  --guard box.IsAcquired-->  m_dwMiniAvatarID (worn) + broadcast (DEFER)
KMiniAvatar.Save/Load  --role-block rbtMiniAvatarData-->  KPlayer::Save/Load
KMiniAvatarSettings.Init  --KWorldSettings::Init-->  MiniAvatarInfo.tab
```

Hook ra file KHÁC KMiniAvatar* (bảng A INTEGRATION):
- `KWorldSettings.{h,cpp}` — member `m_MiniAvatarSettings` + Init/UnInit.
- `KPlayer.h` — `KMiniAvatar m_MiniAvatar;` + `DWORD m_dwMiniAvatarID;` (offset compiler tự lo).
- `KPlayer` init — `m_MiniAvatar.Init(this)`.
- `KRoleDBDataDef.h` — `rbtMiniAvatarData` trước `rbtTotal`.
- `KPlayer.cpp` — `SAVE_ROLE_BLOCK` + `case rbtMiniAvatarData:` (khuôn rbtHairBoxData).
- `KScriptCenter.cpp` — `Luna<KMiniAvatar>::Register` (`#ifdef _SERVER`).
- `KLuaPlayer.cpp` — `REGISTER_LUA_FUNC(KPlayer, GetMiniAvatarMgr/SetMiniAvatar)`.
- `KLuaMiniAvatar.cpp` (mới) — `DEFINE_LUA_CLASS` + 4 method.

## §2 Data structures (pinned từ DWARF — PHẢI khớp)

```
KMiniAvatarInfo  0x0c : DWORD dwID@0; DWORD dwForceID@4; BOOL bFreeAcquire@8;   (KMiniAvatarSettings.h)
KMiniAvatarSettings 0x18 : std::map<DWORD,KMiniAvatarInfo> m_MiniAvatarInfoMap@0;
KMiniAvatar 0x1c : std::set<DWORD> m_MiniAvatarSet@0; KPlayer* m_pPlayer@0x18;   (KMiniAvatar.h)
   + static char szClassName[]="KMiniAvatar"; static Luna<KMiniAvatar>::KLuaData LuaInterface[];
```
In-mem: acquired id = DWORD thuần trong set (sorted-unique). Worn id = `KPlayer::m_dwMiniAvatarID`
(DWORD). Config map key = dwID.

## §3 Persistence (DB byte-layout — PHẢI byte-exact)  [Save@08216712 / Load@08216a1c]

```
[WORD wWornID]              = (short) player->m_dwMiniAvatarID
[WORD wCount]              = m_MiniAvatarSet.size()
[WORD wID] × wCount        = (short) từng element (sorted asc)
```
- KHÔNG có blockSize prefix (role-block header ngoài bọc size).
- Load: đọc worn → count → require leftover≥count*2 → insert từng WORD → **assert leftover==0**.
- `#pragma pack(1)`; mọi field WORD. 2010 KRoleBlockHeader không chunk-key → 1 block.

## §4 Drift phải khoá (negative-array guard `typedef char _CHK[(COND)?1:-1]`)

- acquired id ≤ 0xFFFF (WORD on-disk) — guard giá trị khi `Acquire` (giống hair wID guard).
- worn id ≤ 0xFFFF — guard khi `SetMiniAvatar`.
- `sizeof(KMiniAvatarInfo)==12` + offset dwForceID==4, bFreeAcquire==8.
- box embed + worn-id offset + player force(0x88)/roleType(0xcfc): **cross-check accessor 2010, KHÔNG dùng số v246**.
- role-block enum ordinal: append, self-consistent.

## §5 PORT ORDER (bé→to, mỗi slice có verify)

- [x] **S1. enums+struct + config load** — tạo `KMiniAvatarSettings.h/.cpp` (KMiniAvatarInfo + map +
  Init/UnInit/LoadMiniAvatarInfo/LoadMiniAvatarInfoLine/GetMiniAvatarInfo + AcquireAllFreeMiniAvatar).
  Cột tab: ID/ForceID/FreeAcquire. **Verify: oracle-mirror layout (static_assert 12/0x18) + build host.**
- [x] **S2. KMiniAvatar box skeleton + Init/IsAcquired/CanAcquire/Acquire** — `KMiniAvatar.h/.cpp`.
  **Verify: oracle-mirror logic** (set dedupe, CanAcquire force-gate, Acquire lazy-free + insert).
- [x] **S3. Save/Load role-block** — pack(1) byte layout §3 + append `rbtMiniAvatarData` + wire
  KPlayer::Save/Load. **Verify: oracle-mirror roundtrip byte-exact + leftover==0.**
- [x] **S4. embed KMiniAvatar + m_dwMiniAvatarID trong KPlayer + Init(this)** + getMiniAvatarID +
  wear guard (SetMiniAvatar field-write, broadcast STUB). **Verify: regression boot.**
- [x] **S5. Lua — DECLARE/DEFINE Luna<KMiniAvatar> 4 method + Register @KScriptCenter + KPlayer
  GetMiniAvatarMgr/SetMiniAvatar @KLuaPlayer.** **Verify: regression boot + test_miniavatar.lua**
  (`player:GetMiniAvatarMgr():AcquireMiniAvatar(id)` → IsMiniAvatarAcquired true).
- [x] **S6. coverage-diff (step-8 bắt buộc)** — `comm -13` ported-names vs binary surface; gap == defer.

## §6 Defer (documented)
- `GetSyncMiniAvatar` + broadcasters (`FUN_08058d18`/`FUN_080d557e`/`DoSyncTargetMiniAvatarMisc`
  /`DoSyncTargetTargetMiniAvatarMisc`) = client-facing packet sync (v246-only render) → stub.
- KHÔNG có buy chain (khác hair — không có money defer).

## §7 Remaining RE (nhỏ, làm khi port)
1. Xác nhận cột tab thứ tự đúng (ID/ForceID/FreeAcquire) khớp file `MiniAvatarInfo.tab` thực (nếu có
   trong data 手工端); nếu thiếu file → tạo tab tối thiểu để boot.
2. Xác nhận player+0x88 = force id trong layout KPlayer 2010 (accessor GetForceID / m_nForce).
   AcquireAllFreeMiniAvatar + CanAcquire phụ thuộc.
3. Xác nhận điểm gọi `m_MiniAvatar.Init(this)` + `SAVE_ROLE_BLOCK` trong KPlayer 2010 (khuôn
   rbtHairBoxData đã có sẵn — copy 1:1).
