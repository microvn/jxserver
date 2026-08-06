#!/usr/bin/env python3
"""Target-derived static contracts for the standalone KRecipe translation unit.

Target authority: jx3_dwarf/SO3GameServerD
SHA-256 47716c73e8de281c95759cdc4a478e70e7c61322fb46e8c4e04954e51124b94a.

Every expectation below is bound to a recorded target fact:

  layout          DWARF KRecipeBase 0x00630d4b, KCraftRecipe 0x00688029,
                  KCraftCollection 0x00688fef, KCraftRead 0x00689163,
                  KCraftEnchant 0x00689698, KCraftCopy 0x00689ef8
  name width      DWARF "char[64]" at KRecipe.h:41/185/214/278, plus
                  KCraftRead::setName 0x0834c0b6 / KCraftEnchant::setName
                  0x0834c116 (strncpy bound 0x40, terminator at [0x3f]) and the
                  GetString bound 0x40 inside every LoadLine
  belong width    KCraftEnchant::setBelong 0x0834c0e6 / KCraftRecipe::setBelong
                  0x0834c142 (strncpy bound 0x20)
  loaders         KRecipe<T>::Init 0x080b6c76 / 0x080b3f74 / 0x080b5b04 /
                  0x080b4c3e / 0x080b3c02
  line readers    KRecipe<T>::LoadLine 0x080b5e1a / 0x080b2b20 / 0x080b4f76 /
                  0x080b4210 / 0x080b3042
  id helpers      BookID2RecipeID 0x080b2a82, RecipeID2BookID 0x080b29c9

The .tab column check is data-driven from the shipped settings/Craft/craft.tab
RecipeTabPath column, matching the target dispatch in KCraft::Init 0x082a9f30.
"""
from pathlib import Path
import re
import unittest


ROOT = Path(__file__).resolve().parents[1]
CORE = ROOT / "src/SO3World/Src/KRecipe.cpp"
HEADER = ROOT / "src/SO3World/Src/KRecipe.h"
GLOBAL = ROOT / "include/Include/SO3World/Global.h"
CRAFT_DIR = ROOT.parent / "镜像端/extracted/root/settings/Craft"

# Ordered ITabFile reads recovered from each target LoadLine.  Loop bodies read
# a formatted column name, so the loop prefixes are listed once with their
# index range instead of being expanded per iteration.
BASE = ["CostStamina", "ProfessionID", "RequireLevel", "ToolItemType",
        "ToolItemIndex", "EquipmentType", "Exp", "PrepareFrame"]

TARGET_READS = {
    "KCraftRecipe": BASE + ["ID", "Name", "Belong", "ResultOnLevel",
                            "RequireBranchID", "RequireDoodad"],
    "KCraftCollection": ["CostThew", "ProfessionID", "RequireLevel",
                         "ToolItemType", "ToolItemIndex", "EquipmentType",
                         "Exp", "PrepareFrame", "DoodadTemplateID"],
    "KCraftRead": ["ID", "SubID", "Name", "CostStamina", "ProfessionID",
                   "RequireLevel", "ToolItemType", "ToolItemIndex",
                   "EquipmentType", "PrepareFrame", "PlayerExp",
                   "ProfessionExp", "ExtendProfessionID1", "ExtendExp1",
                   "ExtendProfessionID2", "ExtendExp2", "CreateItemTab",
                   "CreateItemIndex", "CreateItemStackNum", "BuffID",
                   "BuffLevel", "Train", "ScriptName"],
    "KCraftEnchant": BASE + ["ID", "Name", "Belong", "EnchantID",
                             "RequireBranchID", "RequireDoodad"],
    "KCraftCopy": ["ID", "SubID", "Name", "CostThew", "ProfessionID",
                   "RequireLevel", "ProfessionIDExt", "RequireLevelExt",
                   "ToolItemType", "ToolItemIndex", "EquipmentType", "Exp",
                   "PrepareFrame"],
}

for _i in range(1, 9):
    TARGET_READS["KCraftRecipe"] += ["RequireItemType%d" % _i,
                                     "RequireItemIndex%d" % _i,
                                     "RequireItemCount%d" % _i]
    TARGET_READS["KCraftEnchant"] += ["RequireItemType%d" % _i,
                                      "RequireItemIndex%d" % _i,
                                      "RequireItemCount%d" % _i]
for _i in range(1, 9):
    TARGET_READS["KCraftRecipe"] += ["CreateItemType%d" % _i,
                                     "CreateItemIndex%d" % _i,
                                     "CreateItemMin%d" % _i,
                                     "CreateItemMax%d" % _i,
                                     "CreateItemProbability%d" % _i]
TARGET_READS["KCraftRecipe"] += ["CoolDownID", "ScriptName"]
TARGET_READS["KCraftEnchant"] += ["ScriptName"]
for _i in range(1, 5):
    TARGET_READS["KCraftCopy"] += ["RequireItemType%d" % _i,
                                   "RequireItemIndex%d" % _i,
                                   "RequireItemCount%d" % _i]
TARGET_READS["KCraftCopy"] += ["CreateItemType", "CreateItemIndex",
                               "CreateItemStackNum", "CoolDownID",
                               "DoodadTemplateID", "RequirePlayerLevel"]

# Target callsite counts per LoadLine (loop bodies counted once), taken from the
# decompiles: 24 / 9 / 23 / 18 / 22 == 96 reads, each followed by one guard.
TARGET_CALLSITES = {
    "KCraftRecipe": 24,
    "KCraftCollection": 9,
    "KCraftRead": 23,
    "KCraftEnchant": 18,
    "KCraftCopy": 22,
}

# CraftType -> owning record, from the KCraft::Init dispatch at 0x082a9f30.
CRAFT_TYPE = {1: "KCraftCollection", 2: "KCraftRecipe", 3: "KCraftRead",
              4: "KCraftEnchant", 6: "KCraftCopy"}


def body(text, signature):
    start = text.index(signature)
    brace = text.index("{", start)
    depth = 0
    for index in range(brace, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[brace + 1:index]
    raise AssertionError("unbalanced body: %s" % signature)


def ordered(text, fragments):
    positions = [text.index(fragment) for fragment in fragments]
    if positions != sorted(positions):
        raise AssertionError("out of target order: %r" % fragments)


def load_line_body(text, record):
    return body(text, "BOOL KRecipe<%s>::LoadLine(" % record)


def init_body(text, record):
    return body(text, "BOOL KRecipe<%s>::Init(" % record)


class KRecipeTargetContract(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.core = CORE.read_text(encoding="latin-1")
        cls.header = HEADER.read_text(encoding="latin-1")
        cls.glob = GLOBAL.read_text(encoding="latin-1")

    def test_name_width_matches_target_char64(self):
        """DWARF char[64] + setName strncpy 0x40 + GetString bound 0x40."""
        self.assertIn("#define MAX_RECIPE_NAME_LEN 64", self.glob)
        self.assertIn("#define MAX_BELONG_NAME_LEN 32", self.glob)
        # Every szName declaration must go through the macro, so the widened
        # value reaches all four target records.
        self.assertEqual(
            self.header.count("char szName[MAX_RECIPE_NAME_LEN]"), 4)
        self.assertEqual(
            self.header.count("char szBelong[MAX_BELONG_NAME_LEN]"), 2)
        # no szName may hardcode a literal width, or the macro would not reach it
        self.assertNotIn("char szName[64]", self.header)
        self.assertNotIn("char szName[32]", self.header)

    def test_record_field_order_matches_target_offsets(self):
        """DWARF member order for the six target records."""
        base = body(self.header, "struct KRecipeBase")
        ordered(base, ["eCraftType", "dwProfessionID",
                       "dwRequireProfessionLevel", "dwToolItemType",
                       "dwToolItemIndex", "nEquipmentType", "dwProfessionExp",
                       "nPrepareFrame", "dwScriptID"])
        recipe = body(self.header, "struct KCraftRecipe : public KRecipeBase")
        ordered(recipe, ["dwID", "szName", "szBelong", "nStamina",
                         "dwRequireBranchID", "dwRequireDoodadID",
                         "dwRequireItemType", "dwRequireItemIndex",
                         "dwRequireItemCount", "dwCreateItemType",
                         "dwCreateItemIndex", "dwCreateItemMinCount",
                         "dwCreateItemMaxCount", "dwCreateItemProbability",
                         "dwCoolDownID", "bResultOnLevel"])
        read = body(self.header, "struct KCraftRead : public KRecipeBase")
        ordered(read, ["dwID", "dwSubID", "nStamina", "nPlayerExp",
                       "dwExtendProfessionID1", "dwExtendExp1",
                       "dwExtendProfessionID2", "dwExtendExp2",
                       "dwCreateItemTab", "dwCreateItemIndex", "nStackNum",
                       "dwBuffID", "dwBuffLevel", "nTrain", "szName"])
        copy = body(self.header, "struct KCraftCopy : public KRecipeBase")
        ordered(copy, ["dwID", "dwSubID", "szName", "nThew", "dwCoolDownID",
                       "dwDoodadTemplateID", "dwRequireItemType",
                       "dwRequireItemIndex", "dwRequireItemCount",
                       "nRequirePlayerLevel", "dwCreateItemType",
                       "dwCreateItemIndex", "nStackNum", "dwProfessionIDExt",
                       "dwRequireProfessionLevelExt"])
        # mutation: swapping two members must be detected
        broken = read.replace("dwSubID", "@@", 1).replace("dwID", "dwSubID", 1)
        with self.assertRaises((AssertionError, ValueError)):
            ordered(broken, ["dwID", "dwSubID"])

    def test_every_tab_read_is_guarded_like_target(self):
        """Target checks all 96 reads with KGLOG_PROCESS_ERROR(bRetCode)."""
        total_reads = 0
        for record, expected in TARGET_CALLSITES.items():
            text = load_line_body(self.core, record)
            reads = re.findall(r"bRetCode\s*=\s*piTabFile->Get\w+\(", text)
            guards = re.findall(r"KGLOG_PROCESS_ERROR\(bRetCode\);", text)
            self.assertEqual(len(reads), expected,
                             "%s read count" % record)
            self.assertEqual(len(guards), expected,
                             "%s guard count" % record)
            total_reads += len(reads)
            # the tolerant candidate-only sink must be gone everywhere
            self.assertNotIn("(void)bRetCode", text)
        self.assertEqual(total_reads, 96)

    def test_guard_follows_each_read_in_target_order(self):
        """A guard must directly follow its own read, not merely co-occur."""
        for record in TARGET_CALLSITES:
            text = load_line_body(self.core, record)
            events = [(m.start(), "R") for m in
                      re.finditer(r"bRetCode\s*=\s*piTabFile->Get\w+\(", text)]
            events += [(m.start(), "G") for m in
                       re.finditer(r"KGLOG_PROCESS_ERROR\(bRetCode\);", text)]
            seq = "".join(kind for _, kind in sorted(events))
            self.assertEqual(seq, "RG" * TARGET_CALLSITES[record],
                             "%s read/guard interleaving" % record)

    def test_load_line_column_names_and_order(self):
        """Ordered column literals per target LoadLine."""
        for record, columns in TARGET_READS.items():
            text = load_line_body(self.core, record)
            # non-loop literals appear directly; loop columns come from the
            # RECIPE_*/ENCHANT_* prefixes formatted with nIndex + 1
            literal = [c for c in columns if not re.search(r"\d$", c)]
            resolved = text
            defines = re.findall(r'#define\s+(\w+)\s+"([^"]*)"', self.core)
            # longest name first, on word boundaries: RECIPE_PROFESSION_ID is a
            # prefix of RECIPE_PROFESSION_ID_EXT and must not clobber it
            for name, value in sorted(defines, key=lambda d: -len(d[0])):
                resolved = re.sub(r"\b%s\b" % re.escape(name),
                                  '"%s"' % value, resolved)
            present = re.findall(r'"([A-Za-z][A-Za-z0-9_]*)"', resolved)
            for column in literal:
                self.assertIn(column, present,
                              "%s missing column %s" % (record, column))
            ordered_present = [c for c in present if c in literal]
            expected_order = [c for c in literal if c in ordered_present]
            self.assertEqual(
                [c for i, c in enumerate(ordered_present)
                 if i == 0 or c != ordered_present[i - 1]][:len(expected_order)],
                expected_order, "%s column order" % record)

    def test_init_guards_and_target_constants(self):
        """Init guard order, MAX_RECIPE_ID 0x400 and the craft-type writes."""
        self.assertIn("#define MAX_RECIPE_ID 1024", self.glob)
        recipe = init_body(self.core, "KCraftRecipe")
        ordered(recipe, ['"%s/Craft/%s"', "g_OpenTabFile(szFilePath)",
                         "KGLOG_PROCESS_ERROR(piTabFile)",
                         "nHeight >= 2",
                         "LoadLine(piTabFile, 2,",
                         "for (nIndex = 3; nIndex <= nHeight; nIndex++)",
                         "CraftRecipe.dwID < MAX_RECIPE_ID",
                         "CraftRecipe.eCraftType = actProduce;",
                         "KG_COM_RELEASE(piTabFile)"])
        for record, craft_type in (("KCraftCollection", "actCollection"),
                                   ("KCraftRead", "actRead"),
                                   ("KCraftEnchant", "actEnchant"),
                                   ("KCraftCopy", "actCopy")):
            self.assertIn("eCraftType = %s;" % craft_type,
                          init_body(self.core, record))
        # KCraftRead/KCraftCopy route the key through BookID2RecipeID
        for record in ("KCraftRead", "KCraftCopy"):
            self.assertIn("BookID2RecipeID(", init_body(self.core, record))
        # KCraftCopy zeroes the row every iteration (target memset 0xc4)
        self.assertIn("ZeroMemory(&CraftCopy, sizeof(KCraftCopy));",
                      init_body(self.core, "KCraftCopy"))

    def test_init_guard_order_for_every_specialisation(self):
        """Each Init must carry its own target guard order, not just Recipe's."""
        # (record, guards that must appear in this exact target order)
        expected = {
            "KCraftRecipe": ["KGLOG_PROCESS_ERROR(piTabFile)", "nHeight >= 2",
                             "LoadLine(piTabFile, 2,",
                             "for (nIndex = 3; nIndex <= nHeight; nIndex++)"],
            "KCraftCollection": ["KGLOG_PROCESS_ERROR(piTabFile)",
                                 "nHeight >= 2", "LoadLine(piTabFile, 2,",
                                 "for (nIndex = 3; nIndex <= nHeight; nIndex++)"],
            "KCraftRead": ["KGLOG_PROCESS_ERROR(szFileName)",
                           "KGLOG_PROCESS_ERROR(piTabFile)", "nHeight >= 2",
                           "LoadLine(piTabFile, 2,",
                           "for (nIndex = 3; nIndex <= nHeight; nIndex++)"],
            "KCraftEnchant": ["KGLOG_PROCESS_ERROR(szFileName)",
                              "KGLOG_PROCESS_ERROR(piTabFile)", "nHeight >= 2",
                              "LoadLine(piTabFile, 2,",
                              "for (nIndex = 3; nIndex <= nHeight; nIndex++)"],
            "KCraftCopy": ["KGLOG_PROCESS_ERROR(szFileName)",
                           "KGLOG_PROCESS_ERROR(piTabFile)", "nHeight >= 2",
                           "LoadLine(piTabFile, 2,",
                           "for (nIndex = 3; nIndex <= nHeight; nIndex++)"],
        }
        for record, fragments in expected.items():
            ordered(init_body(self.core, record), fragments)

    def test_negative_guards_absent_where_target_has_none(self):
        """Target 0x080b3f74 / 0x080b5b04 have no MAX_RECIPE_ID guard."""
        for record in ("KCraftCollection", "KCraftRead"):
            self.assertNotIn("MAX_RECIPE_ID", init_body(self.core, record),
                             "%s Init must not gate on MAX_RECIPE_ID" % record)
        # only Read/Enchant/Copy validate szFileName (0x2e1 / 0x37a / 0x417)
        for record in ("KCraftRecipe", "KCraftCollection"):
            self.assertNotIn("KGLOG_PROCESS_ERROR(szFileName)",
                             init_body(self.core, record),
                             "%s Init has no target szFileName guard" % record)
        # Collection and Copy read no ScriptName column
        for record in ("KCraftCollection", "KCraftCopy"):
            text = load_line_body(self.core, record)
            self.assertNotIn("RECIPE_SCRIPT_NAME", text)
            self.assertIn("dwScriptID = 0;", text)
        # KCraftCollection has no GetName override (target has no such symbol)
        self.assertNotIn("KCraftCollection::GetName", self.core)

    def test_cost_gates_and_local_sets_match_target(self):
        """Target CanCast gates the cost check; Enchant uses nRetCode."""
        # 0x080b7a2e / 0x080b77a4 / 0x080b74d0 gate on nStamina > 0
        self.assertEqual(self.core.count("if (nStamina > 0)"), 3)
        # 0x080b7186 gates on nThew > 0; 0x080b78f2 adds the loot-list test
        self.assertEqual(self.core.count("if (nThew > 0)"), 1)
        self.assertIn("if (pDoodad->m_pLootList == NULL && nThew > 0)",
                      self.core)
        # every gated comparison must sit inside its gate, not before it
        for gate, cmp_ in (("if (nStamina > 0)", "m_nCurrentStamina >= nStamina"),
                           ("if (nThew > 0)", "m_nCurrentThew >= nThew")):
            for start in [i for i in range(len(self.core))
                          if self.core.startswith(gate, i)]:
                window = self.core[start:start + 220]
                self.assertIn(cmp_, window)
        # target 0x080b74d0 local set has nRetCode, no bRetCode, no nCurrentStamina
        enchant = body(self.core,
                       "CRAFT_RESULT_CODE KCraftEnchant::CanCast(")
        self.assertIn("nRetCode = Target.GetTarget(&pTargetItem);", enchant)
        self.assertIn("KGLOG_PROCESS_ERROR(nRetCode);", enchant)
        self.assertNotIn("bRetCode", enchant)
        self.assertNotIn("int                 nCurrentStamina", self.core)
        # 0x080b7186 orders the thew gate BEFORE the player-level check; the two
        # return different codes, so the order is part of the contract
        copy = body(self.core, "CRAFT_RESULT_CODE KCraftCopy::CanCast(")
        ordered(copy, ["if (nThew > 0)",
                       "m_nCurrentThew >= nThew",
                       "m_nLevel >= nRequirePlayerLevel"])

    def test_blocked_weapon_branch_keeps_its_polarity_note(self):
        """The C1 mitigation IS the polarity note; it must not vanish."""
        check = body(self.core,
                     "CRAFT_RESULT_CODE KRecipeBase::CheckCast(")
        self.assertIn("POLARITY", check)
        self.assertIn("reaches crcSuccess", check)
        self.assertIn("crcWeaponError", check)
        self.assertIn("including polarity", check)

    def test_book_id_bit_math_matches_target(self):
        """BookID2RecipeID 0x080b2a82 / RecipeID2BookID 0x080b29c9."""
        self.assertIn("#define MAX_READ_BOOK_ID_BIT 9", self.glob)
        self.assertIn("#define MAX_READ_BOOK_SUB_ID_BIT 3", self.glob)
        to_recipe = body(self.core, "int BookID2RecipeID(")
        ordered(to_recipe, ["nResult = -1;",
                            "dwBookID >= 1 && dwBookID <= MAX_READ_BOOK_ID",
                            "dwSubID >= 1 && dwSubID <= MAX_READ_BOOK_SUB_ID",
                            "((dwBookID - 1) << MAX_READ_BOOK_SUB_ID_BIT) | (dwSubID - 1)"])
        to_book = body(self.core, "BOOL RecipeID2BookID(")
        # target computes both outputs before validating them
        ordered(to_book, ["dwBookID = (dwRecipeID >> MAX_READ_BOOK_SUB_ID_BIT) + 1;",
                          "dwSubID = (dwRecipeID % (1 << MAX_READ_BOOK_SUB_ID_BIT)) + 1;",
                          "dwBookID >= 1 && dwBookID <= MAX_READ_BOOK_ID",
                          "dwSubID >= 1 && dwSubID <= MAX_READ_BOOK_SUB_ID",
                          "bResult = true;"])

    def test_no_candidate_only_nretcode_in_load_line(self):
        """Target LoadLine local sets contain no nRetCode."""
        for record in TARGET_CALLSITES:
            self.assertNotIn("nRetCode", load_line_body(self.core, record),
                             "%s has candidate-only nRetCode" % record)
        # CheckCast genuinely uses it (target 0x080b6f74 local set)
        self.assertIn("nRetCode", body(
            self.core, "CRAFT_RESULT_CODE KRecipeBase::CheckCast("))

    def test_blocked_edges_keep_owned_markers(self):
        """Unresolved target edges stay marked, never stubbed."""
        markers = re.findall(r"PORT-(?:UNKNOWN_REQUIRED|TODO)\[[A-Z_]+\]",
                             self.core)
        self.assertGreaterEqual(len(markers), 10)
        for marker in set(markers):
            self.assertIn(marker, self.core)
        # every marker block names an owner and a next action
        for block in re.findall(r"//\s*PORT-(?:UNKNOWN_REQUIRED|TODO)\[[A-Z_]+\][^\n]*",
                                self.core):
            self.assertIn("owner=", block)
        self.assertIn("next_action=", self.core)
        # a blocked edge must not have been replaced by a fake failure
        self.assertNotIn("return crcFailed; // PORT", self.core)

    def test_shipped_craft_tabs_expose_every_target_column(self):
        """craft.tab RecipeTabPath rows must carry all target-read columns."""
        index = CRAFT_DIR / "craft.tab"
        if not index.exists():
            self.skipTest("deploy tree not mounted: %s" % index)
        rows = index.read_bytes().decode("gbk", "replace").splitlines()
        checked = 0
        for row in rows[1:]:
            cells = row.split("\t")
            if len(cells) < 6 or not cells[5].strip():
                continue
            record = CRAFT_TYPE.get(int(cells[2]))
            if record is None:
                continue
            path = CRAFT_DIR / cells[5].strip().replace("\\", "/")
            self.assertTrue(path.exists(), "missing recipe tab %s" % path)
            header = path.read_bytes().split(b"\n")[0]
            have = {c.strip() for c in
                    header.decode("gbk", "replace").rstrip("\r").split("\t")}
            missing = [c for c in TARGET_READS[record] if c not in have]
            self.assertEqual(missing, [],
                             "%s (%s) missing %s" % (path.name, record, missing))
            checked += 1
        self.assertGreaterEqual(checked, 10)


if __name__ == "__main__":
    unittest.main()
