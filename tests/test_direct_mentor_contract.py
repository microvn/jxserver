from pathlib import Path


ROOT = Path(__file__).parents[1]
SRC = ROOT / "src/SO3World/Src"


def test_target_direct_mentor_layouts_are_present():
    header = (SRC / "KDirectMentorCache.h").read_text()
    relay = (ROOT / "include/Include/Relay_GS_Protocol.h").read_text(errors="ignore")
    assert "DWORD dwTAEquipsScore;" in header
    assert "KDirectMentorSyncInfo" in relay
    assert "struct R2S_SYNC_DIRECT_MENTOR_DATA" in relay
    assert "struct R2S_DELETE_DIRECT_MENTOR_RECORD" in relay
    assert "struct R2S_UPDATE_DIRECT_MENTOR_RECORD" in relay


def test_direct_mentor_registration_and_frontier_pickup():
    relay = (SRC / "KRelayClient.cpp").read_text(errors="ignore")
    player = (SRC / "KPlayer.cpp").read_text(errors="ignore")
    assert "REGISTER_INTERNAL_FUNC(162, &KRelayClient::OnSyncMentorData, 6)" in relay
    assert "REGISTER_INTERNAL_FUNC(163, &KRelayClient::OnSyncDirectMentorData, 6)" in relay
    assert "REGISTER_INTERNAL_FUNC(164, &KRelayClient::OnDeleteMentorRecord, 10)" in relay
    assert "REGISTER_INTERNAL_FUNC(165, &KRelayClient::OnDeleteDirectMentorRecord, 10)" in relay
    assert "REGISTER_INTERNAL_FUNC(166, &KRelayClient::OnUpdateMentorRecord, 19)" in relay
    assert "REGISTER_INTERNAL_FUNC(167, &KRelayClient::OnUpdateDirectMentorRecord, 15)" in relay
    assert "m_DirectMentorCache.PickupTAEquipsScore(m_dwID)" in player
    assert "BOOL KPlayer::AddTAEquipsScore(int nDeltaScore)" in player


def test_target_world_lifecycle_initializes_required_caches():
    world = (SRC / "KSO3World.cpp").read_text(errors="ignore")
    assert "m_TongDiplomacyCache.Init()" in world
    assert "m_TongDiplomacyCache.UnInit()" in world
    assert "m_TongServer.Init()" in world
    assert "m_TongServer.UnInit()" in world
