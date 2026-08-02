# Provenance

- V4 input: `../journey-contract-prototype/journey-contract-v4.target-static.json`.
- The adapter consumes that JSON plus `graphengine/evidence.sqlite` in
  read-only mode. It selects target DWARF fields only by the V4-recorded
  artifact SHA-256 and field DIE. It does not read candidate/source, binaries,
  captures, or runtime state.
- Converted contracts retain the original V4 `binary`, `enum`, `payload_type`,
  and field evidence verbatim under `evidence`.
- The overlay and paired ledger are synthetic neutral fixtures. Their only
  purpose is exercising V5's supplied-facts comparison paths.
