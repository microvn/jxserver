# INTEGRATION MAP — KMiniAvatar + KMiniAvatarSettings (caller / callee / binding)

Điền theo ARCHITECTURE.md §8 (17 điểm). Mọi dòng kèm bằng chứng (addr v246 hoặc file:line 2010).
Khuôn gần nhất = `docs/hair_port/INTEGRATION.md` (KHÁC 1 điểm: Lua binding = per-player Luna object
lộ qua KPlayer accessor, KHÔNG phải Luna singleton như hair).

## A. Binding points (PHẢI sửa file ngoài KMiniAvatar*)

| §8 | loại | v2.5 (binary) | 2010 (file:line sẽ sửa) | trạng thái |
|---|---|---|---|---|
| 1 | Config Init | Settings @ `DAT_084f67f8+0x926b0`; Init@08217622 → `LoadMiniAvatarInfo("settings/MiniAvatar/MiniAvatarInfo.tab")` | thêm member `KMiniAvatarSettings m_MiniAvatarSettings` vào `KWorldSettings` + `m_MiniAvatarSettings.Init()` trong `KWorldSettings::Init` (`KWorldSettings.cpp:~117`, cạnh `m_HairShop`) + UnInit đối xứng | CHỐT (RE-4) |
| 2 | KPlayer embed (box) | KMiniAvatar box @ KPlayer+0xb934; box+0x18 = m_pPlayer | `KPlayer.h` — `KMiniAvatar m_MiniAvatar;` (compiler layout, KHÔNG hardcode 0xb934) | CHỐT |
| 3 | Init(this) | Init@082163f2 set m_pPlayer | `m_MiniAvatar.Init(this)` cạnh `m_HairBox.Init(this)` trong KPlayer init | CHỐT |
| 2b | KPlayer worn-id field | KPlayer+0xb774 = `DWORD m_dwMiniAvatarID` (worn) | `KPlayer.h` — `DWORD m_dwMiniAvatarID;` (compiler layout) | CHỐT (RE-3) |
| 4 | role-block enum | `rbtMiniAvatarData` (NEW) | `KRoleDBDataDef.h` append GIỮA `rbtHairBoxData`(:41) và `rbtTotal`(:43) | CHỐT (RE-4) |
| 5 | Save | Save@08216712 `(size_t*,BYTE*,size_t)` | `SAVE_ROLE_BLOCK(m_MiniAvatar.Save, rbtMiniAvatarData, 0)` @ `KPlayer::Save` (`KPlayer.cpp:2364`) | CHỐT |
| 6 | Load | Load@08216a1c `(BYTE*,size_t,DWORD)` | `case rbtMiniAvatarData:` trong load-switch (`KPlayer.cpp:2040`) → `m_MiniAvatar.Load` | CHỐT |
| 8/9 | Lua register (box) | `Luna<KMiniAvatar>::Register`@0811c792 gọi từ mass-register `FUN_081134ec`@08113789; metatable "KMiniAvatar" | `Luna<KMiniAvatar>::Register(pLuaState);` trong `#ifdef _SERVER` @ `KScriptCenter.cpp` (cạnh `Luna<KHairShop>`); `.h`: `DECLARE_LUA_CLASS(KMiniAvatar)` (`#ifdef _SERVER`+`#include "Luna.h"`) + `DEFINE_LUA_CLASS_BEGIN/END` trong `KLuaMiniAvatar.cpp` với 4 `REGISTER_LUA_FUNC` | CHỐT (RE-4) |
| 10 | KPlayer Lua methods | `LuaGetMiniAvatarMgr`@08313026, `LuaSetMiniAvatar`@0831848c (decl `KLuaPlayer.cpp`) | `REGISTER_LUA_FUNC(KPlayer, GetMiniAvatarMgr)` + `REGISTER_LUA_FUNC(KPlayer, SetMiniAvatar)` @ `KLuaPlayer.cpp` | CHỐT (RE-4) |
| 12 | packet sync (client) | notify-acquire `FUN_08058d18(&DAT_084e1180,...)`; worn-change `FUN_080d557e(&DAT_084ef4a0,...)`; `DoSyncTargetMiniAvatarMisc`@08081e14 | DEFER — client v246-only; stub broadcaster | HOÃN (bảng E) |
| 15 | apply/wear hook | LuaSetMiniAvatar ghi `m_dwMiniAvatarID` sau guard `IsAcquired` | field write + guard port được; broadcast defer | CHỐT (field), HOÃN (broadcast) |

## B. Callee (KMiniAvatar* GỌI RA)

| hàm | gọi | thuộc hệ | port-first / tái dùng |
|---|---|---|---|
| Settings::Init@08217622 | LoadMiniAvatarInfo@08217324 | self | port cùng |
| Load*Info/Line | `g_OpenTabFile` + ITabFile vtbl (+0x18 GetHeight, +0x28 GetInteger) | KTabFile (có 2010) | tái dùng KTabFile 2010 (khuôn hair LoadHairIndexTable) |
| GetMiniAvatarInfo@08217194 | std::map::find (this+0) | self | port thẳng |
| AcquireAllFreeMiniAvatar@082176c8 | map iterate + set.insert vào player-box; đọc player+0x88 (force) | self + KPlayer force | port cùng; force = accessor 2010 |
| Acquire@08216c90 | AcquireAllFreeMiniAvatar, IsAcquired, CanAcquire, set.insert, notify `FUN_08058d18` | self + Settings + packet | port; notify = stub/defer |
| CanAcquire@08216bfe | Settings.GetMiniAvatarInfo; đọc player+0x88 | Settings + KPlayer force | port cùng |
| IsAcquired@08216354 | set.find | self | port thẳng |
| Save/Load | m_pPlayer->m_dwMiniAvatarID (@0xb774) + set iterate | self + KPlayer field | port cùng (field = member port) |
| GetSyncMiniAvatar@082168de | AcquireAllFreeMiniAvatar, set upper_bound | self + packet | DEFER (client sync) |
| KPlayer::LuaSetMiniAvatar | box.IsAcquired (guard); set m_dwMiniAvatarID; broadcast `FUN_080d557e` | self + packet | port field+guard; broadcast defer |

## C. Caller (ai GỌI VÀO — kích hoạt tính năng)

| entry | gọi | qua đường |
|---|---|---|
| Lua content (NPC/quest grant) | `player:GetMiniAvatarMgr():AcquireMiniAvatar(id)` | KPlayer method → Luna<KMiniAvatar> object |
| Lua content (UI query) | `mgr:IsMiniAvatarAcquired(id)` / `mgr:GetMiniAvatarCount()` / `mgr:GetAllMiniAvatar()` | Luna<KMiniAvatar> |
| Lua content (đeo) | `player:SetMiniAvatar(id)` (guard: phải đã acquire) | KPlayer method |
| login (auto) | AcquireAllFreeMiniAvatar (lazy khi set rỗng, gọi từ Acquire/GetSync) | C++ internal |

## D. Cross-version drift cần khoá

| giả định | v2.5 | 2010 | guard |
|---|---|---|---|
| **DB per-item = WORD** | mỗi acquired id ghi `(short)` — 2 byte | KMINIAVATAR_DB: mỗi id = WORD | KHÓA: acquired id ≤ 0xFFFF (in-mem DWORD set, chỉ low16 vào DB). Guard giá trị khi Acquire |
| **DB worn-id = WORD** | `(short)m_dwMiniAvatarID` | WORD on-disk | KHÓA: worn id ≤ 0xFFFF |
| **DB block layout** | `[WORD wornID][WORD count][WORD id × count]`; Load assert leftover==0. KHÔNG có blockSize prefix | y hệt | `#pragma pack(1)`, roundtrip byte-exact, assert leftover==0 |
| KMiniAvatarInfo | 12B {dwID@0,dwForceID@4,BOOL bFreeAcquire@8} | y hệt trong KMiniAvatarSettings.h | static_assert sizeof==12 + offset |
| box embed | KPlayer+0xb934 | compiler layout | truy qua tên member |
| worn-id field | KPlayer+0xb774 | member m_dwMiniAvatarID | không hardcode offset |
| player force | KPlayer+0x88 (CanAcquire, AcquireAllFree so sánh dwForceID) | dò accessor force/faction 2010 (KCharacter) | dùng accessor, xác minh 0x88 = force trước port |
| player roleType | KPlayer+0xcfc (broadcast arg) | (đã biết từ hair) | accessor |
| role-block enum | `rbtMiniAvatarData` | append trước rbtTotal | ordinal self-consistent |
| Lua method names | GetMiniAvatarCount/GetAllMiniAvatar/IsMiniAvatarAcquired/AcquireMiniAvatar | y hệt trong REGISTER_LUA_FUNC | khớp tên script content |
| tab columns | ID / ForceID / FreeAcquire | y hệt cột MiniAvatarInfo.tab | khớp header |

## E. Defer (hoãn có lý do)

| hàm | lý do | phụ thuộc |
|---|---|---|
| GetSyncMiniAvatar + notify broadcaster `FUN_08058d18` | packet đồng bộ acquired-set về client (v246-only render) | packet GS_CLIENT + client v246 |
| worn-change broadcast `FUN_080d557e`, `DoSyncTargetMiniAvatarMisc`, `DoSyncTargetTargetMiniAvatarMisc` | packet worn-avatar display sync | packet + client v246 |

**KHÔNG có buy chain** (khác hair) — hệ này chỉ grant + free-acquire, không tiền tệ → không defer money.
