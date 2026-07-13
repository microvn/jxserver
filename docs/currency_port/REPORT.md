# KCurrency + KCurrencyList — REPORT

**Status: mechanism DONE + verified; interface + config DEFERRED (documented).**
Tier-2 dependency-root: the capped/periodic in-game currency substrate (限购货币) that buy-chains spend.

## What shipped (mechanism)
- Classes KCurrency (24B) + KCurrencyList (148B, 6-currency array), all methods.
- Anti-hack caps in C++ (AddCurrency enforces total cap + periodic earn cap; spend bypasses, clamps>=0).
- DB persistence (role-block rbtCurrencyData, 217B blob, byte-layout pinned + oracle-roundtripped).
- KPlayer embed + Init/UnInit + Activate tick (weekly reset, guarded no-op while dormant).

## Metrics
- **Methods:** DWARF surface fully implemented (out-of-line + inline get/set). **Forgotten = 0.**
- **Build:** ok=202 fail=0, link 0, undefined 0 (+2 objects). Guard sizeof(KCURRENCY_INFO)==0x24 PASS.
- **Boot:** `Load game settings ... [OK]` — embed/role-block/Activate/Init clean, no regression.
- **Oracle:** DB roundtrip 217B (value/remainSpace/lastReset restored; maxValue from config not blob;
  type from index; short-blob) + caps logic — ALL PASS.
- **Files:** NEW KCurrency.{h,cpp}, KCurrencyList.{h,cpp}; edited KRoleDBDataDef.h, KPlayer.{h,cpp}.

## Risk outcome (character save/load)
Medium (per-player DB), handled like KDesignation: blob byte-layout pinned from binary + getter/setter
machine code; m_nMaxValue NOT persisted (config-derived, restored by Init before Load); Load tolerant
of nCount<6 (old/short blobs). Oracle roundtrip proves it. No role-block enum shift (appended before rbtTotal).

## Deferred (NOT forgotten — the follow-up that makes it live)
1. **Config source (functional gate):** 6 static accessors in KCurrency.cpp return 0 -> every currency
   caps at 0 -> holds nothing. The config subsystem (per-type maxValue/remainDefault/carry + period/
   week-base, at g_pSO3World+0x2bc) is NEW v2.5, source table not yet pinned. Replacing the 6 accessor
   bodies (single wire-point) flips all currencies live.
2. **Lua interface:** large per-type wrapper set in KScriptFuncList (dozens; DWARF xrefs). Deferred to
   the config slice (names meaningful once the 6 types have identities).
3. **Sync/log packets (NEW):** DoSyncCurrency/DoSyncCurrencyList + LogPlayerCurrencyChange — stubbed
   with comments; deferred with the packet tier.

## Why this order
The substrate (caps + persistence + tick) is the risky, security-critical, RE-heavy part — shipped and
verified now. Config + Lua + packets are additive and content/shop-coupled; they land with the shop/
config slice that actually spends currency. Until then currencies exist, persist, and are safe, but
transact 0 (documented dormant).
