import pathlib
import unittest


ROOT = pathlib.Path(__file__).parents[1]


class SyncFrontierContractTest(unittest.TestCase):
    def test_sprint_v2_packet_and_target_order(self):
        protocol = (ROOT / "include/Include/GS_Client_Protocol.h").read_bytes().decode("gbk", errors="ignore")
        server_h = (ROOT / "src/SO3World/Src/KPlayerServer.h").read_bytes().decode("gbk", errors="ignore")
        server_cpp = (ROOT / "src/SO3World/Src/KPlayerServer.cpp").read_bytes().decode("gbk", errors="ignore")
        player = (ROOT / "src/SO3World/Src/KPlayer.cpp").read_bytes().decode("gbk", errors="ignore")
        consts = (ROOT / "include/Include/SO3World/KGWConstList.h").read_bytes().decode("gbk", errors="ignore")
        ini = (ROOT / "src/SO3World/Src/KGWConstList.cpp").read_bytes().decode("gbk", errors="ignore")

        self.assertIn("s2c_sync_sprint_v2 = 299", protocol)
        self.assertIn("struct S2C_SYNC_SPRINT_V2", protocol)
        self.assertIn("DWORD dwSprint;", protocol)
        self.assertIn("BOOL DoSyncSprintV2(KPlayer* pPlayer);", server_h)
        self.assertIn("Notify.dwSprint = (DWORD)g_pSO3World->m_Settings.m_ConstList.nSprintFlagV2;", server_cpp)
        self.assertIn("int nSprintFlagV2;", consts)
        self.assertIn('"SprintFlagV2"', ini)

        finish = player.split("BOOL KPlayer::FinishRoleDataLoad()", 1)[1].split("BOOL KPlayer::OnClientReady", 1)[0]
        self.assertLess(finish.index("DoSyncFellowshipPlayerMiniAvatar"), finish.index("DoSyncSprintV2"))
        self.assertLess(finish.index("DoSyncSprintV2"), finish.index("DoSyncPlayerStateInfo"))

    def test_sprint_v2_wire_fields_are_protocol_plus_dword(self):
        protocol = (ROOT / "include/Include/GS_Client_Protocol.h").read_bytes().decode("gbk", errors="ignore")
        packet = protocol.split("struct S2C_SYNC_SPRINT_V2", 1)[1].split("};", 1)[0]
        self.assertIn("DOWNWARDS_PROTOCOL_HEADER", packet)
        self.assertEqual(packet.count("DWORD"), 1)

    def test_handshake_request_is_target_packed_size(self):
        protocol = (ROOT / "include/Include/GS_Client_Protocol.h").read_bytes().decode(
            "gbk", errors="ignore"
        )
        start = protocol.index("struct C2S_HANDSHAKE_REQUEST")
        end = protocol.index("struct C2S_CLIENT_CONFIRM_READY", start)
        block = protocol[start:end]
        self.assertIn("#pragma pack(1)", protocol[max(0, start - 40):start])
        self.assertIn("#pragma pack()", protocol[end:])
        self.assertIn("DWORD   dwRoleID", block)
        self.assertIn("GUID\tGuid", block)

    def test_all_target_frontier_routes_precede_role_data_over(self):
        player = (ROOT / "src/SO3World/Src/KPlayer.cpp").read_bytes().decode("gbk", errors="ignore")
        finish = player.split("BOOL KPlayer::FinishRoleDataLoad()", 1)[1].split(
            "void KPlayer::SyncSingleDungeonCurrentScore", 1
        )[0]

        # Target OnExtDataLoadFinish in SO3GameServerD inlines this closure.
        target_routes = (
            "UpdateFreeLimitFlag",
            "m_RegressionData.Calculate",
            "m_ItemList.UpdateItemID",
            "AdjustCubPackageSize",
            "DoSyncCubPackageSize",
            "DoApplySingleDungeonLastScore",
            "SyncSingleDungeonCurrentScore",
            "RefreshDailyVariable",
            "DoApplyCoinOperatingFlag",
            "DoSyncCorpsChangeDataRequest",
            "DoSyncCorpsChangeValue",
            "DoUpdateMaxApprenticeNum",
            "PickupTAEquipsScore",
            "AddTAEquipsScore",
            "DoSyncFellowshipPlayerMiniAvatar",
            "DoSyncSprintV2",
            "DoSyncPlayerStateInfo",
            "DoSyncCurrencyList",
            "DoSyncKillPoint",
            "LoadFellowshipData",
            "DoLoginTeamSync",
            "DoSyncBuffList",
            "DoSyncVisitMapInfo",
            "DoSyncRouteNodeOpenList",
            "DoSyncCampInfo",
            "DoSyncAchievementData",
            "DoSyncDesignationData",
            "DoSyncCoin",
            "DoSyncRewards",
            "DoSyncPendentData",
            "DoSyncFellowPetData",
        )
        positions = [finish.index(route) for route in target_routes]
        self.assertEqual(positions, sorted(positions))

        on_finish = player.split("BOOL KPlayer::OnExtDataLoadFinish()", 1)[1].split(
            "BOOL KPlayer::PartialLoadExtData", 1
        )[0]
        self.assertLess(on_finish.index("SyncNewClient(this)"), on_finish.index("m_nNextSaveFrame ="))
        self.assertLess(on_finish.index("m_nNextSaveFrame ="), on_finish.index("GetScene("))
        self.assertLess(on_finish.index("GetScene("), on_finish.index("DoSyncRoleDataOver"))
        self.assertNotIn("DoSyncRoleDataOver", finish)

    def test_login_script_and_confirm_ready_match_target_gates(self):
        player = (ROOT / "src/SO3World/Src/KPlayer.cpp").read_bytes().decode("gbk", errors="ignore")
        server = (ROOT / "src/SO3World/Src/KPlayerServer.cpp").read_bytes().decode("gbk", errors="ignore")

        login = player.split("BOOL KPlayer::CallLoginScript()", 1)[1].split(
            "BOOL KPlayer::", 1
        )[0]
        self.assertLess(login.index("IsScriptExist(PLAYER_SCRIPT)"), login.index("SafeCallBegin"))
        self.assertLess(login.index("SafeCallBegin"), login.index("PushValueToStack(this)"))
        self.assertLess(login.index("PushValueToStack(this)"), login.index('"OnPlayerLogin"'))
        self.assertLess(login.index('"OnPlayerLogin"'), login.index("SafeCallEnd"))

        self.assertIn(
            "REGISTER_EXTERNAL_FUNC(c2s_client_confirm_ready, &KPlayerServer::OnClientConfirmReady",
            server,
        )
        ready = server.split("void KPlayerServer::OnClientConfirmReady", 1)[1].split(
            "void KPlayerServer::OnSyncRoleDataSectionCheckRespond", 1
        )[0]
        self.assertLess(ready.index("GetPlayerByConnection"), ready.index("m_bExtDataLoadFinish"))
        self.assertLess(ready.index("m_bExtDataLoadFinish"), ready.index("gsWaitForSyncClientData"))
        self.assertLess(ready.index("gsWaitForSyncClientData"), ready.index("OnClientReady"))
        self.assertIn("Detach(nConnIndex)", ready)
