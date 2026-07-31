from pathlib import Path


ROOT = Path(__file__).parents[1]
HEADER = (ROOT / "include/Include/GS_Client_Protocol.h").read_text(encoding="utf-8", errors="ignore")
TONG = (ROOT / "include/Include/KTongDef.h").read_bytes().decode("gbk", errors="ignore")
WORLD = (ROOT / "include/Include/SO3World/KSO3World.h").read_bytes().decode("gbk", errors="ignore")
PLAYER = (ROOT / "src/SO3World/Src/KPlayer.cpp").read_bytes().decode("gbk", errors="ignore")


def test_target_tong_wire_layout_is_declared():
    assert "struct KTONG_DIPLOMACY_RELATION_INFO" in TONG
    assert "struct S2C_SYNC_TONG_DIPLOMACY_DATA" in HEADER
    assert "struct S2C_SYNC_TONG_TOTAL_CACHE" in HEADER
    assert "BYTE    byCacheData[7];" in HEADER


def test_tong_cache_members_and_login_order_are_present():
    assert "KTongDiplomacyCache                 m_TongDiplomacyCache;" in WORLD
    assert "KTongServer                          m_TongServer;" in WORLD
    diplomacy = PLAYER.index("m_TongDiplomacyCache.SyncNewClient(this)")
    total = PLAYER.index("m_TongServer.SyncTongTotalCache(m_dwTongID, m_nConnIndex)")
    role_over = PLAYER.index("DoSyncRoleDataOver(m_nConnIndex)")
    assert diplomacy < total < role_over


def test_tong_sender_does_not_use_legacy_protocol_ids():
    source = (ROOT / "src/SO3World/Src/KPlayerServer.cpp").read_text(encoding="utf-8", errors="ignore")
    assert "DoSyncTongDiplomacyData" in source
    assert "s2c_sync_tong_diplomacy_data" in source
    assert "DoSyncTongTotalCache" in source
    assert "s2c_sync_tong_total_cache" in source
