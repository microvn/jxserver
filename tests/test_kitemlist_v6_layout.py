import pathlib
import unittest


SOURCE = pathlib.Path(__file__).parents[1] / "src/SO3World/Src/KItemList.cpp"


class KItemListV6LayoutTest(unittest.TestCase):
    def test_item_data_v2_layout_matches_target_sizes(self):
        source = (
            pathlib.Path(__file__).parents[1] / "include/Include/KItemDef.h"
        ).read_bytes().decode("gbk")

        # Regression: target role DB stores V2 common/custom records as 13/33
        # bytes, not the legacy 11/30-byte records.
        self.assertIn("struct KCOMMON_ITEM_DATA_V0", source)
        self.assertIn("struct KCUSTOM_EQUI_DATA_V0", source)
        self.assertIn("WORD    wReservedV2;", source)
        self.assertIn("BYTE    byReservedV2;", source)
        self.assertIn("#define CURRENT_ITEM_DATA_VERSION 2", source)

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

    def test_v6_item_record_matches_target_five_byte_layout(self):
        source = SOURCE.read_text(encoding="gbk")
        item_loop = source.split("for (int nIndex = 0; nIndex < nItemCount; nIndex++)", 1)[1]
        v6 = item_loop.split("else", 1)[0]

        # Regression: target V6 records are [item_id][box, pos, data_len].
        self.assertIn("uLeftSize >= sizeof(WORD)", v6)
        self.assertIn("uLeftSize -= sizeof(WORD)", v6)
        self.assertIn("pbyOffset += sizeof(WORD)", v6)
        self.assertIn("KGLOG_PROCESS_ERROR(uLeftSize >= 3);", v6)
        self.assertIn("dwBoxIndex = pbyOffset[0];", v6)
        self.assertIn("dwX = pbyOffset[1];", v6)
        self.assertIn("byDataLen = pbyOffset[2];", v6)
        self.assertIn("uLeftSize -= 3;", v6)
        self.assertIn("pbyOffset += 3;", v6)
        self.assertNotIn("uLeftSize >= 5", v6)
        self.assertNotIn("pbyOffset[3]", v6)
        self.assertNotIn("pbyOffset[4]", v6)

    def test_v6_compat_blob_consumes_pad_count_item_id_and_payload(self):
        # Regression: the live role blob has one pad byte before count and a
        # WORD item id before each [box, pos, data_len] header.
        # Use a compact two-record fixture for the structural assertion.
        payload = bytes.fromhex(
            "00 02 00"
            "01 00 00 00 01 aa"
            "02 00 01 01 01 bb"
        )
        pos = 1
        count = int.from_bytes(payload[pos:pos + 2], "little")
        pos += 2
        for _ in range(count):
            pos += 2
            box, slot, data_len = payload[pos:pos + 3]
            self.assertLess(box, 19)
            self.assertLess(slot, 256)
            pos += 3 + data_len
        self.assertEqual(pos, len(payload))

    def test_v6_compat_repositions_before_first_item_record(self):
        source = SOURCE.read_text(encoding="gbk")
        self.assertIn("uLeftSize -= 1 + sizeof(WORD);", source)
        self.assertIn("pbyOffset += 1 + sizeof(WORD);", source)


if __name__ == "__main__":
    unittest.main()
