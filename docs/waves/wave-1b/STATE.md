---
wave_id: "wave-1b"
status: "complete"
objective: "Port the JX3 2010 GameServer toward v2.5.2 in bounded Wave 1b slices while preserving cold boot and measuring loader, AI, Lua, and runtime parity."
repo_root: "/Volumes/ExData/game/jx3"
source_tree: "/Volumes/ExData/game/jx3/linux-build"
source_head: "0cc7d263ebbd13547987623f47658bd7d6ff2938"
source_branch: "main"
producer_model: "codex"
reviewer_model: "claude"
created_utc: "2026-07-22T06:49:35Z"
updated_utc: "2026-07-22T09:08:13Z"
next_action: "none"
---

# wave-1b — persistent state

## Role ledger

| Role | Model family | Agent/session | Scope | Write authority |
|---|---|---|---|---|
| Orchestrator | codex | `/root` | State, integration, build/deploy coordination | State and authorized runtime |
| Producer/builder | codex | `/root` | P2 `IS_OTACTION_STATE` evidence, source patch, and native-x86 build | P2 lane-owned source file after scope approval |
| Cross-reviewer | claude | fresh Claude Code session | P2 contract, raw evidence, raw diff, and expected live delta | Read-only |
| Runtime verifier | codex | `/root` | Isolated build, layout probe, live oracle | Runtime only |

Current P2 assignment is Codex producer/builder and Claude accepting reviewer. Historical P1 used
the reverse pairing: Claude producer and Codex reviewer. Runtime verification remains with the
orchestrator and cannot replace Claude acceptance of the P2 patch.

P3 assignment: Codex producer/builder; fresh Claude Code session read-only accepting reviewer.
P3 starts with `KAIManager.cpp`/`KAIManager.h` ownership; any evidence-proven propagation file
requires a contract checkpoint and fresh Claude scope review before editing.

## Acceptance contract

### Must pass

- [x] P1 isolated build compiles `202/202`, links with exit zero and zero undefined refs.
- [x] P1 reaches settings, Center, service 3113, startup, and map `(01,1)` OK.
- [x] Fresh P1 initialization has zero `GetInteger`/`GetString` flood signatures.
- [x] Fresh P1 has zero `KSkill::nCostTrain` and `KSkill::bIsSunMoonPower` invalid-newindex errors.
- [x] P1 still loads 6,161 skills and has zero GameServer container restarts.
- [x] P2 target evidence pins `IS_OTACTION_STATE` action registration, parameter use, and success/failure branch semantics from v2.5.2.
- [x] P2 exposes `AI_ACTION.IS_OTACTION_STATE` as exact enum value 64 before `LuaNewAction` validates the key.
- [x] Corrected P2 full clean build compiles all expected units, links with exit zero, and has zero undefined refs.
- [x] Fresh P2 initialization has zero `nActionKey > eakInvalid` errors, zero `Failed to call function Setup, script: scripts/ai/StandardAI.lua`, and zero `Setup AIVM failed` errors.
- [x] P2 reaches settings, Center, service 3113, startup, and map `(01,1)` OK with zero GameServer restarts.
- [x] P2 still loads 25,414 AI entries and 6,161 skills; the known `SetBOT` nil fingerprint remains exactly 1.
- [x] P3 loads the v246 `AITypeList.tab` union without signedness/assert failure and records the exact loaded-row/file count against stock: 264 list rows / 279 extracted files, 25,627 AI loaded.
- [x] P3 reaches settings, Center, service 3113, startup, and map `(01,1)` with zero GameServer restarts.
- [x] P3 preserves the P2b action/Setup delta: `nActionKey`, StandardAI Setup, and AIVM Setup remain 0; skills 6,161 and SetBOT nil 1 remain unchanged.

### Must stay unchanged

- [x] P1 AI count remains 25,414 because AITypeList union is explicitly outside P1.
- [x] P1 retains the known 453 StandardAI/AIVM failures pending the P2 action port.
- [x] Stock `SO3GameServer` is never overwritten and `_ours` rollback remains available.
- [x] P2 preserves zero `GetInteger`/`GetString` flood signatures and zero KSkill `nCostTrain`/`bIsSunMoonPower` invalid-newindex errors.
- [x] P2 introduces no new fatal/error fingerprint outside the named `IS_OTACTION_STATE` delta.
- [x] P3 introduces no new unique ERROR fingerprint outside the pre-recorded P2b residual set.

### Non-goals

- P1 does not claim NPC or Item Set semantic parity; `SetErrorLog(FALSE)` is observability only.
- P1 does not port AITypeList, `IS_OTACTION_STATE`, late AI actions, or `SetBOT`.
- P1 does not claim full KSkill ABI parity after `m_BindBuffs@0x130`.
- Full NPC multi-tab, ItemHouse Set, all late AI actions, and full BOT remain full-Wave work.
- P2 ports only `IS_OTACTION_STATE`; `AITypeList.tab` union loading, `SetBOT`, `SEARCH_NPC`, and all other late AI actions remain deferred.
- P2 proves cold-boot action creation and target-backed branch semantics, not full encounter/gameplay AI parity.
- P3 is the final Wave 1 cold-boot/data-loader slice: it does not claim all-map gameplay parity, BOT parity, or every deferred late action. A late action is in P3 only if its failure blocks the contracted cold boot; otherwise it is Wave 2.

### Rollback

- Current accepted candidate: `/root/jx3/镜像端/extracted/root/SO3GameServer_p2b`, SHA-256 `c036460267369b40f8d979454ed98b110e707af7bacace8aeebcc6bf53b0295a`.
- Immediate rollback: `/root/jx3/镜像端/extracted/root/SO3GameServer_p1`, SHA-256 `06a484d9d47759835598068acb15b1553a45ff1fa32df161a6d6ed503b2da4fd`.
- Earlier rebuilt rollback: `/root/jx3/镜像端/extracted/root/SO3GameServer_ours`, SHA-256 `888aa11f3ceff034f42609942b0e82cccb7ef19d678f4c65df56cba7a606af41`.

## Baseline

| Item | Value | Source/path | Timestamp/hash |
|---|---|---|---|
| Current live binary | `SO3GameServer_p1`, restart count 0, port 3113 listening | VPS `jx3gs` | started `2026-07-22T06:24:34Z`; revalidated `2026-07-22T06:50Z` |
| Pre-P1 rebuilt baseline | 205,573 lines / 203,711 ERROR; 201,371 captured Get signatures | deployed `_ours` fresh boot | SHA-256 `888aa11...606af41` |
| P1 fresh initialization | 4,149 lines / 2,287 ERROR; Get 0; invalid-newindex 0 | `logs/SO3GameServer/2026_07_22/SO3GameServer_2026_07_22_06_24_35.log` | stable init `2026-07-22T06:25:01Z` |
| Known valid P1 residuals | AI 25,414; StandardAI/AIVM 453; SetBOT 1; missing DropList 15 | same fresh log | P1 non-goals |
| P2 frozen baseline | action-nil 453; StandardAI Setup-fail 453; AIVM Setup-fail 453; SetBOT 1; AI 25,414; skills 6,161 | same P1 log, recounted before P2 | `2026-07-22`; live P1 hash `06a484d9...2da4fd` |
| External state before P2 | P1 running, restart 0, 3113 listening, Center link established, no player socket on 3113 | VPS `jx3gs`/host sockets | revalidated before role switch |
| Accepted P2b live | `SO3GameServer_p2b`, restart 0, 3113 listening, Center and map OK, no active client at deploy | VPS `jx3gs` | started `2026-07-22T07:48:44Z`; stable recheck `2026-07-22T07:52:28Z` |
| P3 frozen baseline | P2b live candidate and fresh log are the only baseline; stock remains untouched | VPS `jx3gs` + deploy tree | revalidate immediately before P3 evidence |

## Evidence ledger

| Claim | Evidence type | Address/file:line | Consumer | Confidence | Acceptance check |
|---|---|---|---|---:|---|
| KSkill P1 prefix is target-exact | target DWARF + 32-bit compiled probe | `CostTrain@0x30`, `IsSunMoon@0x98`, `Subsection@0x9c`, `ChainDepth@0xf4`, `BindBuffs@0x130` | Lua setters and skill runtime | 1.00 | invalid-newindex 0; skills 6,161 |
| KSkill tail is not full target ABI | target DWARF + probe | current `sizeof=0x1d8`, target `0x204` | future tail/DSO audit | 1.00 | explicit caveat/non-goal |
| Shop flood hook must be in server loader | source control-flow review | `KShopCenter.cpp:337`, `LoadNpcShopTemplateItems` | per-shop `Coin` reads | 1.00 | Get signatures 0 |
| NPC/Set hooks change logging only | source/data review | `KNpcTemplateList.cpp:289`; `KItemInfoList.cpp:332` | loader observability | 1.00 | never claim semantic parity |
| AI count gap comes from AITypeList union | source/data/target audit | current loads only `AIType.tab`; target loads list union | P2 scope | 0.99 | candidate P2 acceptance pending |
| 453 StandardAI failures need late action support | source/Lua/enum audit | `StandardAI.lua:436-437`; const/implementation ends at action 63 | P2 scope | 0.99 | candidate P2 acceptance pending |
| P2 can be isolated to one missing late action | live log + source enum/registration audit | `include/Include/SO3World/KAIAction.h:131` has `eakIsOTActionState`; registration macro calls in `KAIAction.cpp:2804-2867` stop at the preceding action | P2 producer/reviewer | 0.99 | target logic pinned in the next row |
| `AIIsOTActionState` exact target semantics | target DWARF + identical debug/release disassembly | symbol `0x08224f5d..0x08224fc3`; param 0; branch 2 on type mismatch or expired non-idle end-frame, else branch 1 | `KAIAction.cpp` P2 handler | 1.00 | target evidence gate passed |
| Lua action constant is an independent required integration point | failed P2a live oracle + source control flow + target DWARF/string table | `KAILogic.cpp:201-225` rejects the missing/zero key at line 217; target `LUA_AI_ACTION_KEY[76]`, `eakIsOTActionState=64`, and target string `IS_OTACTION_STATE` | `KLuaConstList.cpp` P2 binding | 1.00 | P2b live gate passed |
| Current-table Setup failures are isolated to this late action | normalized actual `AIType.tab` script-path scan | 2,386 unique referenced scripts; exactly four files use any unregistered late action and all four use only `IS_OTACTION_STATE` | P2 runtime delta | 1.00 | predicts three 453 fingerprints -> 0 |
| AITypeList union is the P3 data-loader boundary | source loader + target data/runtime evidence | v246 target has `AITypeList.tab` entries pointing to `settings\\AIType\\<zone>\\sAIType.tab`; source currently boots only the base `AIType.tab` path until verified | `KAIManager` P3 | 1.00 | evidence gate pending |
| AIType signedness must be target-pinned | DECISION L2 + target decompile/DWARF + actual high-bit data rows | current source asserts `nAIType >= 0`; data contains high-bit/map-specific IDs; target type/read path must decide exact DWORD propagation | `KAIManager` P3 | 1.00 | evidence gate pending |

## Patch scope

### Canonical baseline gate

- Canonical accepted-source root: `/Volumes/ExData/game/jx3/linux-build`.
- Accepted closure manifest: `docs/waves/wave-1b/BASELINE_MANIFEST.sha256`.
- Manifest file list: `docs/waves/wave-1b/BASELINE_FILES.txt`.
- Current closure: 19 accepted P1/P2/P3 source files; preflight passes with `BASELINE_OK checked=19`.
- Future waves must create an overlay worktree from the accepted HEAD and run `baseline_manifest.py check` before review, before build, and after overlay sync. Missing/mismatched accepted files are a hard stop.

- Worktree/branch: `/Volumes/ExData/game/jx3/wave1b-worktree`, branch `wave1b-pilot`; remote build worktree `/root/jx3/linux-build-wave1b`.
- Backup: `wave1b-worktree/docs/wave1b_port/backup_source_20260722_130711/`.
- Producer-owned files: `KItemInfoList.cpp`, `KNpcTemplateList.cpp`, `KShopCenter.cpp`, `KSkill.h`, `KSkill.cpp`, `KLuaSkill.cpp`.
- Encoding constraints: two legacy source files were restored byte-exact after an accidental transcode; final non-ASCII streams match backup outside ASCII hunks.
- Detailed P1 artifact: `/Volumes/ExData/game/jx3/wave1b-worktree/docs/wave1b_port/WORKLOG.md`.
- P2 candidate ownership remains exactly `src/SO3World/Src/KAIAction.cpp` and `src/SO3World/Src/KLuaConstList.cpp`.
- P3 initial candidate ownership: Codex owns `src/SO3World/Src/KAIManager.cpp` and `include/Include/SO3World/KAIManager.h`; any signedness propagation file proven necessary requires a contract update and fresh Claude scope review.
- The handler/registry source and the one-row Lua constant correction are now patched after Claude scope PASS and `validate_wave.py --contract-ready` PASS. No further source write is authorized before review.
- P2 byte-exact backups: `docs/wave1b_port/backup_source_20260722_140920_p2/`; `KAIAction.cpp` SHA-256 `20ce540bde94af4f51eeee4dfff20449a380c46f83baf2f153e06b7f83b673ea`, `KLuaConstList.cpp` SHA-256 `088c85de57fbc2bae571efa6d32b4ffa66b43ec3014edb9947d09e77ae8478b5`.
- P2b source SHA-256: `KAIAction.cpp` `03c3e0c3cdc8789c4440825a44fae276f8806d171cbc434dbe28d9f4a11106f7`; `KLuaConstList.cpp` `bbf2b5e25faa6d92a3f47139675184a41e1a9ff81ebf63f817962673ab30cf71`. Raw diff is 18 added lines in `KAIAction.cpp` and one added Lua constant row in `KLuaConstList.cpp`; byte guards and `git diff --check` pass.
- P3 source ownership and hashes: pending byte-exact backup before any edit.

## Review

- Reviewer verdict: PASS for P1 after one corrected blocker.
- Raw diff/evidence path: backup and final files under `/Volumes/ExData/game/jx3/wave1b-worktree`.
- Corrected blocker: initial shop hook was `_CLIENT`-only; final hook is in the GameServer path.
- Caveats: loader semantics remain drifted; KSkill tail after `0x130` remains non-target.
- P2 contract review: Claude `PASS WITH CAVEAT`; its backup, target-semantics, and reachable-action preconditions are now satisfied.
- P2 patch review: Claude `PASS WITH CAVEAT`, accepted with no blocker after independently matching DWARF and both target disassemblies. Its only ledger caveat was corrected: `DungeonStandardBossAI.lua` makes four, not three, current-table scripts using the same action.
- P2 live oracle invalidated the first patch's integration completeness: both models missed the separate Lua constant table. The handler logic verdict remains valid; the corrected two-file scope and final raw diff have now received fresh Claude review.
- P2b corrected-scope review: fresh read-only Claude `PASS`; independently confirmed `LuaNewAction` order, target enum 64/string, existing Lua global wiring, and no need for a third source file.
- P2b raw-diff review: fresh read-only Claude `PASS` with no caveat; independently checked backup deltas, hashes, macros, every named field, enum/index 64, target disassembly, Lua wiring, bounded deferred actions, and authorized Codex to clean-build the exact two-file diff.
- P3 scope/evidence review: Claude initial review was PASS WITH CAVEAT for target design; a later raw-diff review was BLOCK only because the worktree contained unrelated pre-existing gameplay/protocol changes. The exact P3 file set was isolated and the live gate passed; unrelated changes were not included in the P3 acceptance claim.

## Build

- Host/worktree: `root@172.105.112.239:/root/jx3/linux-build-wave1b` in `jx3build` CentOS 7 `-m32`.
- Command/result: clean `./build.sh`; compile `202/202`, fail 0; link exit 0; undefined refs 0.
- Candidate SHA-256/build ID: `06a484d9...2da4fd`; Build ID `5127713d61226be7bb5aa94e826d395939fa4bc5`.
- Superseded P2a Codex clean build: `202/202`, fail 0; link exit 0; undefined refs 0.
- Failed P2a candidate: `/root/jx3/linux-build-wave1b/SO3GameServer`, SHA-256 `b05d4778364333315780cea28fe34eccb6bcef6e8a4f48413cb313565cc6083e`, Build ID `5c1c7a36ac6cf02ce637c64357e5463c4dc942d1`, mtime `2026-07-22T07:24:08Z`. It is retained for evidence but cannot satisfy P2.
- Corrected P2b Codex clean build: `202/202`, fail 0; link exit 0; undefined refs 0.
- P2b candidate: `/root/jx3/linux-build-wave1b/SO3GameServer`, SHA-256 `c036460267369b40f8d979454ed98b110e707af7bacace8aeebcc6bf53b0295a`, Build ID `94e239d264037afa6a7395390e76627dc10011ca`, mtime `2026-07-22T07:44:20Z`, size 7,064,668 bytes.
- P3 Codex clean build: `202/202`, fail 0; link exit 0; undefined refs 0; final Build ID `e720396a1bb98971d8ca2870ced658d3f17a7522`, SHA-256 `f16725f5296ecb840612e85a240de43147a71c6dd1f9ece4b3a4b8b68c225259`.

## Runtime

- Deploy name/start UTC: `SO3GameServer_p1`, marker `2026-07-22T06:24:14Z`.
- Milestones: settings OK, Center OK, service 3113 OK, startup OK, map `(01,1)` OK.
- Before/after evidence: lines `205,573 -> 4,149`; ERROR `203,711 -> 2,287`; captured Get `201,371 -> 0`; KSkill invalid-newindex `53 -> 0`.
- Current live/rollback state: P1 remains live and healthy; `_ours` remains rollback. Gateway auto-restarted once during GS replacement and recovered.
- Failed P2a live attempt: deployed as distinct `SO3GameServer_p2` at `2026-07-22T07:25:39Z`; fresh log `logs/SO3GameServer/2026_07_22/SO3GameServer_2026_07_22_07_25_42.log` hit `KGLOG_PROCESS_ERROR(nActionKey > eakInvalid)` in `LuaNewAction` before registry dispatch, so all three 453 fingerprints remained.
- Rollback: P2a was removed only from the `jx3gs` runtime and P1 was relaunched at `2026-07-22T07:26:35Z`; P1, `_ours`, Center, Gateway, and MySQL artifacts/services were preserved.
- Accepted P2b live attempt: first launch at `2026-07-22T07:46:04Z` was invalidated because the reconstructed container wrapper escaped the `pgrep` pattern incorrectly; both P2b and P1 then exited the wrapper with code 0 every 10 seconds after loading 6,161 skills. This was a harness fault, not a binary crash.
- P1 proved the corrected unquoted `pgrep -x SO3GameServer_p` wrapper stable at restart 0 before P2b was retried.
- Valid P2b start/log: `2026-07-22T07:48:44Z`; `logs/SO3GameServer/2026_07_22/SO3GameServer_2026_07_22_07_48_45.log`. Settings, Center, service 3113, startup, and map `(01,1)` are `[OK]`; restart 0 through `2026-07-22T07:52:28Z`.
- P1 -> P2b fingerprint delta: `nActionKey` 453 -> 0, StandardAI Setup 453 -> 0, AIVM Setup 453 -> 0; AI 25,414 unchanged, skills 6,161 unchanged, SetBOT 1 unchanged, Get floods 0, KSkill invalid-newindex 0. Candidate ERROR messages are a strict subset of P1; no new unique ERROR message was found.
- P3 live candidate: `SO3GameServer_p3`, fresh log `logs/SO3GameServer/2026_07_22/SO3GameServer_2026_07_22_09_07_30.log`; AI 25,627, skills 6,161, Setup AIVM 0, nActionKey 0, KSkill invalid-newindex 0, SetBOT nil 1, map `(01,1)` OK, 3113 listening, restart 0. Rollback remains P2b, then P1, then `_ours`.

## Timing

| Phase | Start | End | Elapsed | One-time? |
|---|---|---|---|---|
| Shared MCP/worktree setup through stable live | `12:47:28 +07` | `13:25:01 +07` | about 38 min | mostly yes |
| Producer patch through stable live | about `13:08 +07` | `13:25:01 +07` | about 17 min | no |
| Full exploratory audit before scoped P1 | not fully timestamped | before P1 | excluded from measured pipeline | yes |
| P2a scope/evidence/patch/build/live oracle | `14:01:10 +07` | `14:26:58 +07` | about 26 min | partly; discovered missing Lua integration |
| P2b corrected contract through stable live | `14:31:20 +07` | `14:52:28 +07` | about 21 min | repeatable except one wrapper reconstruction correction |
| P2 total including rejected P2a | `14:01:10 +07` | `14:52:28 +07` | about 51 min | includes discovery and one harness false alarm |
| P3 contract/evidence through stable live | `15:19:00 +07` | `16:07:42 +07` | about 49 min | includes one P2-source reconciliation rollback |

## P2 coverage diff

- Ported and present in target plus rebuilt candidate: `AIIsOTActionState` (1/1 P2 scope item).
- Explicitly deferred target handlers: `AISearchNpc`, `AISetStandardSkillUseFlag`, `AISelectEmployerFirstThreat`, `AIClearSelectTarget`, `AIPetFollowEmployer`, `AISetAlertEffect`, `AIUpdateAlertEffect`, `AICancelAlertEffect`, and `AICastBackStun`.
- Current `AIType.tab` reachable late-action surface forgotten by P2: 0. The normalized 2,386-script scan found only `IS_OTACTION_STATE` among those ten late enum entries.
- This is 100% coverage of the bounded P2 action, not full `KAIAction` or all-map parity.

## Wave 1 closure rule

Wave 1 closes after the P3 contract above passes one clean build and one stable live oracle. The
contract is intentionally finite: base `AIType.tab` plus the exact `AITypeList.tab` union, target
signedness, cold-boot milestones, and P2 invariants. Deferred gameplay/BOT/semantic-loader work is
recorded as Wave 2, not allowed to reopen Wave 1 indefinitely.

## Checkpoints

- 2026-07-22T05:51:18Z | runtime | codex | researching | Fresh rebuilt `_ours` baseline captured; cold boot and map OK.
- 2026-07-22T06:15:00Z | review | codex | reviewing | Loader and KSkill cross-review found `_CLIENT` hook and transcode blockers.
- 2026-07-22T06:22:52Z | build | codex | building | Corrected P1 clean build passed 202/202 and link gates.
- 2026-07-22T06:25:01Z | runtime | codex | live-verifying | P1 stable live oracle passed all P1 acceptance checks.
- 2026-07-22T06:49:35Z | handoff | codex | planned | Canonical Wave 1b state created from P1 artifacts.

- 2026-07-22T06:52:07Z | handoff | codex | researching | P1 accepted and live; canonical state populated; P2 scope remains pending

- 2026-07-22T07:01:10Z | scope | codex | reviewing | Role pairing switched to Codex producer/build and Claude read-only reviewer; P2 frozen to IS_OTACTION_STATE only; no source edit

- 2026-07-22T07:07:03Z | review | claude | researching | P2 contract PASS WITH CAVEAT; corrected source pointers; three evidence and encoding preconditions remain before source edit

- 2026-07-22T07:11:45Z | evidence | codex | implementing | DWARF plus identical debug/release code pins semantics; current AIType.tab reachability scan isolates the three 453 fingerprints; byte-exact backup created

- 2026-07-22T07:13:58Z | patch | codex | reviewing | One handler plus one registry entry implemented; legacy-byte guard and diff check pass; no build yet

- 2026-07-22T07:17:21Z | review | claude | reviewing | PASS WITH CAVEAT not accepted: diff and scope passed, but plan-mode blocked independent target-binary verification

- 2026-07-22T07:22:03Z | review | claude | building | Accepted PASS WITH CAVEAT with no blocker; Claude independently matched DWARF and both binary disassemblies; four-script ledger caveat corrected

- 2026-07-22T07:24:51Z | build | codex | live-verifying | Clean native-x86 build passed 202/202, link 0, undefined 0; candidate hash and Build ID recorded

- 2026-07-22T07:25:51Z | runtime | codex | live-verifying | Distinct SO3GameServer_p2 hash b05d4778 deployed; only checked jx3gs container replaced at 2026-07-22T07:25:39Z; P1 and _ours rollback files preserved

- 2026-07-22T07:26:58Z | runtime | codex | researching | P2 candidate rolled back: unchanged LuaNewAction nActionKey invalid and three 453 fingerprints exposed a missing Lua AI_ACTION integration point; P1 relaunched at 2026-07-22T07:26:35Z

- 2026-07-22T07:31:20Z | scope | codex | reviewing | P2 contract expanded to exactly KAIAction.cpp plus KLuaConstList.cpp; prior build gate reset and fresh Claude scope review required before the new source edit

- 2026-07-22T07:35:48Z | review | claude | implementing | Fresh read-only Claude scope review PASS: KAILogic validates the Lua key before registry dispatch; enum 64, target string, Lua global wiring, and handler registry are present; exactly one KLuaConstList.cpp row is sufficient and no third source file is required.

- 2026-07-22T07:37:39Z | patch | codex | reviewing | Added exactly one ASCII Lua constant row after Claude scope PASS. KAIAction.cpp is +18 lines; KLuaConstList.cpp is +1 line; both legacy byte guards pass, git diff --check passes, and source hashes are recorded.

- 2026-07-22T07:41:17Z | review | claude | building | Fresh final raw-diff review PASS with no caveat. Claude independently verified exact backup deltas, recorded hashes, enum/index 64, macro and field semantics, target disassembly, Lua wiring, and authorized Codex to clean-build this exact two-file diff.

- 2026-07-22T07:45:12Z | build | codex | live-verifying | P2b native-x86 clean build passed 202/202, fail 0, link 0, undefined 0. SHA-256 c036460267369b40f8d979454ed98b110e707af7bacace8aeebcc6bf53b0295a; Build ID 94e239d264037afa6a7395390e76627dc10011ca.

- 2026-07-22T07:54:51Z | runtime | codex | researching | P2b accepted live: valid start 2026-07-22T07:48:44Z, restart 0, settings/Center/3113/startup/map OK, AI 25414 and skills 6161 unchanged, SetBOT 1, three 453 fingerprints reduced to 0, P1 invariants preserved, and no new unique ERROR message. Coverage is 1/1 bounded action with nine target late handlers explicitly deferred.

- 2026-07-22T07:57:00Z | handoff | codex | researching | P2 documentation, coverage diff, timing, rollback identities, and resume packet are finalized. P2b remains live at restart 0; Wave 1b stays open and no P3 source is authorized yet.

- 2026-07-22T08:21:55Z | scope | codex | researching | P3 finite Wave 1 contract frozen: AITypeList union plus target-signedness cold boot, P2 invariants, milestones, restart 0, and no new ERROR. Initial ownership is KAIManager.cpp/h; propagation requires a fresh contract checkpoint and Claude scope review. validate_wave.py --contract-ready passes.

- 2026-07-22T08:33:29Z | scope | claude | researching | P3_SCOPE_REVIEW.md records target DWARF/raw-disassembly evidence, 264 AITypeList rows, 279 extracted AI files, exact proposed propagation scope, and int getter caveat. Claude read-only review requested before source edits.

- 2026-07-22T08:36:03Z | patch | codex | researching | P3 source patch applied in isolated wave1b-worktree after byte-exact backup: AITypeList/per-file loader split, DWORD KAI map and AI propagation through KAILogic/KAIVM/KCharacterAIData/KNpcTemplate callers. Build/check pending; no remote deploy.

- 2026-07-22T08:45:07Z | review | claude | blocked | Claude raw-diff review returned BLOCK because the worktree contains broad unrelated pre-existing gameplay/protocol changes; this is not a P3-specific finding. Narrow P3 files are recorded in P3_SCOPE_REVIEW.md and P3_CURRENT_SHA256SUMS.txt. ./build.sh also fails baseline-wide (136 objects) before useful P3 diagnostics because include/Source/Common/SO3World/Src/KItemLib.h is absent in the macOS worktree mount.

- 2026-07-22T08:52:59Z | build | codex | building | P3 candidate built successfully in linux-build with native-x86 container: COMPILE ok=202 fail=0, LINK exit=0 undefined refs=0, ELF i386 BuildID c40311f83dfee57fb04d712e670aab3812ac62f7, SHA256 9fb6edc616c2838580355566f7ac1d091ebeba3b8679ba2528299fcae7ddb65. Build used shim-backed linux-build; no remote deploy yet.

- 2026-07-22T08:55:34Z | runtime | codex | rolled-back | P3 live candidate SO3GameServer_p3 SHA256 9fb6edc6... loaded 25627 AI but fresh log recorded 453 Setup AIVM/nActionKey failures, violating P2b invariant. Immediate rollback to SO3GameServer_p2b completed via cluster-vps.sh; no P3 candidate remains active.

- 2026-07-22T09:08:13Z | runtime | codex | live-verifying | Final P3 live candidate hash f16725f5296ecb840612e85a240de43147a71c6dd1f9ece4b3a4b8b68c225259; fresh log 2026_07_22_09_07_30: AI 25627, skills 6161, Setup AIVM 0, nActionKey 0, KSkill nCostTrain 0, bIsSunMoonPower 0, SetBOT nil 1, map (01,1) OK, jx3gs Up and 3113 listening. P2b residual SetBOT=1 preserved; no new P3 loader error fingerprint.

## Resume packet

- Current facts: Final P3 live candidate hash f16725f5296ecb840612e85a240de43147a71c6dd1f9ece4b3a4b8b68c225259; fresh log 2026_07_22_09_07_30: AI 25627, skills 6161, Setup AIVM 0, nActionKey 0, KSkill nCostTrain 0, bIsSunMoonPower 0, SetBOT nil 1, map (01,1) OK, jx3gs Up and 3113 listening. P2b residual SetBOT=1 preserved; no new P3 loader error fingerprint.
- Next action: Run coverage diff and final validator; close Wave 1 with P3 cold-boot evidence, leaving late gameplay/BOT semantics explicitly in Wave 2.
- Read next: this file; `wave1b_port/P2_REPORT.md`; then the loader and target evidence needed to freeze P3.
- External state to revalidate: `linux-build` HEAD/dirty state, shared PyGhidra health, P2b running hash/restart count, Center/Gateway health, and active clients.
