import pathlib
import unittest


ROOT = pathlib.Path(__file__).parents[1]


class PendentV2ContractTest(unittest.TestCase):
    def test_target_record_and_state_shape(self):
        player_h = (ROOT / "src/SO3World/Src/KPlayer.h").read_bytes().decode("gbk", errors="ignore")
        self.assertIn("struct KPendent", player_h)
        self.assertIn("DWORD dwItemIndex;", player_h)
        self.assertIn("time_t nGenTime;", player_h)
        for field in (
            "m_nWaistPendentBoxSize",
            "m_nBackPendentBoxSize",
            "m_nFacePendentBoxSize",
            "m_WaistPendent",
            "m_BackPendent",
            "m_FacePendent",
            "m_dwWaistItemIndex",
            "m_dwBackItemIndex",
            "m_dwFaceItemIndex",
        ):
            self.assertIn(field, player_h)

    def test_v2_consumes_three_box_sections_and_no_leftover(self):
        player = (ROOT / "src/SO3World/Src/KPlayer.cpp").read_bytes().decode("gbk", errors="ignore")
        loader = player.split("BOOL KPlayer::LoadPendentDataV2", 1)[1].split("BOOL KPlayer::SavePendentData", 1)[0]
        self.assertNotIn("KGLOG_PROCESS_ERROR(nVersion == 2);", loader)
        self.assertEqual(loader.count("m_nWaistPendentBoxSize = *(WORD*)"), 1)
        self.assertEqual(loader.count("m_nBackPendentBoxSize = *(WORD*)"), 1)
        self.assertEqual(loader.count("m_nFacePendentBoxSize = *(WORD*)"), 1)
        self.assertEqual(loader.count("sizeof(KPendent)"), 12)
        self.assertIn("KGLOG_PROCESS_ERROR(uLeftSize == 0);", loader)

    def test_target_has_v0_v2_dispatch_and_time_limit_processing(self):
        player = (ROOT / "src/SO3World/Src/KPlayer.cpp").read_bytes().decode("gbk", errors="ignore")
        self.assertIn("BOOL KPlayer::LoadPendentDataV0", player)
        self.assertIn("BOOL KPlayer::LoadPendentDataV2", player)
        dispatcher = player.split("BOOL KPlayer::LoadPendentData(BYTE*", 1)[1].split("BOOL KPlayer::LoadPendentDataV0", 1)[0]
        self.assertIn("nVersion == 0", dispatcher)
        self.assertIn("LoadPendentDataV0", dispatcher)
        self.assertIn("nVersion == 1", dispatcher)
        self.assertIn("nVersion == 2", dispatcher)
        self.assertIn("LoadPendentDataV2", dispatcher)
        self.assertIn("ProcessTimeLimitPendent", player)

    def test_v0_uses_canonical_conversion_table(self):
        player = (ROOT / "src/SO3World/Src/KPlayer.cpp").read_bytes().decode("gbk", errors="ignore")
        self.assertIn("OldPendentRepresentID2ItemID.tab", player)
        self.assertIn("GetOldWaistPendentItemID", player)
        self.assertIn("GetOldBackPendentItemID", player)
        self.assertIn("GetOldFacePendentItemID", player)
        self.assertIn("m_dwWaistItemIndex", player)
        self.assertIn("m_dwBackItemIndex", player)
        self.assertIn("m_dwFaceItemIndex", player)

    def test_role_dispatch_and_save_version_are_explicit(self):
        player = (ROOT / "src/SO3World/Src/KPlayer.cpp").read_bytes().decode("gbk", errors="ignore")
        self.assertIn("case rbtPendentData:", player)
        self.assertIn("LoadPendentData(pbyOffset, pBlock->dwLen, (int)pBlock->dwVer)", player)
        self.assertIn("SAVE_ROLE_BLOCK(SavePendentData, rbtPendentData, 2);", player)
