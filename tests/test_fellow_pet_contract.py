from pathlib import Path


ROOT = Path(__file__).parents[1]
PLAYER_H = ROOT / "src/SO3World/Src/KPlayer.h"
PET_H = ROOT / "src/SO3World/Src/KFellowPetBox.h"
PLAYER_CPP = ROOT / "src/SO3World/Src/KPlayer.cpp"
SERVER_CPP = ROOT / "src/SO3World/Src/KPlayerServer.cpp"
PROTO_H = ROOT / "include/Include/GS_Client_Protocol.h"


def test_target_fellow_pet_layout_and_wire_records():
    text = PET_H.read_text(encoding="utf-8")
    assert "KCustomData<9> m_PetData" in text
    assert "std::map<int, KPetTimeLimitInfo> m_PetTimeLimitInfoMap" in text
    assert "uLeftSize >= 11" in text
    assert "pbyOffset + 3" in text
    assert "pbyOffset + 7" in text
    assert "pbyOffset += 11" in text


def test_target_fellow_pet_versions_and_role_dispatch():
    pet = PET_H.read_text(encoding="utf-8")
    player = PLAYER_CPP.read_text(encoding="utf-8", errors="ignore")
    assert "dwVersion == 0" in pet
    assert "dwVersion == 1" in pet
    assert "rbtFellowPetData" in player
    assert "LoadFellowPetData(pbyOffset, pBlock->dwLen" in player
    assert "SaveFellowPetData, rbtFellowPetData, 1" in player


def test_target_fellow_pet_packet_is_protocol_177_and_uses_box_serializer():
    proto = PROTO_H.read_text(encoding="utf-8")
    server = SERVER_CPP.read_text(encoding="utf-8", errors="ignore")
    assert "s2c_sync_fellow_pet_data = 177" in proto
    assert "BOOL KPlayerServer::DoSyncFellowPetData(KPlayer* pPlayer)" in server
    assert "pPlayer->m_FellowPetBox.Save" in server
    assert "s2c_sync_fellow_pet_data" in server
