from pathlib import Path


ROOT = Path(__file__).parents[1]
ROLE_DEF = (ROOT / "include/Include/KRoleDBDataDef.h").read_text(encoding="gbk", errors="ignore")
PROTOCOL = (ROOT / "include/Include/GS_Client_Protocol.h").read_text(encoding="gbk", errors="ignore")
PLAYER_CPP = (ROOT / "src/SO3World/Src/KPlayer.cpp").read_text(encoding="gbk", errors="ignore")
PLAYER_SERVER_CPP = (ROOT / "src/SO3World/Src/KPlayerServer.cpp").read_text(encoding="gbk", errors="ignore")


def test_bank_password_layout_and_protocol_match_target():
    assert "sizeof(KBANK_PASSWORD_DATA) == 0x84" in ROLE_DEF
    assert "s2c_sync_safe_lock_info = 237" in PROTOCOL
    assert "struct S2C_SYNC_SAFE_LOCK_INFO" in PROTOCOL
    assert "DWORD dwMask;" in PROTOCOL


def test_bank_password_load_save_and_safe_lock_side_effect():
    assert "KPlayer::LoadBankPasswordData(BYTE*" in PLAYER_CPP
    assert "KPlayer::SaveBankPasswordData(size_t*" in PLAYER_CPP
    assert "DoSyncSafeLockInfo(m_nConnIndex, m_dwSafeLockMask)" in PLAYER_CPP
    assert "case rbtBankPasswordData:" in PLAYER_CPP
    assert "SAVE_ROLE_BLOCK(SaveBankPasswordData, rbtBankPasswordData, 0);" in PLAYER_CPP


def test_safe_lock_send_uses_exact_wire_payload():
    assert "KPlayerServer::DoSyncSafeLockInfo(int nConnIndex, unsigned long dwSafeLockMask)" in PLAYER_SERVER_CPP
    assert "Notify.byProtocolID = s2c_sync_safe_lock_info;" in PLAYER_SERVER_CPP
    assert "Notify.dwMask = (DWORD)dwSafeLockMask;" in PLAYER_SERVER_CPP
    assert "return Send(nConnIndex, &Notify, sizeof(Notify));" in PLAYER_SERVER_CPP
