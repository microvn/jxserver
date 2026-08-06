# JX3 GameServer port execution registry

- last_update: 2026-08-06
- authority: target DWARF + target decompile (`SO3GameServerD`, SHA `47716c73e8de...`)
- database: `linux-build/graphengine/evidence.sqlite`
- inventory: 430 grouped target basenames, 7777 target functions, 90 SCCs
- call graph: 6,900 resolved `TARGET_STATIC_CONFIRMED` edges
- worktree_scan: 2026-08-06; 25 changed source/header paths across 13 basenames; changed means unverified, not ported

## Rules for agents

- `port_order` is dependency-first and only covers the confirmed GameServer graph.
- Same `scc` means one bounded closure; do not split it into a false serial order. `KItemList` and `KTarget` are in the same SCC: `KTarget -> KItemList` (3) and `KItemList -> KTarget` (6).
- Files containing multiple classes must be split into class-level tickets before editing.
- `fan_in`/`fan_out` are signals, not dependency proof. `UNVERIFIED` is not `PORTED`.
- Only rows whose target module is `SO3GameServerD` are eligible for the GS file-port queue. Engine/DSO, Gateway, and Center rows are inventory evidence, not GameServer port work.
- Preserve markers: `PORT-UNKNOWN_REQUIRED[ABI|WIRE|CALLER|STATE|FAILURE]`, `PORT-TODO[TARGET_REQUIRED]`, and standalone-only `PORT-DEFERRED_WIRING[CALLER|IMPORT|REGISTRATION]`. Never replace an unresolved edge with a stub/fake success.
- After every bounded port update `status`, `owner`, `evidence`, `markers`, `next_action`, and `last_update`.
- In `master_slave` mode, use the orchestration columns below. A bundle is a
  bounded execution slice, not automatically the whole SCC. For `SCC-066`,
  use one root-specific reciprocal slice until GraphEngine proves a smaller
  shared bundle; do not assign all 70+ rows to one mega-bundle.

## Orchestration projection

The main table carries these fields for every inventory row:

| field | meaning |
|---|---|
| `bundle_id` | Stable bundle or root-slice identifier |
| `closure_mode` | `SCC_BUNDLE`, `RECURSIVE_SLICE`, `SINGLE_ROOT`, `MULTI_FILE_CLOSURE`, `EXCLUDED`, or `TARGET_UNRESOLVED` |
| `orchestration_role` | `MASTER_ELIGIBLE`, `SLAVE_ELIGIBLE`, `MASTER_OR_SLAVE`, `EXCLUDED`, or `BLOCKED` |
| `ownership_policy` | Exclusive source ownership; shared headers require serialized handoff |
| `last_update` | Date of the projection/evidence refresh |

### GraphEngine-confirmed bundle seeds

| bundle_id | members | evidence | rule |
|---|---|---|---|
| `B-RECIP-KITEMLIST-KTARGET` | `KItemList`, `KTarget` | `TARGET_STATIC_CONFIRMED`: `KTarget -> KItemList` = 3 edges; `KItemList -> KTarget` = 6 edges; target SHA `47716c73e8de...` | Resolve as one reciprocal slice; do not leave deferred caller/callee markers between the two roots |
| `B-ROOT-KSHOP` | `KShop` | target GS row `SCC-066`; direct callee inventory includes `KItemList`, `KCurrencyList`, `KCoolDownList`, `KCharacter`, `KLogClient`, and related target owners | Main root candidate; recursively delegate only required in-path edges, with `KShop` protected as parent root |
| `B-MULTI-KSCRIPTFUNCLIST` | `KBaseFuncList.cpp`, `KShopFuncList.cpp`, `kLoginFuncList.cpp` | target GS namespace closure; 223 target functions; `GetBaseFuncList` caller `KScriptCenter::CreateScriptHolder` | Port/review as one multi-file closure, with disjoint sub-agent ownership |

The seed list is deliberately conservative: a same-SCC label is not treated
as proof of a direct reciprocal edge. New bundles require a target call,
registration, ABI, state, or workflow evidence pointer before being added.

| port_order | file | scc | dependency_state | fan_in | fan_out | target_functions | direct_callees | port_status | bundle_id | closure_mode | orchestration_role | ownership_policy | last_update |
|---:|---|---|---|---:|---:|---:|---|---|---|---|---|---|---|
| 1 | `KMath` | `SCC-002` | READY | 109 | 0 | 9 | `-` | FILE_CORRECT | `B-SCC-002` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 2 | `KInventory` | `SCC-003` | READY | 109 | 0 | 7 | `-` | FILE_CORRECT | `B-SCC-003` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 3 | `KTeamServer` | `SCC-024` | READY | 69 | 0 | 17 | `-` | FILE_CORRECT | `B-SCC-024` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 4 | `KSceneObject` | `SCC-018` | READY | 33 | 1 | 1 | `KMath` | FILE_CORRECT | `B-SCC-018` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 5 | `KRecipe` | `SCC-053` | READY | 31 | 0 | 29 | `-` | FILE_BLOCKED | `B-SCC-053` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 6 | `KQuestInfoList` | `SCC-039` | READY | 26 | 0 | 15 | `-` | WORKTREE_CHANGED_UNVERIFIED | `B-SCC-039` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 7 | `KAttribute` | `SCC-008` | READY | 22 | 0 | 3 | `-` | UNVERIFIED | `B-SCC-008` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 8 | `KG_Time` | `SCC-005` | READY | 21 | 0 | 4 | `-` | UNVERIFIED | `B-SCC-005` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 9 | `KIndividualDropList` | `SCC-055` | READY | 20 | 0 | 4 | `-` | UNVERIFIED | `B-SCC-055` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 10 | `KRegressionManager` | `SCC-051` | READY | 18 | 0 | 10 | `-` | UNVERIFIED | `B-SCC-051` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 11 | `KSG_EncodeDecode` | `SCC-071` | READY | 16 | 0 | 27 | `-` | UNVERIFIED | `B-SCC-071` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 12 | `KDesignationList` | `SCC-007` | READY | 14 | 0 | 6 | `-` | UNVERIFIED | `B-SCC-007` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 13 | `KDoodadTemplateList` | `SCC-034` | READY | 11 | 0 | 7 | `-` | UNVERIFIED | `B-SCC-034` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 14 | `KGCRT` | `SCC-004` | READY | 10 | 0 | 5 | `-` | REUSE_EXTERNAL | `B-EXTERNAL-REUSE` | `EXCLUDED` | `EXCLUDED` | `READ_ONLY` | `2026-08-06` |
| 15 | `KScriptCenter` | `SCC-006` | READY | 293 | 4 | 36 | `KGCRT,KG_Time` | UNVERIFIED | `B-SCC-006` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 16 | `KRoadManager` | `SCC-032` | READY | 12 | 8 | 31 | `KMath,KScriptCenter` | UNVERIFIED | `B-SCC-032` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 17 | `KCoolDownList` | `SCC-020` | READY | 10 | 0 | 3 | `-` | UNVERIFIED | `B-SCC-020` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 18 | `KRewardsShopSettings` | `SCC-014` | READY | 9 | 0 | 5 | `-` | UNVERIFIED | `B-SCC-014` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 19 | `KReputationLimit` | `SCC-017` | READY | 9 | 0 | 4 | `-` | UNVERIFIED | `B-SCC-017` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 20 | `KDomesticateCenter` | `SCC-049` | READY | 9 | 0 | 8 | `-` | UNVERIFIED | `B-SCC-049` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 21 | `KAchievementInfoList` | `SCC-019` | READY | 9 | 0 | 4 | `-` | UNVERIFIED | `B-SCC-019` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 22 | `KMapListFile` | `SCC-028` | READY | 8 | 0 | 3 | `-` | WORKTREE_CHANGED_UNVERIFIED | `B-SCC-028` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 23 | `KLocalGSDataStat` | `SCC-064` | READY | 8 | 1 | 11 | `KGCRT` | UNVERIFIED | `B-SCC-064` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 24 | `KBuffManager` | `SCC-009` | READY | 8 | 7 | 13 | `KAttribute` | UNVERIFIED | `B-SCC-009` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 25 | `KWeaponTypeList` | `SCC-037` | READY | 7 | 0 | 6 | `-` | UNVERIFIED | `B-SCC-037` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 26 | `KPatrolPath` | `SCC-027` | READY | 6 | 0 | 8 | `-` | UNVERIFIED | `B-SCC-027` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 27 | `KPatrolPathList` | `SCC-029` | READY | 9 | 4 | 7 | `KMapListFile,KPatrolPath` | UNVERIFIED | `B-SCC-029` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 28 | `KNpcTeamList` | `SCC-033` | READY | 6 | 1 | 7 | `KMapListFile` | UNVERIFIED | `B-SCC-033` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 29 | `KMiniAvatarSettings` | `SCC-016` | READY | 6 | 0 | 6 | `-` | UNVERIFIED | `B-SCC-016` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 30 | `KLevelUpList` | `SCC-038` | READY | 6 | 0 | 5 | `-` | UNVERIFIED | `B-SCC-038` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 31 | `KLevelMoneyDropList` | `SCC-056` | READY | 6 | 0 | 4 | `-` | UNVERIFIED | `B-SCC-056` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 32 | `KLevelDropList` | `SCC-057` | READY | 6 | 0 | 4 | `-` | UNVERIFIED | `B-SCC-057` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 33 | `KFellowPetMgr` | `SCC-015` | READY | 6 | 0 | 6 | `-` | UNVERIFIED | `B-SCC-015` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 34 | `KActivityMgrServer` | `SCC-022` | READY | 6 | 10 | 10 | `KScriptCenter` | UNVERIFIED | `B-SCC-022` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 35 | `KRankListServer` | `SCC-058` | READY | 5 | 0 | 5 | `-` | UNVERIFIED | `B-SCC-058` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 36 | `KNpcOrderList` | `SCC-030` | READY | 5 | 0 | 4 | `-` | UNVERIFIED | `B-SCC-030` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 37 | `KAIPatrolGroupManager` | `SCC-031` | READY | 19 | 6 | 15 | `KMath,KNpcOrderList,KPatrolPathList` | UNVERIFIED | `B-SCC-031` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 38 | `KNpcClassList` | `SCC-041` | READY | 5 | 0 | 5 | `-` | UNVERIFIED | `B-SCC-041` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 39 | `KNpcAdronTab` | `SCC-021` | READY | 5 | 1 | 6 | `KAttribute` | WORKTREE_CHANGED_UNVERIFIED | `B-SCC-021` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 40 | `KGameCardInfoList` | `SCC-043` | READY | 5 | 0 | 3 | `-` | UNVERIFIED | `B-SCC-043` | `SCC_BUNDLE` | `MASTER_ELIGIBLE` | `EXCLUSIVE_SOURCE_PATH` | `2026-08-06` |
| 41 | `KDoodadClassList` | `SCC-040` | READY | 5 | 0 | 4 | `-` | UNVERIFIED | B-SCC-040 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 42 | `lzo1x_c` | `SCC-025` | READY | 4 | 0 | 4 | `-` | REUSE_EXTERNAL | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| 43 | `KTransmissionList` | `SCC-050` | READY | 4 | 0 | 4 | `-` | UNVERIFIED | B-SCC-050 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 44 | `KGameServerEyes` | `SCC-075` | READY | 4 | 0 | 11 | `-` | UNVERIFIED | B-SCC-075 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 45 | `KGWConstList` | `SCC-042` | READY | 4 | 0 | 4 | `-` | WORKTREE_CHANGED_UNVERIFIED | B-SCC-042 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 46 | `CRC32` | `SCC-001` | READY | 4 | 0 | 9 | `-` | REUSE_EXTERNAL | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| 47 | `KTongConstList` | `SCC-046` | READY | 3 | 0 | 3 | `-` | WORKTREE_CHANGED_UNVERIFIED | B-SCC-046 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 48 | `KTerrainData` | `SCC-063` | READY | 3 | 2 | 6 | `KG_Time` | UNVERIFIED | B-SCC-063 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 49 | `KRewardsBox` | `SCC-062` | READY | 3 | 0 | 7 | `-` | UNVERIFIED | B-SCC-062 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 50 | `KReputeLootBuffList` | `SCC-036` | READY | 3 | 0 | 2 | `-` | WORKTREE_CHANGED_UNVERIFIED | B-SCC-036 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 51 | `KProfession` | `SCC-054` | READY | 3 | 0 | 3 | `-` | UNVERIFIED | B-SCC-054 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 52 | `KPendentOldDataInfoList` | `SCC-045` | READY | 3 | 0 | 2 | `-` | WORKTREE_CHANGED_UNVERIFIED | B-SCC-045 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 53 | `KG_InterlockSingleList_GNUC_x86` | `SCC-010` | READY | 3 | 0 | 8 | `-` | UNVERIFIED | B-SCC-010 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 54 | `KGMList` | `SCC-047` | READY | 3 | 0 | 5 | `-` | UNVERIFIED | B-SCC-047 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 55 | `KGJumpList` | `SCC-059` | READY | 3 | 0 | 4 | `-` | UNVERIFIED | B-SCC-059 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 56 | `KCharacterActionList` | `SCC-035` | READY | 3 | 0 | 4 | `-` | UNVERIFIED | B-SCC-035 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 57 | `KAntiFarmerSettings` | `SCC-044` | READY | 3 | 0 | 4 | `-` | WORKTREE_CHANGED_UNVERIFIED | B-SCC-044 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 58 | `lzo_init` | `SCC-060` | READY | 2 | 0 | 4 | `-` | REUSE_EXTERNAL | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| 59 | `lzo1x_d` | `SCC-026` | READY | 2 | 0 | 3 | `-` | REUSE_EXTERNAL | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| 60 | `KRecipeMaster` | `SCC-052` | READY | 2 | 0 | 5 | `-` | UNVERIFIED | B-SCC-052 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 61 | `KModelInfo` | `SCC-061` | READY | 2 | 0 | 3 | `-` | UNVERIFIED | B-SCC-061 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 62 | `new` | `SCC-012` | READY | 1 | 0 | 12 | `-` | REUSE_EXTERNAL | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| 63 | `my_md5` | `SCC-072` | READY | 1 | 0 | 21 | `-` | REUSE_EXTERNAL | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| 64 | `easycrypt` | `SCC-073` | READY | 4 | 1 | 12 | `my_md5` | REUSE_EXTERNAL | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| 65 | `KSCOMTypes` | `SCC-065` | READY | 1 | 0 | 6 | `-` | UNVERIFIED | B-SCC-065 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 66 | `KRecorderFactory` | `SCC-088` | READY | 1 | 2 | 1 | `KGCRT` | UNVERIFIED | B-SCC-088 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 67 | `KG_InterlockSingleList` | `SCC-011` | READY | 1 | 0 | 4 | `-` | UNVERIFIED | B-SCC-011 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 68 | `KG_Memory` | `SCC-013` | READY | 201 | 5 | 36 | `KG_InterlockSingleList,KG_InterlockSingleList_GNUC_x86,new` | UNVERIFIED | B-SCC-013 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 69 | `KG_Package` | `SCC-023` | READY | 5 | 4 | 11 | `KG_Memory` | UNVERIFIED | B-SCC-023 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 70 | `KGWServerConstList` | `SCC-048` | READY | 1 | 0 | 3 | `-` | WORKTREE_CHANGED_UNVERIFIED | B-SCC-048 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KAIAction` | `SCC-066` | SCC | 1 | 175 | 82 | `KAIPatrolGroupManager,KAISearchTactics,KAISkillSelector,KAIVM,KBuffList,KMapListFile,KMath,KNpcReviveManager` | UNVERIFIED | B-ROOT-KAIAction | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KAILogic` | `SCC-066` | SCC | 8 | 9 | 10 | `KAIManager,KScriptCenter` | UNVERIFIED | B-ROOT-KAILogic | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KAIManager` | `SCC-066` | SCC | 6 | 7 | 10 | `KAIAction,KAILogic,KGCRT,KG_Time,KScriptCenter` | UNVERIFIED | B-ROOT-KAIManager | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KAISearchTactics` | `SCC-066` | SCC | 9 | 14 | 22 | `KCharacter,KMath,KSceneObject,KThreatList` | UNVERIFIED | B-ROOT-KAISearchTactics | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KAISkillSelector` | `SCC-066` | SCC | 23 | 25 | 14 | `KMath,KTarget,KThreatList` | UNVERIFIED | B-ROOT-KAISkillSelector | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KAIState` | `SCC-066` | SCC | 2 | 1 | 3 | `KAILogic` | UNVERIFIED | B-ROOT-KAIState | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KAIVM` | `SCC-066` | SCC | 18 | 15 | 10 | `KAILogic,KAIManager,KAIState,KG_Time,KScriptCenter` | UNVERIFIED | B-ROOT-KAIVM | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KAchievement` | `SCC-066` | SCC | 18 | 27 | 15 | `KAchievementInfoList,KDesignation,KLogClient,KPlayerServer,KRelayClient,KScriptCenter,KStatDataServer` | UNVERIFIED | B-ROOT-KAchievement | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KAntiFarmerServer` | `SCC-066` | SCC | 7 | 4 | 9 | `KItemList,KRelayClient` | UNVERIFIED | B-ROOT-KAntiFarmerServer | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KAttrModifier` | `SCC-066` | SCC | 1 | 94 | 456 | `KAIVM,KBuffList,KCDTimerList,KFellowship,KItemList,KMath,KMentorCache,KPlayerServer` | UNVERIFIED | B-ROOT-KAttrModifier | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KBookList` | `SCC-066` | SCC | 3 | 3 | 10 | `KPlayerServer,KRecipe` | UNVERIFIED | B-ROOT-KBookList | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KBuff` | `SCC-066` | SCC | 61 | 5 | 11 | `KBuffManager,KPlayerServer,KSkill` | UNVERIFIED | B-ROOT-KBuff | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KBuffList` | `SCC-066` | SCC | 43 | 76 | 44 | `KBuff,KBuffManager,KMath,KPlayerServer,KScriptCenter,KSkill,KSkillDebug,KTeamServer` | UNVERIFIED | B-ROOT-KBuffList | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KCDTimerList` | `SCC-066` | SCC | 38 | 3 | 8 | `KPlayerServer` | UNVERIFIED | B-ROOT-KCDTimerList | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KCampInfo` | `SCC-066` | SCC | 13 | 29 | 21 | `KBuffList,KCenterRemote,KDesignation,KLogClient,KRelayClient,KScriptCenter,KScriptServer` | UNVERIFIED | B-ROOT-KCampInfo | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KCenterRemote` | `SCC-066` | SCC | 11 | 10 | 11 | `KRelayClient,KScriptCenter,lzo1x_c,lzo1x_d` | UNVERIFIED | B-ROOT-KCenterRemote | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KCharacter` | `SCC-066` | SCC | 20 | 3 | 8 | `KFellowship,KTongDiplomacyCache` | UNVERIFIED | B-ROOT-KCharacter | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KCraft` | `SCC-066` | SCC | 7 | 106 | 17 | `KBookList,KBuffList,KCDTimerList,KCoolDownList,KItemList,KLogClient,KLootList,KPlayerServer` | UNVERIFIED | B-ROOT-KCraft | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KCurrency` | `SCC-066` | SCC | 13 | 6 | 5 | `KLogClient,KPlayerServer` | UNVERIFIED | B-ROOT-KCurrency | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KCurrencyList` | `SCC-066` | SCC | 21 | 3 | 6 | `KCurrency` | UNVERIFIED | B-ROOT-KCurrencyList | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KDesignation` | `SCC-066` | SCC | 17 | 39 | 29 | `KBuffList,KCDTimerList,KCoolDownList,KDesignationList,KPlayerServer,KRelayClient,KStatDataServer` | UNVERIFIED | B-ROOT-KDesignation | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KDiamondManager` | `SCC-066` | SCC | 8 | 113 | 27 | `KItemList,KLogClient,KPlayerServer,KSO3World,KStatDataServer` | UNVERIFIED | B-ROOT-KDiamondManager | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KDirectMentorCache` | `SCC-066` | SCC | 9 | 6 | 13 | `KPlayerServer,KRelayClient` | UNVERIFIED | B-ROOT-KDirectMentorCache | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KDomesticate` | `SCC-066` | SCC | 5 | 46 | 38 | `KDomesticateCenter,KItemList,KLogClient,KPlayerServer,KSO3World,KScriptCenter,KStatDataServer` | UNVERIFIED | B-ROOT-KDomesticate | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KDoodadReviveManager` | `SCC-066` | SCC | 1 | 9 | 15 | `KActivityMgrServer,KG_Time,KSO3World` | UNVERIFIED | B-ROOT-KDoodadReviveManager | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KDropCenter` | `SCC-066` | SCC | 3 | 139 | 33 | `KBuffList,KDoodadClassList,KDoodadTemplateList,KDropList,KIndividualDropList,KItemList,KLevelDropList,KLevelMoneyDropList` | UNVERIFIED | B-ROOT-KDropCenter | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KDropList` | `SCC-066` | SCC | 48 | 2 | 11 | `KSO3World` | UNVERIFIED | B-ROOT-KDropList | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KEscortQuestManager` | `SCC-066` | SCC | 5 | 31 | 18 | `KBuffList,KNpcReviveManager,KPlayerServer,KQuestInfoList,KQuestList,KSceneObject,KScriptCenter,KTarget` | UNVERIFIED | B-ROOT-KEscortQuestManager | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KExterior` | `SCC-066` | SCC | 21 | 37 | 28 | `KExteriorBox,KG_Memory,KItemList,KLogClient,KRelayClient,KScriptCenter,KStatDataServer` | UNVERIFIED | B-ROOT-KExterior | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KExteriorBox` | `SCC-066` | SCC | 17 | 24 | 44 | `KExterior,KPlayerServer,KScriptServer` | UNVERIFIED | B-ROOT-KExteriorBox | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KFellowPetBox` | `SCC-066` | SCC | 3 | 6 | 16 | `KFellowPetMgr,KPlayerServer` | UNVERIFIED | B-ROOT-KFellowPetBox | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KFellowship` | `SCC-066` | SCC | 44 | 30 | 31 | `KAchievement,KAchievementInfoList,KG_Memory,KPlayerServer,KRelayClient` | UNVERIFIED | B-ROOT-KFellowship | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KGPQ` | `SCC-066` | SCC | 12 | 7 | 21 | `KRelayClient,KSO3World,KScriptCenter` | UNVERIFIED | B-ROOT-KGPQ | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KHairBox` | `SCC-066` | SCC | 11 | 8 | 15 | `KHairShop,KPlayerServer` | UNVERIFIED | B-ROOT-KHairBox | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KHairShop` | `SCC-066` | SCC | 10 | 23 | 17 | `KHairBox,KItemList,KLogClient,KRelayClient,KScriptCenter,KStatDataServer` | UNVERIFIED | B-ROOT-KHairShop | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KItemList` | `SCC-066` | SCC | 306 | 315 | 171 | `KAIVM,KAchievement,KBuffList,KCDTimerList,KDesignation,KDiamondManager,KInventory,KLogClient` | WORKTREE_CHANGED_UNVERIFIED | B-RECIP-KITEMLIST-KTARGET | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KLogClient` | `SCC-066` | SCC | 206 | 9 | 59 | `KCurrencyList,KDiamondManager,KExteriorBox,KG_Memory,KHairBox,KRelayClient` | UNVERIFIED | B-ROOT-KLogClient | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KLootList` | `SCC-066` | SCC | 33 | 60 | 34 | `KItemList,KLogClient,KPlayerServer,KQuestInfoList,KQuestList,KScriptCenter,KStatDataServer,KTeamServer` | UNVERIFIED | B-ROOT-KLootList | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KMentorCache` | `SCC-066` | SCC | 16 | 7 | 11 | `KPlayerServer,KRelayClient` | UNVERIFIED | B-ROOT-KMentorCache | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KMiniAvatar` | `SCC-066` | SCC | 2 | 4 | 12 | `KMiniAvatarSettings,KPlayerServer` | UNVERIFIED | B-ROOT-KMiniAvatar | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KNewExtPointManager` | `SCC-066` | SCC | 4 | 9 | 12 | `KLogClient,KPlayerServer,KRelayClient,KRewardsShop,KScriptCenter` | UNVERIFIED | B-ROOT-KNewExtPointManager | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KNpcReviveManager` | `SCC-066` | SCC | 3 | 15 | 21 | `KAIVM,KActivityMgrServer,KG_Time,KNpcAdronTab,KSO3World,KThreatList` | UNVERIFIED | B-ROOT-KNpcReviveManager | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KNpcTemplateList` | `SCC-066` | SCC | 17 | 3 | 13 | `KShopCenter` | UNVERIFIED | B-ROOT-KNpcTemplateList | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KPlayerServer` | `SCC-066` | SCC | 403 | 820 | 543 | `CRC32,KAchievement,KActivityMgrServer,KBookList,KBuffList,KCDTimerList,KCampInfo,KCharacter` | UNVERIFIED | B-ROOT-KPlayerServer | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KPlayerServerBase` | `SCC-066` | SCC | 308 | 29 | 28 | `KFellowship,KGCRT,KG_Package,KLogClient,KPlayerServer,KRelayClient,KSO3World,KSceneObject` | UNVERIFIED | B-ROOT-KPlayerServerBase | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KProfessionList` | `SCC-066` | SCC | 22 | 11 | 20 | `KCraft,KPlayerServer,KProfessionManager,KRecipeList,KStatDataServer` | UNVERIFIED | B-ROOT-KProfessionList | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KProfessionManager` | `SCC-066` | SCC | 14 | 7 | 16 | `KCraft,KProfession,KRecipeMaster` | UNVERIFIED | B-ROOT-KProfessionManager | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KQuestList` | `SCC-066` | SCC | 34 | 165 | 63 | `KAchievement,KBookList,KCDTimerList,KCampInfo,KCenterRemote,KCoolDownList,KCurrency,KCurrencyList` | UNVERIFIED | B-ROOT-KQuestList | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KRecipeList` | `SCC-066` | SCC | 3 | 9 | 13 | `KCraft,KPlayerServer,KProfessionList,KProfessionManager` | UNVERIFIED | B-ROOT-KRecipeList | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KRegressionPlayerData` | `SCC-066` | SCC | 1 | 23 | 10 | `KPlayerServer,KRegressionManager,KScriptCenter` | UNVERIFIED | B-ROOT-KRegressionPlayerData | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KRelayClient` | `SCC-066` | SCC | 158 | 508 | 375 | `KAntiFarmerServer,KCampInfo,KCenterRemote,KCharacter,KDirectMentorCache,KExterior,KFellowship,KGPQ` | UNVERIFIED | B-ROOT-KRelayClient | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KReputation` | `SCC-066` | SCC | 12 | 11 | 13 | `KPlayerServer,KReputationLimit` | UNVERIFIED | B-ROOT-KReputation | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KRewardsShop` | `SCC-066` | SCC | 8 | 31 | 8 | `KFellowPetBox,KItemList,KLogClient,KMiniAvatar,KPlayerServer,KRelayClient,KRewardsBox,KRewardsShopSettings` | UNVERIFIED | B-ROOT-KRewardsShop | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KSO3World` | `SCC-066` | SCC | 78 | 134 | 31 | `KAIManager,KAIPatrolGroupManager,KActivityMgrServer,KAntiFarmerServer,KAttrModifier,KBuffManager,KCampInfo,KCenterRemote` | UNVERIFIED | B-ROOT-KSO3World | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KScriptServer` | `SCC-066` | SCC | 24 | 13 | 13 | `KPlayerServer,KScriptCenter,lzo1x_c,lzo1x_d` | UNVERIFIED | B-ROOT-KScriptServer | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KShop` | `SCC-066` | SCC | 19 | 150 | 20 | `KAchievement,KCDTimerList,KCharacter,KCoolDownList,KCurrency,KCurrencyList,KItemList,KLogClient` | UNVERIFIED | B-ROOT-KSHOP | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KShopCenter` | `SCC-066` | SCC | 32 | 8 | 19 | `KItemList,KPlayerServer,KSO3World,KShop` | UNVERIFIED | B-ROOT-KShopCenter | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KSkill` | `SCC-066` | SCC | 34 | 135 | 68 | `KAISearchTactics,KBuff,KBuffList,KCDTimerList,KCharacter,KItemList,KLocalGSDataStat,KMath` | UNVERIFIED | B-ROOT-KSkill | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KSkillDebug` | `SCC-066` | SCC | 8 | 1 | 9 | `KPlayerServer` | UNVERIFIED | B-ROOT-KSkillDebug | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KSkillList` | `SCC-066` | SCC | 17 | 40 | 23 | `KG_Time,KItemList,KLogClient,KPlayerServer,KReputation,KScriptCenter,KSkill,KSkillManager` | UNVERIFIED | B-ROOT-KSkillList | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KSkillManager` | `SCC-066` | SCC | 70 | 43 | 54 | `KAttribute,KG_Time,KPlayerServer,KScriptCenter,KSkill,KSkillList` | UNVERIFIED | B-ROOT-KSkillManager | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KSkillRecipeList` | `SCC-066` | SCC | 7 | 14 | 21 | `KPlayerServer,KSkillList,KSkillManager` | UNVERIFIED | B-ROOT-KSkillRecipeList | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KSmartDialogList` | `SCC-066` | SCC | 3 | 2 | 4 | `KNpcTemplateList,KPlayerServer` | UNVERIFIED | B-ROOT-KSmartDialogList | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KStatDataServer` | `SCC-066` | SCC | 132 | 3 | 56 | `KRelayClient` | UNVERIFIED | B-ROOT-KStatDataServer | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KTalentSkill` | `SCC-066` | SCC | 1 | 14 | 12 | `KPlayerServer,KScriptCenter,KSkill,KSkillList,KSkillManager` | UNVERIFIED | B-ROOT-KTalentSkill | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KTarget` | `SCC-066` | SCC | 220 | 3 | 21 | `KItemList` | UNVERIFIED | B-RECIP-KITEMLIST-KTARGET | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KThreatList` | `SCC-066` | SCC | 55 | 11 | 37 | `KAIVM,KMath,KNpcTemplateList,KPlayerServer` | UNVERIFIED | B-ROOT-KThreatList | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KTimeLimitationBindItemMgr` | `SCC-066` | SCC | 5 | 8 | 13 | `KScriptServer` | UNVERIFIED | B-ROOT-KTimeLimitationBindItemMgr | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KTongDiplomacyCache` | `SCC-066` | SCC | 16 | 5 | 15 | `KPlayerServer,KRelayClient,KTongServer` | UNVERIFIED | B-ROOT-KTongDiplomacyCache | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KTongServer` | `SCC-066` | SCC | 11 | 4 | 12 | `KPlayerServer,KRelayClient` | UNVERIFIED | B-ROOT-KTongServer | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KTradingBox` | `SCC-066` | SCC | 20 | 61 | 22 | `KDirectMentorCache,KItemList,KLogClient,KMentorCache,KPlayerServer,KShopCenter,KTimeLimitationBindItemMgr` | UNVERIFIED | B-ROOT-KTradingBox | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KUserPreferences` | `SCC-066` | SCC | 1 | 1 | 8 | `KPlayerServer` | UNVERIFIED | B-ROOT-KUserPreferences | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 71 | `KWorldSettings` | `SCC-066` | SCC | 6 | 85 | 5 | `KAchievementInfoList,KAntiFarmerSettings,KCharacterActionList,KCoolDownList,KDesignationList,KDoodadClassList,KDoodadTemplateList,KGMList` | WORKTREE_CHANGED_UNVERIFIED | B-ROOT-KWorldSettings | RECURSIVE_SLICE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 72 | `KSO3GameServer` | `SCC-089` | READY | 4 | 26 | 5 | `KGameServerEyes,KLocalGSDataStat,KPlayerServerBase,KRecorderFactory,KRelayClient,KSO3World,KStatDataServer` | UNVERIFIED | B-SCC-089 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 73 | `KBandwidthOptimizeTactics` | `SCC-067` | READY | 1 | 0 | 2 | `-` | UNVERIFIED | B-SCC-067 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 74 | `KRegion` | `SCC-069` | READY | 9 | 9 | 41 | `KBandwidthOptimizeTactics,KG_Memory,KSO3World,KTerrainData` | UNVERIFIED | B-SCC-069 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 75 | `Main` | `SCC-090` | READY | 0 | 6 | 9 | `KG_Memory,KSO3GameServer` | UNVERIFIED | B-SCC-090 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 76 | `KPlayer` | `SCC-087` | READY | 0 | 5 | 3 | `KScriptCenter` | WORKTREE_CHANGED_UNVERIFIED | B-SCC-087 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 77 | `KPKController` | `SCC-086` | READY | 0 | 8 | 1 | `KBuffList,KThreatList` | UNVERIFIED | B-SCC-086 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 78 | `KLuaTarget` | `SCC-085` | READY | 0 | 11 | 6 | `KTarget` | UNVERIFIED | B-SCC-085 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 79 | `KLuaSkill` | `SCC-084` | READY | 0 | 5 | 20 | `KBuff,KBuffManager` | UNVERIFIED | B-SCC-084 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 80 | `KLuaRewardsShop` | `SCC-083` | READY | 0 | 2 | 2 | `KRewardsShop,KRewardsShopSettings` | UNVERIFIED | B-SCC-083 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 81 | `KLuaProfession` | `SCC-082` | READY | 0 | 1 | 1 | `KProfession` | UNVERIFIED | B-SCC-082 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 82 | `KLuaPlayer` | `SCC-081` | READY | 0 | 1 | 2 | `KRecipe` | UNVERIFIED | B-SCC-081 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 83 | `KLuaMentorCache` | `SCC-080` | READY | 0 | 2 | 5 | `KMentorCache` | UNVERIFIED | B-SCC-080 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 84 | `KLuaHairShop` | `SCC-079` | READY | 0 | 6 | 6 | `KHairBox,KHairShop` | UNVERIFIED | B-SCC-079 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 85 | `KLuaExterior` | `SCC-078` | READY | 0 | 11 | 8 | `KExterior,KExteriorBox` | UNVERIFIED | B-SCC-078 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 86 | `KLuaDirectMentorCache` | `SCC-077` | READY | 0 | 1 | 5 | `KDirectMentorCache` | UNVERIFIED | B-SCC-077 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 87 | `KLuaCampInfo` | `SCC-076` | READY | 0 | 6 | 9 | `KCampInfo` | UNVERIFIED | B-SCC-076 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 88 | `KG_Socket` | `SCC-074` | READY | 0 | 22 | 54 | `KG_Memory,KSG_EncodeDecode,easycrypt` | UNVERIFIED | B-SCC-074 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 89 | `KGMoveProcessor` | `SCC-070` | READY | 0 | 48 | 23 | `KAIVM,KMath,KRegion` | UNVERIFIED | B-SCC-070 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| 90 | `KCampActiveStat` | `SCC-068` | READY | 0 | 1 | 5 | `KRelayClient` | UNVERIFIED | B-SCC-068 | SCC_BUNDLE | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| - | `AloneFile` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-AloneFile | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `CharacterSetDepend` | `NONE` | NO_GRAPH | 0 | 0 | 3 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-CharacterSetDepend | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `Debug` | `NONE` | NO_GRAPH | 0 | 0 | 3 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-Debug | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `EDOneTimePad` | `NONE` | NO_GRAPH | 0 | 0 | 2 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-EDOneTimePad | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `File` | `NONE` | NO_GRAPH | 0 | 0 | 21 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-File | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `FilePath` | `NONE` | NO_GRAPH | 0 | 0 | 25 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-FilePath | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `IniFile` | `NONE` | NO_GRAPH | 0 | 0 | 5 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-IniFile | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `KAccountSecurityManager` | `NONE` | NO_GRAPH | 0 | 0 | 6 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KActivityManager` | `NONE` | NO_GRAPH | 0 | 0 | 4 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KActivityMgrCenter` | `NONE` | NO_GRAPH | 0 | 0 | 21 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KAntiFarmer` | `NONE` | NO_GRAPH | 0 | 0 | 5 | `-` | GS_AUDITED_READY | B-ROOT-KAntiFarmer | SINGLE_ROOT | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| - | `KAntiFarmerManager` | `NONE` | NO_GRAPH | 0 | 0 | 15 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KArenaManager` | `NONE` | NO_GRAPH | 0 | 0 | 48 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KAuctionDB` | `NONE` | NO_GRAPH | 0 | 0 | 24 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KAuctionDBThread` | `NONE` | NO_GRAPH | 0 | 0 | 18 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-KAuctionDBThread | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `KAuctionManager` | `NONE` | NO_GRAPH | 0 | 0 | 19 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KBMItemSet` | `NONE` | NO_GRAPH | 0 | 0 | 6 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KBMManager` | `NONE` | NO_GRAPH | 0 | 0 | 18 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KBMSale` | `NONE` | NO_GRAPH | 0 | 0 | 20 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KBMSettings` | `NONE` | NO_GRAPH | 0 | 0 | 5 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KBattleFieldManager` | `NONE` | NO_GRAPH | 0 | 0 | 46 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KCampManager` | `NONE` | NO_GRAPH | 0 | 0 | 20 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KClientFileCheckSum` | `NONE` | NO_GRAPH | 0 | 0 | 3 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KDBBase` | `NONE` | NO_GRAPH | 0 | 0 | 7 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-KDBBase | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `KDBTools` | `NONE` | NO_GRAPH | 0 | 0 | 65 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KDirectMentorCenter` | `NONE` | NO_GRAPH | 0 | 0 | 18 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KDoodadFile` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | GS_AUDITED_FACTORY_WIRED | B-ROOT-KDoodadFile | SINGLE_ROOT | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| - | `KDungeonQueuingManager` | `NONE` | NO_GRAPH | 0 | 0 | 36 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KEnumConvertor` | `NONE` | NO_GRAPH | 0 | 0 | 2 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-KEnumConvertor | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `KExteriorDef` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-KExteriorDef | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `KFellowshipCenter` | `NONE` | NO_GRAPH | 0 | 0 | 8 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-KFellowshipCenter | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `KFile` | `NONE` | NO_GRAPH | 0 | 0 | 9 | `-` | OUT_OF_GS_SCOPE_REUSE_ENGINE | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KFileRecord` | `NONE` | NO_GRAPH | 0 | 0 | 4 | `-` | OUT_OF_GS_SCOPE_REUSE_ENGINE | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KGItemHouse` | `NONE` | NO_GRAPH | 0 | 0 | 2 | `-` | OUT_OF_GS_SCOPE_REUSE_ITEMHOUSE | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KGItemInfoList` | `NONE` | NO_GRAPH | 0 | 0 | 49 | `-` | OUT_OF_GS_SCOPE_REUSE_ITEMHOUSE | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KGLog` | `NONE` | NO_GRAPH | 0 | 0 | 5 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-KGLog | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `KGPQCenter` | `NONE` | NO_GRAPH | 0 | 0 | 24 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KGPack` | `NONE` | NO_GRAPH | 0 | 0 | 4 | `-` | OUT_OF_GS_SCOPE_REUSE | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KGPackFile` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | OUT_OF_GS_SCOPE_REUSE | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KGPackFileV3` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | OUT_OF_GS_SCOPE_REUSE | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KGPackFileV3Adapter` | `NONE` | NO_GRAPH | 0 | 0 | 3 | `-` | OUT_OF_GS_SCOPE_REUSE | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KGPack_AllocFileBuffer` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | OUT_OF_GS_SCOPE_REUSE | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KGPack_CheckAndLogPackCRC` | `NONE` | NO_GRAPH | 0 | 0 | 4 | `-` | OUT_OF_GS_SCOPE_REUSE | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KGPack_Decompressor` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | OUT_OF_GS_SCOPE_REUSE | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KGPack_FileMappingFileBuffer` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | OUT_OF_GS_SCOPE_REUSE | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KGPack_IFileBuffer` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | OUT_OF_GS_SCOPE_REUSE | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KGPack_MiniBackup` | `NONE` | NO_GRAPH | 0 | 0 | 4 | `-` | OUT_OF_GS_SCOPE_REUSE | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KGPack_PackageFile` | `NONE` | NO_GRAPH | 0 | 0 | 9 | `-` | OUT_OF_GS_SCOPE_REUSE | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KGPack_PackageFileMergeIndexManager32` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | OUT_OF_GS_SCOPE_REUSE | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KGPack_PackageFileMergeIndexManager64` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | OUT_OF_GS_SCOPE_REUSE | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KGPack_PackageFilePatcher` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | OUT_OF_GS_SCOPE_REUSE | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KGPack_Wrapper` | `NONE` | NO_GRAPH | 0 | 0 | 4 | `-` | OUT_OF_GS_SCOPE_REUSE | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KG_CreateGUID` | `NONE` | NO_GRAPH | 0 | 0 | 3 | `-` | OUT_OF_GS_SCOPE_REUSE | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KG_FileMapping` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | OUT_OF_GS_SCOPE_REUSE | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KG_FileNameHash` | `NONE` | NO_GRAPH | 0 | 0 | 8 | `-` | OUT_OF_GS_SCOPE_REUSE | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |

| - | `KG_GetTypeFromAPC` | `NONE` | NO_GRAPH | 0 | 0 | 6 | `-` | GS_SOURCE_MISSING_UNRESOLVED | PENDING-TARGET-KG_GetTypeFromAPC | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `KG_IniFile` | `NONE` | NO_GRAPH | 0 | 0 | 6 | `-` | OUT_OF_GS_SCOPE_REUSE_ENGINE | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KG_InterlockedVariable` | `NONE` | NO_GRAPH | 0 | 0 | 9 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-KG_InterlockedVariable | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `KG_LockFreeRingQueue` | `NONE` | NO_GRAPH | 0 | 0 | 6 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-KG_LockFreeRingQueue | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `KGameCardManager` | `NONE` | NO_GRAPH | 0 | 0 | 37 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KGameCenterEyes` | `NONE` | NO_GRAPH | 0 | 0 | 17 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KGameServer` | `NONE` | NO_GRAPH | 0 | 0 | 395 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KGatewayAgency` | `NONE` | NO_GRAPH | 0 | 0 | 48 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KGlobalCustomDataManager` | `NONE` | NO_GRAPH | 0 | 0 | 7 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KGlobalSystemValueManager` | `NONE` | NO_GRAPH | 0 | 0 | 3 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KHometownList` | `NONE` | NO_GRAPH | 0 | 0 | 2 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-KHometownList | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `KIDSequence` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KItemDispatchAgency` | `NONE` | NO_GRAPH | 0 | 0 | 9 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KItemDispatchManager` | `NONE` | NO_GRAPH | 0 | 0 | 6 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KLogServerAgency` | `NONE` | NO_GRAPH | 0 | 0 | 76 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KLuaActivityManager` | `NONE` | NO_GRAPH | 0 | 0 | 4 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-KLuaActivityManager | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `KLuaDirectMentorCenter` | `NONE` | NO_GRAPH | 0 | 0 | 18 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-KLuaDirectMentorCenter | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `KLuaItemDispatchManager` | `NONE` | NO_GRAPH | 0 | 0 | 4 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-KLuaItemDispatchManager | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `KLuaMentorCenter` | `NONE` | NO_GRAPH | 0 | 0 | 19 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-KLuaMentorCenter | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `KLuaPackage` | `NONE` | NO_GRAPH | 0 | 0 | 10 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-KLuaPackage | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `KLuaQuestInfo` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-KLuaQuestInfo | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `KLuaScriptEx` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | OUT_OF_GS_SCOPE_REUSE_ENGINE | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KLuaTong` | `NONE` | NO_GRAPH | 0 | 0 | 54 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-KLuaTong | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `KMailBox` | `NONE` | NO_GRAPH | 0 | 0 | 14 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KMailManager` | `NONE` | NO_GRAPH | 0 | 0 | 49 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KMapCopy` | `NONE` | NO_GRAPH | 0 | 0 | 23 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KMapInfo` | `NONE` | NO_GRAPH | 0 | 0 | 19 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KMapManager` | `NONE` | NO_GRAPH | 0 | 0 | 31 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KMemBase` | `NONE` | NO_GRAPH | 0 | 0 | 11 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-KMemBase | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `KMemStack` | `NONE` | NO_GRAPH | 0 | 0 | 7 | `-` | OUT_OF_GS_SCOPE_REUSE_ENGINE | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KMentorCenter` | `NONE` | NO_GRAPH | 0 | 0 | 21 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KMiscDB` | `NONE` | NO_GRAPH | 0 | 0 | 83 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KMiscDBThread` | `NONE` | NO_GRAPH | 0 | 0 | 65 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-KMiscDBThread | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `KNpcFile` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | GS_AUDITED_FACTORY_WIRED | B-ROOT-KNpcFile | SINGLE_ROOT | MASTER_ELIGIBLE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| - | `KOccupyList` | `NONE` | NO_GRAPH | 0 | 0 | 8 | `-` | OUT_OF_GS_SCOPE_REUSE_ENGINE | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KPlayerRelatedPetAttrParam` | `NONE` | NO_GRAPH | 0 | 0 | 2 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-KPlayerRelatedPetAttrParam | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `KPolygon` | `NONE` | NO_GRAPH | 0 | 0 | 11 | `-` | OUT_OF_GS_SCOPE_REUSE_ENGINE | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KRankDataDef` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-KRankDataDef | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `KRemoteRoleManager` | `NONE` | NO_GRAPH | 0 | 0 | 32 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KRemoteScript` | `NONE` | NO_GRAPH | 0 | 0 | 11 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KRoadCenter` | `NONE` | NO_GRAPH | 0 | 0 | 4 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KRole` | `NONE` | NO_GRAPH | 0 | 0 | 12 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KRoleDB` | `NONE` | NO_GRAPH | 0 | 0 | 29 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KRoleDBThread` | `NONE` | NO_GRAPH | 0 | 0 | 43 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-KRoleDBThread | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `KRoleDataVersionConvert` | `NONE` | NO_GRAPH | 0 | 0 | 6 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-KRoleDataVersionConvert | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `KRoleDungeonStat` | `NONE` | NO_GRAPH | 0 | 0 | 6 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KRoleLoginTimeLimit` | `NONE` | NO_GRAPH | 0 | 0 | 9 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KRoleManager` | `NONE` | NO_GRAPH | 0 | 0 | 34 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KSG_MD5_String` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-KSG_MD5_String | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `KSG_StringProcess` | `NONE` | NO_GRAPH | 0 | 0 | 2 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-KSG_StringProcess | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `KSO3GameCenter` | `NONE` | NO_GRAPH | 0 | 0 | 7 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KSO3GameCenterSettings` | `NONE` | NO_GRAPH | 0 | 0 | 5 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KScriptFuncList` | `NONE` | NO_GRAPH | 0 | 0 | 134 | `-` | GS_MULTI_FILE_CLOSURE_UNRESOLVED | B-MULTI-KSCRIPTFUNCLIST | MULTI_FILE_CLOSURE | MASTER_OR_SLAVE | EXCLUSIVE_SOURCE_PATH | 2026-08-06 |
| - | `KScriptManager` | `NONE` | NO_GRAPH | 0 | 0 | 21 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KSemaphore` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | OUT_OF_GS_SCOPE_REUSE_ENGINE | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KSerialNumberGenerator` | `NONE` | NO_GRAPH | 0 | 0 | 3 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-KSerialNumberGenerator | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `KSingleDungeonRankingList` | `NONE` | NO_GRAPH | 0 | 0 | 18 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KSndaAgency` | `NONE` | NO_GRAPH | 0 | 0 | 3 | `-` | OUT_OF_GS_SCOPE_REUSE_GATEWAY | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KStatDataManager` | `NONE` | NO_GRAPH | 0 | 0 | 29 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KSwitchCenterManager` | `NONE` | NO_GRAPH | 0 | 0 | 16 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KTeamCenter` | `NONE` | NO_GRAPH | 0 | 0 | 32 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KTextFilter` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | OUT_OF_GS_SCOPE_REUSE_ENGINE | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KThread` | `NONE` | NO_GRAPH | 0 | 0 | 6 | `-` | OUT_OF_GS_SCOPE_REUSE_ENGINE | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KTong` | `NONE` | NO_GRAPH | 0 | 0 | 65 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KTongBattleFieldManager` | `NONE` | NO_GRAPH | 0 | 0 | 31 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KTongDiplomacyCenter` | `NONE` | NO_GRAPH | 0 | 0 | 44 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KTongHistory` | `NONE` | NO_GRAPH | 0 | 0 | 17 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KTongManager` | `NONE` | NO_GRAPH | 0 | 0 | 49 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KTongRepertory` | `NONE` | NO_GRAPH | 0 | 0 | 19 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `KZoneClient` | `NONE` | NO_GRAPH | 0 | 0 | 149 | `-` | OUT_OF_GS_SCOPE_REUSE_CENTER | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `Luna` | `NONE` | NO_GRAPH | 0 | 0 | 48 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-Luna | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `Mutex` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-Mutex | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `ObjCache` | `NONE` | NO_GRAPH | 0 | 0 | 11 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-ObjCache | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `PackFile` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-PackFile | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `PakWriteTrans` | `NONE` | NO_GRAPH | 0 | 0 | 7 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-PakWriteTrans | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `Random` | `NONE` | NO_GRAPH | 0 | 0 | 3 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-Random | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `Tab` | `NONE` | NO_GRAPH | 0 | 0 | 2 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-Tab | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `TabFile` | `NONE` | NO_GRAPH | 0 | 0 | 4 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-TabFile | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `Text` | `NONE` | NO_GRAPH | 0 | 0 | 22 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-Text | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `Timer` | `NONE` | NO_GRAPH | 0 | 0 | 8 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-Timer | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `Utf8AndWideChar` | `NONE` | NO_GRAPH | 0 | 0 | 3 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-Utf8AndWideChar | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `XPackFile` | `NONE` | NO_GRAPH | 0 | 0 | 28 | `-` | OUT_OF_GS_SCOPE_REUSE_ENGINE | B-EXTERNAL-REUSE | EXCLUDED | EXCLUDED | READ_ONLY | 2026-08-06 |
| - | `XPackList` | `NONE` | NO_GRAPH | 0 | 0 | 16 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-XPackList | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `adler32` | `NONE` | NO_GRAPH | 0 | 0 | 2 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-adler32 | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `alloc` | `NONE` | NO_GRAPH | 0 | 0 | 5 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-alloc | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `arc4` | `NONE` | NO_GRAPH | 0 | 0 | 3 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-arc4 | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `array` | `NONE` | NO_GRAPH | 0 | 0 | 12 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-array | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `asn` | `NONE` | NO_GRAPH | 0 | 0 | 3 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-asn | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `bchange` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-bchange | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `bmove_upp` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-bmove_upp | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `buffer` | `NONE` | NO_GRAPH | 0 | 0 | 23 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-buffer | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `cert_wrapper` | `NONE` | NO_GRAPH | 0 | 0 | 28 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-cert_wrapper | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `charset` | `NONE` | NO_GRAPH | 0 | 0 | 20 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-charset | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `charset-def` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-charset-def | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `client` | `NONE` | NO_GRAPH | 0 | 0 | 39 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-client | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `coding` | `NONE` | NO_GRAPH | 0 | 0 | 4 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-coding | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `compress` | `NONE` | NO_GRAPH | 0 | 0 | 3 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-compress | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `crc32` | `NONE` | NO_GRAPH | 0 | 0 | 3 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-crc32 | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `crypto_wrapper` | `NONE` | NO_GRAPH | 0 | 0 | 11 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-crypto_wrapper | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `ctype` | `NONE` | NO_GRAPH | 0 | 0 | 9 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-ctype | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `ctype-big5` | `NONE` | NO_GRAPH | 0 | 0 | 9 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-ctype-big5 | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `ctype-bin` | `NONE` | NO_GRAPH | 0 | 0 | 17 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-ctype-bin | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `ctype-cp932` | `NONE` | NO_GRAPH | 0 | 0 | 9 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-ctype-cp932 | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `ctype-czech` | `NONE` | NO_GRAPH | 0 | 0 | 4 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-ctype-czech | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `ctype-euc_kr` | `NONE` | NO_GRAPH | 0 | 0 | 5 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-ctype-euc_kr | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `ctype-eucjpms` | `NONE` | NO_GRAPH | 0 | 0 | 6 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-ctype-eucjpms | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `ctype-gb2312` | `NONE` | NO_GRAPH | 0 | 0 | 5 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-ctype-gb2312 | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `ctype-gbk` | `NONE` | NO_GRAPH | 0 | 0 | 10 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-ctype-gbk | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `ctype-latin1` | `NONE` | NO_GRAPH | 0 | 0 | 6 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-ctype-latin1 | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `ctype-mb` | `NONE` | NO_GRAPH | 0 | 0 | 19 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-ctype-mb | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `ctype-simple` | `NONE` | NO_GRAPH | 0 | 0 | 39 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-ctype-simple | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `ctype-sjis` | `NONE` | NO_GRAPH | 0 | 0 | 9 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-ctype-sjis | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `ctype-tis620` | `NONE` | NO_GRAPH | 0 | 0 | 5 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-ctype-tis620 | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `ctype-uca` | `NONE` | NO_GRAPH | 0 | 0 | 16 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-ctype-uca | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `ctype-ucs2` | `NONE` | NO_GRAPH | 0 | 0 | 35 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-ctype-ucs2 | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `ctype-ujis` | `NONE` | NO_GRAPH | 0 | 0 | 6 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-ctype-ujis | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `ctype-utf8` | `NONE` | NO_GRAPH | 0 | 0 | 16 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-ctype-utf8 | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `ctype-win1250ch` | `NONE` | NO_GRAPH | 0 | 0 | 3 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-ctype-win1250ch | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `default` | `NONE` | NO_GRAPH | 0 | 0 | 10 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-default | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `deflate` | `NONE` | NO_GRAPH | 0 | 0 | 16 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-deflate | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `des` | `NONE` | NO_GRAPH | 0 | 0 | 2 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-des | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `dh` | `NONE` | NO_GRAPH | 0 | 0 | 5 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-dh | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `dsa` | `NONE` | NO_GRAPH | 0 | 0 | 23 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-dsa | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `errmsg` | `NONE` | NO_GRAPH | 0 | 0 | 2 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-errmsg | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `errors` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-errors | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `file` | `NONE` | NO_GRAPH | 0 | 0 | 5 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-file | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `gthr-default` | `NONE` | NO_GRAPH | 0 | 0 | 20 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-gthr-default | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `inffast` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-inffast | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `inflate` | `NONE` | NO_GRAPH | 0 | 0 | 12 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-inflate | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `inftrees` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-inftrees | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `inoutmac` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-inoutmac | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `int2str` | `NONE` | NO_GRAPH | 0 | 0 | 2 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-int2str | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `integer` | `NONE` | NO_GRAPH | 0 | 0 | 50 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-integer | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `io` | `NONE` | NO_GRAPH | 0 | 0 | 2 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-io | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `is_prefix` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-is_prefix | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `libmysql` | `NONE` | NO_GRAPH | 0 | 0 | 153 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-libmysql | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `list` | `NONE` | NO_GRAPH | 0 | 0 | 7 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-list | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `log` | `NONE` | NO_GRAPH | 0 | 0 | 3 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-log | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `longlong2str` | `NONE` | NO_GRAPH | 0 | 0 | 2 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-longlong2str | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `luna` | `NONE` | NO_GRAPH | 0 | 0 | 3 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-luna | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `md5` | `NONE` | NO_GRAPH | 0 | 0 | 4 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-md5 | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `mf_arr_appstr` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-mf_arr_appstr | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `mf_dirname` | `NONE` | NO_GRAPH | 0 | 0 | 3 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-mf_dirname | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `mf_fn_ext` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-mf_fn_ext | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `mf_format` | `NONE` | NO_GRAPH | 0 | 0 | 2 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-mf_format | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `mf_loadpath` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-mf_loadpath | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `mf_pack` | `NONE` | NO_GRAPH | 0 | 0 | 7 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-mf_pack | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `mf_qsort` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-mf_qsort | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `misc` | `NONE` | NO_GRAPH | 0 | 0 | 4 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-misc | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `mulalloc` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-mulalloc | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `my_alloc` | `NONE` | NO_GRAPH | 0 | 0 | 9 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-my_alloc | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `my_compress` | `NONE` | NO_GRAPH | 0 | 0 | 5 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-my_compress | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `my_div` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-my_div | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `my_error` | `NONE` | NO_GRAPH | 0 | 0 | 6 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-my_error | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `my_fopen` | `NONE` | NO_GRAPH | 0 | 0 | 3 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-my_fopen | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `my_gethostbyname` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-my_gethostbyname | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `my_getwd` | `NONE` | NO_GRAPH | 0 | 0 | 4 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-my_getwd | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `my_init` | `NONE` | NO_GRAPH | 0 | 0 | 2 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-my_init | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `my_lib` | `NONE` | NO_GRAPH | 0 | 0 | 6 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-my_lib | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `my_malloc` | `NONE` | NO_GRAPH | 0 | 0 | 5 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-my_malloc | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `my_messnc` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-my_messnc | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `my_net` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-my_net | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `my_once` | `NONE` | NO_GRAPH | 0 | 0 | 4 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-my_once | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `my_open` | `NONE` | NO_GRAPH | 0 | 0 | 3 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-my_open | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `my_read` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-my_read | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `my_realloc` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-my_realloc | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `my_strtoll10` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-my_strtoll10 | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `my_symlink` | `NONE` | NO_GRAPH | 0 | 0 | 4 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-my_symlink | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `my_sync` | `NONE` | NO_GRAPH | 0 | 0 | 3 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-my_sync | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `my_time` | `NONE` | NO_GRAPH | 0 | 0 | 19 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-my_time | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `my_vsnprintf` | `NONE` | NO_GRAPH | 0 | 0 | 2 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-my_vsnprintf | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `n2_99` | `NONE` | NO_GRAPH | 0 | 0 | 17 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-n2_99 | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `n2b_d` | `NONE` | NO_GRAPH | 0 | 0 | 6 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-n2b_d | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `n2b_to` | `NONE` | NO_GRAPH | 0 | 0 | 3 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-n2b_to | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `n2d_d` | `NONE` | NO_GRAPH | 0 | 0 | 6 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-n2d_d | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `n2d_to` | `NONE` | NO_GRAPH | 0 | 0 | 3 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-n2d_to | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `n2e_d` | `NONE` | NO_GRAPH | 0 | 0 | 6 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-n2e_d | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `n2e_to` | `NONE` | NO_GRAPH | 0 | 0 | 3 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-n2e_to | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `net` | `NONE` | NO_GRAPH | 0 | 0 | 13 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-net | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `pack` | `NONE` | NO_GRAPH | 0 | 0 | 3 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-pack | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `password` | `NONE` | NO_GRAPH | 0 | 0 | 15 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-password | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `random` | `NONE` | NO_GRAPH | 0 | 0 | 3 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-random | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `rsa` | `NONE` | NO_GRAPH | 0 | 0 | 8 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-rsa | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `runtime` | `NONE` | NO_GRAPH | 0 | 0 | 33 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-runtime | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `sha1` | `NONE` | NO_GRAPH | 0 | 0 | 4 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-sha1 | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `socket_wrapper` | `NONE` | NO_GRAPH | 0 | 0 | 12 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-socket_wrapper | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `stat` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-stat | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `str2int` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-str2int | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `str_alloc` | `NONE` | NO_GRAPH | 0 | 0 | 2 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-str_alloc | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `strcend` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-strcend | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `strend` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-strend | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `strmake` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-strmake | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `strnmov` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-strnmov | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `strtod` | `NONE` | NO_GRAPH | 0 | 0 | 2 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-strtod | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `strxmov` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-strxmov | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `trees` | `NONE` | NO_GRAPH | 0 | 0 | 8 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-trees | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `typelib` | `NONE` | NO_GRAPH | 0 | 0 | 6 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-typelib | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `ucl_crc` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-ucl_crc | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `ucl_init` | `NONE` | NO_GRAPH | 0 | 0 | 7 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-ucl_init | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `ucl_mchw` | `NONE` | NO_GRAPH | 0 | 0 | 21 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-ucl_mchw | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `ucl_ptr` | `NONE` | NO_GRAPH | 0 | 0 | 2 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-ucl_ptr | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `ucl_str` | `NONE` | NO_GRAPH | 0 | 0 | 4 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-ucl_str | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `ucl_swd` | `NONE` | NO_GRAPH | 0 | 0 | 30 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-ucl_swd | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `ucl_util` | `NONE` | NO_GRAPH | 0 | 0 | 8 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-ucl_util | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `uncompr` | `NONE` | NO_GRAPH | 0 | 0 | 1 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-uncompr | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `vio` | `NONE` | NO_GRAPH | 0 | 0 | 4 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-vio | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `viosocket` | `NONE` | NO_GRAPH | 0 | 0 | 18 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-viosocket | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `viossl` | `NONE` | NO_GRAPH | 0 | 0 | 8 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-viossl | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `viosslfactories` | `NONE` | NO_GRAPH | 0 | 0 | 5 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-viosslfactories | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `xml` | `NONE` | NO_GRAPH | 0 | 0 | 13 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-xml | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `yassl_imp` | `NONE` | NO_GRAPH | 0 | 0 | 16 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-yassl_imp | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `yassl_int` | `NONE` | NO_GRAPH | 0 | 0 | 174 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-yassl_int | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |
| - | `zutil` | `NONE` | NO_GRAPH | 0 | 0 | 5 | `-` | NO_TARGET_EVIDENCE_UNRESOLVED | PENDING-TARGET-zutil | TARGET_UNRESOLVED | BLOCKED | UNASSIGNED | 2026-08-06 |

## Scope exclusions from the NO_GRAPH tail

The following rows were rescanned against target DWARF and belong to
`jx3_dwarf/libEngine_Lua5D.so`, not `SO3GameServerD`. Keep them as
`OUT_OF_GS_SCOPE_REUSE`; do not spawn `$jx3-file-port` for them:

- `KGPack*` / `KGPack_*` package, file-buffer, decompressor, and wrapper units
- `KG_CreateGUID`
- `KG_FileNameHash`

Evidence: target DWARF `decl_file` paths are under
`Base/SourceCode/Engine/...`, and the target module is `libEngine_Lua5D.so`
(target SHA prefix `e62e063b...`). Their implementation is an Engine boundary
reused by GS. `KG_GetTypeFromAPC` is intentionally not included: GraphEngine
has both GameCenter and GameServer observations, so it needs a separate
GameServer-scope audit before classification.

### Full NO_GRAPH rescan result

The entire 268-row `NO_GRAPH` tail was rescanned against target symbol
observations and target module origin:

| classification | count | queue decision |
|---|---:|---|
| `GS_SCOPE_AUDIT_REQUIRED` (pre-audit) | 5 | Rescanned into the per-file results below |
| `OUT_OF_GS_SCOPE_REUSE_CENTER` | 57 | Do not port in this GS wave; Center remains stock |
| `OUT_OF_GS_SCOPE_REUSE_GATEWAY` | 1 | Do not port in this GS wave; Gateway remains stock |
| `OUT_OF_GS_SCOPE_REUSE_ENGINE` | 11 | Reuse Engine boundary; do not duplicate in GS |
| `OUT_OF_GS_SCOPE_REUSE_ITEMHOUSE` | 2 | Reuse ItemHouse boundary; do not duplicate in GS |
| `OUT_OF_GS_SCOPE_REUSE` | 18 | Previously audited package/utility reuse rows |
| `NO_TARGET_EVIDENCE_UNRESOLVED` | 174 | Not eligible for port ordering until target module/symbol evidence is found |

The five audited GS candidates and their next actions are recorded below.
`NO_GRAPH` rows are therefore no longer treated as implicit P0 items.

Post-audit status of those five rows: `GS_AUDITED_READY` = 1,
`GS_AUDITED_FACTORY_WIRED` = 2, `GS_MULTI_FILE_CLOSURE_UNRESOLVED` = 1,
and `GS_SOURCE_MISSING_UNRESOLVED` = 1. The earlier 268-row classification
table is retained as the pre-audit rescan baseline; the per-row status and
orchestration projection below are authoritative for new runs.

### GS audit: the five former `GS_SCOPE_AUDIT_REQUIRED` rows

| file / closure | target evidence | source / wiring evidence | result | next order |
|---|---|---|---|---|
| `KAntiFarmer` | 9 target methods in `SO3GameServerD`; `Judge` and `Load` have confirmed callers `KPlayer::ProcessAntiFarmer` and `KPlayer::PartialLoadExtData` | `src/SO3World/Src/KAntiFarmer.cpp/.h`; embedded as `KPlayer::m_AntiFarmer` | `GS_AUDITED_READY` | Port before the `KPlayer` consumer closure |
| `KDoodadFile` | 11 target methods in `SO3GameServerD` | `CreateDoodadFileInterface` is called by `KScene::ServerLoad`; source factory returns `KMemory::New<KDoodadFile>()` | `GS_AUDITED_FACTORY_WIRED` | Parallel with `KNpcFile`; before `KScene::ServerLoad` wiring |
| `KNpcFile` | 11 target methods in `SO3GameServerD` | `CreateNpcFileInterface` is called by `KScene::ServerLoad`; source factory returns `KMemory::New<KNpcFile>()` | `GS_AUDITED_FACTORY_WIRED` | Parallel with `KDoodadFile`; before `KScene::ServerLoad` wiring |
| `KScriptFuncList` | 223 target functions in `SO3GameServerD`; `GetBaseFuncList` is called by `KScriptCenter::CreateScriptHolder` | Source is split across `KBaseFuncList.cpp`, `KShopFuncList.cpp`, and `kLoginFuncList.cpp`; header is only the namespace declaration | `GS_MULTI_FILE_CLOSURE_UNRESOLVED` | Split into base/shop/login sub-closures; do not spawn as one file |
| `KG_GetTypeFromAPC` | Exact target function exists in both `SO3GameServerD` and `SO3GameCenterD`; GS caller is `KScriptFuncList::LuaGetTypeFromAPC` | No corresponding implementation found in current 2010/candidate source tree | `GS_SOURCE_MISSING_UNRESOLVED` | Resolve source/Engine ownership first; retain `PORT-UNKNOWN_REQUIRED[CALLER]` or `[FAILURE]` as applicable |

The `KDoodadFile` and `KNpcFile` rows are independent siblings at the file
implementation level and can be ported in parallel. Their shared downstream
consumer is `KScene::ServerLoad`, so only that wiring step is sequential.

## Worktree rescan

The following source/header paths are dirty or untracked in `linux-build` at `last_update`. Their registry rows are `WORKTREE_CHANGED_UNVERIFIED`; inspect the diff and attach target evidence before promoting any status.

| basename | changed paths |
|---|---|
| `KAntiFarmerSettings` | `include/Include/SO3World/KAntiFarmerSettings.h` + `src/SO3World/Src/KAntiFarmerSettings.cpp` |
| `KGWConstList` | `include/Include/SO3World/KGWConstList.h` + `src/SO3World/Src/KGWConstList.cpp` |
| `KGWServerConstList` | `include/Include/SO3World/KGWServerConstList.h` + `src/SO3World/Src/KGWServerConstList.cpp` |
| `KItemList` | `src/SO3World/Src/KItemList.cpp` + `src/SO3World/Src/KItemList.h` |
| `KMapListFile` | `src/SO3World/Src/KMapListFile.cpp` |
| `KNpcAdronTab` | `include/Include/SO3World/KNpcAdronTab.h` + `src/SO3World/Src/KNpcAdronTab.cpp` |
| `KPendentOldDataInfoList` | `include/Include/SO3World/KPendentOldDataInfoList.h` + `src/SO3World/Src/KPendentOldDataInfoList.cpp` |
| `KPlayer` | `src/SO3World/Src/KPlayer.cpp` + `src/SO3World/Src/KPlayer.h` |
| `KQuestInfoList` | `include/Include/SO3World/KQuestInfoList.h` + `src/SO3World/Src/KQuestInfoList.cpp` |
| `KReputeLootBuffList` | `include/Include/SO3World/KReputeLootBuffList.h` + `src/SO3World/Src/KReputeLootBuffList.cpp` |
| `KScene` | `include/Include/SO3World/KScene.h` + `src/SO3World/Src/KScene.cpp`; no matching target-DWARF file row yet |
| `KTongConstList` | `include/Include/SO3World/KTongConstList.h` + `src/SO3World/Src/KTongConstList.cpp` |
| `KWorldSettings` | `include/Include/SO3World/KWorldSettings.h` + `src/SO3World/Src/KWorldSettings.cpp` |

`KScene` has changed source/header paths but no matching target-DWARF file row in this inventory; keep it `WORKTREE_ONLY_NO_TARGET_FUNCTION_ROW` until target function/type evidence is reconciled. `WORKTREE_CHANGED_UNVERIFIED` does not imply `FILE_CORRECT`, `FILE_BUILD_READY`, or runtime acceptance.

## Per-agent update fields

```yaml
owner: <agent-or-ticket>
root_kind: translation_unit|class|scc_closure
integration_mode: standalone|workflow
status: UNVERIFIED
evidence: <target address/DIE/decompile/compare artifact>
dependencies: <SCC/file IDs>
markers: []
next_action: <one bounded action>
last_update: YYYY-MM-DD
```

## Agent updates

### `KMath` / `SCC-002`

```yaml
owner: codex/jx3-file-port
root_kind: translation_unit
integration_mode: standalone
status: FILE_CORRECT
evidence: linux-build/compare-engine/staging/blocker/kmath-file-port-20260806/{template.md,evidence-matrix.md,surface-matrix.md,state.md}; target DWARF SHA-256 47716c73e8de281c95759cdc4a478e70e7c61322fb46e8c4e04954e51124b94a; reviewer FILE_PORT_REVIEW_PASS
dependencies: external callers deferred; native i686/gnu++98 ABI probe; compare-engine global-function normalizer
markers:
  - PORT-DEFERRED_WIRING[CALLER] owner=downstream caller tickets; target direct-call lane; RUNTIME
  - PORT-UNKNOWN_REQUIRED[ABI] owner=jx3-ticket/KMath-ABI-probe; PRE_BUILD; root_behavior_impact=NO
  - PORT-TODO[TARGET_REQUIRED] owner=compare-engine/KMath-global-normalizer; REVIEW; root_behavior_impact=NO
next_action: KInventory (SCC-003, port_order=2); keep KMath source unchanged and hand ABI probe/comparator support to the owning follow-up gate.
last_update: 2026-08-06
```

### `KInventory` / `SCC-003`

```yaml
owner: codex/jx3-file-port
root_kind: class
integration_mode: standalone
status: FILE_CORRECT
evidence: linux-build/compare-engine/staging/blocker/kinventory-file-port-20260806/{template.md,evidence-matrix.md,surface-matrix.md,state.md,candidate-manifest.json}; target DWARF class DIE 0x06194035; target decompiles 0x083c21b8, 0x083c2222, 0x083c2322, 0x083c25ae; external KGItemHouse::DestroyItem 0x000219c0; reviewer FILE_PORT_REVIEW_PASS
dependencies: reusable external IItemHouse evidence; external KItemList SCC-066 is WORKTREE_CHANGED_UNVERIFIED and read-only
markers:
  - PORT-DEFERRED_WIRING[CALLER] owner=downstream caller tickets; KItemList SCC-066 target caller lane; root_behavior_impact=NO
  - PORT-UNKNOWN_REQUIRED[ABI] owner=jx3-ticket/KInventory-ABI-probe; target i686 layout; PRE_BUILD; root_behavior_impact=NO
next_action: hand off the non-root i686 ABI probe to jx3-ticket; keep KItemList caller wiring deferred. Resume sequential selection at KTeamServer (SCC-024, port_order=3).
last_update: 2026-08-06
```

### `KTeamServer` / `SCC-024`

```yaml
owner: codex/jx3-file-port
root_kind: class
integration_mode: standalone
status: FILE_CORRECT
evidence: linux-build/compare-engine/staging/blocker/kteamserver-file-port-20260806/{template.md,evidence-matrix.md,surface-matrix.md,state.md,candidate-manifest.json}; target DWARF class DIE 0x0008e794 and KTEAM_MEMBER_INFO DIE 0x0008c79f; target decompiles 0x0837d854 and 0x08387d3e; VPS GCC-4.8 i686/GNU++98 ABI MATCH at compare/KTeamServer-20260806-abi/abi/abi-compare.json and compare/KTeamServer-20260806-abi-member/abi-compare.json; reviewer FILE_PORT_REVIEW_PASS + ABI_MARKER_CLOSE_APPROVED
dependencies: read-only dirty KPlayer/SCC-087 source-absent script callee formula reconstructed at the target import edge; external KPlayerServer/SCC-066 caller/record-tail consumers deferred
markers:
  - PORT-DEFERRED_WIRING[IMPORT] owner=KPlayer/SCC-087; evidence=0x0837d974->0x08387d3e; next_action=port KPlayer::CallAutoCastFormationScript in its own owner ticket; RUNTIME; root_behavior_impact=NO
  - PORT-DEFERRED_WIRING[CALLER] owner=KPlayerServer/SCC-066; evidence=0x080e31ce,0x0807ef4c,0x0807eec6; next_action=port target-backed formation and energy/sun/moon member-sync caller wiring; RUNTIME; root_behavior_impact=NO
next_action: ABI probe is closed; hand off only the canonical GameServer build gate to jx3-ticket, and keep KPlayer/KPlayerServer external wiring deferred. Resume sequential selection at KSceneObject (SCC-018, port_order=4).
last_update: 2026-08-06
```

### `KSceneObject` / `SCC-018`

```yaml
owner: codex/jx3-file-port
root_kind: class
integration_mode: standalone
status: FILE_CORRECT
evidence: linux-build/compare-engine/staging/blocker/ksceneobject-file-port-20260806/{scope.md,template.md,evidence-matrix.md,surface-matrix.md,state.md,native-abi-contract.md,candidate-manifest.json}; target DWARF KSceneObject DIE 0x00e40fb8 (0x58) and KSceneObjNode DIE 0x00e9e04c (0x10); target decompiles/objdump 0x08055eea,0x08056b00,0x0811208c,0x08112136,0x0811213c,0x08112176,0x081122f2,0x0811227c,0x08355f44,0x08355fae,0x08355fe8; VPS GCC-4.8 i686 GNU++98 forced-object ABI assertions and class dump; contract test PASS 5/5; reviewer FILE_PORT_REVIEW_PASS
dependencies: KMath/SCC-002 FILE_CORRECT for integer range; read-only source contracts KBaseObject/KSceneObjNode/KCell/KScene; external derived/caller owners deferred
markers:
  - PORT-DEFERRED_WIRING[CALLER] owner=KCharacter/KDoodad and downstream g_InRange callers; evidence=target derived Init/UnInit callers 0x082a37a8,0x082adb6e and broad 0x08056b00 caller set; next_action=only port target-backed caller wiring in that owner ticket when a workflow crosses it; RUNTIME; root_behavior_impact=NO
next_action: hand off the canonical GameServer build gate only to jx3-ticket; do not treat forced-object ABI compile as a GameServer build. Resume sequential selection at KRecipe (SCC-053, port_order=5).
last_update: 2026-08-06
```

### `KRecipe` / `SCC-053`

```yaml
owner: claude/jx3-file-port
root_kind: translation_unit
integration_mode: standalone
status: FILE_BLOCKED
evidence: linux-build/compare-engine/staging/blocker/krecipe-file-port-20260806/{scope.md,template.md,evidence-matrix.md,surface-matrix.md,state.md,native-abi-contract.md,candidate-manifest.json,target-decompile/,artifact.sha256}; target DWARF type DIEs KRecipeBase 0x00630d4b (0x28), KCraftRecipe 0x00688029 (0x1a0), KCraftCollection 0x00688fef (0x30), KCraftRead 0x00689163 (0xa0), KCraftEnchant 0x00689698 (0xfc), KCraftCopy 0x00689ef8 (0xc4), KRecipe<T> 0x00690532 (0x18); 76 visited target addresses incl. loaders 0x080b6c76/0x080b3f74/0x080b5b04/0x080b4c3e/0x080b3c02, line readers 0x080b5e1a/0x080b2b20/0x080b4f76/0x080b4210/0x080b3042, 0x080b2fba, 0x080b29c9, 0x080b2a82, 0x080b6f74, CanCast 0x080b7a2e/0x080b78f2/0x080b77a4/0x080b74d0/0x080b7186, accessors 0x0834c0b6/0x0834c0e6/0x0834c116/0x0834c142, caller 0x082a9f30; VPS GCC-4.8 i686 GNU++98 forced-object probe = 86 assertions MATCH and KRecipe.cpp implementation_compile.status=0; contract test tests/test_krecipe_target_contract.py PASS 14/14 with 16/16 selected mutations caught; compare KRecipe-20260806-{before,after} + non-template KRecipeBase-20260806-after; reviewer round 1 REQUEST_CHANGES -> round 2 FILE_PORT_REVIEW_PASS
dependencies: KPlayer/SCC-087 (dirty, read-only) for CanCostStamina/CanCostThew/IsTongNewMemberLimited/GetCurrentWeapon; KTongServer/SCC-066 for the craft-tech stamina discount; KItemList/SCC-066 (dirty, read-only) + external ItemHouse for IItemHouse slot 0x38 and KItemProperty +0x8/+0xc/+0x10; shared Global.h COMMON_PLAYER_OPERATION_DISTANCE (34 consumers outside this TU)
markers:
  - PORT-UNKNOWN_REQUIRED[CALLER] owner=KPlayer/SCC-087; evidence=0x080b6f74 weapon branch calls KPlayer::GetCurrentWeapon then IItem vslot+4 GetProperty; POLARITY: target short-circuits so a weaponless caster reaches crcSuccess while the candidate returns crcWeaponError; next_action=port GetCurrentWeapon and restore the branch 1:1 including polarity; PRE_BUILD; root_behavior_impact=YES
  - PORT-UNKNOWN_REQUIRED[ABI] owner=KItemList/SCC-066; evidence=0x080b6f74 compares KItemProperty+0x10 against nEquipmentType; next_action=bind the field to a real declaration; PRE_BUILD; root_behavior_impact=YES
  - PORT-UNKNOWN_REQUIRED[CALLER] owner=KPlayer/SCC-087 + KTongServer/SCC-066; evidence=0x080b7a2e CanCostStamina + m_bStaminaDiscountEnable/GetCraftTechConsumeReduceStaminaRate/IsTongNewMemberLimited with ceil(cost*rate/100.0); next_action=port both owners then restore the block 1:1; PRE_BUILD; root_behavior_impact=YES
  - PORT-UNKNOWN_REQUIRED[CALLER] owner=KPlayer/SCC-087; evidence=0x080b78f2, 0x080b7186 KPlayer::CanCostThew; next_action=port CanCostThew; PRE_BUILD; root_behavior_impact=YES
  - PORT-UNKNOWN_REQUIRED[CALLER] owner=KPlayer/SCC-087; evidence=0x080b77a4, 0x080b74d0 KPlayer::CanCostStamina; next_action=port CanCostStamina; PRE_BUILD; root_behavior_impact=YES
  - PORT-UNKNOWN_REQUIRED[CALLER] owner=KItemList/SCC-066 + ItemHouse; evidence=0x080b74d0 resolves the enchant through g_pSO3World->m_piItemHouse vtable slot 0x38; next_action=recover the IItemHouse declaration and route the lookup through it; PRE_BUILD; root_behavior_impact=YES
  - PORT-UNKNOWN_REQUIRED[ABI] owner=KItemList/SCC-066 + ItemHouse; evidence=0x080b74d0 compares KItemProperty+0x8 == 0 and +0xc == enchant record +0x18; next_action=bind those fields; PRE_BUILD; root_behavior_impact=YES
  - PORT-TODO[TARGET_REQUIRED] owner=shared-constant ticket; evidence=0x080b7a2e/0x080b74d0 set m_nDistance = 0x180 and 0x080b7186 calls g_InRange(..., 0x180) while COMMON_PLAYER_OPERATION_DISTANCE = 320; grouped for all three sites in this TU; next_action=re-derive the constant in a shared ticket, not from KRecipe; PRE_BUILD; root_behavior_impact=YES for this TU
next_action: hand off to jx3-ticket: (1) runtime-gate the 96 restored LoadLine guards — every column the target reads exists in all 13 shipped Craft tabs statically, but only a runtime pass can confirm KCraft::Init still succeeds after LoadLine became fail-fast again; (2) open KPlayer/SCC-087, KTongServer/SCC-066 and a shared-constant ticket for the eight markers above, then reopen this row. Ported this run: MAX_RECIPE_NAME_LEN 32->64 in Global.h (line 108 only, sole consumer is this TU), 96 target guards, 3+2 candidate-only locals removed, 5 target cost gates, Enchant CanCast nRetCode local set. Resume sequential selection at KAttribute (SCC-008, port_order=7): KQuestInfoList (SCC-039, port_order=6) is WORKTREE_CHANGED_UNVERIFIED and therefore skipped.
last_update: 2026-08-06
```
