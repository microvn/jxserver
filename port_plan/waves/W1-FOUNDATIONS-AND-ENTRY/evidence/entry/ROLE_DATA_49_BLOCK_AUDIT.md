# Wave 1 — 49 role-data block audit

This is a target-first comparison of the v2.5 role-data tag space against the
2010 source/candidate tree. It does not claim that a source handler is
semantically equivalent to the target handler.

## Authorities

- Target enum/layout: `SO3GameServerD`, SHA-256
  `47716c73e8de281c95759cdc4a478e70e7c61322fb46e8c4e04954e51124b94a`.
- Target control flow: direct PyGhidra decompile of
  `KPlayer::PartialLoadExtData @ 0x0839b62e`, `KPlayer::LoadStateInfo @
  0x0839b112`, and `KPlayer::OnExtDataLoadFinish @ 0x0839fb50`.
- Source baseline: `linux-build/include/Include/KRoleDBDataDef.h` and
  `KPlayer::LoadExtRoleData`/`Save`.

## Target contract

The target has contiguous tags `rbtInvalid=0` through `rbtManualDrop=48`, with
`rbtTotal=49`. Each block is a 12-byte header (`type`, `version`, `length`)
followed by payload. `PartialLoadExtData` advances one block at a time, emits a
section-check request, and only marks `m_bExtDataLoadFinish` after the final
client acknowledgement. A missing source handler is therefore a missing
contract, not an ignorable enum value.

## Matrix

| ID | Target tag | 2010 source/candidate counterpart | Static result | Port consequence |
|---:|---|---|---|---|
| 0 | `rbtInvalid` | `rbtInvalid` | MATCHED enum sentinel | preserve |
| 1 | `rbtSkill` | `rbtSkillList` / `m_SkillList` | NAME DRIFT; payload not reconciled | audit loader/serializer |
| 2 | `rbtItem` | `rbtItemList` / `m_ItemList` | NAME DRIFT; source handler exists | audit loader/serializer |
| 3 | `rbtQuest` | `rbtQuestList` / `m_QuestState` | NAME DRIFT; source handler exists | audit loader/serializer |
| 4 | `rbtProfession` | `rbtProfessionList` | NAME DRIFT; source handler exists | audit loader/serializer |
| 5 | `rbtState` | `rbtStateInfo` / `LoadStateInfo` | target versioned dispatcher; source incomplete | implement version/CRC contract |
| 6 | `rbtBuff` | `rbtBuffList` | NAME DRIFT; source handler exists | audit loader/serializer |
| 7 | `rbtRepute` | `rbtRepute` | source handler exists | compare payload |
| 8 | `rbtUserPreferences` | same | source handler exists | compare payload |
| 9 | `rbtBookState` | same | source handler exists | compare payload |
| 10 | `rbtSkillRecipe` | `rbtSkillRecipeList` | NAME DRIFT; source handler exists | compare payload |
| 11 | `rbtRecipe` | `rbtRecipeList` | NAME DRIFT; source handler exists | compare payload/version |
| 12 | `rbtCoolDown` | `rbtCoolDownTimer` | NAME DRIFT; source handler exists | compare payload |
| 13 | `rbtRoadOpen` | `rbtRoadOpenList` | NAME DRIFT; source handler exists | compare payload |
| 14 | `rbtCustom` | `rbtCustomData` | NAME DRIFT; source handler exists | compare payload |
| 15 | `rbtVisitedMap` | same | source handler exists | compare payload |
| 16 | `rbtPQ` | `rbtPQList` | NAME DRIFT; source handler exists | compare payload |
| 17 | `rbtHero` | `rbtHeroData` | NAME DRIFT; source handler exists | compare payload |
| 18 | `rbtAchievement` | `rbtAchievementData` | NAME DRIFT; source handler exists | compare payload |
| 19 | `rbtDesignation` | `rbtDesignationData` | NAME DRIFT; source handler exists | compare payload |
| 20 | `rbtRand` | `rbtRandData` | NAME DRIFT; source handler exists | compare payload |
| 21 | `rbtAntiFarmer` | `rbtAntiFarmerData` | NAME DRIFT; source handler exists | compare payload |
| 22 | `rbtMentor` | `rbtMentorData` | NAME DRIFT; source handler exists | compare payload |
| 23 | `rbtPendent` | no source tag/handler found | TARGET-ONLY | target contract missing in source |
| 24 | `rbtActivityVariables` | no source tag/handler found | TARGET-ONLY | target contract missing in source |
| 25 | `rbtCurrency` | `rbtCurrencyData` / `KCurrencyList` | NAME DRIFT; candidate addition exists | verify target payload/version |
| 26 | `rbtBankPassword` | no source tag/handler found | TARGET-ONLY | target contract missing in source |
| 27 | `rbtArena` | no source tag/handler found | TARGET-ONLY | target contract missing in source |
| 28 | `rbtDropSurpriseMask` | no source tag/handler found | TARGET-ONLY | target contract missing in source |
| 29 | `rbtCampActiveStat` | no source tag/handler found | TARGET-ONLY | target contract missing in source |
| 30 | `rbtExteriorBox` | `rbtExteriorData` / `m_ExteriorBox` | NAME DRIFT; source handler exists | compare target payload |
| 31 | `rbtExteriorOther` | no separate source tag | TARGET-SPLIT | do not collapse into exterior box |
| 32 | `rbtHairBox` | `rbtHairBoxData` / `m_HairBox` | NAME DRIFT; source handler exists | compare target payload |
| 33 | `rbtRegression` | `rbtRegressionData` / `KRegressionPlayerData` | source handler exists; target block is 22-byte account payload | compare version/length |
| 34 | `rbtDropSurprise` | no source tag/handler found | TARGET-ONLY | target contract missing in source |
| 35 | `rbtSingleDungeon` | no source tag/handler found | TARGET-ONLY | target contract missing in source |
| 36 | `rbtDelayTradeItem` | no source tag/handler found | TARGET-ONLY | target contract missing in source |
| 37 | `rbtTimeLimitReturn` | no source tag/handler found | TARGET-ONLY | target contract missing in source |
| 38 | `rbtTimeLimitSold` | no source tag/handler found | TARGET-ONLY | target contract missing in source |
| 39 | `rbtDomesticate` | no source tag/handler found | TARGET-ONLY | target contract missing in source |
| 40 | `rbtFacePendent` | no source tag/handler found | TARGET-ONLY | target contract missing in source |
| 41 | `rbtFellowPet` | no source tag/handler found | TARGET-ONLY | target contract missing in source |
| 42 | `rbtDynamicPackageSize` | no source tag/handler found | TARGET-ONLY | target contract missing in source |
| 43 | `rbtTongExt` | no source tag/handler found | TARGET-ONLY | target contract missing in source |
| 44 | `rbtTalentSkill` | no source tag/handler found | TARGET-ONLY | target contract missing in source |
| 45 | `rbtMiniAvatar` | `rbtMiniAvatarData` / `m_MiniAvatar` | NAME DRIFT; source handler exists | compare target payload |
| 46 | `rbtVisitTong` | no source tag/handler found | TARGET-ONLY | target contract missing in source |
| 47 | `rbtRewardsBox` | no source tag/handler found | TARGET-ONLY | target contract missing in source |
| 48 | `rbtManualDrop` | no source tag/handler found | TARGET-ONLY | target contract missing in source |

## Audit result

- **49/49 target IDs are enumerated and named.**
- **23 source/candidate handlers are only provisional matches** (name or
  source-shape match; no semantic equivalence claim).
- **26 target IDs have no distinct 2010 source counterpart found**. These are
  not safe to silently skip or map to another block.
- The current B2–B4 bridge is **not a 49-block implementation**. It sets the
  completion gate around the existing source loader and is therefore a bounded
  build probe, not target parity.

## Required next implementation boundary

Before a role-load acceptance claim, add a target-backed dispatch table and
per-block producer/consumer dossier for all 49 IDs. Unknown payloads may be
preserved opaquely, but their header, version, length, ordering, ownership and
acknowledgement must remain intact. No target-only block may be treated as
optional merely because the 2010 source lacks a symbol.

