"""Target-backed static contract for KInventory (no build/runtime required)."""

from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]
HEADER = (ROOT / "src/SO3World/Src/KInventory.h").read_text(errors="replace")
SOURCE = (ROOT / "src/SO3World/Src/KInventory.cpp").read_text(errors="replace")


class KInventoryTargetContractTest(unittest.TestCase):
    @staticmethod
    def _target_accepts(primary_gen, primary_sub, extra_gen, extra_sub, genre, sub):
        invalid = -1
        if primary_gen == invalid and extra_gen == invalid:
            return True

        primary_match = (
            primary_gen != invalid
            and primary_gen == genre
            and (primary_sub == invalid or primary_sub == sub)
        )
        extra_match = (
            extra_gen != invalid
            and extra_gen == genre
            and (extra_sub == invalid or extra_sub == sub)
        )
        return primary_match or extra_match

    def test_target_layout_requires_six_fields_and_source_slot_adapter(self):
        # Target DWARF class DIE 0x06194035: size 0x94; fields at
        # 0, 4, 0x84, 0x88, 0x8c, and 0x90.  KItem* is the source-side
        # pointer-compatible adapter for external IItem*, while the member
        # spelling remains stable for dirty read-only KItemList callers.
        target_fields = (
            "m_dwSize",
            "m_piItemArray",
            "m_nContainItemGenerType",
            "m_nContainItemSubType",
            "m_nContainItemGenerTypeExtra",
            "m_nContainItemSubTypeExtra",
        )
        self.assertEqual(len(target_fields), 6)
        self.assertIn("KItem*\tm_pItemArray", HEADER)
        self.assertIn("m_nContainItemGenerTypeExtra", HEADER)
        self.assertIn("m_nContainItemSubTypeExtra", HEADER)

    def test_target_filter_and_release_closures_are_reconstructed(self):
        # Target 0x083c2222 evaluates two generic/subtype filter alternatives;
        # target 0x083c21b8 releases via g_pSO3World + 0x608d8, IItemHouse
        # virtual slot +0x18.  KGItemHouse::DestroyItem at 0x219c0 performs
        # Unregister -> UnInit -> Delete; KInventory reconstructs that order
        # against the source-side world item set.
        unregister = "g_pSO3World->m_ItemSet.Unregister(m_pItemArray[i]);"
        uninit = "m_pItemArray[i]->UnInit();"
        delete = "KMemory::Delete(m_pItemArray[i]);"
        self.assertIn(unregister, SOURCE)
        self.assertIn(uninit, SOURCE)
        self.assertIn(delete, SOURCE)
        self.assertLess(SOURCE.index(unregister), SOURCE.index(uninit))
        self.assertLess(SOURCE.index(uninit), SOURCE.index(delete))
        self.assertNotIn("m_ItemManager.FreeItem(m_pItemArray[i])", SOURCE)
        self.assertIn("m_nContainItemGenerTypeExtra = INVALID_CONTAIN_ITEM_TYPE", SOURCE)
        self.assertIn("m_nContainItemSubTypeExtra   = INVALID_CONTAIN_ITEM_TYPE", SOURCE)
        self.assertIn("BOOL bPrimaryMatch", SOURCE)
        self.assertIn("BOOL bExtraMatch", SOURCE)
        self.assertIn("m_nContainItemGenerTypeExtra != INVALID_CONTAIN_ITEM_TYPE", SOURCE)
        self.assertIn("if (!bPrimaryMatch && !bExtraMatch)", SOURCE)
        self.assertNotIn("m_piItemHouse", SOURCE)
        self.assertNotIn("IItem", SOURCE)

    def test_target_dual_filter_truth_table(self):
        # Target 0x083c2222: unconfigured accepts, each configured generic
        # filter optionally constrains subtype, and the two filters are ORed.
        accepts = self._target_accepts
        self.assertTrue(accepts(-1, -1, -1, -1, 3, 9))
        self.assertTrue(accepts(3, -1, -1, -1, 3, 9))
        self.assertFalse(accepts(3, 9, -1, -1, 3, 8))
        self.assertTrue(accepts(3, 9, 4, 8, 4, 8))
        self.assertFalse(accepts(3, 9, 4, 8, 5, 8))

    def test_source_bound_dual_filter_branch_order(self):
        primary = "if (m_nContainItemGenerType != INVALID_CONTAIN_ITEM_TYPE)"
        extra = "if (m_nContainItemGenerTypeExtra != INVALID_CONTAIN_ITEM_TYPE)"
        reject = "if (!bPrimaryMatch && !bExtraMatch)"
        self.assertIn(primary, SOURCE)
        self.assertIn(extra, SOURCE)
        self.assertIn(reject, SOURCE)
        self.assertLess(SOURCE.index(primary), SOURCE.index(extra))
        self.assertLess(SOURCE.index(extra), SOURCE.index(reject))


if __name__ == "__main__":
    unittest.main()
