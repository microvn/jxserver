from pathlib import Path


ROOT = Path(__file__).parents[1]
SRC = ROOT / "src/SO3World/Src"


def test_manager_matches_target_shape_and_lookup_contract():
    text = (SRC / "KNewExtPointManager.h").read_text()
    assert "struct KNewExtPointInfo" in text
    assert "int  nValue;" in text
    assert "BOOL bLocked;" in text
    assert "std::map<int, KNewExtPointInfo> m_NEPMap;" in text
    assert "BOOL AddNewExtPoint(int nKey, int nValue, int bLocked)" in text
    assert "BOOL GetNewExtPoint(int nKey, int* pnValue) const" in text


def test_relay_response_populates_manager_before_login_gate():
    text = (SRC / "KRelayClient.cpp").read_text(errors="ignore")
    add = text.index("m_NewExtPointManager.AddNewExtPoint")
    finish = text.index("if (!pRespond->bySyncFinish)", add)
    assert "SyncNEPInfo[i].nKey" in text[add:finish]
    assert "SyncNEPInfo[i].nValue" in text[add:finish]


def test_get_ext_point_uses_legacy_then_new_manager():
    header = (SRC / "KPlayer.h").read_text(errors="ignore")
    source = (SRC / "KPlayer.cpp").read_text(errors="ignore")
    assert "KNewExtPointManager m_NewExtPointManager" in header
    assert "BOOL GetExtPoint(int nIndex, int& nValue);" in header
    body = source[source.index("BOOL KPlayer::GetExtPoint"):source.index("BOOL KPlayer::SetExtPoint")]
    assert "nIndex < MAX_EXT_POINT_COUNT" in body
    assert "m_NewExtPointManager.GetNewExtPoint" in body


def test_optional_sync_order_matches_target_frontier():
    source = (SRC / "KPlayer.cpp").read_text(errors="ignore")
    body = source[source.index("BOOL KPlayer::FinishRoleDataLoad"):]
    assert body.index("DoSyncCoin(this)") < body.index("DoSyncRewards(this)")
    assert body.index("DoSyncRewards(this)") < body.index("DoSyncPendentData(this)")
    assert body.index("DoSyncPendentData(this)") < body.index("DoSyncFellowPetData(this)")
