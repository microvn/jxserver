# KCurrency — INTEGRATION (ARCHITECTURE.md §8 filled in)

NEW per-player subsystem, Server-side. Two classes: KCurrency (one currency) + KCurrencyList
(per-player container of 6). Persisted via role-block. See WORKLOG for RE.

## A. Binding points
| §8 | point | status | file:line | note |
|----|-------|--------|-----------|------|
| A1 | config .tab | [DEFER] | — | per-type caps/period = NEW v2.5 config, source not pinned (RE-5) |
| B2 | KPlayer embed | [DONE] | KPlayer.h (KCurrencyList m_CurrencyList, after m_RegressionData) + `#include "KCurrencyList.h"` | |
| B3 | Init(this)/UnInit | [DONE] | KPlayer.cpp Init (after m_Designation, infallible) + UnInit | |
| C4 | role-block enum | [DONE] | KRoleDBDataDef.h — `rbtCurrencyData` appended before rbtTotal | |
| C5 | SAVE_ROLE_BLOCK | [DONE] | KPlayer.cpp (m_CurrencyList.Save, rbtCurrencyData, 0) | after m_RegressionData |
| C6 | Load case | [DONE] | KPlayer.cpp `case rbtCurrencyData:` -> m_CurrencyList.Load | |
| C7 | DB byte layout | [DONE] | KCurrencyList Save/Load; KCURRENCY_INFO 0x24, blob 217B; guard sizeof==0x24 | oracle roundtrip |
| — | Activate tick | [DONE] | KPlayer::Activate 10s throttle -> m_CurrencyList.Activate() (guarded no-op) | |
| D8/D10 | Lua | [DEFER] | KScriptFuncList — large per-type wrapper set | RE with config slice |
| E12 | packet | [DEFER] | KPlayerServer::DoSyncCurrency/DoSyncCurrencyList (NEW) | stubbed; packet tier |
| F14 | log | [DEFER] | KLogClient::LogPlayerCurrencyChange (NEW) | stubbed |
| G15 | anti-hack | [DONE] | AddCurrency caps in C++ (total + periodic earn) | security-critical, in place |

## B. Callee (ported fns call OUT to)
| fn | calls | status |
|---|---|---|
| KCurrency::Init/Activate | config accessors (GetCurrencyMaxValue/RemainDefault/Carry*/ResetPeriod/WeekBase) | DORMANT stubs -> 0 |
| AddCurrency/AddRemainSpace/Activate | DoSyncCurrency + LogPlayerCurrencyChange | DEFERRED (commented) |
| KCurrencyList Save/Load | KCurrency get/set (inline) | DONE |

## C. Caller (who calls IN)
- KPlayer::Init/UnInit -> Init/UnInit. KPlayer::Save/Load (role-block) -> Save/Load. KPlayer::Activate -> Activate.
- **DEFERRED IN-callers:** shop/activity/quest code + Lua that earn/spend currency (dozens of sites in
  DWARF xrefs to AddCurrency/GetCurrency) — arrive with the shop/config slice.

## D. Drift to LOCK
| assumption | value | guard |
|---|---|---|
| KCURRENCY_INFO size | 0x24 (36) | `typedef char _CHK[sizeof==0x24]` in KCurrencyList.cpp |
| currency count | 6 | CURRENCY_TYPE_COUNT (v246 cbtTotal); Save writes 6, Load asserts <=6 |
| blob size (6) | 217 (1 + 6*36) | oracle |
| DB field map | value@4, remainSpace@0xc, lastResetTime@0x10 (type=index) | machine-code confirmed (WORKLOG RE-2) |
| type-4 reset-exempt | yes | Activate early-return |

## E. Defer / data-gap
Config source (gate) + Lua interface + sync/log packets — see REPORT "Deferred". Currencies exist,
persist, and are safe now; they transact 0 until the config slice lands.
