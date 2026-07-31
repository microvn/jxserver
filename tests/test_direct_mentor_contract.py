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
    assert "REGISTER_INTERNAL_FUNC(162, &KRelayClient::OnSyncDirectMentorData, 6)" in relay
    assert "REGISTER_INTERNAL_FUNC(163, &KRelayClient::OnDeleteDirectMentorRecord, 10)" in relay
    assert "REGISTER_INTERNAL_FUNC(164, &KRelayClient::OnUpdateDirectMentorRecord, 15)" in relay
    assert "m_DirectMentorCache.PickupTAEquipsScore(m_dwID)" in player
    assert "BOOL KPlayer::AddTAEquipsScore(int nDeltaScore)" in player
