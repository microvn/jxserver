# KCurrency + KCurrencyList (限购货币 / tiền tệ có trần + reset chu kỳ) — port WORKLOG

Classification = **NEW** (both, `grep ::` in 2010 = 0). DWARF-based. Tier-2 dependency-root
(buy-chains của hair/exterior/regression chờ nó). RE pass 2026-07-13.

## [RE-1] Surface + layout (DWARF)
**KCurrency** (0x18 = 24B): m_pPlayer@0, m_nType@4, m_nValue@8, m_nMaxValue@0xc, m_nRemainSpace@0x10,
m_nLastResetTime@0x14. Methods: Init(KPlayer*,int)/UnInit/GetType/SetType/GetValue/SetValue/
AddCurrency(int)/GetRemainSpace/AddRemainSpace(int)/Activate + ctor/dtor.
**KCurrencyList** (0x94 = 148B): m_pPlayer@0, m_CurrencyList@4 = **KCurrency[6]** (6*0x18=0x90).
Methods: Init(KPlayer*)/UnInit/Activate/GetCurrency(int)/Save/Load + ctor/dtor.

6 currency types = index 0..5 (KCurrencyList::Init loops i=0..5 -> KCurrency::Init(player,i)).
No named enum in binary (cbtInvalid/cbtTotal in asserts only). Meaning of each type = DATA-defined.

## [RE-2] DB serialization (Save 081e7eac / Load 081e7c1c) — role-block, backward-simple
**Blob KCURRENCY_DB_DATA:**
```
[BYTE nCount = 6]
[ KCURRENCY_INFO x nCount ]  each 0x24 = 36 bytes:
   +0x00 int nType          (Save writes GetType; Load IGNORES it -> type = loop index)
   +0x04 int nValue         (Save GetValue / Load SetValue)
   +0x08 int (HOLE, 4B)      (Save skips -> garbage in v246; Load skips. We zero it for determinism)
   +0x0c int nRemainSpace    (Save GetRemainSpace / Load Set)
   +0x10 int nLastResetTime  (Save Get / Load Set)
   +0x14 BYTE[16] reserved   (Save memset 0; Load skips)
```
Total = 1 + 6*36 = 217 bytes. Load: nCount<=6 (cbtTotal), leftover must == 0. m_nMaxValue NOT
persisted (config-derived). **Risk = medium (per-player save), same class as KDesignation.**
getters/setters field-mapping CONFIRMED from machine code (read_bytes, decompiler was down):
d0ee GetType(+4), d0fa GetValue(+8), d106 GetRemainSpace(+0x10), 834c GetLastResetTime(+0x14);
setters 8322 SetType(+4), 8330 SetValue(+8), 833e SetRemainSpace(+0x10), 8358 SetLastResetTime(+0x14).

## [RE-3] Logic (decompiled)
- **KCurrency::Init(player,type):** validate type[0,5]; value=0, lastReset=0;
  m_nMaxValue    = CONFIG_maxValue[type]   (= *(g_pSO3World + 0xc + (type+0xac)*4))
  m_nRemainSpace = CONFIG_remainDefault[type] (= *(g_pSO3World + 4 + (type+0xb4)*4))
- **AddCurrency(int add) [ANTI-HACK, must be C++]:** if add>0: if value>=maxValue skip; overflow guard;
  clamp add=min(add, maxValue-value); clamp add=min(add, remainSpace); remainSpace-=add (0 -> notify);
  value+=add; clamp>=0. => enforces BOTH total cap and periodic earn cap.
- **AddRemainSpace(int):** remainSpace+=v (guard overflow, clamp>=0). Refill earn allowance.
- **Activate() [periodic reset]:** if type==4 return (exempt). Compute period index from a week-base
  (g_pSO3World+0x31c + DAT_084df688 + 0x54600) / period(g_pSO3World+0x320). If lastReset==0: init
  lastReset=now, remainSpace=default. Else if new period(s) elapsed: carryover =
  ((remainSpace + default*(periods-1)) * carryPct)/100 (carryPct from CONFIG_carry[type], gated by
  CONFIG_carryFlag[type]); remainSpace = carryover + default; lastReset=now. Notify+sync.
- **GetCurrency(type):** validate[0,5]; return &m_CurrencyList[type].
- **KCurrencyList::Activate:** for i in 0..5: m_CurrencyList[i].Activate().

## [RE-4] Wiring
- **Per-player embed:** KCurrencyList m_CurrencyList in KPlayer (offset free); Init(this)+UnInit.
- **Role-block:** Save/Load are the role-block Save/Load. 2010 enum ROLE_DATA_BLOCK_TYPE ends
  rbtRegressionData -> append **rbtCurrencyData** before rbtTotal (confirm v246 tag name).
  [caller of KPlayer Save/Load role-block dispatch = same pattern as designation — TO WIRE]
- **Activate tick:** KCurrencyList::Activate called per player tick (KPlayer::Activate) — TO WIRE,
  low-frequency (weekly reset -> a slow tick is fine).
- **Lua:** add/get/query bindings (AddCurrency/GetCurrency value + remainSpace). Shape = KPlayer method
  (like designation) — CONFIRM from a v246 LuaXxx wrapper. TO RE the exact binding set.

## [RE-5] **BLOCKER for functionality — the per-type CONFIG is a NEW v2.5 subsystem, source NOT pinned**
Init/Activate read 4 config arrays at g_pSO3World early offsets (~0x2bc region), indexed by type:
  CONFIG_maxValue[6], CONFIG_remainDefault[6], CONFIG_carryFlag[6], CONFIG_carryPct[6] + period + week-base.
These are **NOT in the 2010 KGWConstList** (grep = 0 currency arrays) and NOT obviously named KSO3World
members. They are loaded from an unidentified v2.5 settings source. Data tree has shop/tier/*Currency*.tab
but those are SHOP tables that SPEND currency, not the cap-config.
**Consequence:** without this config, every currency Inits with maxValue=0/remainSpace=0 -> AddCurrency
clamps everything to 0 -> currencies hold nothing -> buy-chains still can't transact. So the config is
the FUNCTIONAL GATE, unlike designation (whose core worked with buff-data dormant).

## Scope fork (recorded; awaiting user decision)
- **A. Port mechanism now, config dormant:** class + DB(roundtrip oracle) + embed + role-block + Activate
  + Lua, with the 4 config arrays as named members defaulting 0 (documented data-gap). Compiles/wires/
  roundtrips/boots; buy-chains can CALL the API but transact 0 until config lands. Follow-up slice hunts
  the config source + loads it -> flips currencies live. (Matches the project's port-mechanism/defer-data pattern.)
- **B. Pin config source first:** RE the config loader (which .tab + KSO3World members + values), then port
  everything functional in one pass. Larger RE up front; deliverable is immediately usable.

---

## [PORT-1..2] Mechanism slice (2026-07-13) — build ok=202 fail=0, boot settings-[OK]
Scope A chosen (mechanism now, config+interface deferred). git revert point before = 246d6ea.

- **[PORT-1] Classes.** NEW `KCurrency.{h,cpp}` + `KCurrencyList.{h,cpp}`.
  - KCurrency: Init(player,type)/UnInit/AddCurrency/AddRemainSpace/Activate + inline get/set. Caps
    logic (AddCurrency) transcribed from 081e846c: earn clamps to (maxValue-value) then remainSpace,
    decrements remainSpace; spend (negative) bypasses caps, value clamps>=0. Activate (081e861a):
    type-4 exempt, period-guarded no-op while config dormant, carryover math faithful.
  - Config (per-type maxValue/remainDefault/carryFlag/carryPct + period/week-base) = 6 static
    accessors in KCurrency.cpp returning 0 (DORMANT). Single wire-point for the config slice.
  - DB (KCurrencyList Save/Load) byte-layout pinned from 081e7eac/081e7c1c + getter/setter machine
    code (read_bytes, decompiler was down): [BYTE 6][6x KCURRENCY_INFO 0x24]. Entry = nType@0,
    nValue@4, hole@8, nRemainSpace@0xc, nLastResetTime@0x10, reserved[16]@0x14. m_nMaxValue NOT
    persisted (config). Load: type=index, restore value/remainSpace/lastResetTime. Compile guard
    sizeof(KCURRENCY_INFO)==0x24. Blob = 217B for 6 currencies.
  - Oracle-mirror: DB roundtrip (217B, value/remainSpace/lastReset restored, maxValue from Init not
    blob, type from index, short-blob nCount<6) + caps (earn-cap clamp, total-cap clamp, exhausted
    no-gain, spend bypass+clamp, dormant no-earn) ALL PASS.
- **[PORT-2] Wiring.** KPlayer embed `m_CurrencyList` (KPlayer.h) + Init(this)/UnInit; role-block
  `rbtCurrencyData` appended before rbtTotal (KRoleDBDataDef.h) + SAVE_ROLE_BLOCK + case in KPlayer
  Save/Load dispatch; `m_CurrencyList.Activate()` in KPlayer::Activate (10s throttle, guarded no-op).

## Coverage-diff — forgotten = 0
KCurrency + KCurrencyList: all out-of-line methods implemented (B-A = 2 dtors, false-positive of the
`[A-Za-z_]` regex; both present). Inline get/set match v246 (not standalone symbols).

## DEFER (documented — the interface + config, NOT forgotten)
1. **Config source (functional gate):** the 6 static accessors return 0 -> caps 0 -> currencies hold
   nothing. Next slice: RE the config loader (which .tab writes g_pSO3World+0x2bc region, the 6-type
   values) and replace the accessor bodies -> currencies go live.
2. **Lua bindings:** DWARF shows the currency Lua interface is a LARGE set of per-type wrappers in
   KScriptFuncList (dozens, seen via GetCurrency/AddCurrency xrefs FUN_0832124x.., FUN_0838999x..).
   Deferred to the config slice (per-type names are meaningful once the 6 types have identities).
3. **Sync + log packets (NEW):** KPlayerServer::DoSyncCurrency/DoSyncCurrencyList +
   KLogClient::LogPlayerCurrencyChange — client-facing NEW packets, stubbed with comments in
   AddCurrency/AddRemainSpace/Activate; deferred with the packet-tier work.

So the substrate (class + anti-hack caps + DB persistence + tick) is complete & verified; the
data (config) + interface (Lua/packets) are the follow-up that flips it live and lets shops spend.

---

## [PORT-3] Config slice (2026-07-13) — currency now LIVE (was DEFER #1)
Config source FOUND + wired + runtime-proven. The 6-type config is GameWorldConstList.ini [CURRENCY].

- **KGWConstList (2010) extended** with the v2.5 fields (int[6] unless noted): nCurrencyMaxValue,
  nCurrencyRemainSpace, nCurrencyRemainSpaceCanAccumulate, nCurrencyAccumulateRate,
  nLogCurrencyThreshold, + scalars nCurrencyRemainSpaceResetCycle/ResetOffest. (DWARF: same fields
  at ConstList+0x230..0x2ac; m_ConstList sits at g_pSO3World+0x74, deltas all 0x74.)
- **Loader** (KGWConstList.cpp LoadData): [CURRENCY] section, per-type keys by name prefix
  {Contribution,Prestige,Justice,ExamPrint,ArenaAward,ActivityAward} (order = CURRENCY_DATA_BLOCK_TYPE):
  Max%s / %sRemainSpace / %sRemainSpaceCanAccumulate / %sAccumulateRate / Log%sThreshold +
  RemainSpaceResetCycle / RemainSpaceResetOffest.
- **KCurrency accessors** now read g_pSO3World->m_Settings.m_ConstList.* (was return-0 stubs).
  Activate week-base = ResetOffest + 0x54600 (345600s=4d -> Monday anchor; runtime tz DAT_084df688=0).
- **6 types confirmed** (CURRENCY_DATA_BLOCK_TYPE): 0 cbtContribution(贡献), 1 cbtPrestige(威望),
  2 cbtJustice(侠义值), 3 cbtExamPrint(考评印), 4 cbtArenaAward(竞技场, reset-EXEMPT), 5 cbtActivityAward(活动).

**RUNTIME PROOF (temp log, then removed):**
`[CURRENCY-CHK] Max=200000/200000/50000/1500/50000/500 Remain=55000/55000/6000/400/99999999/180
Cycle=604800 Off=25200` — matches GameWorldConstList.ini [CURRENCY] exactly. Config is NOT dormant:
currencies hold/earn/spend/cap/reset with real values. build ok=202, boot settings-[OK].

## Remaining DEFER (interface/parity — currency already functions server-side)
2. **Lua interface:** large per-type wrapper set in KScriptFuncList (dozens; xrefs). Not needed for
   the currency to work in C++; needed for content scripts. Follow-on.
3. **Sync/log packets (NEW):** DoSyncCurrency/DoSyncCurrencyList (client realtime display) +
   LogPlayerCurrencyChange. Without them client sees currency only after relog (Load). Follow-on (packet tier).

---

## [PORT-4] Lua interface slice (2026-07-13) — 4 non-legacy currencies usable from content
Un-defers #2 (Lua) for the currencies that have NO 2010 legacy field. RE'd the wrapper set via
__PRETTY strings + decompile (LuaAddJustice 0833887e = GetCurrency->AddCurrency direct;
LuaAddJusticeRemainSpace 08322398 = AddRemainSpace; LuaGetActivityAwardRemainSpace 08321240).

- **12 KPlayer Lua wrappers** (macro DEFINE_LUA_CURRENCY) for Justice(2)/ExamPrint(3)/ArenaAward(4)/
  ActivityAward(5): LuaAdd{Name}(value[,action]) -> AddCurrency; LuaAdd{Name}RemainSpace(value) ->
  AddRemainSpace; LuaGet{Name}RemainSpace() -> GetRemainSpace. Registered. build ok=202, boot [OK] (no nil).
- **Contribution(0)/Prestige(1) deliberately NOT ported to Lua-currency:** they have full legacy 2010
  systems (m_nContribution/m_nCurrentPrestige + AddContribution/DECLARE_LUA_INTEGER + LuaAddPrestige).
  v246 routes LuaAddContribution through KPlayer::AddContribution (dual old-field+KCurrency); porting
  that = behavior/DB change on live fields -> OUT OF SCOPE. Types 0/1 stay legacy; KCurrency slots 0/1
  are reserved/unused. Documented.
- **No value-getter:** v246 exposes NO LuaGet{Type} value (only RemainSpace); the balance reaches the
  client via the sync packet (deferred), not Lua read-back. Matches our port.

## Still DEFER (after PORT-4)
- Lua for Contribution/Prestige currency-migration (legacy-field coexistence; needs a migration decision).
- Value-add stat/log (UpdateCurrencyStat / LogPlayerCurrencyChange) on the delta.
- Sync packets DoSyncCurrency/DoSyncCurrencyList (client realtime balance display).
- KShop currency-buy drift + the deferred hair/exterior buy-chains (the actual "spend currency in a shop"
  flow) + per-shop cost config + content consumers (e.g. KNpc::LootActivityAward drops currency).
