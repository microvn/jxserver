# WORKLOG — KMiniAvatar + KMiniAvatarSettings (小玩伴/迷你形象 acquire subsystem)

RE READ-ONLY. Ground-truth: DWARF `jx3_dwarf/SO3GameServerD` (layout+signatures) + decompile
stripped release `SO3GameServer-3c8199` by address (matches DWARF nm 1:1). NEW subsystem
(grep `KMiniAvatar` in 2010 `src`/`include` = 0). Khuôn = KHairShop/KHairBox (`docs/hair_port/`).

小玩伴 = một "pet/companion hình thu nhỏ" cosmetic. Hệ này = **acquire-set** (sưu tập các mini
avatar mở khoá được) + **worn id** (cái đang đeo). KHÔNG có buy chain (khác hair) — chỉ grant +
free-acquire theo môn phái.

---

## [RE-1] Classify + layout (DWARF)

- **NEW subsystem** — `LC_ALL=C grep -rIna 'MiniAvatar' src include` = 0 dòng.
- decl files (DWARF `DW_AT_decl_file`):
  - `Source/Common/SO3World/Src/KMiniAvatar.h` (class KMiniAvatar, line 14; ctor 16, Init 19, szClassName/LuaInterface 55)
  - `Source/Common/SO3World/Src/KMiniAvatarSettings.h` (KMiniAvatarSettings line 17; KMiniAvatarInfo line 10)

### KMiniAvatarInfo (config struct) — byte_size 0x0c = 12B  [DWARF 0x000a43eb]
| field | off | type | evidence |
|---|---|---|---|
| dwID | 0x0 | DWORD | DW_OP_plus_uconst 0x0 |
| dwForceID | 0x4 | DWORD | 0x4 — môn phái gate (0 = free for all) |
| bFreeAcquire | 0x8 | BOOL (4B) | 0x8 — auto-grant khi vào game nếu force khớp |

### KMiniAvatarSettings (config singleton) — byte_size 0x18 = 24B  [DWARF 0x000a5784]
| member | off | type |
|---|---|---|
| m_MiniAvatarInfoMap | 0x0 | `std::map<DWORD, KMiniAvatarInfo, less, KAllocator>` (0x18) — key=dwID [DWARF 0x000a52bd] |

### KMiniAvatar (per-player box) — byte_size 0x1c = 28B  [DWARF 0x00f5aaa8]
| member | off | type | evidence |
|---|---|---|---|
| m_MiniAvatarSet | 0x0 | `std::set<DWORD, less, KAllocator>` (0x18) — acquired ids, sorted-unique [DWARF 0x00f4db9f, spec 0x00ea51b5] |
| m_pPlayer | 0x18 | KPlayer* | DW_OP_plus_uconst 0x18 |
| (static) szClassName | — | char[] = "KMiniAvatar" | `_ZN11KMiniAvatar11szClassNameE` |
| (static) LuaInterface | — | `Luna<KMiniAvatar>::KLuaData[]` | `_ZN11KMiniAvatar12LuaInterfaceE` @084c8660 |

→ **Luna<KMiniAvatar>** object binding (per-player object), khác hair (hair = Luna singleton).

### KPlayer adjacent fields (v246 offsets — compiler-free trong port, KHÔNG hardcode)
- **KPlayer+0xb934** = KMiniAvatar box embed (thấy ở `LuaGetMiniAvatarMgr` push `player+0xb934`,
  và `AcquireAllFreeMiniAvatar` insert vào set @ `player+0xb934`).
- **KPlayer+0xb774** = `DWORD m_dwMiniAvatarID` = mini avatar **đang đeo** (worn/displayed).
  `getMiniAvatarID()` trả nó; `LuaSetMiniAvatar` ghi nó (guard IsAcquired); Save/Load persist nó.
- **KPlayer+0x88** = force/faction id (CanAcquire so với dwForceID). **KPlayer+0xcfc** = roleType
  (broadcast arg). → dùng accessor 2010, không transcribe offset.

---

## [RE-2] Function surface + addresses (nm DWARF, c++filt -n)

KMiniAvatar (12 real + ctor/dtor):
```
082163f2 Init(KPlayer*)              08216bc2 UnInit()
08216712 Save(size_t*,BYTE*,size_t)  08216a1c Load(BYTE*,size_t,DWORD)   [3rd arg = role guid, unused-ish]
08216c90 Acquire(DWORD)             08216bfe CanAcquire(DWORD)
08216354 IsAcquired(DWORD)          082168de GetSyncMiniAvatar(DWORD*,WORD*,int)
08216dee LuaAcquireMiniAvatar       082165c6 LuaGetAllMiniAvatar
0821652c LuaGetMiniAvatarCount      08216448 LuaIsMiniAvatarAcquired
08340ff8 LuaGetObj(lua_State*)      [Luna push helper]
```
KMiniAvatarSettings (6 real + ctor/dtor):
```
08217622 Init()                     0821768c UnInit()
08217324 LoadMiniAvatarInfo(char*)  08217200 LoadMiniAvatarInfoLine(ITabFile*,int,const KMiniAvatarInfo&,KMiniAvatarInfo*)
08217194 GetMiniAvatarInfo(DWORD)   082176c8 AcquireAllFreeMiniAvatar(KPlayer*)
```
KPlayer-side (wear+expose, KLuaPlayer.cpp / KPlayer.h — part of feature):
```
08313026 KPlayer::LuaGetMiniAvatarMgr(lua_State*)   [push box @player+0xb934 as Luna userdata]
0831848c KPlayer::LuaSetMiniAvatar(lua_State*)       [wear: guard IsAcquired → set m_dwMiniAvatarID + broadcast]
0833d0e6 KPlayer::getMiniAvatarID()                  [return player+0xb774]
```
KPlayerServer packet (client-facing, version-gapped → DEFER):
```
08081e14 DoSyncTargetMiniAvatarMisc(KPlayer*)   0807f40e DoSyncTargetTargetMiniAvatarMisc(KPlayer*)
FUN_08058d18(&DAT_084e1180,...)  = notify-acquire broadcaster (in Acquire)
FUN_080d557e(&DAT_084ef4a0,...)  = worn-change broadcaster (in LuaSetMiniAvatar)
```

---

## [RE-3] Logic (decompile by address on -3c8199)

### KMiniAvatarSettings::LoadMiniAvatarInfoLine @08217200  — column read-order
3 cột đọc qua `ITabFile` vtbl+0x28 (GetInteger, có default):
- `"ID"` → out+0 (dwID)     `"ForceID"` → out+4 (dwForceID)     `"FreeAcquire"` → out+8 (bFreeAcquire)

### KMiniAvatarSettings::LoadMiniAvatarInfo @08217324
`g_OpenTabFile` → rows = GetHeight (vtbl+0x18); if rows<2 fail. memset default 12B. Loop row 3..rows:
đọc 1 dòng → assert `dwID != 0` → `map[dwID] = info` (assert InsRet.second, không trùng key). 
Init @08217622 gọi `LoadMiniAvatarInfo("settings/MiniAvatar/MiniAvatarInfo.tab")`.

### KMiniAvatarSettings::GetMiniAvatarInfo(DWORD id) @08217194
`map.find(id)`; end→NULL; else trả `&(iter->second)` (= node+4, bỏ key). Dùng bởi CanAcquire.

### KMiniAvatarSettings::AcquireAllFreeMiniAvatar(KPlayer*) @082176c8
Duyệt cả map; với mỗi info: `if (bFreeAcquire != 0 && (dwForceID==0 || dwForceID==player+0x88))`
→ `player-box(@0xb934).set.insert(dwID)`. (grant free avatars khớp môn phái, không dedup lỗi.)

### KMiniAvatar::Init(KPlayer*) @082163f2
assert pPlayer; `m_pPlayer = pPlayer` (this+0x18). Không clear set (ctor lo).

### KMiniAvatar::IsAcquired(DWORD id) @08216354
`m_MiniAvatarSet.find(id) != end()`. (set::find @0821712c, end @08109d30, iter!= @08109820)

### KMiniAvatar::CanAcquire(DWORD id) @08216bfe
`info = Settings.GetMiniAvatarInfo(id)`; null→fail(log). else
`return (info->dwForceID==0) || (player->force(@0x88)==info->dwForceID)`.

### KMiniAvatar::Acquire(DWORD id) @08216c90
- assert id != 0.
- if set EMPTY → `Settings.AcquireAllFreeMiniAvatar(player)` (lazy free-grant on first touch).
- if `!IsAcquired(id)`: if `CanAcquire(id)`: `InsRet = set.insert(id)`; if `.second` →
  broadcast `FUN_08058d18(&DAT_084e1180, player, id)` (notify-acquire) + return 1; else log "InsRet.second".
- (đã sở hữu → trả 0, không lỗi.)

### KMiniAvatar::GetSyncMiniAvatar(DWORD* pdwCursor, WORD* pwSyncData, int nMax) @082168de
Incremental sync của **acquired-set** (KHÔNG phải worn id): assert 2 ptr; if set empty →
AcquireAllFreeMiniAvatar; iterate set từ bookmark (`upper_bound(*pdwCursor)` @08216fba) → ghi mỗi
element WORD vào pwSyncData[i], tối đa nMax; `*pdwCursor = 0; if count>0: *pdwCursor = last id sent`.
Client-facing → **DEFER** (packet version-gapped).

### KMiniAvatar::Save(size_t* puUsedSize, BYTE* buf, size_t size) @08216712  — DB byte layout
asserts puUsedSize/pbyBuffer. Ghi tuần tự (mọi field WORD, `uLeftSize >= sizeof(WORD)` mỗi bước):
```
[WORD wWornID]  = (short) m_pPlayer->m_dwMiniAvatarID (@player+0xb774)   // line 0x52
[WORD wCount]   = set.size()                                             // line 0x57
[WORD wID] × wCount = (short) each set element                          // line 0x5e
```
`*puUsedSize = size - leftover`. **KHÔNG có blockSize prefix** (khác hair) — role-block header
ngoài đã bọc size.

### KMiniAvatar::Load(BYTE* data, size_t len, DWORD roleGuid) @08216a1c  — mirror Save
```
m_pPlayer->m_dwMiniAvatarID = (uint) firstWORD          // worn id
count = nextWORD
require leftover >= count*2
for i in count: id = WORD; set.insert(id) (via FUN_0809095a)
assert leftover == 0   // line 0x42 "uLeftSize == 0"
```

### Lua methods (Luna<KMiniAvatar> — self = per-player box)
- `LuaAcquireMiniAvatar` @08216dee: top==1; id=tonumber(1); `Acquire(id)`; pushboolean(ok).
- `LuaIsMiniAvatarAcquired` @08216448: top==1; `IsAcquired(id)`; pushboolean.
- `LuaGetMiniAvatarCount` @0821652c: top==0; pushnumber(set.size()).
- `LuaGetAllMiniAvatar` @082165c6: top==0; `lua_createtable`; iterate set → `t[i++]=id`; trả table.
- `LuaGetObj` @08340ff8: `ud=lua_newuserdata(L,4); *ud=this; setmetatable "KMiniAvatar"` → kiểu (a).

### Script-facing Luna method names (đọc LuaInterface @084c8660 → name strings @0840f5b0)
`"GetMiniAvatarCount"`, `"GetAllMiniAvatar"`, `"IsMiniAvatarAcquired"`, `"AcquireMiniAvatar"`.

### KPlayer::LuaGetMiniAvatarMgr @08313026  (expose box to Lua)
top==0 → `m_MiniAvatar(@0xb934).LuaGetObj(L)` → push userdata. Script: `player:GetMiniAvatarMgr()`.

### KPlayer::LuaSetMiniAvatar @0831848c  (WEAR / apply)
top==1; newID=tonumber. if `player->m_dwMiniAvatarID(@0xb774) != newID`:
  if `newID != 0 && !box.IsAcquired(newID)` → return 0 (reject: chưa sở hữu).
  `m_dwMiniAvatarID = newID`; broadcast `FUN_080d557e(&DAT_084ef4a0, player->id(@+4), newID, roleType(@+0xcfc))`.

### KPlayer::getMiniAvatarID @0833d0e6 → `return m_dwMiniAvatarID (@0xb774)`.

---

## [RE-4] Cross-check 2010 (binding sites)

- **role-block enum**: `ROLE_DATA_BLOCK_TYPE` @ `include/Include/KRoleDBDataDef.h`. Last trước
  `rbtTotal`(:43) = `rbtHairBoxData`(:41). → **append `rbtMiniAvatarData` giữa :41 và :43**.
  Không có struct DB MiniAvatar sẵn trong include (NEW).
- **Settings singleton placement**: v246 truy `DAT_084f67f8 + 0x926b0` (member của g_pSO3World /
  world-settings block). Port: thêm `KMiniAvatarSettings m_MiniAvatarSettings` vào `KWorldSettings`
  cạnh `m_HairShop`/`m_Exterior` + gọi `.Init()` trong `KWorldSettings::Init` (khuôn hair). Config
  path = `settings/MiniAvatar/MiniAvatarInfo.tab`.
- **Luna register**: `Luna<KMiniAvatar>::Register` @0811c792 gọi từ mass-register `FUN_081134ec`
  @08113789 = `KScriptCenter` register-all. Port: thêm `Luna<KMiniAvatar>::Register(pLuaState);`
  vào `#ifdef _SERVER` ở `KScriptCenter.cpp` (cạnh Luna<KHairShop> etc.).
- **KPlayer Lua methods**: `LuaGetMiniAvatarMgr` + `LuaSetMiniAvatar` decl `KLuaPlayer.cpp`
  (getMiniAvatarID = accessor `KPlayer.h:1532`). Port: `REGISTER_LUA_FUNC(KPlayer, GetMiniAvatarMgr)`
  + `REGISTER_LUA_FUNC(KPlayer, SetMiniAvatar)` @ `KLuaPlayer.cpp`.
- **apply/wear field**: v246 KPlayer+0xb774. Port = member `DWORD m_dwMiniAvatarID` trên KPlayer
  (compiler layout free). Broadcast (FUN_080d557e / DoSyncTargetMiniAvatarMisc) = packet → DEFER
  (client v246 only). Field-write + IsAcquired guard = phần port được.
- **force field** player+0x88: CanAcquire so sánh với dwForceID. Cross-check accessor faction 2010
  (KCharacter force id) trước khi port guard. player+0xcfc = roleType (đã biết từ hair).

## [PORT-1..6] DONE — code+verify (batch-1, cùng khuôn hair)
- Files: KMiniAvatarSettings.h/.cpp (config), KMiniAvatar.h/.cpp (box set<DWORD>+Save/Load+Lua),
  KLuaMiniAvatar.cpp (Luna 4 method). Wire 7 điểm (KWorldSettings, KRoleDBDataDef rbtMiniAvatarData,
  KPlayer embed+m_dwMiniAvatarID+Init+save/load, KScriptCenter Luna register, KLuaPlayer
  GetMiniAvatarMgr/SetMiniAvatar).
- VERIFY: oracle layout+roundtrip PASS; build host **ok=197 fail=0 link 0**; boot **settings-OK**.
- COVERAGE: impl 17 / binary 22 → GAP 6 = GetSyncMiniAvatar (defer packet) + 4 ctor/dtor implicit +
  LoadMiniAvatarInfoLine (folded vào LoadMiniAvatarInfo). Forgotten=0.
