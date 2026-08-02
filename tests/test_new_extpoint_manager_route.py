from pathlib import Path


ROOT = Path(__file__).parents[1]
SRC = ROOT / "src/SO3World/Src"
INCLUDE = ROOT / "include/Include"


def test_manager_exposes_target_state_and_lookup_contract():
    text = (SRC / "KNewExtPointManager.h").read_text()
    assert "struct KNewExtPointInfo" in text
    assert "int nValue" in text
    assert "BOOL bLocked" in text
    assert "KMemory::KAllocator<std::pair<int, KNewExtPointInfo> >" in text
    assert "KNEW_EXT_POINT_MAP m_NEPMap" in text
    assert "BOOL AddNewExtPoint(int nKey, int nValue, BOOL bLocked)" in text
    assert "BOOL GetNewExtPoint(int nIndex, int* pnValue) const" in text


def test_player_get_ext_point_delegates_non_legacy_indexes():
    header = (SRC / "KPlayer.h").read_text(errors="ignore")
    source = (SRC / "KPlayer.cpp").read_text(errors="ignore")
    assert '#include "KNewExtPointManager.h"' in header
    assert "KNewExtPointManager m_NewExtPointManager" in header
    assert "BOOL GetExtPoint(int nIndex, int& nValue);" in header
    body = source[source.index("BOOL KPlayer::GetExtPoint") : source.index("BOOL KPlayer::SetExtPoint")]
    assert "nIndex < MAX_EXT_POINT_COUNT" in body
    assert "m_NewExtPointManager.GetNewExtPoint" in body


def test_relay_new_extpoint_wire_contract_and_registration_exist():
    protocol = (INCLUDE / "Relay_GS_Protocol.h").read_text(errors="ignore")
    relay = (SRC / "KRelayClient.cpp").read_text(errors="ignore")
    assert "r2s_sync_new_ext_point_respond" in protocol
    assert "r2s_apply_gs_new_ext_point" in protocol
    assert "r2s_change_new_ext_point_respond" in protocol
    assert "KSyncGSNEPInfo" in protocol
    assert "KRelayClient::OnSyncNewExtPointRespond" in relay
    assert "KRelayClient::OnSyncGSNewExtPoint" in relay
    assert "KRelayClient::OnApplyGSNewExtPoint" in relay
    assert "KRelayClient::OnChangeNewExtPointRespond" in relay
    assert "REGISTER_INTERNAL_FUNC(139, &KRelayClient::OnSyncNewExtPointRespond, 19)" in relay
    assert "REGISTER_INTERNAL_FUNC(141, &KRelayClient::OnChangeNewExtPointRespond, 26)" in relay
    assert "REGISTER_INTERNAL_FUNC(142, &KRelayClient::OnApplyGSNewExtPoint, 14)" in relay
    assert "REGISTER_INTERNAL_FUNC(143, &KRelayClient::OnSyncGSNewExtPoint, 15)" in relay
    assert "REGISTER_INTERNAL_FUNC(r2s_v246_unused_155, &KRelayClient::OnNoOpRespond, 19)" in relay
    assert "REGISTER_INTERNAL_FUNC(r2s_v246_unused_157, &KRelayClient::OnNoOpRespond, 26)" in relay
    assert "REGISTER_INTERNAL_FUNC(r2s_v246_unused_158, &KRelayClient::OnNoOpRespond, 14)" in relay
    assert "REGISTER_INTERNAL_FUNC(r2s_v246_unused_159, &KRelayClient::OnNoOpRespond, 15)" in relay
    assert "REGISTER_INTERNAL_FUNC(r2s_sync_new_ext_point_respond" not in relay
    assert "r2s_sync_new_ext_point_respond,\t// v246 id=139" in protocol
    assert "r2s_change_new_ext_point_respond,\t// v246 id=141" in protocol
    assert "r2s_apply_gs_new_ext_point,\t// v246 id=142" in protocol
    assert "r2s_sync_gs_new_ext_point,\t// v246 id=143" in protocol


def test_new_extpoint_response_populates_manager_before_followup():
    relay = (SRC / "KRelayClient.cpp").read_text(errors="ignore")
    add = relay.index("m_NewExtPointManager.AddNewExtPoint")
    followup = relay.index("DoSyncNewExtPointRequest", add)
    assert "SyncNEPInfo[i].nKey" in relay[add:followup]
    assert "SyncNEPInfo[i].nValue" in relay[add:followup]


def test_gs_sync_uses_target_completion_contract():
    manager = (SRC / "KNewExtPointManager.h").read_text(errors="ignore")
    relay = (SRC / "KRelayClient.cpp").read_text(errors="ignore")
    assert "int* pnCount, BOOL* pbSyncFinish" in manager
    assert "const int MAX_SYNC_COUNT = 0x100;" in relay
    assert "&nCount, &bSyncFinish" in relay
    assert "pRequest->bySyncFinish = (BYTE)bSyncFinish;" in relay
    assert "&nCount, &nLastKey" not in relay


def test_set_ext_point_routes_new_indexes_and_response_does_not_resend():
    player = (SRC / "KPlayer.h").read_text(errors="ignore")
    player_cpp = (SRC / "KPlayer.cpp").read_text(errors="ignore")
    relay = (SRC / "KRelayClient.cpp").read_text(errors="ignore")
    manager = (SRC / "KNewExtPointManager.cpp").read_text(errors="ignore")
    assert "BOOL SetExtPoint(int nIndex, int nChangeValue);" in player
    body = player_cpp[player_cpp.index("BOOL KPlayer::SetExtPoint") : player_cpp.index("#endif // _SERVER", player_cpp.index("BOOL KPlayer::SetExtPoint"))]
    assert "nIndex >= MAX_EXT_POINT_COUNT" in body
    assert "m_NewExtPointManager.SetNewExtPoint" in body
    assert "OnChangeNewExtPoint(" in relay
    response = relay[relay.index("void KRelayClient::OnChangeNewExtPointRespond") : relay.index("void KRelayClient::OnConfirmPlayerLoginRespond")]
    assert "SetNewExtPoint" not in response
    assert "pRespond->nOldValue" in response
    assert "DoChangeNewExtPointRequest" in manager


def test_bit_accessor_uses_target_32_bit_boundary():
    manager = (SRC / "KNewExtPointManager.cpp").read_text(errors="ignore")
    assert "nBitIndex + nBitLength <= 32" in manager
    assert "nBitLength == 32 ? 0xffffffffU" in manager
    assert "nBitLength <= 31" not in manager


def test_player_bit_helpers_cover_legacy_and_new_extpoint_owners():
    header = (SRC / "KPlayer.h").read_text(errors="ignore")
    source = (SRC / "KPlayer.cpp").read_text(errors="ignore")
    assert "BOOL GetExtPointByBits(int nIndex, int nBitIndex, int nBitLength, int& nValue);" in header
    assert "BOOL SetExtPointByBits(int nIndex, int nBitIndex, int nBitLength, int& nValue);" in header
    assert "BOOL CanSetExtPoint(int nIndex);" in header
    assert "m_ExtPointInfo.nExtPoint[nIndex]" in source
    assert "m_NewExtPointManager.GetNewExtPointByBits" in source
    assert "m_NewExtPointManager.SetExtPointByBits" in source
    assert "m_NewExtPointManager.CanSetExtPoint" in source


def test_lua_extpoint_bindings_use_player_boundary_and_are_registered():
    header = (SRC / "KPlayer.h").read_text(errors="ignore")
    lua = (SRC / "KLuaPlayer.cpp").read_text(errors="ignore")
    for name in ("LuaGetExtPointByBits", "LuaSetExtPointByBits", "LuaCanSetExtPoint"):
        assert f"int {name}(Lua_State* L);" in header
        assert f"int KPlayer::{name}(Lua_State* L)" in lua
        assert f"REGISTER_LUA_FUNC(KPlayer, {name[3:]})" in lua
    get_body = lua[lua.index("int KPlayer::LuaGetExtPoint(Lua_State* L)") : lua.index("int KPlayer::LuaSetExtPoint", lua.index("int KPlayer::LuaGetExtPoint"))]
    assert "GetExtPoint(nExtPointIndex, nExtPointValue)" in get_body
