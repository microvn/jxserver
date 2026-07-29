import pathlib
import unittest


SOURCE = pathlib.Path(__file__).parents[1] / "src/SO3World/Src/KItemList.cpp"


class KItemListV6LayoutTest(unittest.TestCase):
    def test_v6_prefix_reads_target_offsets(self):
        source = SOURCE.read_text(encoding="gbk")
        v6 = source.split("if (bVersion6)", 1)[1].split("else", 1)[0]
        load = source.split(
            "BOOL KItemList::Load(BYTE* pbyData, size_t uDataLen, int nVersion)",
            1,
        )[1].split("BOOL KItemList::Save", 1)[0]

        # Regression: target V6 keeps a DWORD between money and bank count.
        self.assertEqual(load.count("uLeftSize -= sizeof(int);"), 1)
        self.assertEqual(load.count("pbyOffset += sizeof(int);"), 1)
        self.assertEqual(load.count("uLeftSize -= sizeof(DWORD);"), 1)
        self.assertEqual(load.count("pbyOffset += sizeof(DWORD);"), 1)
        self.assertIn("m_nEnabledBankPackageCount = *(WORD*)pbyOffset;", v6)
        self.assertIn("uLeftSize -= sizeof(WORD);", v6)
        self.assertIn("pbyOffset += sizeof(WORD);", v6)
        self.assertIn("nItemCount = *(WORD*)pbyOffset;", v6)
        self.assertNotIn("pbyOffset + 4", v6)
        self.assertNotIn("pbyOffset + 3", v6)

    def test_v6_role_block_reads_item_count_after_full_prefix(self):
        source = SOURCE.read_text(encoding="gbk")
        v6 = source.split("if (bVersion6)", 1)[1].split("else", 1)[0]

        # Regression: target V6 stores three bytes between bank count and the
        # item count; the old ten-byte skip left the parser misaligned.
        self.assertIn("uLeftSize >= 3", v6)
        self.assertIn("uLeftSize -= 3", v6)
        self.assertIn("pbyOffset += 3", v6)

    def test_v6_item_header_matches_target_three_byte_layout(self):
        source = SOURCE.read_text(encoding="gbk")
        item_loop = source.split("for (int nIndex = 0; nIndex < nItemCount; nIndex++)", 1)[1]
        v6 = item_loop.split("else", 1)[0]

        # Regression: target V6 item headers are [box, pos, data_len].
        self.assertIn("KGLOG_PROCESS_ERROR(uLeftSize >= 3);", v6)
        self.assertIn("dwBoxIndex = pbyOffset[0];", v6)
        self.assertIn("dwX = pbyOffset[1];", v6)
        self.assertIn("byDataLen = pbyOffset[2];", v6)
        self.assertIn("uLeftSize -= 3;", v6)
        self.assertIn("pbyOffset += 3;", v6)
        self.assertNotIn("uLeftSize >= 5", v6)
        self.assertNotIn("pbyOffset[3]", v6)
        self.assertNotIn("pbyOffset[4]", v6)


if __name__ == "__main__":
    unittest.main()
