from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
HEADER = (ROOT / "src/SO3World/Src/KPlayer.h").read_text(encoding="utf-8", errors="ignore")
SOURCE = (ROOT / "src/SO3World/Src/KPlayer.cpp").read_bytes().decode("gb18030")


def test_target_pendent_state_shape_is_declared():
    assert "typedef std::vector<KPendent> KPendentVec;" in HEADER
    assert "int                 m_nWaistPendentBoxSize;" in HEADER
    assert "int                 m_nBackPendentBoxSize;" in HEADER
    assert "int                 m_nFacePendentBoxSize;" in HEADER
    assert "KPendentVec         m_WaistPendent;" in HEADER
    assert "KPendentVec         m_BackPendent;" in HEADER
    assert "KPendentVec         m_FacePendent;" in HEADER


def test_pendent_save_wire_order_is_explicit():
    save = SOURCE[SOURCE.index("BOOL KPlayer::SavePendentData"):]
    order = [
        "SAVE_PENDENT_VECTOR(m_nWaistPendentBoxSize, m_WaistPendent, m_dwWaistItemIndex);",
        "SAVE_PENDENT_VECTOR(m_nBackPendentBoxSize, m_BackPendent, m_dwBackItemIndex);",
        "SAVE_PENDENT_VECTOR(m_nFacePendentBoxSize, m_FacePendent, m_dwFaceItemIndex);",
        "*puUsedSize = uBufferSize - uLeftSize;",
    ]
    positions = [save.index(item) for item in order]
    assert positions == sorted(positions)
    assert "*(WORD*)pbyOffset = (WORD)(BoxSize)" in save
    assert "*(WORD*)pbyOffset = (WORD)uCount" in save
    assert "uCount * sizeof(KPendent)" in save


def test_pendent_load_dispatch_and_expiry_route_exist():
    assert "case rbtPendentData:" in SOURCE
    assert "LoadPendentData(pbyOffset, pBlock->dwLen, (int)pBlock->dwVer)" in SOURCE
    assert "if (nVersion == 0)" in SOURCE
    assert "else if (nVersion == 1 || nVersion == 2)" in SOURCE
    assert "ProcessTimeLimitPendent(m_WaistPendent" in SOURCE
    assert "ProcessTimeLimitPendent(m_BackPendent" in SOURCE
    assert "ProcessTimeLimitPendent(m_FacePendent" in SOURCE


def test_add_pendent_uses_target_type_capacity_and_duplicate_route():
    start = SOURCE.index("BOOL KPlayer::AddPendent")
    end = SOURCE.index("BOOL KPlayer::LoadPendentDataV2", start)
    add = SOURCE[start:end]
    assert "IsPendentExist(dwItemIndex)" in add
    assert "GetDuplicateItemID(dwItemIndex)" in add
    assert "pItemInfo->nDetail == 0x0b" in add
    assert "pItemInfo->nDetail == 0x0e" in add
    assert "pItemInfo->nDetail == 0x11" in add
    assert "m_WaistPendent.push_back(pendent)" in add
    assert "m_BackPendent.push_back(pendent)" in add
    assert "m_FacePendent.push_back(pendent)" in add
    assert '"OnAddPendent"' in add


def test_legacy_v0_uses_old_pendent_owner_and_custom_data_cursor():
    start = SOURCE.index("BOOL KPlayer::LoadPendentDataV0")
    end = SOURCE.index("BOOL KPlayer::LoadPendentData(", start)
    v0 = SOURCE[start:end]
    assert "KCustomData<32> customData;" in v0
    assert "GetOldWaistPendentItemID" in v0
    assert "GetOldBackPendentItemID" in v0
    assert "GetOldFacePendentItemID" in v0
    assert "customData.GetBit(i, &bSet)" in v0
    assert "AddPendent(dwItemIndex, 0, false)" in v0
