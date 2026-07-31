from pathlib import Path


ROOT = Path(__file__).parents[1]
ROLE_DEF = (ROOT / "include/Include/KRoleDBDataDef.h").read_text(encoding="gbk", errors="ignore")
PLAYER_CPP = (ROOT / "src/SO3World/Src/KPlayer.cpp").read_text(encoding="gbk", errors="ignore")
PLAYER_H = (ROOT / "src/SO3World/Src/KPlayer.h").read_text(encoding="gbk", errors="ignore")


def test_activity_variables_target_layout_is_explicit():
    assert "wPrensentCodeCounters[32][2]" in ROLE_DEF
    assert "sizeof(KACTIVITY_VARIABLES) == 0x80" in ROLE_DEF


def test_activity_variables_has_load_save_and_dispatch_paths():
    assert "LoadActivityVariables(BYTE*" in PLAYER_H
    assert "SaveActivityVariables(size_t*" in PLAYER_H
    assert "KPlayer::LoadActivityVariables(BYTE*" in PLAYER_CPP
    assert "KPlayer::SaveActivityVariables(size_t*" in PLAYER_CPP
    assert "case rbtActivityVariables:" in PLAYER_CPP
    assert "SAVE_ROLE_BLOCK(SaveActivityVariables, rbtActivityVariables, 0);" in PLAYER_CPP


def test_activity_variables_consumes_exact_target_payload():
    assert "sizeof(KACTIVITY_VARIABLES)" in PLAYER_CPP
    assert "memcpy(m_wPresentCodeCounters" in PLAYER_CPP
