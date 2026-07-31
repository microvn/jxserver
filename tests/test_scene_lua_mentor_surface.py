from pathlib import Path


ROOT = Path(__file__).parents[1]


def read(relative):
    return (ROOT / relative).read_text(encoding="latin-1")


def test_scene_lua_properties_and_copy_paths_match_target_surface():
    scene_header = read("include/Include/SO3World/KScene.h")
    scene_lua = read("src/SO3World/Src/KLuaScene.cpp")
    world = read("src/SO3World/Src/KSO3World.cpp")
    relay = read("src/SO3World/Src/KRelayClient.cpp")

    fields = ["m_bCanTongWar", "m_bCanPK", "m_bCanDuel"]
    assert all(field in scene_header for field in fields)
    assert all(field in scene_header for field in (
        "m_szDisplayName",
        "m_nMaxLootRange",
        "m_dwBanUseItemMask",
        "m_bIsArenaMap",
        "m_bBroadcastTargetFlag",
        "m_nAOECountPercent",
        "m_bGongFangFightFlag",
        "m_FightList",
        "m_bNeedCampBuff",
    ))
    assert "std::set<uint64_t, std::less<uint64_t>, KMemory::KAllocator<uint64_t> >" in scene_header
    assert "offsetof(KScene, m_bCanTongWar) == 0xb0" in scene_header
    assert "offsetof(KScene, m_bCanPK) == 0xb4" in scene_header
    assert "offsetof(KScene, m_bCanDuel) == 0xb8" in scene_header
    assert all(f"REGISTER_LUA_BOOL_READONLY(KScene, {name})" in scene_lua
               for name in ("CanTongWar", "CanPK", "CanDuel"))
    assert all(f"pScene->{field}" in world and f"pMapParams->{param}" in world
               for field, param in (("m_bCanTongWar", "bCanTongWar"),
                                    ("m_bCanPK", "bCanPK"),
                                    ("m_bCanDuel", "bCanDuel")))
    assert all(f"pScene->{field}" in relay and f"pMapParams->{param}" in relay
               for field, param in (("m_bCanTongWar", "bCanTongWar"),
                                    ("m_bCanPK", "bCanPK"),
                                    ("m_bCanDuel", "bCanDuel")))


def test_mentor_value_wire_layout_and_lua_api_match_target():
    role_db = read("include/Include/KRoleDBDataDef.h")
    player_h = read("src/SO3World/Src/KPlayer.h")
    player_cpp = read("src/SO3World/Src/KPlayer.cpp")
    player_lua = read("src/SO3World/Src/KLuaPlayer.cpp")
    log_protocol = read("include/Base/protocol/KG_CSLogServerProtocal.h")
    log_client = read("src/SO3World/Src/KLogClient.cpp")

    assert "int     nUsableMentorValue;" in role_db
    assert "sizeof(KMENTOR_DATA_ON_PLAYER) == 0x24" in role_db
    assert "offsetof(KMENTOR_DATA_ON_PLAYER, nUsableMentorValue) == 0x04" in role_db
    assert "offsetof(KMENTOR_DATA_ON_PLAYER, dwTAEquipsScore) == 0x0c" in role_db
    assert "KGRADUATED_MENTOR_DATA_DB" in role_db
    assert role_db.index("nAcquiredMentorValue") < role_db.index("nUsableMentorValue")
    assert "DECLARE_LUA_INTEGER(UsableMentorValue)" in player_h
    assert "BOOL KPlayer::AddUsableMentorValue(int nDeltaMentorValue)" in player_cpp
    assert "pMentorData->nUsableMentorValue" in player_cpp
    assert "m_nUsableMentorValue = pMentorData->nUsableMentorValue" in player_cpp
    assert "LoadMentorData(BYTE* pbyData, size_t uDataLen, int nVersion)" in player_h
    assert "LoadMentorData(pbyOffset, pBlock->dwLen, pBlock->dwVer)" in player_cpp
    assert "m_GraduateMentorData" in player_cpp
    assert "m_GraduateApprenticeData" in player_cpp
    assert "REGISTER_LUA_INTEGER(KPlayer, UsableMentorValue)" in player_lua
    assert "REGISTER_LUA_FUNC(KPlayer, AddUsableMentorValue)" in player_lua
    assert "PLAYER_ACTION_USABLE_MV_CHANGE = 60" in log_protocol
    assert "PLAYER_ACTION_USABLE_MV_CHANGE" in log_client
    assert 'Call(m_nConnIndex, "OnSyncMentorData")' in player_cpp
    assert "LogUsableMentorValueChange(nDeltaValue, this, NULL, pszComment)" in player_lua


def test_role_loader_uses_target_hair_and_mini_avatar_signatures():
    player_h = read("src/SO3World/Src/KPlayer.h")
    player_cpp = read("src/SO3World/Src/KPlayer.cpp")
    mini_h = read("src/SO3World/Src/KMiniAvatar.h")
    mini_cpp = read("src/SO3World/Src/KMiniAvatar.cpp")

    assert "case rbtHairBoxData:" in player_cpp
    assert "m_HairBox.Load(pbyOffset, pBlock->dwLen)" in player_cpp
    assert "case rbtMiniAvatarData:" in player_cpp
    assert "m_MiniAvatar.Load(pbyOffset, pBlock->dwLen, pBlock->dwVer)" in player_cpp
    assert "BOOL Load(BYTE* pbyData, size_t uDataLen, unsigned long nVersion);" in mini_h
    assert "BOOL KMiniAvatar::Load(BYTE* pbyData, size_t uDataLen, unsigned long nVersion)" in mini_cpp
    assert "KGLOG_PROCESS_ERROR(nVersion <= 1);" in mini_cpp


def test_activity_manager_lua_object_is_registered_before_getter_use():
    script_center = read("src/SO3World/Src/KScriptCenter.cpp")
    base_functions = read("src/SO3World/Src/KBaseFuncList.cpp")
    activity = read("src/SO3World/Src/KActivityMgrServer.cpp")

    assert "Luna<KActivityMgrServer>::Register(pLuaState);" in script_center
    assert '"GetActivityMgrServer",      LuaGetActivityMgrServer' in base_functions
    assert "return g_pSO3World->m_ActivityMgrServer.LuaGetObj(L);" in base_functions
    assert "REGISTER_LUA_FUNC(KActivityMgrServer, GetActivityState)" in activity
