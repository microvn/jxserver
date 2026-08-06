# V4 to V5 target-contract-facts adapter

This frozen fixture exercises `--target-v4`: it reads only the existing V4
JSON artifact and converts each usable per-binary enum/payload observation into
`journey-target-contract-facts/v1`. Contract keys are binary-scoped and use
only the observed V4 binary SHA-256, enum name/value, payload type, and DIE.
The conversion retains the original V4 binary/enum/payload/field evidence in
each contract's `evidence` member.

V4 exposes field names and DIEs but not offsets/order. The adapter uses only
each V4 payload's target artifact SHA-256 and field DIE to reopen the target
DWARF observations read-only, then emits explicit offsets and deterministic
order (byte offset, then DIE). A missing/mismatched observation remains
unresolved evidence; no layout fact is invented. The `process` field is the
observed V4 `binary_path`, not a process-route claim.

Run from `linux-build/`:

```bash
python3 compare-engine/tools/query/journey_contract_query.py \
  --seed v4-derived-field-layout-fixture \
  --protocol r2s_player_login_request \
  --max-direct-calls 16 --max-outbound-candidates 16 --max-outbound-second-hop 4 \
  --output port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/journey-contract-v4-v5-adapter/v4-derived-with-field-layout.json

python3 compare-engine/tools/query/journey_contract_query.py \
  --seed v4-v5-adapter-fixture \
  --target-v4 port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/journey-contract-prototype/journey-contract-v4.target-static.json \
  --converted-target-facts-output port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/journey-contract-v4-v5-adapter/target-contract-facts.v4-converted.json \
  --candidate-overlay port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/journey-contract-v4-v5-adapter/candidate-overlay.synthetic.json \
  --probe-ledger port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/journey-contract-v4-v5-adapter/probe-ledger.synthetic.jsonl \
  --output port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/journey-contract-v4-v5-adapter/v5-overlay-probe-result.json

python3 -m json.tool port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/journey-contract-v4-v5-adapter/target-contract-facts.v4-converted.json >/dev/null
python3 -m json.tool port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/journey-contract-v4-v5-adapter/v4-derived-with-field-layout.json >/dev/null
python3 -m json.tool port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/journey-contract-v4-v5-adapter/v5-overlay-probe-result.json >/dev/null
python3 -m py_compile compare-engine/tools/query/journey_contract_query.py
(cd port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/journey-contract-v4-v5-adapter && sha256sum -c SHA256SUMS)
```

The candidate overlay and probe ledger are deliberately neutral synthetic
fixtures. They do not read candidate source/binaries or run/capture a process.
Real candidate overlay and paired stock/candidate probe-ledger inputs remain
required for conclusions about a real candidate or runtime behavior.
