from pathlib import Path


SOURCE = (Path(__file__).resolve().parents[1] / "src/SO3World/Src/KItemList.cpp").read_bytes().decode("gb18030")
HEADER = (Path(__file__).resolve().parents[1] / "src/SO3World/Src/KItemList.h").read_bytes().decode("gb18030")


def test_versioned_item_list_symbols_and_dispatcher_exist():
    for name in ("LoadItemList", "LoadItemList_V0", "LoadItemList_V1", "LoadItemList_V2", "LoadItemList_V3", "LoadItemList_V4", "LoadItemList_V5", "LoadItemList_V6"):
        assert name in HEADER
        assert f"KItemList::{name}" in SOURCE
    assert "case 0: return LoadItemList_V0" in SOURCE
    assert "case 6: return LoadItemList_V6" in SOURCE


def test_target_v0_v1_addpendent_guard_uses_equip_positions():
    start = SOURCE.index("if (dwBoxIndex == ibEquip)")
    end = SOURCE.index("bRetCode = m_Box[dwBoxIndex].PlaceItem", start)
    guard = SOURCE[start:end]
    assert "nVersion == 0" in guard
    assert "dwX == 0x0c" in guard
    assert "dwX == 0x17" in guard
    assert "nVersion == 1" in guard
    assert "dwX == 0x0d" in guard
    assert "dwX == 0x18" in guard
    assert "m_pPlayer->AddPendent(pItem->m_GenParam.dwIndex, 0, false)" in guard


def test_legacy_pendent_is_consumed_before_inventory_placement():
    start = SOURCE.index("const BOOL bLegacyPendent")
    end = SOURCE.index("bRetCode = m_Box[dwBoxIndex].PlaceItem", start)
    block = SOURCE[start:end]
    assert "m_pPlayer->AddPendent(pItem->m_GenParam.dwIndex, 0, false);" in block
    assert "continue;" in block


def test_v0_v1_equipment_position_remap_is_version_specific():
    start = SOURCE.index("/* Target V0/V1 use different historical equipment numbering. */")
    block = SOURCE[start:SOURCE.index("bRetCode = m_Box[dwBoxIndex].PlaceItem", start)]
    assert "if (nVersion == 0)" in block
    assert "dwX >= 1 && dwX <= 0x0b" in block
    assert "dwX > 0x17" in block
    assert "else if (nVersion == 1)" in block
    assert "dwX >= 0x0d && dwX <= 0x17" in block
    assert "dwX > 0x18" in block
