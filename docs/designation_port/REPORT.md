# KDesignation (称号) DRIFT port — REPORT

**Status: DONE + verified.** DRIFT reconciliation of 2010's 15-method KDesignation up to the
v2.5.2 (v246) 31-method timed/equip-gated/buff-granting subsystem.

## Metrics
- **Methods:** v246 surface = 31. Ported/matched = 29 (+ dtor). Folded = 1 (LoadCurrentDesignation
  -> Load). 2010-kept = 1 (SetCurrentDesignation, back-compat). **Forgotten = 0.**
- **Lua:** +7 new bindings, +1 rename alias, 3 v2.5-removed bindings kept. All registered.
- **Build:** ok=199 fail=0, link exit=0, undefined refs=0 (every slice).
- **Boot:** reaches `Load game settings ... [OK]` with no designation error/assert (every slice).
- **Files touched:** KDesignation.{h,cpp}, KDesignationList.{h,cpp}, SO3Result.h (ect enum),
  KPlayer.{h,cpp}, KLuaPlayer.cpp, KPlayerServer.cpp, KPlayerClient.cpp.

## Verify nets used
- **Oracle-mirror (local):** config layout (sizeof 24/20, offsetof nType 0x14); DB roundtrip 4/4
  (empty->72B==2010 blob, timed->89B, old-blob load, postfix-only->79B); acquire branch logic 12/12.
- **Build host** (Linode, native x86 m32): clean every slice; `sizeof(time_t)==4` compile-guard
  PASSED => the 4-byte end-time wire model is correct on the real build.
- **Boot no-regression:** settings-[OK] on all 5 boots.

## Risk outcome (character save/load)
- Header 8B byte-identical to 2010 (only field-name change byDisplayFlag->byBynameDisplay).
- End-time block APPEND-ONLY + OPTIONAL: old 72-byte blobs load unchanged; new blobs with no timed
  designation are byte-identical to old. No migration, no role-block enum change. Backward- AND
  forward-compatible (oracle-proven both directions).
- Load kept the direct field-set (byte-verified, non-fatal) instead of routing through
  SetCurrentPrefix; a since-removed prefix leaves the independent flag false rather than failing
  the whole character load.

## Deferred / data-gap (NOT forgotten)
- **Data-gap (dormant):** BuffID / CoolDownID / OwnDuration / Type columns are blank in this leak's
  `DesignationPrefixInfo.tab` / `DesignationPostfixInfo.tab` (only AnnounceType filled). So the
  timed-expiry, buff-on-equip, cooldown-gated-unequip, and independent-prefix code paths are
  complete but never execute until data provides those columns. Not a code defer.
- **Version-gap:** the `ectDesignationNotifyCode` (=31) unequip-on-cooldown message renders only on
  a v246 client (PAP2 client lacks the enum). Harmless while the CD path is dormant.
- **Activate() tick frequency:** wired at the existing 10s player-throttle (staggered by m_dwID).
  Exact v246 caller frequency not pinned (dormant maps => any frequency is a cheap no-op here).

## Live-oracle follow-on (env-gated, not blocking)
Timed acquire -> advance clock -> expire, and equip/unequip via Lua, need a running cluster + v246
client. Left as `test_designation.lua` scope. Oracle-mirror + no-regression boot carry the slices.
