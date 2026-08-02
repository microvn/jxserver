# Exact stock runtime baseline — 2026-07-27

## Result

The exact local v2.5.2 stock GameServer completed the user-observed path:

```text
authenticate -> choose existing role -> enter world
```

This is a **stock-only runtime baseline**.  It is not a candidate acceptance
and does not complete any Wave 1 card.

## Provenance correction

The VPS file previously launched under the plain `SO3GameServer` name was not
the local stock artifact.  It was a rebuilt, debug-bearing executable:

| Artifact | SHA-256 | Status |
|---|---|---|
| Exact local stock / verified VPS backup | `3002bf4ad08d4c0fb9d4fc10f2d66b1f6cd8f70d398ff792136a3b7d5c416f11` | authoritative runtime arm |
| Misnamed VPS plain-path executable | `0048aadcec951c9adc3c39f84df7ff70f566bf067a13c0bea26dc666784cf4b6` | rebuilt; invalid as stock oracle |

The three runtime DSOs matched the local target set.  Generated GS output
(`recorder`, `AIRuntimeStat`, `PackageStat.tab`) was moved to a recoverable
backup and recreated before the exact-stock run; persistent database data was
not modified.

## Launch correction

The first exact-stock launcher used a 14-character process match while Linux
reports the daemon as `SO3GameServer_w` (15 characters).  Its supervisor
therefore restarted GS during boot and produced the client maintenance state.
The corrected supervisor uses the exact 15-character process name and stayed
at zero restarts through startup.

## Static/runtime facts observed

- Stock reports v2.5.2 build metadata and reaches settings load, Center
  connection, listener start, GameServer startup, and map creation.
- Center registers the new GS and creates map `(1, 1)`.
- The public GS listener is live before the client test.
- The user then authenticated, selected a role, and entered the world.

## Consequence

Any earlier diagnosis based on the VPS plain-path `SO3GameServer`, including a
claimed stock-side Relay denial or transport failure, is **invalidated as a
stock comparison**.  Future candidate work must use this hash-pinned exact
stock arm and the same launcher/config/data identities.
