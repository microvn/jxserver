from pathlib import Path


ROOT = Path(__file__).parents[1]
PROTO = ROOT / "include/Include/Relay_GS_Protocol.h"
RELAY_H = ROOT / "src/SO3World/Src/KRelayClient.h"
RELAY_CPP = ROOT / "src/SO3World/Src/KRelayClient.cpp"
WORLD_CPP = ROOT / "src/SO3World/Src/KSO3World.cpp"
PLAYER_H = ROOT / "src/SO3World/Src/KPlayer.h"
PLAYER_CPP = ROOT / "src/SO3World/Src/KPlayer.cpp"
REG_H = ROOT / "src/SO3World/Src/KRegressionPlayerData.h"
REG_CPP = ROOT / "src/SO3World/Src/KRegressionPlayerData.cpp"
MANAGER_CPP = ROOT / "src/SO3World/Src/KRegressionManager.cpp"
PLAYER_SERVER_H = ROOT / "src/SO3World/Src/KPlayerServer.h"
PLAYER_SERVER_CPP = ROOT / "src/SO3World/Src/KPlayerServer.cpp"
CLIENT_PROTO = ROOT / "include/Include/GS_Client_Protocol.h"


def read(path):
    return path.read_text(encoding="utf-8", errors="ignore")


def test_account_routes_and_payload_shapes_are_target_named():
    text = read(PROTO)
    assert "r2s_save_account_data_respond" in text
    assert "r2s_sync_account_data" in text
    assert "r2s_load_account_data" in text
    assert "struct S2R_LOAD_ACCOUNT_DATA_REQUEST" in text
    assert "DWORD   dwPlayerID;" in text
    assert "struct R2S_LOAD_ACCOUNT_DATA" in text
    assert "BOOL    bSucceed;" in text
    assert "size_t  uAccountDataLen;" in text
    assert "struct R2S_SYNC_ACCOUNT_DATA" in text
    assert "size_t  uOffset;" in text
    assert "struct R2S_SAVE_ACCOUNT_DATA_RESPOND" in text


def test_account_handlers_and_buffers_are_registered():
    header = read(RELAY_H)
    source = read(RELAY_CPP)
    assert "m_pbySyncAccountBuffer" in header
    assert "m_uSyncAccountOffset" in header
    assert "m_pbySaveAccountBuffer" in header
    for name in (
        "OnSaveAccountDataRespond",
        "OnSyncAccountData",
        "OnLoadAccountData",
        "SaveAccountData",
        "DoSaveAccountData",
        "DoSyncAccountData",
    ):
        assert name in header
        assert name in source
    assert "REGISTER_INTERNAL_FUNC(r2s_save_account_data_respond" in source
    assert "REGISTER_INTERNAL_FUNC(r2s_sync_account_data" in source
    assert "REGISTER_INTERNAL_FUNC(r2s_load_account_data" in source


def test_account_chunk_handler_validates_wire_size_before_subtracting_header():
    source = read(RELAY_CPP)
    start = source.index("void KRelayClient::OnSyncAccountData")
    end = source.index("void KRelayClient::OnLoadAccountData", start)
    body = source[start:end]

    assert "KGLOG_PROCESS_ERROR(pbyData);" in body
    assert body.index("uDataLen >= sizeof(R2S_SYNC_ACCOUNT_DATA)") < body.index(
        "uAccountDataLen = uDataLen - sizeof(R2S_SYNC_ACCOUNT_DATA)"
    )
    assert "MAX_ACCOUNT_DATA_SIZE - m_uSyncAccountOffset >= uAccountDataLen" in body


def test_frontier_relay_senders_match_target_wire_contracts():
    protocol = read(PROTO)
    header = read(RELAY_H)
    source = read(RELAY_CPP)

    for struct_name, fields in (
        ("S2R_APPLY_SINGLE_DUNGEON_LAST_SCORE", ("DWORD dwPlayer;",)),
        ("S2R_APPLY_COIN_OPERATING_FLAG", ("DWORD dwPlayerID;",)),
        ("S2R_SYNC_CORPS_CHANGE_DATA_REQUEST", ("DWORD dwPlayerID;", "time_t nChangeTime;", "time_t nWeekTime;", "time_t nSeasonTime;")),
        ("S2R_SYNC_FELLOWSHIP_PLAYER_MINI_AVATAR", ("DWORD dwPlayerID;", "WORD  wMiniAvatarID;", "WORD  wRoleType;")),
    ):
        assert "struct %s" % struct_name in protocol
        for field in fields:
            assert field in protocol

    for name in (
        "DoApplySingleDungeonLastScore",
        "DoApplyCoinOperatingFlag",
        "DoSyncCorpsChangeDataRequest",
        "DoSyncFellowshipPlayerMiniAvatar",
    ):
        assert name in header
        assert "KRelayClient::%s" % name in source


def test_login_frontier_wires_mini_avatar_after_apprentice_sync():
    source = read(PLAYER_CPP)
    start = source.index("BOOL KPlayer::FinishRoleDataLoad")
    body = source[start:source.index("BOOL KPlayer::OnClientReady", start)]
    assert body.index("DoUpdateMaxApprenticeNum") < body.index(
        "DoSyncFellowshipPlayerMiniAvatar"
    )
    assert "m_dwID, m_dwMiniAvatarID, (int)m_eRoleType" in body


def test_single_dungeon_role_block_matches_target_layout():
    header = read(PLAYER_H)
    source = read(PLAYER_CPP)
    assert "LoadSingleDungeonData(BYTE* pbyData, size_t uDataLen)" in header
    assert "SaveSingleDungeonData(size_t* puUsedSize" in header
    assert "uDataLen >= 0x424" in source
    assert "m_dwSingleDungeonMaxLevel <= 128" in source
    assert "memcpy(m_dwSingleDungeonScore, pbyData + 4" in source
    assert "SaveSingleDungeonData, rbtSingleDungeonData, 0" in source


def test_single_dungeon_frontier_matches_target_guard_and_packet():
    source = read(PLAYER_CPP)
    server = read(PLAYER_SERVER_CPP)
    assert "m_dwSingleDungeonScore[0] != 0" in source
    assert "DoApplySingleDungeonLastScore(m_dwID)" in source
    assert "SyncSingleDungeonCurrentScore();" in source
    assert "s2c_sync_single_dungeon_current_score" in server
    assert "DoSyncSingleDungeonCurrentScore" in server


def test_arena_corps_block_and_frontier_match_target():
    header = read(PLAYER_H)
    source = read(PLAYER_CPP)
    server = read(PLAYER_SERVER_CPP)
    assert "LoadArenaData(BYTE* pbyData, size_t uDataLen)" in header
    assert "SaveArenaData(size_t* puUsedSize" in header
    # Regression: candidate used the payload size as a field offset and wrote a
    # 0x66-byte arena block, while target KARENA_ROLE_DATA is packed and 0x42 bytes.
    assert "struct KARENA_ROLE_DATA" in source
    assert "sizeof(KARENA_ROLE_DATA) == 0x42" in source
    assert "uDataLen >= sizeof(KARENA_ROLE_DATA)" in source
    assert "uBufferSize >= sizeof(KARENA_ROLE_DATA)" in source
    assert "pbyData + 0x42" not in source
    assert "pbyBuffer + 0x42" not in source
    assert "*puUsedSize = sizeof(KARENA_ROLE_DATA)" in source
    assert "m_dwCorpsSystemID != 0" in source
    assert "DoSyncCorpsChangeDataRequest" in source
    assert "DoSyncCorpsChangeValue" in server
    assert "s2c_sync_corps_change_value" in server


def test_ta_equips_relay_payloads_match_target_layout():
    protocol = read(PROTO)
    client = read(RELAY_H)
    source = read(RELAY_CPP)
    assert "s2r_add_ta_equips_score_request = 178" in protocol
    assert "s2r_pickup_ta_equips_score_request = 179" in protocol
    assert "struct S2R_ADD_TA_EQUIPS_SCORE_REQUEST" in protocol
    assert "struct S2R_PICKUP_TA_EQUIPS_SCORE_REQUEST" in protocol
    assert "int     nDeltaScore;" in protocol
    assert "BYTE    byDirectMentor;" in protocol
    assert "DoAddTAEquipsScoreRequest(int nDirectMentor" in client
    assert "DoPickupTAEquipsScoreRequest(int nDirectMentor" in client
    assert "pPacket->nDeltaScore = nDeltaScore;" in source
    assert "pPacket->byDirectMentor = (BYTE)nDirectMentor;" in source


def test_account_save_response_matches_target_failure_gate():
    source = read(RELAY_CPP)
    start = source.index("void KRelayClient::OnSaveAccountDataRespond")
    end = source.index("void KRelayClient::OnSyncAccountData", start)
    body = source[start:end]

    assert "m_nNextQuitingSaveTime = 0" in body
    assert "m_PlayerSet.GetObj(pRespond->dwPlayerID)" in body
    assert "pRespond->nUserValue == gsWaitForTransmissionSave" in body
    assert "!pRespond->bSucceed" in body
    assert "|| pRespond->bSucceed" not in body


def test_account_save_and_load_use_target_global_wrapper():
    source = read(PLAYER_CPP)
    load = source[source.index("BOOL KPlayer::LoadAccountData"):source.index("BOOL KPlayer::Load(BYTE*", source.index("BOOL KPlayer::LoadAccountData"))]
    save = source[source.index("BOOL KPlayer::SaveAccount"):source.index("#define SAVE_ROLE_BLOCK", source.index("BOOL KPlayer::SaveAccount"))]

    assert "KRoleDataHeader* pGlobalHeader" in load
    assert "pbyOffset = pbyData + sizeof(KRoleDataHeader)" in load
    assert "uLeftSize = uDataLen - sizeof(KRoleDataHeader)" in load
    assert "pGlobalHeader->dwLen == uLeftSize" in load
    assert "CRC32(0, pbyOffset, (DWORD)uLeftSize)" in load
    assert "KRoleBlockHeader* pBlock" in load
    assert "pGlobalHeader->dwCRC = CRC32(0, pbyBuffer + sizeof(KRoleDataHeader)" in save
    assert "pBlock->dwVer = 0" in save
    assert "pbyTail - pbyOffset) >= sizeof(KRoleBlockHeader)" in save
    assert "pHeader->dwCRC = CRC32" not in save


def test_account_loader_matches_target_block_failure_and_ordering():
    source = read(PLAYER_CPP)
    start = source.index("BOOL KPlayer::LoadAccountStateInfo")
    end = source.index("BOOL KPlayer::Load(BYTE*", start)
    body = source[start:end]

    assert "KGLOG_PROCESS_ERROR(uDataLen >= 0x4c);" in body
    assert "KGLOG_PROCESS_ERROR(uDataLen == 0x4c);" not in body
    assert "(void)m_RegressionData.LoadAccountData(pbyOffset, pBlock->dwLen);" in body
    assert "KGLOG_PROCESS_ERROR(m_RegressionData.LoadAccountData" not in body
    assert "Unknown account data block(%d, %u)" in body

    finish = source[source.index("BOOL KPlayer::FinishRoleDataLoad"):]
    assert finish.index("m_ItemList.m_bFinishLoadData = true;") < finish.index("UpdateFreeLimitFlag();")
    assert finish.index("UpdateFreeLimitFlag();") < finish.index("m_RegressionData.Calculate(")


def test_finish_frontier_places_timers_after_tong_and_before_scene():
    source = read(PLAYER_CPP)
    start = source.index("BOOL KPlayer::OnExtDataLoadFinish")
    end = source.index("BOOL KPlayer::PartialLoadExtData", start)
    body = source[start:end]

    tong = body.index("SyncNewClient(this)")
    timer = body.index("m_nNextSaveFrame =")
    scene = body.index("GetScene(")
    assert tong < timer < scene
    assert "m_nNextKillPointReduceTime =" in body

    finish_start = source.index("BOOL KPlayer::FinishRoleDataLoad")
    finish_end = source.index("void KPlayer::SyncSingleDungeonCurrentScore", finish_start)
    finish = source[finish_start:finish_end]
    assert "m_nNextSaveFrame =" not in finish
    assert "m_nNextKillPointReduceTime =" not in finish


def test_player_account_timestamp_is_distinct_from_role_timestamp():
    player = read(PLAYER_H)
    assert "m_nAccountLastSaveTime" in player
    assert "LoadAccountData(BYTE* pbyData, size_t uDataLen)" in player
    assert "SaveAccount(size_t* puUsedSize" in player
    assert "SaveAccountData(size_t* puUsedSize" not in player
    assert "m_nAccContinuousLoginCount" in player
    assert "m_bContinuousLoginRewardFlag" in player


def test_regression_has_v246_account_persistence_methods():
    text = read(REG_H)
    assert "LoadAccountData(BYTE* pbyData, size_t uDataLen)" in text
    assert "SaveAccountData(size_t* puUsedSize" in text


def test_account_chunk_and_regression_payload_match_target_limits():
    protocol = read(PROTO)
    regression = read(REG_CPP)
    assert "#define MAX_ACCOUNT_DATA_PAK_SIZE (1024 * 256)" in protocol
    assert "KGLOG_PROCESS_ERROR(uDataLen >= REG_ACCOUNT_DATA_SIZE)" in regression
    assert "KGLOG_PROCESS_ERROR(uDataLen == REG_ACCOUNT_DATA_SIZE)" not in regression
    player = read(PLAYER_CPP)
    assert "KGLOG_PROCESS_ERROR(uDataLen >= 0x4c);" in player


def test_load_account_request_does_not_serialize_pointer_or_account_string():
    source = read(RELAY_CPP)
    start = source.index("BOOL KRelayClient::DoLoadAccountDataRequest")
    end = source.index("BOOL KRelayClient::DoLoadRoleDataRequest", start)
    body = source[start:end]
    assert "pLoadAccountData->dwPlayerID" in body
    assert "pLoadAccountData->pszAccount" not in body
    assert "m_dwSyncAccountID = dwRoleID" not in body
    assert "m_uSyncAccountOffset = 0" not in body


def test_account_save_is_wired_at_target_save_boundaries():
    relay = read(RELAY_CPP)
    world = read(WORLD_CPP)

    switch_start = relay.index("if (pRespond->bSwitchServer)")
    switch_end = relay.index("else\n\t{", switch_start)
    switch_body = relay[switch_start:switch_end]
    assert switch_body.index("SaveRoleData(pPlayer)") < switch_body.index("SaveAccountData(pPlayer)")

    deleting = world[world.index("case gsDeleting:"):world.index("case gsPlaying:")]
    assert deleting.index("SaveRoleData(pPlayer)") < deleting.index("SaveAccountData(pPlayer)")

    playing_start = world.index("case gsPlaying:")
    playing = world[playing_start:world.index("if ((g_pSO3World->m_nGameLoop", playing_start)]
    assert playing.index("SaveRoleData(pPlayer)") < playing.index("SaveAccountData(pPlayer)")


def test_account_save_refreshes_both_target_timers():
    source = read(PLAYER_CPP)
    start = source.index("BOOL KPlayer::SaveAccount")
    end = source.index("#define SAVE_ROLE_BLOCK", start)
    body = source[start:end]

    assert "m_nAccountLastSaveTime = g_pSO3World->m_nCurrentTime" in body
    assert "m_nNextSaveFrame = g_pSO3World->m_nGameLoop" in body
    assert "nSaveInterval * GAME_FPS" in body


def test_sync_data_load_state_is_cleaned_like_target():
    base = read(ROOT / "src/SO3World/Src/KPlayerServerBase.cpp")
    world = read(WORLD_CPP)

    detach = base[base.index("BOOL KPlayerServer::Detach"):base.index("BOOL KPlayerServer::Send", base.index("BOOL KPlayerServer::Detach"))]
    timeout = world[world.index("case gsWaitForPermit:"):world.index("case gsWaitForLoginLoading:", world.index("case gsWaitForPermit:"))]

    assert "case gsWaitForSyncClientData:" in detach
    assert "case gsWaitForSyncClientData:" in timeout


def test_role_save_has_no_candidate_only_partial_load_guard():
    source = read(RELAY_CPP)
    start = source.index("BOOL KRelayClient::SaveRoleData")
    end = source.index("BOOL KRelayClient::", start + len("BOOL KRelayClient::SaveRoleData"))
    body = source[start:end]

    assert "Skip role save before ext data load completes" not in body


def test_regression_calculate_and_reward_contract():
    header = read(REG_H)
    source = read(REG_CPP)
    player = read(PLAYER_CPP)
    manager = read(MANAGER_CPP)

    assert "void Calculate(time_t nAccLastSaveTime, time_t nPlayerLastSaveTime)" in header
    assert "BOOL AddRewardItem(int nDailyIndex, int nItemIndex, DWORD dwKungFuID)" in header
    assert "BOOL CallAddRewardItemScript" in header
    assert "m_nCurrentGradeID(-1)" in source
    assert "m_RegressionData.Calculate(m_nAccountLastSaveTime, m_nLastSaveTime)" in player
    assert "ItemInfo.ItemInfoMap[(DWORD)nKungFu] = Item" in manager
    assert "ItemInfoMap.find(dwKungFuID)" in source
    assert "pItemInfo->nRegressionDailyCount > m_nRegressionDailyCount" in source
    assert "int    GetRewardGradeID(long nLossDays)" in read(ROOT / "src/SO3World/Src/KRegressionManager.h")
    assert "AddRegressionRewardItem" in source


def test_regression_reward_wire_contract():
    protocol = read(CLIENT_PROTO)
    server_h = read(PLAYER_SERVER_H)
    server = read(PLAYER_SERVER_CPP)

    assert "s2c_sync_regression_player_data = 292" in protocol
    assert "struct S2C_SYNC_REGRESSION_PLAYER_DATA" in protocol
    assert "struct C2S_ADD_REGRESSION_REWARD_ITEM" in protocol
    assert "DoSyncRegressionPlayerData" in server_h
    assert "OnAddRegressionRewardItem" in server_h
    assert "REGISTER_EXTERNAL_FUNC(c2s_add_regression_reward_item" in server
    assert "Notify.byProtocolID = s2c_sync_regression_player_data" in server
    assert "pPlayer->m_RegressionData.AddRewardItem" in server
