# Wave 1 static audit checkpoint

Date: 2026-07-25
Mode: static continuation under unavailable Windows-client oracle
Status: research complete for the current static gate; runtime acceptance remains open

## Working hypothesis

Assume authentication and existing-role selection succeed while the selected
role fails before scene-ready. This is a debugging hypothesis, not a measured
client result.

## Card audit

| Card | Static result | Runtime result | Decision |
|---|---|---|---|
| FND-RUNTIME-INITIALIZATION | Target/candidate boot milestones, hashes, residual-error policy, and provenance gap recorded. | Bounded candidate boot observed; no client dependency. | `reviewing`; provenance manifest and opposite-family review remain. |
| FND-CLIENT-GS-SECURE-TRANSPORT | Target security/provider route and real-`libcommon` candidate path recorded. | First client frame/security packet unavailable. | `researching`; no PASS from historical artifacts. |
| FND-RELAY-ROLE-ENVELOPE | GS/Center peer, typed sizes, producer/consumer symbols, and 302-vs-wire framing conflict recorded. | Selected-role Center→GS capture unavailable. | `researching`; framing/order/sentinel remain open. |
| FND-ROLE-LOAD-SPINE | Target version dispatcher, base layout, map consumer, and `rbtTotal=49` recorded. | Block headers, selected version, and round-trip unavailable. | `researching`; no source-write lease. |
| PLY-ENTER-WORLD | Target ready/apply/sync handlers, layouts, registration observations, and candidate route absence recorded. | Scene-ready and initial-sync sequence unavailable. | `researching`; static evidence is not acceptance or a blind patch authorization. |

## Permitted

- Prepare card-scoped baseline manifests and opposite-model review packets.
- Continue read-only target/source closure work.
- Resume with the named milestones below when the client returns.

## Forbidden

- Marking any client-facing card `accepted`.
- Treating the login/enter-world hypothesis as runtime fact.
- Copying historical Wave 2/3 fixes without current target-backed diff and
  opposite-model review.
- Bypassing ready gates, synthetic success packets, truncating unknown blocks,
  or changing protocol sizes to fit the hypothesis.

## Next bounded oracle

Run one existing-role scenario against stock and candidate and record hashes,
config identities, first GS frame/security identity, Center→GS role envelope
and framing, block headers and `rbtStateInfo` version, the ready/apply/initial
sync milestones, and the scene-ready result. A UI message alone is insufficient.

## Source of truth hierarchy

Feature cards are public milestone truth; `WAVE.md` is integrated-scenario
truth; `STATE.md` is the private evidence/resume ledger. This checkpoint records
the static gate and does not replace those ledgers.
