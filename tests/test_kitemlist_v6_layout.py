import pathlib
import unittest


SOURCE = pathlib.Path(__file__).parents[1] / "src/SO3World/Src/KItemList.cpp"


class KItemListV6LayoutTest(unittest.TestCase):
    def test_v6_prefix_reads_target_offsets(self):
        source = SOURCE.read_text(encoding="gbk")
        v6 = source.split("if (bVersion6)", 1)[1].split("else", 1)[0]

        self.assertIn("m_nEnabledBankPackageCount = *(WORD*)pbyOffset;", v6)
        self.assertIn("uLeftSize -= sizeof(WORD);", v6)
        self.assertIn("pbyOffset += sizeof(WORD);", v6)
        self.assertIn("nItemCount = *(WORD*)pbyOffset;", v6)
        self.assertNotIn("pbyOffset + 4", v6)
        self.assertNotIn("pbyOffset + 3", v6)

    def test_v6_role_block_reads_item_count_after_full_prefix(self):
        source = SOURCE.read_text(encoding="gbk")
        v6 = source.split("if (bVersion6)", 1)[1].split("else", 1)[0]

        # Regression: the live v6 role block stores ten bytes between the
        # bank count and the item count; reading at +9 produced zero items.
        self.assertIn("uLeftSize >= 10", v6)
        self.assertIn("uLeftSize -= 10", v6)
        self.assertIn("pbyOffset += 10", v6)


if __name__ == "__main__":
    unittest.main()
