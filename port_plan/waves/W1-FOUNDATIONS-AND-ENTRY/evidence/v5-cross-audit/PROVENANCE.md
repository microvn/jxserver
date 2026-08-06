# Provenance

- Historical main session: SHA-256 `76ff0f7bfca7c2b80e70e8806cd19299494727ee73e69fb87e3a234d74f1da14`; only cited observations at session lines `5414`, `5439-5440`, and `5447-5455` were used.
- Target V4 input: SHA-256 `d53548d3688e32f24334b4c8970e4a4868b8c13264bdf0dc974a30be23ce2a26`.
- Graph DB at query time: SHA-256 `25e726bf747b3e6ecfe7c045bd7c1a1e1c22610a4ab917cbc37a93f4d45449ba`.
- Query executable: `compare-engine/tools/query/journey_contract_query.py`; V5 adapter mode produces `journey-contract-query/v5`; its blind target-static path currently labels its output schema `journey-contract-query/v4`.
- The historical overlay intentionally records only the observed 306-byte pre-fix size and no inferred field offsets.  It has no claim about any current candidate.
- No source file, C++ source, Graph DB, runtime, deploy target, Wave/STATE/card/Atlas, or query tool was modified.  This directory is the sole write target.
