# Wave 1 Mermaid target-evidence audit

**Scope.** Read-only audit of the four Mermaid views in
`port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/WAVE-ATLAS.md`. The audit uses only
the target v2.5.2 artifacts and saved GraphEngine/PyGhidra/DWARF evidence:

- `jx3_dwarf/SO3GameServerD` (SHA-256
  `47716c73e8de281c95759cdc4a478e70e7c61322fb46e8c4e04954e51124b94a`);
- `jx3_dwarf/SO3GameCenterD` (SHA-256
  `655b28126b972d1b3e1b3c02b827356c4d2ad6cf421b258152b130ec3c214d79`);
- target decompile/call-closure exports and DWARF records cited by the W1
  dossiers;
- GraphEngine snapshot `25e726bf747b3e6ecfe7c045bd7c1a1e1c22610a4ab917cbc37a93f4d45449ba`.

Source/candidate observations are deliberately not used to prove target
edges. They are mentioned only where the Atlas currently labels a divergence.
Runtime observations are not substituted for static target evidence.

## Overall finding

The Atlas diagrams are useful architecture views, but several arrows currently
state a stronger order or ownership relation than the target evidence proves.
The largest issues are:

1. the sequence diagram presents an unproven `Gateway → Center → GS` role
   envelope and a direct `Center → DB` path as if both were target call edges;
2. the state machine collapses target states and uses labels (`gsWaitForRoleData`,
   `sceneAttached`) that are not independently pinned by the cited target
   evidence;
3. the data-flow diagram puts `AddPlayer/KScene` after
   `OnExtDataLoadFinish` without proving that total order, and omits the
   separate account-data load consumer and the client ACK feedback boundary;
4. the feature dependency diagram is a planning relation, not a target
   execution relation; it is acceptable as a card map but should be explicitly
   labelled `planning dependency`.

No source edit or Atlas edit is authorized by this report.

## 1. Component / execution sequence audit

| Diagram edge/node | Target evidence | Verdict | Exact correction |
|---|---|---|---|
| `Client -> Gateway: authenticate and select existing role` | No GameServerD/CenterD evidence in the selected target corpus proves the external client-to-Gateway exchange. | **UNRESOLVED boundary** | Keep as an external trigger, but label `external/auth trigger (not GS target evidence)` or link a Gateway artifact. |
| `Gateway -> Center: account/role operation` | No cited GS/Center decompile proves this edge. | **UNRESOLVED** | Do not render as target-confirmed. Use a dashed `external handoff` edge. |
| `Center -> GS: player login envelope` | Center DWARF proves producer `KGameServer::DoPlayerLoginRequest @ 0x08084436`; GS-side consumer is named in the Relay dossier, while the exact protocol closure is still open. `R2S_PLAYER_LOGIN_REQUEST` is 302 bytes, `BaseInfo @ +0x82`, `KRoleBaseInfo` 162 bytes; target response is 38 bytes. | **TARGET STATIC CONFIRMED, closure partial** | Annotate edge with producer/consumer and artifact (CenterD/GS D); mark handler/ID closure partial rather than generic “envelope”. |
| `GS -> P: NewPlayer + LoadBaseInfo` | Target `LoadBaseInfo @ 0x0839dce2` and target login closure support base materialization; `NewPlayer` is not represented by a pinned address in this Atlas evidence. | **PARTIAL** | Split node: `target LoadBaseInfo @ 0x0839dce2`; leave `NewPlayer` as runtime-instance creation with evidence link, not a confirmed call edge. |
| `P --> GS: scene/position/GUID validation` | `LoadBaseInfo` calls `KSO3World::GetScene` using target base fields; GUID/permit predicates are not all pinned in the target dossiers. | **PARTIAL** | Say `LoadBaseInfo → GetScene (map/copy fields)`; do not combine GUID and scene validation into one target edge. |
| `GS --> Center: permit/endpoint response` | Center target response `S2R_PLAYER_LOGIN_RESPOND` size 38 is pinned; exact GS producer/endpoint semantics are not fully exported. | **PARTIAL** | Label `S2R_PLAYER_LOGIN_RESPOND (target size 0x26)` and mark endpoint/permit fields unresolved. |
| `Center --> Gateway: selected GS handoff` | Not proven by GS/Center target evidence. | **UNRESOLVED external** | Dashed external handoff or link Gateway artifact. |
| `Gateway --> Client: reconnect/switch to GS` | Not proven by selected target artifacts. | **UNRESOLVED external** | Keep as external trigger only. |
| `Client -> GS: secure handshake and enter-scene request` | Target `OnApplyEnterScene @ 0x0807a300` and the ready protocol are pinned; security handshake transport is not pinned by these dossiers. | **PARTIAL** | Split into `secure transport (unresolved here)` and `C2S_APPLY_ENTER_SCENE → OnApplyEnterScene @ 0x0807a300`. |
| `GS -> Center: account data + role data requests` | Target `DoLoadAccountDataRequest @ 0x080cfe8a` is called by `OnApplyEnterScene`; target `DoLoadRoleDataRequest` is also recorded. | **TARGET STATIC CONFIRMED** | Keep, but show two separate arrows: account request and role-data request. They are distinct persistence spines. |
| `Center -> DB: versioned role blocks` | The selected GS/Center evidence proves peer protocol/data ownership, not a direct DB call from this execution path. | **UNRESOLVED implementation boundary** | Rename to `Center/Relay persistence producer (DB internals not target-proven here)` or dashed edge. |
| `DB --> Center: role block stream` | Same limitation; block stream is a protocol/data contract, not a proven DB call edge. | **PARTIAL** | Use `persistence artifact → Center/Relay role-block producer`, not `DB` as a confirmed component. |
| `Center --> GS: chunks and load completion` | Target role-load dossier pins `OnLoadRoleData @ 0x080e110c`; section-check target handlers are pinned. Exact completion packet/ID closure remains incomplete. | **TARGET STATIC CONFIRMED, protocol partial** | Annotate `R2S role chunks/completion (exact completion ID unresolved)` and include section-check request/response as separate boundary. |
| `GS -> P: assemble, Load, section-check/ext-data completion` | `KPlayer::Load @ 0x083a02b8`, `PartialLoadExtData`, `OnExtDataLoadFinish @ 0x0839fb50`, and `OnSyncRoleDataSectionCheckRespond @ 0x08079c9a` are target-backed. | **TARGET STATIC CONFIRMED, order constrained** | Draw two branches: role buffer → `OnLoadRoleData`; ext-data section-check → `OnExtDataLoadFinish`. Do not imply all calls are one linear function. |
| `P -> P: attach scene and target ready gate` | `OnClientConfirmReady @ 0x08079dfc` requires `m_bExtDataLoadFinish` and `gsWaitForSyncClientData`; `OnClientReady @ 0x0839f87e` is pinned. Scene attach ownership is separate. | **PARTIAL / order ambiguous** | Separate `AddPlayer/GetScene` scene ownership from `OnClientConfirmReady → OnClientReady`. |
| `P --> C: role-over, initial state/object sync` | Target fan-out from `OnExtDataLoadFinish`/`OnClientReady` is partially known; exact body/order and role-over packet are unresolved. | **PARTIAL** | Label `target fan-out (body/order unresolved)` and link entry dossier. |
| `C --> GS: ready/ACK boundary` | `c2s_client_confirm_ready = 2`, header-only size 6, and handler address are target-pinned. | **TARGET STATIC CONFIRMED** | Name exact route and size: `C2S_CLIENT_CONFIRM_READY (6 bytes) → OnClientConfirmReady @ 0x08079dfc`. |
| `GS -> P: gsPlaying` | `OnClientReady @ 0x0839f87e` sets `gsPlaying`; exact field write should remain annotated as target decompile evidence. | **TARGET STATIC CONFIRMED** | Draw `OnClientConfirmReady → OnClientReady → gsPlaying`, not a generic `GS -> P` edge. |

### Missing target relationships in the sequence view

- `OnApplyEnterScene @ 0x0807a300 → DoLoadAccountDataRequest @ 0x080cfe8a → OnLoadAccountData @ 0x080e0f06 → KPlayer::LoadAccountData @ 0x08399acc`.
- `PartialLoadExtData → DoSyncRoleDataSectionCheckRequest @ 0x0805c2b8 → client section response → OnSyncRoleDataSectionCheckRespond @ 0x08079c9a`.
- `OnSyncRoleDataSectionCheckRespond → OnExtDataLoadFinish @ 0x0839fb50 → m_bExtDataLoadFinish`.
- `OnClientConfirmReady → OnClientReady`, including the guard `gsWaitForSyncClientData`.

## 2. State machine audit

| State/transition | Target evidence | Verdict | Correction |
|---|---|---|---|
| `[∗] → noPlayer` | Conceptual initial state; no issue. | **Architecture label** | Keep, mark conceptual. |
| `noPlayer → gsWaitForConnect: player login + NewPlayer/GUID` | Target player-login/base-info path is supported; exact state write and NewPlayer address are not fully cited. | **PARTIAL** | Add `target login request/base-info admission`; keep state label only where target field write is evidenced. |
| `gsWaitForConnect → gsWaitForPermit: secure handshake + Attach` | Attach/handshake names are present in architecture documents, but selected target dossier does not provide address/decompile for the exact transition. | **UNRESOLVED target edge** | Mark dashed/inferred until target handler and state write are exported. |
| `gsWaitForPermit → gsWaitForLoginLoading: Center permit/base sync` | Target permit response is known at protocol level; state transition address/order is not in current target evidence. | **PARTIAL** | Link Relay response dossier and mark transition static-partial. |
| `gsWaitForLoginLoading → gsWaitForRoleData: OnApplyEnterScene` | `OnApplyEnterScene @ 0x0807a300` is target-confirmed; exact enum/state assignment needs decompile citation. | **PARTIAL** | Annotate function/address; avoid asserting state write unless raw decompile line is linked. |
| `gsWaitForRoleData → gsWaitForSyncClientData: Load + ext-data/section gate` | `OnSyncRoleDataSectionCheckRespond @ 0x08079c9a` and `OnClientConfirmReady` guard `gsWaitForSyncClientData` are target-backed. | **PARTIAL** | Make producer explicit: section-check response → `OnExtDataLoadFinish`/flag → state gate; do not merge `Load` with state transition. |
| `gsWaitForSyncClientData → gsPlaying: OnClientConfirmReady/OnClientReady` | Fully target-backed: handler `0x08079dfc`, `OnClientReady 0x0839f87e`, guard/transition. | **CONFIRMED target static** | Keep and add exact protocol `c2s_client_confirm_ready=2`, 6 bytes. |
| `gsWaitForConnect → detached: invalid GUID/session` | Detach on invalid input is not directly evidenced in the cited W1 target exports. | **UNRESOLVED** | Mark as candidate failure hypothesis or attach target detach decompile. |
| `gsWaitForPermit → detached: deny/timeout` | Permit denial may be target behavior, timeout cleanup is historical/uncited here. | **PARTIAL** | Keep deny only if target handler evidence is linked; label timeout unresolved. |
| `gsWaitForRoleData → detached: stream/parser/scene failure` | Parser/load failure paths are plausible, but a combined target detach edge is not pinned. | **PARTIAL** | Split parser failure, scene failure, and detach; each needs evidence or be marked inferred. |
| `gsWaitForSyncClientData → detached: ready/region/ACK failure` | `OnClientConfirmReady` decompile logs/detaches on guard failure; exact region-failure path is target-backed in entry dossier, ACK timeout is not. | **PARTIAL** | Keep guard failure/region failure as target-backed; mark timeout/ACK-loss separately unresolved. |
| `gsPlaying → detached: logout/kick/disconnect` | Not covered by current target evidence. | **UNRESOLVED outside W1 entry closure** | Either omit from W1 state diagram or label non-goal/external lifecycle. |

### State diagram correction

The current labels `gsWaitForRoleData` and `sceneAttached` are useful
architecture names but must not be read as proven target enum values unless the
state-field writes are linked. The target-proven gate is specifically
`gsWaitForSyncClientData` plus `m_bExtDataLoadFinish`, followed by protocol 2
and `OnClientReady`.

## 3. Data and ownership flow audit

| Node/edge | Target evidence | Verdict | Correction |
|---|---|---|---|
| `Center login envelope → OnPlayerLoginRequest` | Center producer and 302-byte request are target-pinned; GS consumer closure is partial but named. | **CONFIRMED static / partial closure** | Include protocol `R2S_PLAYER_LOGIN_REQUEST`, size 302, `BaseInfo +0x82`. |
| `OnPlayerLoginRequest → NewPlayer runtime instance` | Runtime instance creation is architectural target fact; exact address not pinned in current evidence. | **PARTIAL** | Add evidence link and avoid asserting a specific call edge. |
| `NewPlayer → KROLE_POSITION_DB / LoadBaseInfo` | `LoadBaseInfo @ 0x0839dce2` and target position layout are proven; `NewPlayer → KROLE_POSITION_DB` is an ownership simplification. | **PARTIAL** | Represent `R2S base-info → LoadBaseInfo → KROLE_POSITION_DB fields`, not direct NewPlayer ownership. |
| `KROLE_POSITION_DB / LoadBaseInfo → GetScene` | Target `LoadBaseInfo` calls `GetScene`; offsets/layout are DWARF-backed. | **CONFIRMED target static** | Annotate `KSO3World::GetScene`, and include `nCenterIndex @ 0`, map/copy offsets. |
| `Account data request/load → NewPlayer` | Target account path is `DoLoadAccountDataRequest → OnLoadAccountData → LoadAccountData`; it does not prove direct producer-to-NewPlayer edge. | **MISLEADING** | Draw separate account spine ending in `KPlayer::LoadAccountData`; merge only at player account state. |
| `Role blocks → KRelayClient buffer` | Target role chunk accumulation is supported; exact buffer owner/field should link role dossier. | **CONFIRMED static, partial fields** | Add `OnSyncRoleData` and offset/total checks. |
| `buffer → KPlayer::Load` | Target `OnLoadRoleData @ 0x080e110c` call closure includes `KPlayer::Load`; target-backed. | **CONFIRMED static** | Keep, annotate address. |
| `KPlayer::Load → PartialLoadExtData + section checks` | Target `KPlayer::Load` calls/relates to ext-data loading; section-check producer/consumer is separately pinned. | **CONFIRMED, not one linear edge** | Branch `Load → PartialLoadExtData` and `PartialLoadExtData → DoSyncRoleDataSectionCheckRequest`; response returns to `OnSyncRoleDataSectionCheckRespond`. |
| `PartialLoadExtData → OnExtDataLoadFinish` | Target response handler calls both; `KPlayer::Load` also appears as caller in Graph closure. | **CONFIRMED with order caveat** | Show a gate/alternate completion path, not an unconditional sequential arrow. |
| `OnExtDataLoadFinish → AddPlayer / KScene` | Target static closure proves `OnExtDataLoadFinish` fan-out, but current evidence does not prove `AddPlayer` is its callee or that it occurs after it. | **UNSUPPORTED ORDER/OWNERSHIP** | Remove direct arrow or replace with `scene ownership/initial-sync fan-out` dashed relation until call edge is exported. |
| `AddPlayer / KScene → ready gate + initial sync` | Target `OnClientReady` performs ready work and scene region validation; `AddPlayer` ownership is separate. | **PARTIAL** | Separate scene attachment from ready gate; do not make AddPlayer a predecessor of ready without a pinned edge. |
| `ready gate + initial sync → client and gsPlaying` | `OnClientReady → gsPlaying` is target-confirmed; client packet body/order is unresolved. | **PARTIAL** | Keep `OnClientReady → gsPlaying`; draw client output as target fan-out with unresolved body/order. |

### Omitted target data-flow relationships

```text
OnApplyEnterScene @ 0x0807a300
  ├─ DoLoadAccountDataRequest @ 0x080cfe8a
  │    → OnLoadAccountData @ 0x080e0f06
  │    → KPlayer::LoadAccountData @ 0x08399acc
  └─ DoLoadRoleDataRequest

PartialLoadExtData
  → DoSyncRoleDataSectionCheckRequest @ 0x0805c2b8
  → S2C_SYNC_ROLE_DATA_SECTION_CHECK_REQUEST (3 bytes, bySectionType @ +2)
  → C2S_SYNC_ROLE_DATA_SECTION_CHECK_RESPOND (6 bytes)
  → OnSyncRoleDataSectionCheckRespond @ 0x08079c9a
  → OnExtDataLoadFinish @ 0x0839fb50
  → m_bExtDataLoadFinish

C2S_CLIENT_CONFIRM_READY (ID 2, 6 bytes)
  → OnClientConfirmReady @ 0x08079dfc
  → OnClientReady @ 0x0839f87e
  → gsPlaying
```

This is the minimum target-backed data-flow correction; it is not a full
GraphEngine dump.

## 4. Feature dependency graph audit

The feature graph is not a binary call graph. It is a wave planning relation,
so target addresses cannot prove or disprove every arrow. The following are
architecturally reasonable but should be labelled explicitly as **planning
dependencies**, not target execution edges:

| Edge | Target relation available? | Verdict | Correction |
|---|---|---|---|
| `FND-RUNTIME-INITIALIZATION → FND-CLIENT-GS-SECURE-TRANSPORT` | Runtime/DSO initialization is a prerequisite concept; no single target call edge needed. | **Valid planning dependency** | Add legend `planning dependency; not a target call edge`. |
| `FND-RUNTIME-INITIALIZATION → FND-RELAY-ROLE-ENVELOPE` | Same. | **Valid planning dependency** | Keep with legend. |
| `FND-CLIENT-GS-SECURE-TRANSPORT → FND-RELAY-ROLE-ENVELOPE` | Transport must carry the envelope, but this is not a direct target function edge. | **Valid planning dependency** | Keep, label contract dependency. |
| `FND-RELAY-ROLE-ENVELOPE → FND-ROLE-LOAD-SPINE` | Target request/role stream sequence supports this boundary dependency. | **Valid planning dependency** | Keep; link Relay and role-load dossiers. |
| `FND-ROLE-LOAD-SPINE → PLY-ENTER-WORLD` | Target ready gate requires ext-data completion; this is target-backed as a contract dependency. | **Valid planning dependency** | Keep; annotate `m_bExtDataLoadFinish + gsWaitForSyncClientData`. |
| `FND-CLIENT-GS-SECURE-TRANSPORT → PLY-ENTER-WORLD` | Client ready protocol is transported over the client socket; contract dependency, not call edge. | **Valid planning dependency** | Keep with protocol/transport label. |
| `FND-RELAY-ROLE-ENVELOPE → PLY-ENTER-WORLD` | Account/role data is a prerequisite input; exact runtime ordering needs capture. | **Valid planning dependency, runtime partial** | Keep but mark static contract dependency. |

The graph omits a useful planning node/edge for the separate
`ACCOUNT-DATA-SPINE` inside `FND-ROLE-LOAD-SPINE`. It need not become a new
card, but the Atlas should show it as a sub-boundary because target evidence
has separate request/load functions and it is not interchangeable with role
blocks.

## Target-backed minimum architecture

The diagrams should converge on this target-backed shape (labels can remain
high-level; addresses belong on the edges or in the adjacent table):

```text
R2S_PLAYER_LOGIN_REQUEST (CenterD producer, 302 bytes)
  → GS admission / LoadBaseInfo @ 0x0839dce2
  → OnApplyEnterScene @ 0x0807a300
      ├─ account request @ 0x080cfe8a
      │    → account response @ 0x080e0f06
      │    → LoadAccountData @ 0x08399acc
      └─ role-data request / chunk accumulation
           → OnLoadRoleData @ 0x080e110c
           → KPlayer::Load @ 0x083a02b8
           → PartialLoadExtData
                → section-check request @ 0x0805c2b8
                → section-check response @ 0x08079c9a
                → OnExtDataLoadFinish @ 0x0839fb50
                → m_bExtDataLoadFinish
  → gsWaitForSyncClientData
  → C2S_CLIENT_CONFIRM_READY (ID 2, 6 bytes)
  → OnClientConfirmReady @ 0x08079dfc
  → OnClientReady @ 0x0839f87e
  → gsPlaying + target initial-sync fan-out
```

This is a target evidence index, not a claim that the complete runtime packet
order or all indirect calls are recovered.

## Audit conclusion

The Atlas is usable as a high-level architecture map after the following
semantic corrections are made by the Atlas owner:

1. separate account-data and role-data branches;
2. show section-check request/response and `m_bExtDataLoadFinish` ownership;
3. avoid asserting `OnExtDataLoadFinish → AddPlayer` or a total order not
   present in target call closure;
4. show the exact ready route and guard;
5. label Gateway/DB arrows as external or persistence boundaries unless their
   target artifacts are linked;
6. label the feature dependency Mermaid as planning dependencies, not call
   graph;
7. retain unresolved/partial markers for wire body/order, active runtime
   provenance, and unexported state writes.

The report does not declare Wave 1 complete and does not authorize source
changes.
