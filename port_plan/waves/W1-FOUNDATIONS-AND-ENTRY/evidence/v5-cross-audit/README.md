# W1 historical dossier × V5 cross-audit

Scope: read-only audit of historical login-to-enter-world observations.  Historical session/dossier material is an overlay source only; target facts come from the current GraphEngine query and its V4-to-V5 target-facts adapter.  Nothing here establishes current source, candidate, deployed binary, runtime parity, review acceptance, or a packet reconstruction.

## Result

V5 independently reproduces one bounded historical conflict: the pre-fix `R2S_PLAYER_LOGIN_REQUEST` observation of `306` bytes conflicts with the target-static `302` bytes in both independently retained target binary contracts.  It cannot establish the historical post-fix candidate, the 10-byte confirm envelope, role-block/version semantics, entry acknowledgements, or `CheckPackage` runtime predicate.

`s2r_confirm_player_login_request` deliberately remains `UNRESOLVED`: its target protocol-payload record has not been materialized in GraphEngine.  No probe ledger was generated because historical material lacks a properly paired, hash-proven stock/candidate observation set.

See `report.json` for the inventory and exact missing target/static/probe hooks.  Query outputs are retained verbatim JSON; `historical-dossier.r2s-player-login.overlay.json` is explicitly non-current.

## Reproduce

Run from `linux-build/`:

```bash
python3 graphengine/tools/query/journey_contract_query.py --seed 'W1 historical dossier cross-audit / role-login envelope' --protocol r2s_player_login_request --output port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/v5-cross-audit/r2s-player-login.fresh-target-static.json
python3 graphengine/tools/query/journey_contract_query.py --seed 'W1 historical dossier cross-audit / confirm player login' --protocol s2r_confirm_player_login_request --output port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/v5-cross-audit/s2r-confirm-player-login.fresh-target-static.json
python3 graphengine/tools/query/journey_contract_query.py --seed 'W1 historical dossier cross-audit / historical 306-byte role-login observation' --target-v4 port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/journey-contract-prototype/journey-contract-v4.target-static.json --converted-target-facts-output port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/v5-cross-audit/r2s-player-login.target-facts.v5-adapted.json --candidate-overlay port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/v5-cross-audit/historical-dossier.r2s-player-login.overlay.json --output port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/v5-cross-audit/r2s-player-login.historical-overlay-comparison.v5.json
jq empty port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/v5-cross-audit/*.json
shasum -a 256 -c SHA256SUMS
```
