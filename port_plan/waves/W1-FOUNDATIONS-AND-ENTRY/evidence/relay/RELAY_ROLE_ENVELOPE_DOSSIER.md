# FND-RELAY-ROLE-ENVELOPE — research dossier

Status: `researching`. This is a revalidation dossier, not an acceptance
claim. Historical Wave 2/3 notes are retained as leads and are not promoted
without a current paired capture.

## Target-backed facts

The Center peer is now pinned separately from the GameServer target:

| Artifact | SHA-256 | Authority |
|---|---|---|
| `jx3_dwarf/SO3GameCenterD` | `655b28126b972d1b3e1b3c02b827356c4d2ad6cf421b258152b130ec3c214d79` | Center symbols and peer-side wire types |
| `jx3_dwarf/SO3GameServerD` | `47716c73e8de281c95759cdc4a478e70e7c61322fb46e8c4e04954e51124b94a` | GameServer handler/layout contract |

Center DWARF proves the peer-side producer
`KGameServer::DoPlayerLoginRequest` at `0x08084436` and consumer
`KGameServer::OnPlayerLoginRespond` at `0x0807ea58`. It defines
`R2S_PLAYER_LOGIN_REQUEST` as 302 bytes (`0x12e`) with `BaseInfo` at offset
`0x82` and target `KRoleBaseInfo` occupying the following 162 bytes. The
target response `S2R_PLAYER_LOGIN_RESPOND` is 38 bytes (`0x26`).

The historical 304-byte capture must therefore be treated as an unresolved
framing observation: it may include the two-byte internal wire header around
the 302-byte typed payload. Do not “fix” either size until a paired raw
capture labels the framing boundary.

## Target-backed leads

| Claim | Evidence | Current disposition |
|---|---|---|
| `KRelayClient::OnPlayerLoginRequest` is the GS consumer of the Center role-login request | Wave 3 retry capture: `R2S_PLAYER_LOGIN_REQUEST` (304 bytes) reached GS; target/source research names the handler | Reopenable runtime lead; current candidate capture still required |
| Candidate must not deny login before role sections complete | Wave 3 direct diagnosis: `m_bSyncDataOver=false` when mentor-sync completion was routed to `OnNoOpRespond` | Strong historical lead; target route/registration must be re-pinned |
| Base role data is embedded in Relay packets | Wave 2 `CONSUMER_INVENTORY.md`: `KRoleBaseInfo` in `Relay_GS_Protocol.h` at the login/save packet definitions | Target layout evidence exists; peer Center artifact and current route capture remain open |
| `KROLE_POSITION_DB` layout affects role-envelope interpretation | Wave 2 DWARF contract: target `nCenterIndex` offset 0, `dwMapID` offset 4; `KRoleBaseInfo` size 162 | ABI lead is target-backed, but acceptance needs paired stock/candidate envelope observation |
| Unknown role blocks must be preserved | Wave 2 role round-trip probe and `ROLE_DATA_BLOCK_TYPE` contract | Reusable evidence; current W1 card must link exact artifact hashes |

## Required target closure still missing

1. Export the exact target protocol ID/handler closure for the selected
   `R2S_PLAYER_LOGIN_REQUEST` and completion/sentinel message.
2. Capture stock and candidate packet/state transitions with active binary
   hashes, including section order, byte sizes, completion flag, and response.
3. Reconcile the current P3+transport candidate against the v2.5 role layout;
   do not reuse the older `KROLE_POSITION_DB` fix without checking its source
   closure and current artifact.

## Explicit boundary

This card is not blocked by the absence of a successful client scene entry yet;
it is blocked by missing current Relay evidence. The correct next measurement
is a stock-vs-candidate Center→GS role-envelope capture, not a guessed source
patch and not a generic client “unknown error” interpretation.
