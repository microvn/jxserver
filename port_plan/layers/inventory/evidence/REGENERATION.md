# PORT_MAP regeneration record

## Current inventory — regenerated 2026-07-28

| Input | Identity |
|---|---|
| Target surface | `SO3GameServerD`, SHA-256 `47716c73e8de…` (non-stripped DWARF build) |
| Candidate surface | rebuilt `SO3GameServer` ELF, SHA-256 `f16725f5296e…` (built 2026-07-22, = committed baseline; uncommitted overlays are **not** included) |
| Source index | `src/`, `include/`, `compat/` — 834 files, 5696 distinct `Class::method` occurrences |

Commands (all read-only against the artifacts):

```bash
layers/inventory/extract_symbols.sh <target-DWARF>  v25_members.c.txt  v25_classes.c.txt
layers/inventory/extract_symbols.sh <candidate-ELF> ours_members.c.txt ours_classes.c.txt
# v25_sig_map.tsv: same nm|c++filt pipeline without -p, first signature per qualified name
# src_qualified.txt / src_barenames.txt: Class::method occurrences over src|include|compat
layers/inventory/classify_symbol_drift.sh <work-dir>
layers/inventory/generate_method_inventory.py --input-dir <work-dir> --out <port_plan-dir>
```

## Result

| Measure | 2026-07-11 snapshot | 2026-07-28 | Δ |
|---|---:|---:|---:|
| Target methods / classes | 4820 / 235 | 4820 / 235 | 0 |
| Candidate methods / classes | 3335 / 198 | 3485 / 207 | +150 / +9 |
| `PORT_MAP.csv` rows | 1716 | 1597 | −119 |
| `DRIFT` / `NEW` | 1278 / 438 | 1260 / 337 | −18 / −101 |
| `hard` / `uncertain(name-elsewhere)` | 1581 / 135 | 1458 / 139 | −123 / +4 |
| `PRESENT_EXACT` (excluded from CSV) | 0 | 2 | +2 |

121 rows closed since the snapshot, 2 rows newly appeared.

The two counters move in opposite directions on purpose: backlog rows go **down**,
candidate surface goes **up**. They do not cancel exactly because of the 150 new
candidate methods only 117 fall inside the target surface; the other 33 are
candidate-only helpers with no target counterpart (candidate-only total rose from
231 to 264).

Largest closures: `KPlayer` 24, `KDesignation` 16, `KHairShop` 13, `KMiniAvatar` 11,
`KHairBox` 11, `KRegressionManager` 10, `KCurrencyList` 8, `KCurrency` 7,
`KRankListServer` 5.

Cross-check that the run is sound: `KHairShop` drops from 21 to 8 open rows, which
matches the coverage-diff recorded for the hair port (24 ported / 3 folded /
8 deferred). `KCurrency` and `KRankListServer` reach 0. `KExterior` stays at 24
rows — consistent with the known finding that it shipped roughly half its class.

## Reading limits

- Symbol-emission based: inline/template methods present in source but not emitted
  appear as missing. `PRESENT_EXACT` caught only 2; the 139
  `uncertain(name-elsewhere)` rows are the remaining suspicion band.
- The candidate ELF predates the current uncommitted work, so the true remaining
  count is at most this, never more.
- A `DRIFT` row means "target emits this qualified name, candidate does not". It
  says nothing about whether the 3485 methods the candidate already has behave
  correctly. Known real defects — an `ENCODE_DECODE_MODE` argument, a `KS2R`
  enum value, a 3-byte request declared as 4, an unregistered protocol slot —
  produce **no row in this file**.

## Snapshots retained

- `PORT_MAP.snapshot-2026-07-11.csv`
- `missing_full.snapshot-2026-07-24.tsv`, `per_class_full.snapshot-2026-07-24.tsv`
