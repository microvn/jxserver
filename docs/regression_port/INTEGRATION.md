# KRegression — INTEGRATION (ARCHITECTURE §8 checklist, filled in)

The caller/callee/binding map so nobody re-RE's this. Walks the 17 §8 points; only the ones that
apply to this subsystem are recorded. This is a PORT DESIGN reference — no source has been edited.

## A. Binding points — every place the port must edit

| §8 | what | file:line (2010) | value / shape for KRegression |
|----|------|------------------|--------------------------------|
| A1 | Config manager Init | NOT KWorldSettings. `KSO3World::Init` in `src/SO3World/Src/KSO3World.cpp` | KRegressionManager is a **direct KSO3World member** (like `m_MentorCache`, `include/Include/SO3World/KSO3World.h:234`). Add `KRegressionManager m_RegressionManager;` + `#include` its header; call `m_RegressionManager.Init()` in KSO3World::Init (BOOL-checked, add `UnInit()` to the rollback/UnInit path). Manager reads `settings/ServerConstListByVer.ini [REGRESSION]` + `settings/RegressionReward.tab`. |
| B2 | Embed per-player member | `src/SO3World/Src/KPlayer.h:312-313` (beside `m_ExteriorBox`,`m_HairBox`) | `KRegressionPlayerData m_RegressionData;` + `#include "KRegressionPlayerData.h"`. Offset is compiler-free (v246 had +0x94e4 — do NOT match). |
| B3 | `Init(this)` per-player | `src/SO3World/Src/KPlayer.cpp:141-149` (beside `m_Designation/m_ExteriorBox/m_HairBox.Init(this)`) | `bRetCode = m_RegressionData.Init(this); KGLOG_PROCESS_ERROR(bRetCode);` |
| C4 | Role-block enum | `include/Include/KRoleDBDataDef.h:41` (after `rbtHairBoxData`, before `rbtTotal`) | append `rbtRegressionData,` (becomes 33, matching v246). NEVER insert mid-enum. |
| C5 | Save dispatch | `src/SO3World/Src/KPlayer.cpp:2365` (after `m_HairBox.Save` line) | `SAVE_ROLE_BLOCK(m_RegressionData.SaveRegression, rbtRegressionData, 0);` — combined saver writes account chunk (22B) then player chunk (46B); see D. |
| C6 | Load dispatch | `src/SO3World/Src/KPlayer.cpp:~2044` (add `case rbtRegressionData:` beside `rbtHairBoxData`) | `case rbtRegressionData: bRetCode = m_RegressionData.LoadRegression(pbyOffset, pBlock->dwLen); ...` — reads account 22B then player 46B. |
| D8 | Lua binding SHAPE | `src/SO3World/Src/KLuaPlayer.cpp` | **shape (c) KPlayer methods** (v246 binds them on KPlayer, `this = player+0x94e4`, NOT `Luna<T>`). Define `LuaGetRegressionGradeID/LuaGetRegressionDailyCount/LuaGetRegressionData/LuaRegressionFinished/LuaAddRegressionReward` on KPlayer + `REGISTER_LUA_FUNC(KPlayer, LuaXxx)` for each. |
| E12 | Client packet | `KPlayerServer` (`src/.../KPlayerServer.cpp`) | `DoSyncRegressionPlayerData(id, gradeID, dailyCount, BYTE mark[8])` + client cmd `OnAddRegressionRewardItem`. **DEFER** (client 2.5.2-only render; see PORT_DESIGN §defer). Calculate/AddRewardItem call it — stub to a no-op until client parity. |
| G15 | Apply/grant hook | Lua `scripts/player/PlayerScript.lua :: AddRegressionRewardItem(player,type,index,count)` | item granting is script-side. C++ `CallAddRewardItemScript` just invokes it. No representID/equip hook. |

## B. Callee — what the ported code calls OUT to

| ported fn | calls | subsystem | action |
|-----------|-------|-----------|--------|
| Manager::LoadConstList | `g_OpenIniFile` + KIniFile vtbl | ini reader | reuse 2010 (exists — used by KWorldSettings) |
| Manager::LoadRegressionReward | `g_OpenTabFile` + KTabFile | tab reader | reuse 2010 (exists) |
| PlayerData::Calculate | Manager IsWork/GetRewardGradeID/GetRegressionFinishedTime/IsCrossDays; DoSyncRegressionPlayerData | self + KPlayerServer | port Manager helpers first; sync = defer/stub |
| PlayerData::AddRewardItem | Manager GetRewardItemInfo; CallAddRewardItemScript; DoSync | self + KScriptCenter | reuse script-call plumbing (KMentorCache/KPlayer already call Lua) |
| PlayerData::CallAddRewardItemScript | KScriptCenter call `scripts/player/PlayerScript.lua` | Lua | reuse 2010 script-invoke API (same as CallLoginScript/CallTrapScript) |
| LuaGetRegressionData | lua_createtable/pushnumber/pushstring/settable | lua5 | reuse |

## C. Caller — who calls IN

- **Manager::Init** <- `KSO3World::Init` (once, at boot). UnInit <- KSO3World::UnInit rollback.
- **PlayerData::Init** <- `KPlayer` init chain (:141-149 region).
- **PlayerData::Calculate** <- `KPlayer::OnExtDataLoadFinish` (v246) = the 2010 login post-role-load
  hook. 2010 has no method by that exact name; nearest equivalents are around
  `KPlayer::CallLoginScript()` (`KPlayer.cpp:2076`) / the `LoadExtRoleData` completion path
  (`KPlayer.cpp:1903`). Recommend: call `m_RegressionData.Calculate(m_nLastLoginTime, m_nCurrentLoginTime)`
  in the login-complete path (after all role blocks loaded, before enter-scene). **Verify the chosen
  hook actually runs post-load** (RE remaining item).
- **Lua bindings** <- content scripts (regression UI panel), e.g. `GetPlayer():GetRegressionData()`.
- **AddRewardItem** <- `LuaAddRegressionReward` <- client claim action via a UI script.

## D. Drift to lock (compile-time guards, -std=gnu++98 -> negative-array trick)
- `rbtRegressionData == 33` — `typedef char _CHK1[(rbtRegressionData==33)?1:-1];`
- Account chunk size == 22, player chunk == 46:
  `typedef char _CHK2[(sizeof(KACCOUNT_REGRESSION_INFO)==22)?1:-1];`
  `typedef char _CHK3[(sizeof(KREGRESSION_DB_DATA)==46)?1:-1];` (`#pragma pack(1)` both).
- REGRESSION_DAILY_MAX_COUNT==8, REGRESSION_ITEM_MAX_COUNT==8, m_byItemMark size==8.
- Grade/Ver/DailyCount are BYTE on disk -> guard values < 256 on save (Save already caps daily to 0xFF).

## E. Defer (with reason)
- **DoSyncRegressionPlayerData / OnAddRegressionRewardItem packets** — client-facing GS_CLIENT
  protocol; only the v2.5 client renders the regression panel. PAP2/2010 client has no UI. Stub
  the sync to a no-op (or add the protocol id but don't wire client) until client parity. Server
  logic (Calculate/AddRewardItem) is fully functional without it — marks/grades still persist.
- **Account-level sharing across characters** — v246 stores account chunk per-character-role-block
  (chunk-keyed). 2010 KRoleBlockHeader has no chunk key, so the port combines account+player into
  ONE `rbtRegressionData` block (see PORT_DESIGN §persistence). True cross-character account sharing
  (same account, different roles seeing the same account grade/ver) would need a center-side account
  store — DEFER; per-character copy is behaviorally correct for a single character.
