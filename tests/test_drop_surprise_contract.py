from pathlib import Path


ROOT = Path(__file__).parents[1]
ROLE_DEF = (ROOT / "include/Include/KRoleDBDataDef.h").read_text(encoding="gbk", errors="ignore")
PLAYER_CPP = (ROOT / "src/SO3World/Src/KPlayer.cpp").read_text(encoding="gbk", errors="ignore")
PLAYER_H = (ROOT / "src/SO3World/Src/KPlayer.h").read_text(encoding="gbk", errors="ignore")


def test_drop_surprise_target_layout_is_explicit():
    assert "byMask[16]" in ROLE_DEF
    assert "byReserved[16]" in ROLE_DEF
    assert "sizeof(KDROP_SURPRISE_DATA) == 0x20" in ROLE_DEF


def test_drop_surprise_load_save_and_dispatch_are_present():
    assert "LoadDropSurpriseData(BYTE*" in PLAYER_H
    assert "SaveDropSurpriseData(size_t*" in PLAYER_H
    assert "KPlayer::LoadDropSurpriseData(BYTE*" in PLAYER_CPP
    assert "KPlayer::SaveDropSurpriseData(size_t*" in PLAYER_CPP
    assert "case rbtDropSurpriseData:" in PLAYER_CPP
    assert "SAVE_ROLE_BLOCK(SaveDropSurpriseData, rbtDropSurpriseData, 0);" in PLAYER_CPP


def test_drop_surprise_consumes_only_target_mask_and_zeroes_reserved_bytes():
    assert "memcpy(m_byDropSurpriseMask, pData->byMask" in PLAYER_CPP
    assert "memset(&Data, 0, sizeof(Data));" in PLAYER_CPP
