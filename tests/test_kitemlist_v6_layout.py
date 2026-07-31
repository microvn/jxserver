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
        v6 = source.split("if (bVersion6 || bVersion5)", 1)[1].split("else", 1)[0]
        load = source.split(
            "BOOL KItemList::Load(BYTE* pbyData, size_t uDataLen, int nVersion)",
            1,
        )[1].split("BOOL KItemList::Save", 1)[0]

        # Regression: target V6 consumes money, score, bank count, equip IDs,
        # three FEA-active bytes, then item count.
        self.assertEqual(load.count("uLeftSize -= sizeof(int);"), 1)
        self.assertEqual(load.count("pbyOffset += sizeof(int);"), 1)
        self.assertEqual(load.count("uLeftSize -= sizeof(DWORD);"), 1)
        self.assertEqual(load.count("pbyOffset += sizeof(DWORD);"), 1)
        self.assertIn("m_nEnabledBankPackageCount = *(WORD*)pbyOffset;", v6)
        self.assertIn("uLeftSize -= sizeof(WORD);", v6)
        self.assertIn("pbyOffset += sizeof(WORD);", v6)
        self.assertIn("m_nEquipIDArray[nIndex] = pbyOffset[nIndex];", v6)
        self.assertIn("nItemCount = *(WORD*)pbyOffset;", v6)
        self.assertIn("m_bFEAActiveFlag[nIndex] = pbyOffset[nIndex + 3] != 0;", v6)
        self.assertNotIn("m_Box[nBoxIndex].m_dwSize = pbyOffset[nBoxIndex - ibEquip - 1];", v6)

    def test_v6_role_block_reads_item_count_after_full_prefix(self):
        source = SOURCE.read_text(encoding="gbk")
        v6 = source.split("if (bVersion6)", 1)[1].split("else", 1)[0]

        # Regression: target V6 stores three box-size bytes between equip IDs
        # and the item count; omitting them shifts every item record.
        self.assertIn("uLeftSize >= 3", v6)
        self.assertIn("uLeftSize -= 3", v6)
        self.assertIn("pbyOffset += 3", v6)

    def test_v6_item_record_matches_target_five_byte_layout(self):
        source = SOURCE.read_text(encoding="gbk")
        item_loop = source.split("for (int nIndex = 0; nIndex < nItemCount; nIndex++)", 1)[1]
        v6 = item_loop.split("if (bVersion6 || bVersion5)", 1)[1].split("else", 1)[0]

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

    def test_v6_has_no_speculative_reposition(self):
        source = SOURCE.read_text(encoding="gbk")
        self.assertNotIn("bV6FiveByteRecord", source)
        self.assertNotIn("uLeftSize -= 1 + sizeof(WORD);", source)

    def test_item_block_is_saved_as_target_v6(self):
        player = (pathlib.Path(__file__).parents[1] / "src/SO3World/Src/KPlayer.cpp").read_bytes().decode("gbk", errors="ignore")
        item = SOURCE.read_text(encoding="gbk")
        self.assertIn("SAVE_ROLE_BLOCK(m_ItemList.Save, rbtItemList, 6);", player)
        self.assertIn("*(DWORD*)pbyOffset = (DWORD)m_nTotalEquipScore;", item)
        self.assertIn("*(WORD*)pbyOffset = (WORD)dwNextItemID;", item)
        self.assertIn("pbyOffset[nIndex] = (BYTE)m_bFEAActiveFlag[nIndex];", item)

    def test_time_limit_return_loader_uses_target_record_order(self):
        source = SOURCE.read_text(encoding="gbk")
        loader = source.split("BOOL KItemList::LoadTimeLimitReturnInfo", 1)[1].split("BOOL KItemList::LoadTimeLimitSoldListInfo", 1)[0]
        item_pos = loader.index("memcpy(&dwItemID")
        shop_pos = loader.index("memcpy(&dwShopTemplateID")
        index_pos = loader.index("memcpy(&dwShopItemIndex")
        end_pos = loader.index("memcpy(&dwEndTime")
        self.assertLess(item_pos, shop_pos)
        self.assertLess(shop_pos, index_pos)
        self.assertLess(index_pos, end_pos)
        self.assertIn("if (dwEndTime > (DWORD)g_pSO3World->m_nCurrentTime)", loader)

    def test_time_limited_role_blocks_dispatch_to_item_state_loaders(self):
        player = (pathlib.Path(__file__).parents[1] / "src/SO3World/Src/KPlayer.cpp").read_bytes().decode("gbk", errors="ignore")
        for block, loader in (
            ("rbtDelayTradeItemData", "LoadDelayTradeInfo"),
            ("rbtTimeLimitReturnItemData", "LoadTimeLimitReturnInfo"),
            ("rbtTimeLimitSoldListInfoData", "LoadTimeLimitSoldListInfo"),
        ):
            self.assertIn("case %s:" % block, player)
            self.assertIn("m_ItemList.%s" % loader, player)

    def test_time_limited_role_blocks_are_saved_after_item_list(self):
        player = (pathlib.Path(__file__).parents[1] / "src/SO3World/Src/KPlayer.cpp").read_bytes().decode("gbk", errors="ignore")
        item = SOURCE.read_text(encoding="gbk")
        save = player.split("BOOL KPlayer::Save(", 1)[1].split("#undef SAVE_ROLE_BLOCK", 1)[0]
        for method, block in (
            ("SaveDelayTradeInfo", "rbtDelayTradeItemData"),
            ("SaveTimeLimitReturnInfo", "rbtTimeLimitReturnItemData"),
            ("SaveTimeLimitSoldListInfo", "rbtTimeLimitSoldListInfoData"),
        ):
            self.assertIn(method, item)
            self.assertIn("SAVE_ROLE_BLOCK(m_ItemList.%s, %s, 0);" % (method, block), save)
        self.assertLess(save.index("rbtItemList"), save.index("rbtDelayTradeItemData"))
        for method in ("SaveDelayTradeInfo", "SaveTimeLimitReturnInfo"):
            body = item.split("BOOL KItemList::%s" % method, 1)[1].split("BOOL KItemList::", 1)[0]
            self.assertIn("nEndTime <= g_pSO3World->m_nCurrentTime", body)

    def test_item_data_is_marked_complete_before_target_finish_gates(self):
        player = (pathlib.Path(__file__).parents[1] / "src/SO3World/Src/KPlayer.cpp").read_bytes().decode("gbk", errors="ignore")
        finish = player.split("BOOL KPlayer::FinishRoleDataLoad()", 1)[1].split("BOOL KPlayer::OnClientReady()", 1)[0]
        self.assertLess(finish.index("m_ItemList.m_bFinishLoadData = true;"), finish.index("UpdateFreeLimitFlag();"))
        self.assertLess(finish.index("UpdateFreeLimitFlag();"), finish.index("m_RegressionData.Calculate("))

    def test_ext_data_finish_resolves_scene_before_role_data_over(self):
        player = (pathlib.Path(__file__).parents[1] / "src/SO3World/Src/KPlayer.cpp").read_bytes().decode("gbk", errors="ignore")
        finish = player.split("BOOL KPlayer::OnExtDataLoadFinish()", 1)[1].split("BOOL KPlayer::PartialLoadExtData", 1)[0]
        self.assertIn("GetScene(m_SavePosition.dwMapID, m_SavePosition.nMapCopyIndex)", finish)
        self.assertLess(finish.index("GetScene("), finish.index("DoSyncRoleDataOver"))
        self.assertIn("(void)g_PlayerServer.DoSyncRoleDataOver", finish)
        self.assertNotIn("KGLOG_PROCESS_ERROR(bRetCode);", finish.split("DoSyncRoleDataOver", 1)[0] + "DoSyncRoleDataOver")

    def test_finish_syncs_currency_before_kill_point_and_not_free_limit_flag(self):
        player = (pathlib.Path(__file__).parents[1] / "src/SO3World/Src/KPlayer.cpp").read_bytes().decode("gbk", errors="ignore")
        finish = player.split("BOOL KPlayer::FinishRoleDataLoad()", 1)[1].split("BOOL KPlayer::OnClientReady", 1)[0]
        self.assertLess(finish.index("DoSyncCurrencyList"), finish.index("DoSyncKillPoint"))
        self.assertNotIn("DoSyncFreeLimitFlagInfo", finish)

    def test_package_size_uses_target_range_and_sync_protocol(self):
        root = pathlib.Path(__file__).parents[1]
        item = SOURCE.read_bytes().decode("gbk", errors="ignore")
        player = (root / "src/SO3World/Src/KPlayer.cpp").read_bytes().decode("gbk", errors="ignore")
        consts = (root / "include/Include/SO3World/KGWConstList.h").read_bytes().decode("gbk", errors="ignore")
        ini = (root / "src/SO3World/Src/KGWConstList.cpp").read_bytes().decode("gbk", errors="ignore")
        protocol = (root / "include/Include/GS_Client_Protocol.h").read_bytes().decode("gbk", errors="ignore")
        self.assertIn("nCubPackageRoomRange[2]", consts)
        self.assertIn('"CubPackageRoomRange"', ini)
        self.assertIn("m_Box[ibCubPackage].m_dwSize", item)
        self.assertIn("AdjustCubPackageSize", player)
        self.assertIn("nCubPackageRoomRange", consts)
        self.assertIn("s2c_sync_cub_package_size", protocol)

    def test_target_item_loader_dispatch_and_v5_v6_prefixes_are_explicit(self):
        header = (pathlib.Path(__file__).parents[1] / "src/SO3World/Src/KItemList.h").read_bytes().decode("gbk", errors="ignore")
        source = SOURCE.read_text(encoding="gbk")
        player = (pathlib.Path(__file__).parents[1] / "src/SO3World/Src/KPlayer.cpp").read_bytes().decode("gbk", errors="ignore")
        for version in range(7):
            self.assertIn("LoadItemList_V%d" % version, header)
            self.assertIn("KItemList::LoadItemList_V%d" % version, source)
        self.assertIn("KItemList::LoadItemList(BYTE* pbyData, size_t uDataLen, int nVersion)", source)
        self.assertIn("m_ItemList.LoadItemList(pbyOffset, pBlock->dwLen", player)
        self.assertIn("bVersion5", source)
        self.assertIn("V5/V6 prefix", source)

    def test_target_item_id_reconciliation_uses_64bit_map_and_three_helpers(self):
        header = (pathlib.Path(__file__).parents[1] / "src/SO3World/Src/KItemList.h").read_bytes().decode("gbk", errors="ignore")
        source = SOURCE.read_text(encoding="gbk")
        self.assertIn("std::map<DWORD, DWORD> m_ItemChangeIDMap", header)
        self.assertIn("std::map<unsigned long long, KDelayTradeInfo> m_DelayTradeMap", header)
        self.assertIn("std::map<DWORD, KTimeLimitReturnInfo> m_TimeLimitReturnMap", header)
        self.assertIn("std::map<DWORD, long> m_TimeLimitSoldListInfoMap", header)
        self.assertNotIn("std::map<unsigned long long, KTimeLimitReturnInfo> m_TimeLimitReturnMap", header)
        self.assertNotIn("std::map<unsigned long long, long> m_TimeLimitSoldListInfoMap", header)
        body = source.split("void KItemList::UpdateItemID()", 1)[1].split("BOOL KItemList::LoadDelayTradeInfo", 1)[0]
        for helper in ("UpdateDelayTradeItemID()", "UpdateTimeLimitReturnItemID()", "UpdateTimeLimitSoldListItemID()"):
            self.assertIn(helper, body)
        self.assertLess(body.index("UpdateDelayTradeItemID()"), body.index("m_ItemChangeIDMap.clear()"))

    def test_target_item_state_map_key_widths_match_dwarf(self):
        header = (pathlib.Path(__file__).parents[1] / "src/SO3World/Src/KItemList.h").read_bytes().decode("gbk", errors="ignore")
        # DWARF target: delay-trade key is unsigned long long; return and sold
        # list keys are unsigned long, which is 32-bit on the target build.
        self.assertIn("AddTimeLimitReturnItemInfo(DWORD dwItemID", header)
        self.assertIn("AddTimeLimitSoldListInfo(DWORD dwItemID", header)

    def test_target_item_id_sync_routes_are_declared(self):
        root = pathlib.Path(__file__).parents[1]
        proto = (root / "include/Include/GS_Client_Protocol.h").read_bytes().decode("gbk", errors="ignore")
        server = (root / "src/SO3World/Src/KPlayerServer.h").read_bytes().decode("gbk", errors="ignore")
        self.assertIn("s2c_sync_delay_trade_item = 293", proto)
        self.assertIn("s2c_sync_time_limit_return_item = 294", proto)
        self.assertIn("s2c_sync_time_limit_sold_list_info = 295", proto)
        for method in ("DoSyncDelayTradeItem", "DoSyncTimeLimitReturnItem", "DoSyncTimeLimitSoldListInfo"):
            self.assertIn(method, server)

    def test_v3_and_v4_prefixes_match_target_byte_counts(self):
        source = SOURCE.read_text(encoding="gbk")
        load = source.split("BOOL KItemList::Load(BYTE* pbyData, size_t uDataLen, int nVersion)", 1)[1]
        self.assertIn("BOOL        bVersion3  = (nVersion == 3);", load)
        self.assertIn("BOOL        bVersion4  = (nVersion == 4);", load)
        self.assertIn("else if (bVersion4 || bVersion3)", load)
        self.assertIn("m_nEquipIDArray[nIndex] = pbyOffset[nIndex];", load)
        self.assertIn("if (bVersion4)", load)

    def test_v6_reconciles_old_item_ids_and_sold_list_position(self):
        source = SOURCE.read_text(encoding="gbk")
        item_loop = source.split("for (int nIndex = 0; nIndex < nItemCount; nIndex++)", 1)[1]
        self.assertIn("wOldItemID = *(WORD*)pbyOffset;", item_loop)
        self.assertIn("m_ItemChangeIDMap[wOldItemID] = pItem->m_dwID;", item_loop)
        self.assertIn("if (dwBoxIndex == ibSoldList)", item_loop)
        self.assertIn("m_nNextSoldListPos = (m_nNextSoldListPos + 1) % MAX_SOLDLIST_PACKAGE_SIZE;", item_loop)

    def test_v6_save_uses_persisted_id_sequence_and_records_runtime_mapping(self):
        source = SOURCE.read_text(encoding="gbk")
        save = source.split("BOOL KItemList::Save(", 1)[1].split("void KItemList::TidyUpSoldList", 1)[0]
        self.assertIn("dwNextItemID", save)
        self.assertIn("(DWORD)m_ItemChangeIDMap.size() + 1", save)
        self.assertIn("*(WORD*)pbyOffset = (WORD)dwNextItemID;", save)
        self.assertIn("m_ItemChangeIDMap[pItem->m_dwID] = dwNextItemID;", save)
        self.assertIn("dwNextItemID++;", save)
        self.assertNotIn("*(WORD*)pbyOffset = (WORD)pItem->m_dwID;", save)
        self.assertNotIn("if (nBoxIndex == ibSoldList)", save)


if __name__ == "__main__":
    unittest.main()
