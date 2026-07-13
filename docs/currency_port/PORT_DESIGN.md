# KCurrency — PORT_DESIGN

Full RE + logic in WORKLOG.md. This = dependency/order/verify summary.

## 1. Dependency graph (bé→to)
```
KCurrency (one currency: value + total cap + periodic earn cap + reset)   <-- leaf
      |
KCurrencyList (per-player array of 6; GetCurrency/Activate/Save/Load)
      |
      +-- KPlayer embed (Init/UnInit) + role-block rbtCurrencyData (Save/Load) + Activate tick
      +-- [DEFER] config source (per-type caps/period)   <-- functional gate
      +-- [DEFER] Lua interface (per-type wrappers)
      +-- [DEFER] sync/log packets (DoSyncCurrency, LogPlayerCurrencyChange)
```

## 2. Data structures (pinned)
- KCurrency 0x18: m_pPlayer@0, m_nType@4, m_nValue@8, m_nMaxValue@0xc, m_nRemainSpace@0x10, m_nLastResetTime@0x14.
- KCurrencyList 0x94: m_pPlayer@0, m_CurrencyList@4 (KCurrency[6]).
- DB KCURRENCY_INFO 0x24 (#pragma pack(1)): nType@0, nValue@4, hole@8, nRemainSpace@0xc,
  nLastResetTime@0x10, reserved[16]@0x14. Blob = [BYTE count][count × 0x24]. m_nMaxValue NOT persisted.

## 3. Persistence + packet
- **Persistence:** role-block rbtCurrencyData. Save writes 6 entries; Load restores value/remainSpace/
  lastResetTime, type=index, maxValue from Init (config). Backward-tolerant (nCount<6 keeps Init defaults).
- **Packet:** DoSyncCurrency/DoSyncCurrencyList (NEW) — DEFERRED.

## 4. Port order (each verified)
1. Classes + config-accessor stubs — oracle (layout + caps + roundtrip). DONE.
2. Wiring (embed + role-block + Activate) — build + boot. DONE.
3. Coverage-diff — forgotten=0. DONE.
4. [NEXT SLICE] config source RE + load; Lua interface; sync/log packets.

## 5. Remaining RE / Defer
- Config loader: which .tab + KSO3World members write g_pSO3World+0x2bc region (maxValue[6],
  remainDefault[6], carryFlag[6], carryPct[6], period, week-base). Replace the 6 static accessors.
- Lua: RE the per-type wrapper set (KScriptFuncList) + shape.
- Packets: DoSyncCurrency/DoSyncCurrencyList protocol ids + handlers; LogPlayerCurrencyChange.
- The 6 type identities (what each currency is + what shops spend it on) — RE from shop/tier tables.
