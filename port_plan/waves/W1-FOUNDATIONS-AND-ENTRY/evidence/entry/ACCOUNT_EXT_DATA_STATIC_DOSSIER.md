# W1 Account-data and extended-data static dossier

Status: `researching`. This dossier is a target/static closure for the
account-data and extended-role-data boundaries. It is not a source-write
authorization and does not claim that the active candidate reaches scene-ready.

## Scope and authority

Target authority is `SO3GameServerD`, SHA-256
`47716c73e8de281c95759cdc4a478e70e7c61322fb46e8c4e04954e51124b94a`.
The Center peer is `SO3GameCenterD`, SHA-256
`655b28126b972d1b3e1b3c02b827356c4d2ad6cf421b258152b130ec3c214d79`.
Source/candidate observations are comparison material only. Runtime packet
ordering and the active candidate hash remain unresolved until a paired arm is
captured.

## Target account-data spine

The target `OnApplyEnterScene @ 0x0807a300` contains two independent requests;
account data is not an implicit side effect of role-data loading:

```text
OnApplyEnterScene
  ├─ DoLoadAccountDataRequest @ 0x080cfe8a
  │    └─ OnLoadAccountData @ 0x080e0f06
  │         └─ KPlayer::LoadAccountData @ 0x08399acc
  └─ DoLoadRoleDataRequest
       └─ role-data / section-check spine
```

Target-confirmed facts:

| Claim | Evidence | Status |
|---|---|---|
| `OnApplyEnterScene` initiates account loading before role loading | target decompile raw export, entry dossier | `TARGET_STATIC_CONFIRMED` |
| `DoLoadAccountDataRequest` exists at `0x080cfe8a` | target DWARF/Graph symbol evidence | `TARGET_STATIC_CONFIRMED` |
| `OnLoadAccountData` exists at `0x080e0f06` | target DWARF/Graph symbol evidence | `TARGET_STATIC_CONFIRMED` |
| `KPlayer::LoadAccountData` exists at `0x08399acc` and loads account/regression state | target DWARF/decompile dossier | `TARGET_STATIC_CONFIRMED` |
| Candidate exposes the complete request/response/load chain | candidate source inventory | `UNRESOLVED` / currently absent by inventory |
| Exact account blob protocol ID, payload size, and Center/DB producer | current GS target evidence | `UNRESOLVED` |
| Account-load completion is the same gate as ext-data completion | target evidence | `UNRESOLVED`; do not merge the flags |

The candidate currently has no `DoLoadAccountDataRequest`, `OnLoadAccountData`,
or `KPlayer::LoadAccountData` implementation; the only matching occurrence is a
comment in `KRegressionPlayerData.cpp`. This is a candidate absence, not proof
of the full wire contract.

## Target extended-role-data spine

The target extended-data path is a request/response loop and a completion
producer, not a flag assignment copied into `OnLoadRoleData`:

```text
PartialLoadExtData
  └─ DoSyncRoleDataSectionCheckRequest @ 0x0805c2b8
       └─ client section response
            └─ OnSyncRoleDataSectionCheckRespond @ 0x08079c9a
                 └─ PartialLoadExtData / section handling
                      └─ OnExtDataLoadFinish @ 0x0839fb50
                           ├─ m_bExtDataLoadFinish
                           ├─ DoSyncRoleDataOver
                           └─ target initial-sync fan-out
```

Target wire facts:

| Contract | Target fact | Evidence | Status |
|---|---|---|---|
| `S2C_SYNC_ROLE_DATA_SECTION_CHECK_REQUEST` | size 3; `bySectionType @ +2` | DWARF DIE `0x003258ea` | `TARGET_STATIC_CONFIRMED` |
| `C2S_SYNC_ROLE_DATA_SECTION_CHECK_RESPOND` | size 6; header-only payload | DWARF DIE `0x0032853f` | `TARGET_STATIC_CONFIRMED` |
| `c2s_sync_role_data_section_check_respond` | protocol ID 4 | DWARF enum evidence | `TARGET_STATIC_CONFIRMED` |
| `DoSyncRoleDataSectionCheckRequest` | target address `0x0805c2b8` | Graph/decompile closure | `TARGET_STATIC_CONFIRMED` |
| `OnSyncRoleDataSectionCheckRespond` | target address `0x08079c9a` | Graph/decompile closure | `TARGET_STATIC_CONFIRMED` |
| `OnExtDataLoadFinish` | target address `0x0839fb50`; large sync fan-out | Graph/decompile closure | `TARGET_STATIC_CONFIRMED` |
| exact section producer/consumer ownership and order | not fully reconstructed | target decompile/runtime | `TARGET_PARTIAL` |
| candidate has matching section-check request, response handler and completion state | source inventory | candidate tree | `UNRESOLVED` / currently absent by inventory |

`OnExtDataLoadFinish` has a large target fan-out, including role-data-over,
currency, player-state, camp, buff, exterior, designation, achievement and
fellow-pet synchronization. It must not be copied wholesale as a first patch.

## Ready-state dependency

The target ready handler consumes, rather than creates, the completion gate:

```text
gsWaitForSyncClientData
  + m_bExtDataLoadFinish
  + C2S_CLIENT_CONFIRM_READY (ID 2, 6 bytes)
    → OnClientConfirmReady @ 0x08079dfc
    → KPlayer::OnClientReady @ 0x0839f87e
    → gsPlaying
```

The producer of `m_bExtDataLoadFinish`, the field initialization layout, and
the exact state write into `gsWaitForSyncClientData` are not all proven in one
reopenable closure. Therefore this dossier does not authorize adding the flag,
changing `OnLoadRoleData`, or moving `gsPlaying` on its own.

## Candidate divergence and safe lease boundary

| Candidate surface | Current observation | Decision |
|---|---|---|
| account request/load | absent from candidate inventory | separate future lease; protocol closure required |
| section-check request/response | enum exists but structs/handlers/registration are absent | separate future lease; target registration and owner closure required |
| ext-data completion | no matching method/field/registration | separate future lease; field/init and producer required |
| ready ACK/registration surface | target handlers/types are independently pinned | may receive a narrow static lease after constructor export and `KPlayer` layout comparison |
| direct candidate `gsPlaying` in `OnLoadRoleData` | differs from target intermediate gate | must not be duplicated or silently retained in an entry patch |

## Required next evidence

1. Export the complete target constructor registration rows for IDs 2 and 4,
   including handler addresses and entry object ownership.
2. Pin target account request/response protocol IDs, payload sizes, producer,
   consumer and persistence boundary.
3. Reopen target DWARF/decompile for `m_bExtDataLoadFinish` field offset,
   constructor/init write and the state producer for `gsWaitForSyncClientData`.
4. Compare candidate compiled `KPlayer` layout before adding fields.
5. Capture stock/candidate selected-role traffic when Windows is available:
   account response, section-check request/response, completion signal and
   ready ACK, with active binary hashes.

## Current decision

This dossier closes the static account/ext-data research gate enough to split
the implementation leases, but not enough to implement account persistence or
the ext-data producer. The next permitted implementation unit is the narrow
ACK/registration surface only, after its constructor/layout export and the
card-scoped baseline manifest are recorded. Account-data, section-check and
`m_bExtDataLoadFinish` remain `researching`.

## Direct evidence reopen audit (2026-07-27)

This dossier was re-opened against the current artifacts instead of relying
only on the earlier entry dossier.

| Probe | Direct result | Disposition |
|---|---|---|
| `nm jx3_dwarf/SO3GameServerD \| c++filt` for account/ext symbols | Confirms `DoLoadAccountDataRequest @ 0x080cfe8a`, `OnLoadAccountData @ 0x080e0f06`, `KPlayer::LoadAccountData @ 0x08399acc`, `OnExtDataLoadFinish @ 0x0839fb50`, and section request/response handlers at `0x0805c2b8`/`0x08079c9a` | `TARGET_STATIC_CONFIRMED` for symbol existence/address |
| `evidence_query.py function` for the same account/ext symbols | Current GraphEngine CLI returned no rows/output for these names | Graph closure is `UNRESOLVED`; no Graph result is claimed here |
| Saved PyGhidra role-load export | Contains raw target symbol records for `DoLoadAccountDataRequest` and `OnExtDataLoadFinish`, but those records have empty caller/callee arrays | Symbol export is direct static evidence; call closure remains unresolved |
| Saved target DWARF protocol/type rows cited above | Pins section-check sizes/IDs but does not pin the account-data payload ID/size | Section-check = `TARGET_STATIC_CONFIRMED`; account wire contract = `UNRESOLVED` |

The dossier therefore distinguishes **binary/DWARF symbol proof** from
**GraphEngine call/protocol closure**. The latter must not be reported as
confirmed until the importer/query surface returns a saved evidence row.
