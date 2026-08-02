# Blind candidate overlay and probe ledger

This package validates the generic extension in `journey_contract_query.py` with
only synthetic neutral facts. It contains no captured packet, source/candidate
code, Graph DB input, or target-specific value.

Run from `linux-build/`:

```bash
python3 graphengine/tools/query/journey_contract_query.py \
  --seed synthetic-neutral \
  --target-contract-facts port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/journey-contract-overlay-probe/target-contract-facts.synthetic.json \
  --candidate-overlay port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/journey-contract-overlay-probe/candidate-overlay.synthetic.json \
  --probe-ledger port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/journey-contract-overlay-probe/probe-ledger.synthetic.jsonl \
  --output port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/journey-contract-overlay-probe/synthetic-result.json
```

Schemas:

- `journey-target-contract-facts/v1`: target-origin-only facts. Every contract
  has `contract_key`, `process`, `protocol.enum.{name,value}`,
  `payload_byte_size`, fields (`name`, `offset`, `order`), and a source
  location/hash.
- `journey-candidate-overlay/v1`: the identical structural shape with origin
  `candidate`. It is accepted only with explicit target facts. Results preserve
  both origins and emit only `MATCHES`, `CONFLICTS`, or `MISSING`.
- `journey-probe-ledger/v1` JSONL: one event per `(variant, contract_key,
  sequence)`, containing `role`, `process`, `rung`, packet length/hash, state
  fingerprint, and active binary/config IDs. The IDs are retained as provenance
  and never compared between stock and candidate. The comparator returns the
  deterministic first supplied mismatch by `(sequence, contract_key)`, or
  `UNRESOLVED` for a missing pair or an all-matching finite ledger.

The extension does not infer field meaning, process routing, causal runtime
semantics, or runtime acceptance. It does not query candidate/source code and
the probe mode does not run, capture, or mutate any runtime.

See `PROVENANCE.md` for the mandatory record of incidental documentation
exposure before this package was implemented.

Frozen SHA-256 is in `SHA256SUMS` (from this directory, run
`sha256sum -c SHA256SUMS`) after JSON validation.
