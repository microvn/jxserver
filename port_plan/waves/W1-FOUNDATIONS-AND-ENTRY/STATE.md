---
wave_id: "W1-FOUNDATIONS-AND-ENTRY"
status: "implementing"
objective: "Wave 1: establish target-backed runtime initialization, transport, Relay role envelope, versioned role load, and bounded enter-world contracts. Entry implementation is batched from a target-first static closure; runtime acceptance remains separately required."
repo_root: "/Volumes/ExData/game/jx3"
source_tree: "/Volumes/ExData/game/jx3/linux-build"
source_head: "4e60f1697bf71f2dd46ff390f24f3a6a357e300f"
source_branch: "main"
producer_model: "codex"
reviewer_model: "claude"
created_utc: "2026-07-24T16:28:19Z"
updated_utc: "2026-07-28T02:45:00Z"
next_action: "Run one selected-role attempt against candidate SHA 8d529ff477a76eca500d123bde9d58ebce3ca3db44cb27b6607b6fc22366ef52 and capture whether Center accepts protocol 57 role-load at the target size without disconnect; then clean temporary W1 diagnostics before acceptance."
---

# W1-FOUNDATIONS-AND-ENTRY — persistent state

## Role ledger

| Role | Model family | Agent/session | Scope | Write authority |
|---|---|---|---|---|
| Orchestrator | codex | `/root` | Wave state, card boundaries, integration | State only |
| Evidence researcher | codex | `/root` | FND runtime static evidence; no source writes | Read-only |
| Producer/builder | codex | `/root` | B1 account Relay/parser isolated overlay | B1 files only |
| Cross-reviewer | claude | foreground TUI review: `REQUEST_CHANGES` | Adversarial read-only evidence/diff review | Read-only |
| Runtime verifier | codex | `/root` | Authorized stock/candidate measurements only | Runtime only; no deployment in this checkpoint |

## Acceptance contract

### Must pass

- [x] `FND-RUNTIME-INITIALIZATION`: record a fresh stock boot arm with target
  executable/config/data identities and named milestones through `Load game
  settings ... [OK]` or a target-proven earlier stop.
- [ ] Pin the target static initialization closure rooted at
  `KSO3World::Init(IRecorderFactory*)@0x0818f592`, including the selected
  settings/table/Lua/DSO consumers used by the boot contract.
- [ ] Before any implementation claim, capture the same normalized boot
  milestones and named fingerprints for a hash-pinned candidate arm.
- [x] Existing P3 candidate arm reaches the target foundation milestones with
  `25627` AI and `6161` skills; source/compiler provenance still needs the
  card manifest and review.
- [x] B1--B4 historical overlay has a card-scoped isolated candidate, target
  evidence package, and Codex-producer / Claude-reviewer assignment.
- [x] Claude issued `REQUEST_CHANGES`; rejected-overlay blockers are recorded
  in `evidence/B1_B4_CLAUDE_REVIEW_2026-07-26.md`.

### Must stay unchanged

- [ ] The stock `SO3GameServer` SHA-256 remains
  `3002bf4ad08d4c0fb9d4fc10f2d66b1f6cd8f70d398ff792136a3b7d5c416f11`.
- [ ] The target DWARF `SO3GameServerD` SHA-256 remains
  `47716c73e8de281c95759cdc4a478e70e7c61322fb46e8c4e04954e51124b94a`.
- [x] Canonical local source, target data and running process remain untouched.
- [x] Existing dirty worktree changes were snapshotted into an isolated B1
  baseline; the B1 delta is applied only to isolated local/VPS candidate trees.

### Non-goals

- Client entry, Relay envelope semantics, role-load semantics, scene-ready,
  AI behavior, and any source patch are separate Feature Cards or later W1
  steps. This checkpoint makes no cold-boot parity or gameplay claim.

### Execution constraint

- A Windows-client oracle is available for the final paired scenario, but has
  not yet been applied to this new integrated candidate. Static proof does not
  substitute for protocol/runtime acceptance.
- Each public card now records `evidence_state` and `runtime_constraint`.
  `researching` replaces `live-verifying` for transport/Relay because their
  prior capture window expired without a selected-role event.  The next client
  session must run their named paired captures before Wave acceptance.

### Rollback

- Candidate: none deployed or built by this Wave; the local observed rebuilt
  binary SHA is `f16725f5296ecb840612e85a240de43147a71c6dd1f9ece4b3a4b8b68c225259`.
- Rollback artifact: untouched target stock `SO3GameServer` SHA-256
  `3002bf4ad08d4c0fb9d4fc10f2d66b1f6cd8f70d398ff792136a3b7d5c416f11`.

## Baseline

| Item | Value | Source/path | Timestamp/hash |
|---|---|---|---|
| Target DWARF | `SO3GameServerD`, SHA `47716c73…51124b94a` | target static contract | recorded 2026-07-24T16:06Z |
| Stock runtime binary | `SO3GameServer`, SHA `3002bf4a…5c416f11` | target runtime arm on native Linux x86 | captured 2026-07-24T16:47Z |
| Observed local rebuilt binary | `SO3GameServer`, SHA `f16725f…8c225259` | candidate clue only; `OBSERVED_NOT_LINKED` to current source | recorded 2026-07-24T16:34Z |
| Source HEAD | `4e60f1697bf71f2dd46ff390f24f3a6a357e300f`, branch `main` | baseline commit anchor; worktree is dirty | recorded 2026-07-24T16:28Z |
| Canonical P3 source baseline | `1516b8b0832bb071f9d3cc0e773136a6e0962208`; 19-file manifest passes | isolated detached worktree; provenance record `evidence/CANONICAL_P3_SOURCE_BASELINE.md` | verified 2026-07-27; foundation only, not enter-world acceptance |
| Graph DB | SHA `25e726bf747b3e6ecfe7c045bd7c1a1e1c22610a4ab917cbc37a93f4d45449ba` | static evidence substrate only | recorded 2026-07-24T16:33Z |
| Stock boot log | SHA `402a5943…2b882d015` | native Linux x86, target binary above | captured 2026-07-24T16:47Z |
| Candidate P3+transport runtime binary | SHA `0df22836…782833a` | active `/deploy/SO3GameServer_w1_p3_candidate` on native Linux x86 | captured 2026-07-24T17:12Z |
| Candidate P3+transport boot log | SHA `5368d512…7209ef93` | same config/DSO/data arm | captured 2026-07-24T17:12Z |
| Known valid errors | target reaches boot success despite .tab optional-column and DropList fingerprints | target runtime log above | captured 2026-07-24T16:47Z |

## Evidence ledger

| Claim | Evidence type | Address/file:line | Consumer | Confidence | Acceptance check |
|---|---|---|---|---:|---|
| `KSO3World::Init(IRecorderFactory*)` is a target root at `0x0818f592` | target symbol + decompile-backed call observation | DWARF binary SHA above; Graph symbol evidence `29992`, call evidence `30016` | FND runtime closure | 1.00 | reopenable graph/dwarf evidence |
| `KSO3GameServer::Init` calls target `KSO3World::Init` | target callsite | caller `0x08050b0c`, callsite `0x08050d71`, evidence `30016` | process-to-world boundary | 1.00 | graph caller query |
| Target `KSO3World::Init` directly invokes settings/script/manager initialization, including `KWorldSettings::Init`, `KScriptCenter::Init`, `KAIManager::Init`, `KSkillManager::Init`, `KDropCenter::Init` | target decompile/direct-call inventory | target decompile evidence `22427`; call closure evidence `30016` | select bounded loader consumers | 0.95 | decompile order audit still required |
| Candidate `KSO3World::Init` is at `src/SO3World/Src/KSO3World.cpp:71`, SHA `ebb412ad…dcf3f60a`; candidate `KWorldSettings::Init` is `KWorldSettings.cpp:8`, SHA `367b2778…a66b8ec9b` | current candidate source observation | source files | candidate comparison | 1.00 | file hash before any lease |
| Graph reports exact qualified-name reconciliation only; semantic equivalence is unresolved | graph reconciliation | function dossier static-semantic claim | prevents source-to-target assumption | 1.00 | blocks semantic port claim |
| Runtime observations are absent from Graph | Graph runtime status | `UNRESOLVED` | requires stock/candidate capture | 1.00 | paired runtime arm |
| Target runtime reaches loader, Center connection, listener, startup, and map-create milestones | native Linux x86 stock probe | target binary `3002bf4a…`, log `402a5943…` | runtime baseline | 1.00 | fresh target log contains each named `[OK]` milestone |
| Candidate P3 reaches loader, Center connection, listener, startup, and map-create milestones | native Linux x86 candidate probe | candidate `1854b8aa…`, log `cc553868…` | FND runtime comparison | 1.00 | fresh candidate log contains each named `[OK]` milestone and `25627/6161` counts |
| Candidate P3 residual Lua fingerprints are known historical residuals | runtime log + Wave 3 ledger | `SetBOT` and bounded `GetActivityMgrServer` errors | non-fatal residual classification | 0.95 | reviewer confirms no new init regression |
| VPS file previously named `SO3GameServer` is not this target artifact | hash comparison | VPS named file SHA `0048aad…784cf4b6`; target SHA above | prevents false stock baseline | 1.00 | never use VPS named file as target oracle without provenance |
| Required V3 probe path in the skill is absent after GraphEngine relocation | filesystem/doc audit | active Graph docs are `graphengine/docs/*`; only archived copy of the named V3 report exists | documentation-path discrepancy; does not invalidate direct query evidence | 1.00 | do not cite archived report as current gate |
| Target has `OnClientConfirmReady @0x08079dfc` and `KPlayer::OnClientReady @0x0839f87e`; confirm-ready requires ext data plus `gsWaitForSyncClientData` and otherwise detaches | DWARF symbol, graph target call closure, direct PyGhidra decompile | entry dossier/raw target decompile | PLY enter-world gate | 1.00 | target route/state producer still needed before write |
| Target has `OnApplyCharacter` and three `OnSyncNew*Respond` acknowledgement handlers with target wire layouts; target constructor registers these handlers | DWARF types/symbols, graph constructor call observations, direct PyGhidra decompile | entry dossier/raw | PLY bounded initial-sync closure | 1.00 | target registration/order still needs direct constructor confirmation |
| Current candidate source has older `c2s_apply_enter_scene` only and lacks declarations, registrations, implementations, and KPlayer acknowledgement state for the target entry handlers | source inventory | `KPlayerServer.{h,cpp}`, `KPlayer.{h,cpp}` | candidate drift hypothesis | 1.00 | source diff must be reviewed; no write lease exists |
| Candidate security key handoff is correct for the captured selected-role session | direct candidate ptrace watcher + server key-frame inversion | candidate Init/Recv `0x082acf10` / `0x082ad290`; capture SHA `5033f9d0…a6dbd19` | excludes swapped/lost receive-send key theory | 1.00 | reopen `evidence/TRANSPORT_KSG_LOOP_INDEX_ROOT_CAUSE_2026-07-27.md` |
| Target/candidate `KSG_DecodeEncode` both start at `word_count-1`; original off-by-one is false | Claude raw-binary review + bit-exact randomized comparison | target `0x083cc0dc`; candidate `KSG_EncodeDecode.o @0x082b1070` | excludes unsafe codec patch | 1.00 | `evidence/CLAUDE_KSG_CODEC_REVIEW_2026-07-27.md` |
| Target/candidate `KG_AsyncSocketStream::Recv`, `KG_Packager::PackUp`, and `KG_Package_RecvBuffer` have the same broad framing sequence; no framing patch is proven | target/candidate disassembly comparison | `evidence/transport/target_async_recv_20260728.txt`, `candidate_async_recv_20260728.txt`, `target_packup_recvbuffer_20260728.txt`, `candidate_packup_recvbuffer_20260728.txt` | narrows frontier past raw socket framing | 0.90 | retain static-only status until runtime decode is observed |
| Target/candidate `KPlayerServer::CheckPackage` differ in protocol upper bound (`0xd0` vs `0xdc`), size/handler table bases, and surrounding `ProcessPackage` state offsets | target/candidate disassembly comparison | `0x08052640` / `0x081e2efa`, callers `0x0805432a` / `0x081e2bc4`; dossier `evidence/transport/KPLAYER_CHECKPACKAGE_DRIFT_2026-07-28.md` | identifies a concrete post-receive contract boundary | 1.00 static; runtime predicate unresolved | bounded diagnostic must record decoded ID/length and failed predicate before edit |

## Patch scope

- Worktree/branch: `/Volumes/ExData/game/jx3/.worktrees/w1-b1-account`, detached
  from `4e60f16` then overlaid with the pre-existing relevant local candidate
  edits. Remote build copy: `/root/jx3/linux-build-w1-b1`.
- Canonical successor: `.worktrees/canonical-w1-p3` is pinned at `1516b8b` and
  verified by its accepted 19-file manifest. `.worktrees/overlay-w1-entry`
  (`w1-entry-overlay`) contains the byte-identical B1--B4 patch from the
  isolated historical tree. Its 11 permitted paths and raw patch hash are in
  `evidence/OVERLAY_W1_ENTRY_FILES.txt`; review/build/runtime remain open.
- Backup: canonical source and `/root/jx3/linux-build` remain untouched; remote
  candidate was copied before patching.
- Producer-owned files: `Relay_GS_Protocol.h`, `KRoleDBDataDef.h`,
  `KRelayClient.{h,cpp}`, `KPlayer.{h,cpp}`, and `KRegressionPlayerData.{h,cpp}`.
- Encoding constraints: any future legacy-source write must preserve byte
  encoding/newline style, use ASCII-only comments, and pass `check_legacy_bytes.py`.

## Review

- Reviewer verdict: `REQUEST_CHANGES` from foreground Claude TUI review. The
  earlier background sessions `043e0850` and `826a7878` lost their daemon, but
  this recorded verdict is independent and read-only.
- Raw diff/evidence path: `W1_STATIC_CLOSURE_MATRIX.md`,
  `entry/ACCOUNT_EXT_DATA_STATIC_DOSSIER.md`, target raw decompile export, and
  the isolated B1 overlay above.
- Blockers/caveats: the integrated historical B1--B4 overlay has a target Relay
  enum mismatch and a 4-byte candidate section-check packet where target is 3
  bytes; it must not be built or deployed. See
  `evidence/B1_B4_CLAUDE_REVIEW_2026-07-26.md`.

## Build

- Host/worktree: native x86 VPS `/root/jx3/linux-build-w1-b1`.
- Command/result: `./build.sh` completed; candidate artifact exists.
- Candidate SHA-256/build ID: `53c0b53035ee265b12cb7729122c88c73d92614cfc3b9c5adfbb21e703b4c09c`.
  This is the integrated B1–B4 isolated build; it is not deployed and not accepted.

## Runtime

- Deploy name/start UTC: no deployment by this Wave.
- Milestones: target log records `Load game settings ... [OK]`, Center
  connection, listener start, server startup, and map creation. It also records
  `19153` scripts searched, `25627` AI loaded, and `6161` skills loaded.
- Before/after evidence: stock target log `402a5943…`; candidate P3 log
  `cc553868…`; both reach foundation milestones with residual Lua fingerprints
  explicitly retained.
- Current live/rollback state: temporary stock probe removed; the pre-probe
  Wave 3 GS binary was backed up and its container is running again.

## Timing

| Phase | Start | End | Elapsed | One-time? |
|---|---|---|---|---|
| Setup | 2026-07-24T16:28Z | 2026-07-24T16:36Z | 8m | yes |
| Research | 2026-07-24T16:32Z | in progress | in progress | no |
| Patch | not started | not started | 0m | no |
| Review | not started | not started | 0m | no |
| Build | not started | not started | 0m | no |
| Runtime | not started | not started | 0m | no |

## Checkpoints

- 2026-07-24T16:28:19Z | scope | orchestrator | planned | State created; define contract and baseline next.
- 2026-07-24T16:31:00Z | contract | codex | researching | FND-RUNTIME-INITIALIZATION opened as the only active card. Target GS/DWARF and Graph DB hashes pinned; no source/build/deploy mutation. Next: fresh stock cold-boot capture.

- 2026-07-24T16:32:32Z | scope | codex | researching | Contract-ready validation passed; target DWARF/stock/Graph identities and no-write scope recorded in EVIDENCE_RUNTIME_INIT.md.

- 2026-07-24T16:50:23Z | runtime | codex | researching | Native Linux x86 target arm passed loader/Center/listener/startup/map milestones with target hash 3002bf4a and log hash 402a5943; temporary probe removed and Wave 3 GS restored.
- 2026-07-24T16:54:42Z | runtime | codex | researching | Existing P3 candidate reached 25627 AI/6161 skills, loader, Center, listener, startup, and map milestones; candidate hash 1854b8aa and log hash cc553868 recorded. Source/compiler manifest and opposite-model review remain open.

- 2026-07-24T16:57:19Z | runtime | codex | researching | Existing P3 candidate 1854b8aa reached 25627 AI/6161 skills and all foundation milestones; log cc553868 recorded; source/compiler manifest and Claude review remain open.

- 2026-07-24T17:01:45Z | review | codex | researching | Claude reviewer session b90fc821 started read-only but was stopped after no verdict; candidate P3 runtime evidence remains recorded and no reviewer edits occurred.
- 2026-07-24T17:06:38Z | provenance | codex | researching | Created `CANDIDATE_P3_PROVENANCE.md`; active `/proc/10/exe` matches candidate SHA `1854b8aa`, but the historical remote build tree has no Git/source closure, so provenance is explicitly `RUNNABLE_HASH_PINNED / OBSERVED_NOT_LINKED`, not fresh-build acceptance.
- 2026-07-24T17:08:00Z | transport | codex | researching | Created `EVIDENCE_TRANSPORT.md`. Historical transport artifact `f1a81adf` proves real `libcommon.a` security/key exchange and protocol dispatch, but it differs from active P3 `1854b8aa`; transport PASS is not copied to P3. Next: build or otherwise prove a combined P3+transport artifact, then rerun the bounded security oracle.
- 2026-07-24T17:10:23Z | build | codex | live-verifying | Rebuilt `/root/jx3/linux-build-wave3` with real `libs/libcommon.a`: `202/202`, fail 0, link exit 0, undefined refs 0, Build ID `66096d56159ab882a7e23a508c66de05dd252345`, candidate SHA `0df22836e682954d2606adef9b7ff6c0c4209ee783cdeeb540f194d27782833a`.
- 2026-07-24T17:12:17Z | runtime | codex | live-verifying | Deployed exact combined candidate under the existing supervisor path; active `/proc/9/exe` hash equals `0df22836`. Fresh log SHA `5368d512f29da7b2d2baccf1e3021a04fd6c07abad4e93d60d85a0267209ef93`; 25627 AI, 6161 skills, settings/listener/startup OK. Final real-client packet capture remains open.

- 2026-07-24T17:06:38Z | review | codex | researching | Created docs/waves/w1-foundations-and-entry/CANDIDATE_P3_PROVENANCE.md. Candidate 1854b8aa is active as /proc/10/exe and runtime milestones remain valid, but source reproducibility is explicitly NOT_PROVEN. Bounded Claude review attempt 31491e7a failed to produce a session/log; no verdict claimed.

- 2026-07-24T17:07:23Z | review | codex | researching | Transport evidence saved at docs/waves/w1-foundations-and-entry/EVIDENCE_TRANSPORT.md. Historical f1a81adf transport PASS is not copied to active P3 1854b8aa; combined artifact proof remains required.

- 2026-07-24T17:14:02Z | runtime | codex | live-verifying | Combined P3+transport build passed 202/202, link 0, undefined 0; active process /proc/9/exe matches 0df22836; fresh log 5368d512 reaches 25627 AI, 6161 skills, settings/listener/startup OK. EVIDENCE_TRANSPORT.md updated.

- 2026-07-24T17:21:47Z | runtime | codex | live-verifying | Updated public card frontmatter to reflect actual milestones: runtime=reviewing with provenance/review open; transport=live-verifying with packet capture open. No card marked accepted.

- 2026-07-24T17:24:53Z | runtime | codex | live-verifying | Audited FEATURE_ROADMAP.md and W1 WAVE.md; synchronized WAVE status=active and card table (runtime=reviewing, transport=live-verifying). Added hierarchy propagation rules to jx3-wave-port skill.

- 2026-07-24T17:27:10Z | evidence | codex | researching | Saved relay/RELAY_ROLE_ENVELOPE_DOSSIER.md from Wave2/3 evidence leads. Graph function query was attempted but did not complete on the local SQLite surface; no graph claim was made. Updated FND-RELAY-ROLE-ENVELOPE evidence_package and next_action.

- 2026-07-24T17:34:04Z | evidence | codex | researching | FND-ROLE-LOAD-SPINE dossier pins target LoadStateInfo version dispatcher (0x0839b112), LoadBaseInfo map consumer (0x0839dce2), KRoleBaseInfo size 162, and rbtTotal 49; graph query timeout preserved as missing coverage; active candidate 0df22836 revalidated.

- 2026-07-24T17:35:48Z | evidence | codex | researching | Relay peer now pinned to target Center DWARF SHA 655b2812: DoPlayerLoginRequest at 0x08084436, R2S_PLAYER_LOGIN_REQUEST typed size 302/BaseInfo offset 0x82, and S2R response size 38; historical 304-byte capture retained as unresolved framing.

- 2026-07-24T17:37:02Z | runtime | codex | live-verifying | Candidate Relay capture armed on native Linux for the active 0df22836 GameServer; tcpdump records Center-to-GS traffic on the Relay listener for five minutes. No source/config/deploy mutation.
- 2026-07-25T02:15:00Z | evidence | codex | researching | Windows-client oracle unavailable; expired capture is not treated as evidence. Pinned PLY static entry acknowledgement closure in `entry/ENTRY_STATIC_DOSSIER.md`: target confirm-ready/client-ready/initial-sync handlers exist while the candidate lacks their route/handler/state surface. Public cards, Wave manifest, template, and skill now distinguish active delivery status from strongest evidence and runtime constraint. No source/build/deploy mutation.

- 2026-07-24T17:50:20Z | evidence | codex | researching | Static-only constraint recorded; entry ACK dossier and synchronized Card/Wave/skill evidence-state policy added; no source/build/deploy mutation.
- 2026-07-25T03:05:00Z | audit | codex | researching | Completed W1 static audit under the explicit login-success/enter-world-failure hypothesis. Runtime-dependent cards remain unaccepted; no source/build/deploy mutation. Checkpoint: `docs/waves/w1-foundations-and-entry/W1_STATIC_AUDIT_CHECKPOINT.md`.

- 2026-07-24T18:17:21Z | evidence | codex | researching | docs/waves/w1-foundations-and-entry/entry/ENTRY_STATIC_DOSSIER.md
- 2026-07-28T00:40:00Z | runtime/build | codex | live-verifying | Capture localized the selected-role stall after Center permit: target sends `S2C_SYNC_PLAYER_BASE_INFO` size `0xa1` at `0x0807a2c7`, while candidate emitted 153 bytes and no `C2S_APPLY_ENTER_SCENE`. Added target-backed tail fields to the candidate producer, rebuilt natively, and deployed hash `78449eb32cc02c0d1cd03bd970e8f31ea572fa01bc0e85c51a348e0e16669546`. Next gate is one fresh selected-role capture; no acceptance claim yet.

- 2026-07-24T18:22:39Z | evidence | codex | researching | docs/waves/w1-foundations-and-entry/entry/ENTRY_STATIC_DOSSIER.md

- 2026-07-24T18:32:50Z | evidence | codex | researching | docs/waves/w1-foundations-and-entry/entry/ENTRY_STATIC_DOSSIER.md

- 2026-07-24T18:36:29Z | evidence | codex | researching | docs/waves/w1-foundations-and-entry/entry/ENTRY_STATIC_DOSSIER.md

- 2026-07-24T18:40:01Z | evidence | codex | researching | docs/waves/w1-foundations-and-entry/entry/ENTRY_STATIC_DOSSIER.md

- 2026-07-26T17:15:46Z | evidence | codex | researching | linux-build/docs/waves/w1-foundations-and-entry/entry/ACCOUNT_EXT_DATA_STATIC_DOSSIER.md

- 2026-07-26T17:19:30Z | evidence | codex | researching | linux-build/docs/waves/w1-foundations-and-entry/entry/ACCOUNT_EXT_DATA_STATIC_DOSSIER.md

- 2026-07-26T17:32:08Z | evidence | codex | researching | linux-build/docs/waves/w1-foundations-and-entry/W1_STATIC_CLOSURE_MATRIX.md

- 2026-07-27T00:00:00Z | implementation | codex | implementing | Completed the
  target-first B1 account Relay/parser overlay: request ID 58; account chunk/final
  routes 63/64; 76-byte account state and 22-byte regression block parser. Patch
  dry-run and actual apply succeeded only in `/root/jx3/linux-build-w1-b1`; native
  x86 build produced SHA `888aa11f…a606af41`. Claude review infrastructure emitted
  no verdict. No production source or runtime deployment changed.

- 2026-07-26T18:19:44Z | handoff | codex | implementing | Migrated the one live W1 State and all W1 evidence from linux-build/docs/waves/w1-foundations-and-entry into this canonical Wave package; historical checkpoint paths remain historical only.

- 2026-07-26T18:20:24Z | handoff | codex | implementing | Reconciled the public W1 manifest and PLY-ENTER-WORLD card to the private execution ledger: only B1 is compile-proven; B2–B4, acceptance review, deployment, and paired runtime measurement remain open.

- 2026-07-27T00:00:00Z | provenance | codex | implementing | Created isolated canonical P3 source worktree at `1516b8b`; the accepted Wave-1b manifest passes `19/19`. This replaces the unversioned remote P3 tree as the source baseline for future entry overlays. No candidate build/deploy/runtime claim changed.

- 2026-07-27T00:00:00Z | isolation | codex | implementing | Created clean `w1-entry-overlay` directly from canonical P3; its baseline manifest also passes `19/19`. It is intentionally empty until the historical B1 changes are minimized, evidence-pinned, and cross-reviewed.

- 2026-07-27T00:00:00Z | overlay | codex | implementing | Applied the exact 11-file B1--B4 patch from the isolated historical B1 worktree to `w1-entry-overlay`. Both raw diffs hash to `c479f26a…7934f23f`; `git diff --check` passes. This is source isolation/provenance only, not a new reviewer, build, deploy, or runtime result.

- 2026-07-26T19:29:44Z | review | claude | reviewing | Foreground Claude TUI completed an independent read-only review of the exact 11-file overlay and returned `REQUEST_CHANGES`. High-confidence blockers: candidate `KS2R_PROTOCOL` account request is 55 vs target 57; locally declared section-check request compiles to 4 bytes vs target 3; ready-gate removal is unproven. Report: `evidence/B1_B4_CLAUDE_REVIEW_2026-07-26.md`. No build/deploy/runtime action followed.

- 2026-07-27T02:45:00Z | implementation | codex | implementing | Created isolated `w1-entry-overlay-r1` from canonical P3 plus the byte-identical historical B1--B4 overlay. Corrected only the reviewer-confirmed static blockers: three target `KS2R_PROTOCOL` placeholders restore role/account request values to 57/58; the named section-check request is packed to target DWARF size 3. Mixed-encoding byte checks and `git diff --check` pass. Evidence: `evidence/R1_RELAY_ENUM_AND_SECTION_CHECK_STATIC.md`. No build/deploy/runtime claim; Claude R1 review is next.

- 2026-07-27T03:15:00Z | review | claude | implementing | Read-only Claude R1 review returned `PASS WITH CAVEATS` for exact diff `05e6094f…daf90b86`: five KS2R anchors match target DWARF; the section-check request is target size 3/field offset 2; nested `pack(push,1)/pop` preserves the surrounding header pack region. Pre-existing Relay tail drift, ready/account semantics, and runtime parity remain open. This authorizes native build only. Report: `evidence/R1_CLAUDE_REVIEW_2026-07-27.md`.

- 2026-07-27T04:00:00Z | runtime | codex | live-verifying | Reviewed R1 was built natively (`202/202`) and the active `jx3gs-wave3` process is `/deploy/SO3GameServer_w1_r1`, SHA-256 `deb22cbf…2c830661`. Boot reaches 25627 AI, 6161 skills, Center, listener, and map `(1,1)`. A real selected-role attempt reaches `OnPlayerLoginRequest`, then fails target-equivalent `LoadBaseInfo -> GetScene(CurrentPos.mapID, copy)` because the active topology contains only `(1,1)` while tested role rows include `(16,1)`. Target decompile confirms this handler has no GS-side fallback/create-map behavior. A reversible DB scene probe backed up all eight `BaseInfo` values in `role_baseinfo_before_sceneprobe_20260727` and temporarily set `CurrentPos`/`LastEntry` to known live `(1,1)` coordinates. This is environment diagnosis, not a source-port acceptance.

- 2026-07-27T02:22:01Z | runtime-baseline | codex | researching | Corrected stock provenance. The VPS plain-path `SO3GameServer` SHA `0048aad…` was a rebuilt debug executable, not the local target. Exact local stock SHA `3002bf4a…5c416f11` was launched from a byte-identical VPS backup after recoverably resetting GS-generated outputs. A launcher process-name mismatch initially restarted GS during boot; corrected supervisor stayed at zero restarts. Exact stock then booted, registered/map-created with Center, and the user authenticated, selected a role, and entered world. This invalidates all prior diagnoses that used the plain-path VPS file as stock evidence. See `evidence/EXACT_STOCK_RUNTIME_BASELINE_2026-07-27.md`.

- 2026-07-27T00:00:00Z | runtime-localization | codex | reviewing | A hash-pinned candidate selected-role capture and direct stream watcher prove `_SendSecurityKey`/`KG_AsyncSocketStream::Init` receive-send values are not swapped, lost, or overwritten. Target `KSG_DecodeEncode @0x083cc0dc` starts at `word_count`; candidate archive `KG_Socket.o @0x082b1070` starts at `word_count-1`, reproducing the garbage received by the player path. Saved `evidence/TRANSPORT_KSG_LOOP_INDEX_ROOT_CAUSE_2026-07-27.md`. No source/archive/build/deploy mutation. Next: Claude raw-binary review of a proposed isolated archive overlay.

- 2026-07-27T00:00:00Z | review | claude | researching | Claude session `a3303275` returned `FAIL`: target also decrements before the KSG loop body, so target/candidate codec behavior is bit-exact; the proposed six-byte patch would process an extra dword and write out of bounds. Corrected archive owner to `KSG_EncodeDecode.o`; no overlay/build/deploy was created. The temporary capture/replay artifacts are not retained locally, so their codec claim is withdrawn. Evidence: `evidence/CLAUDE_KSG_CODEC_REVIEW_2026-07-27.md`. Next: frame boundary/helper/call-count investigation.

## Resume packet

- Current facts: R1 is live with an exact native-build/process hash. The socket/framing comparison is broadly similar, while target/candidate `CheckPackage` and constructor registration/table layouts are materially different. The live predicate remains unknown.
- Next action: Run one selected-role attempt against candidate SHA 8d529ff477a76eca500d123bde9d58ebce3ca3db44cb27b6607b6fc22366ef52 and capture whether Center accepts protocol 57 role-load at the target size without disconnect; then clean temporary W1 diagnostics before acceptance.
- Read next: `entry/ENTRY_STATIC_DOSSIER.md`, `graphengine/docs/graphengine-operational-guide.md`, and the card `port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/cards/PLY-ENTER-WORLD.md`.
- External state to revalidate: source HEAD/dirty state, Graph DB hash, stock executable/data/config identity, any active process/client, and the current GraphEngine query surface.
- 2026-07-28T00:00:00Z | evidence | codex | researching | Compared target/candidate `KG_AsyncSocketStream::Recv`, `KG_Packager::PackUp`, `KG_Package_RecvBuffer`, `KPlayerServer::ProcessPackage`, and `KPlayerServer::CheckPackage`. Framing is broadly similar, but target/candidate packet validator and constructor registration/table regions differ materially (`0x08052640` vs `0x081e2efa`; target protocol bound `0xd0`, candidate `0xdc`; target/candidate table/state offsets differ). Raw disassemblies and `evidence/transport/KPLAYER_CHECKPACKAGE_DRIFT_2026-07-28.md` saved. No source/build/deploy mutation. Next: bounded runtime predicate diagnostic.
- 2026-07-28T00:00:00Z | runtime-localization | codex | reviewing | Fresh candidate capture/log proved handshake proto 1 and repeated ping proto 6 pass `CheckPackage`; no proto 3 or `OnConfirmPlayerLoginRespond` followed. Target DWARF/disassembly pinned `S2R_CONFIRM_PLAYER_LOGIN_REQUEST` as 10 bytes with `dwPlayerID@+2` and `dwIP@+6`; candidate sends a 6-byte envelope and calls the sender with one argument. Dossier: `evidence/relay/CONFIRM_PLAYER_LOGIN_REQUEST_DRIFT_2026-07-28.md`. Next: isolate/review this relay envelope only; do not change CheckPackage constants.
- 2026-07-28T00:00:00Z | build | codex | reviewing | Instrumentation-only `ProcessPackage/CheckPackage` build passed `204/204`, link exit `0`, undefined refs `0`; BuildID `23db311799f2bc429999dad7e7dedeb65717bc04`, SHA `d2ac0e73c42b5c51393e6d146cedec1c475949f144cc0672a0b180d49e1c8d33`. No semantic source claim.
- 2026-07-28T00:00:00Z | deploy | codex | reviewing | Replaced the active candidate GS after backup `.w1_diag1.before-process-20260727T174524Z`, restarted only `jx3gs-w1-entry`, and verified `/deploy/SO3GameServer_w1_fast_key` hash equals the diagnostic build. Runtime log and pcap show handshake/ping pass and stop before proto 3. Next: target-sized relay confirm overlay, opposite-model review required.
- 2026-07-28T00:00:00Z | deploy | codex | live-verifying | Capture at 18:44 showed the active binary still emitted 153 bytes. Audit found the VPS worktree header had reverted to the pre-tail declaration, so the previous 78449e... deploy was stale. Reconciled target `perRepresentCount=33` plus the seven base-info tail fields, rebuilt natively, and restarted only `jx3gs-w1-entry`; active `/deploy/SO3GameServer_w1_fast_key` is now `e607ee5414d835188f2083c797162e808b71383b9ee0aa66d1410865fcd24ddd`. Disassembly proves `Send(..., 0xa1)`. Next: one selected-role capture against this hash; require 161-byte base-info and subsequent C2S_APPLY_ENTER_SCENE.

- 2026-07-28T02:15:00Z | runtime-localization | codex | reviewing | Fresh capture after the 161-byte base-info fix reached the next boundary, then Center rejected `GS -> Center: 08 00 37 00 06 00 00 00`. Target DWARF `KS2R_PROTOCOL` proves ID 55 is `s2r_player_enter_scene_notify` and ID 57 is `s2r_load_role_data_request`; candidate disassembly of `DoLoadRoleDataRequest` allocated 6 bytes but wrote ID 55. Root cause is Relay enum drift, not socket framing or client wording. Evidence: `evidence/entry/RELAY_ENUM_DRIFT_2026-07-28.md`.

- 2026-07-28T02:25:00Z | implementation | codex | reviewing | Pinned target-backed Relay IDs 52--60 in `include/Include/Relay_GS_Protocol.h`, added target `s2r_load_account_data_request = 58`, and preserved the remote header's unrelated accepted changes. Remote backup: `/root/jx3/linux-build-w1-fast/.w1_backups/Relay_GS_Protocol.h.before-enum-drift-20260728`. The first Docker attempt was invalid (204 compile reports but only 56 objects at link); it was discarded and not deployed.

- 2026-07-28T02:42:00Z | build | codex | reviewing | Retried through a foreground native-x86 build session; compile `204/204`, object closure `204`, link exit `0`, undefined refs `0`, BuildID `d5805aba0a533386abb4869d41595ff57a0bd5b`, candidate SHA `8d529ff477a76eca500d123bde9d58ebce3ca3db44cb27b6607b6fc22366ef52`. Disassembly proves `DoLoadRoleDataRequest` writes protocol `0x39` (57). Temporary W1 diagnostic logging remains observability-only and is not an acceptance claim; clean-debug removal is a required post-diagnosis gate.

- 2026-07-28T02:45:00Z | deploy | codex | live-verifying | Backed up the prior active binary as `.w1_backups_active_fast_key.before-enum57-20260728`, replaced only the GS executable, restarted `jx3gs-w1-entry`, and verified `/proc/911803/exe` hash equals `8d529ff477a76eca500d123bde9d58ebce3ca3db44cb27b6607b6fc22366ef52`. Next action: one fresh selected-role capture; verify Center sees protocol 57 with the target role-load size and no `uPakSize` disconnect, then continue from the next first divergence. Do not mark the card complete; clean temporary diagnostics before final acceptance.

- 2026-07-28T04:44:00Z | runtime-provenance-correction | codex | researching | The first sequential stock capture was invalid because the VPS plain-path `SO3GameServer` was SHA `0048aad...`, a rebuilt/debug executable, while the pinned local/deploy stock is SHA `3002bf4ad08d4c0fb9d4fc10f2d66b1f6cd8f70d398ff792136a3b7d5c416f11`. The byte-identical stock was found at `root_partial_bak/SO3GameServer` and `SO3GameServer_w1_stockprobe`; the invalid capture is retained but must not be used as stock oracle. The VPS `0048...` file was backed up as `SO3GameServer.before-local-stock-20260728`, and the exact `3002...` stock is now active in `jx3gs` after live hash verification. This rule is now also encoded in the jx3-wave-port skill to prevent same-name stock confusion.

- 2026-07-28T04:54:00Z | runtime-probe-correction | codex | researching | A clean sequential pair was finally captured with candidate `1254f922...44c1` and exact stock `3002bf4a...5c416f11`; both arms reached all 6 coarse connectivity rungs. The first comparison conflict was a false positive caused by ephemeral client source ports being included in `state_fingerprint`; the normalizer now canonicalizes non-service ports as `ephemeral`. The final comparison is `UNRESOLVED` with no mechanical divergence: all rungs and packet lengths match, while encrypted payload SHA remains raw evidence but is no longer a comparison dimension for the coarse profile. This probe proves transport/connectivity parity only; it does not prove role-load or enter-world parity. Invalid earlier captures where candidate still owned port 3113 are excluded.
- 2026-07-28T09:33:00Z | overlay-build-deploy | codex | live-verifying | Rebased Role-State V2 onto the canonical `linux-build-w1-fast` baseline at `1516b8b`, preserving the accepted Relay/transport/ABI overlay. Build passed `204/204`, link undefined `0`; target preflight used regular file `/root/jx3/targets/SO3GameServerD`. Timestamped artifact `SO3GameServer_w1_fast_rolev2_20260728_093037`, SHA `7734d5a3e15eac674b06adda1b559fc2aa98b5972b28f2fa8c8cc530bf83d69a`, BuildID `c2c24dbb58013e48ec45608970afb035c4640901`. Active `/proc/<pid>/exe` matches this hash on port 3113. Runtime acceptance is pending; `w1_fast_key` SHA `1254f922...44c1` remains the rollback oracle.
- 2026-07-28T09:40:00Z | runtime-localization | codex | researching | Combined candidate reaches loading 100%, then first fatal role-load divergence is `KItemList::Load` leftover-bytes check for `ThatTu<6>`: item load fails, `LoadExtRoleData` fails, then `OnLoadRoleData` fails. Target DWARF/decompile proves `KItemList::LoadItemList(data,len,version)` with V0..V6 dispatch; legacy candidate calls unversioned `m_ItemList.Load(data,len)` and ignores `KRoleBlockHeader::dwVer`. No client/transport conclusion is drawn. Next diagnostic boundary is the actual item block `dwVer`/`dwLen`, followed by a target-backed version path only.

- 2026-07-28T09:55:00Z | journey-audit | codex | live-verifying | Clean sequential V6 stock/candidate capture completed. Exact stock `3002bf4a…5c416f11` and active candidate `6bf935a1…474357` are DISTINCT; both reach all 7 mechanical public-handoff rungs with identical packet lengths and state fingerprints. The encrypted payload SHA differs and is retained as raw evidence only. Journey Audit: `w1.v6.provenance.distinct_pair=PASS`, `w1.v6.runtime.public_handoff=PASS`, `w1.v6.static.role_item_block_v6=NOT_APPLICABLE`, `w1.v6.terminal.enter_world=NOT_APPLICABLE`. The latter two are not passes: the audit lacks a machine matcher for target/candidate V0–V6 parser semantics. Next is a target-backed, machine-readable role-item parser evidence artifact and matcher; enter-world remains unproven.

- 2026-07-28T17:43:00Z | runtime-provenance | codex | live-verifying | A requested service reset temporarily attempted exact stock `/deploy/SO3GameServer` (`3002bf4a…5c416f11`), but its GS process exited after `Connect to center 192.168.200.105:5003 [Failed]`; that arm produced no valid full-profile capture and is excluded. The stock container was preserved as `jx3gs-w1-stock-failed-20260728_174255`. The previously backed-up candidate container was restored as `jx3gs-w1-entry`; live `/proc` resolves to `/deploy/SO3GameServer_w1_itemv6_20260728_101810` (`6bf935a1…474357`). Ports 3113/5003/5004/9001 are listening. No stock/candidate parity claim changed.

- 2026-07-28T18:00:00Z | runtime-topology | codex | live-verifying | Runtime audit found no duplicate host-network GS/Center containers. The only topology defect was `jx3gw` exited, so port 5004 was absent while GS `6bf935a1…474357` and Center remained live. Started the existing `jx3gw` container; Gateway now owns 5004 and the four expected service ports 3113/5003/5004/9001 are listening. Active GS remains candidate V6, not stock.

- 2026-07-28T18:05:00Z | runtime-localization | codex | live-verifying | Fresh candidate log during a selected-role attempt reaches role-load, then fails at `GenerateItemFromBinaryData`/`KItemList`-owned item decoding for `ThatTu<6>`. It reports unexpected role blocks `(36,2)`, `(37,2)`, `(38,2)`, then `LoadSkillRecipeList` → `LoadExtRoleData` → `Role data error` → `OnLoadRoleData failed`. Subsequent `W1_CHECKPACKAGE_OK proto=6` entries are ping traffic only. Gateway/Center are healthy; this is the known V6 role-block/parser frontier, not a port collision or transport diagnosis.
