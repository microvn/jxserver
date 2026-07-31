from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
BUILD = Path(__file__).resolve().parents[1]
SET_SOURCE = BUILD / "src/SO3World/Src/KItemInfoList.cpp"
DROP_SOURCE = BUILD / "src/SO3World/Src/KDropCenter.cpp"
DROP_LIST_SOURCE = BUILD / "src/SO3World/Src/KDropList.cpp"
MAP_LIST_HEADER = BUILD / "include/Include/SO3World/KMapListFile.h"
MAP_LIST_SOURCE = BUILD / "src/SO3World/Src/KMapListFile.cpp"
SET_FILE = ROOT / "镜像端/extracted/root/settings/item/Set.tab"
MAP_DROP_FILE = ROOT / "镜像端/extracted/root/settings/MapDrop.tab"


def _header(path: Path):
    return path.read_text(encoding="gbk").splitlines()[0].split("\t")


def test_set_loader_uses_target_attribute_half_and_preserves_optional_blanks():
    source = SET_SOURCE.read_text()
    header = _header(SET_FILE)

    assert "2_1" in header and "2_2" in header
    assert "13_1" in header and "13_2" in header
    assert 'sprintf(szKey, "%d", i + 2)' not in source
    assert 'sprintf(szKey, "%d_1", i + 2)' in source
    assert "GetString" in source
    assert "KGLOG_PROCESS_ERROR" in source


def test_map_drop_ids_resolve_through_target_master_table():
    source = DROP_SOURCE.read_text()
    header = _header(MAP_DROP_FILE)
    rows = MAP_DROP_FILE.read_text(encoding="gbk").splitlines()[1:]

    assert header[:5] == ["ID", "MapDrop1", "DropType1", "MapDrop2", "DropType2"]
    ids = {int(row.split("\t")[0]) for row in rows if row.split("\t")[0].isdigit()}
    assert set(range(1, 13)).issubset(ids)
    assert 13 not in ids
    assert '"MapDrop.tab"' in source
    assert '"MapDrop%d"' in source
    assert "m_mapMapDropID2MapDropTabItem" in source
    assert "m_mapMapDropID2DropList[nDropIndex]" in source
    assert "DropType%d" in source
    assert "DropCounts[8]" in source
    assert "KLevelDropList" in source
    # Target AddLevelDroplistToMap receives the MapDrop master-row ID as the
    # upper key, while DropType is stored as metadata on that row.
    assert "((uint64_t)it->first << 32)" in source
    assert "((uint64_t)it->second.dwMapDropTypeMask[nDropIndex] << 32)" not in source
    assert "LevelDropList.Init" in source
    map_drop_body = source.split("BOOL KDropCenter::MapDropInit", 1)[1].split("BOOL KDropCenter::DoodadTemplateDropInit", 1)[0]
    assert "IndividualDropList.Init(szFileName)" not in map_drop_body
    assert "DROP_LIST_DIR" in source
    assert '"%s/%s/%s", SETTING_DIR, DROP_LIST_DIR, pMapParams->szDropName' not in source


def test_map_drop_missing_file_remains_a_hard_load_failure():
    source = DROP_SOURCE.read_text()

    assert "if (!bRetCode) continue" not in source
    assert "KGLOG_PROCESS_ERROR(bRetCode)" in source


def test_map_drop_does_not_copy_owning_drop_list_into_the_map():
    source = DROP_SOURCE.read_text()
    body = source.split("BOOL KDropCenter::MapDropInit", 1)[1].split(
        "BOOL KDropCenter::DoodadTemplateDropInit", 1
    )[0]

    # KDropList owns m_ItemList and has no copy constructor.  Copying a local
    # initialized list into std::map leaves a dangling pointer after scope exit
    # and reproduces the observed double-free after skill loading.
    assert "std::make_pair(it->first, DropList)" not in body
    assert "m_mapMapID2DropList.insert(std::make_pair(it->first, DropList))" not in body
    assert "MAP_INT64_2_DROP_LIST::iterator" in body
    assert "InsRet.first->second.Init(it->second)" in body


def test_drop_list_accepts_target_exact_probability_total():
    source = DROP_LIST_SOURCE.read_text()

    assert "nCurrentDropRate <= MILLION_NUM" in source
    assert "nCurrentDropRate < MILLION_NUM" not in source


def test_map_drop_consumer_matches_target_route_and_signature():
    header = (BUILD / "include/Include/SO3World/KDropCenter.h").read_text()
    source = DROP_SOURCE.read_text()
    doodad = (BUILD / "src/SO3World/Src/KDoodad.cpp").read_text(encoding="gbk")

    # Regression: target NpcMapDrop routes map -> MapDropID -> slot -> level;
    # the old candidate only looked up the removed map-ID table.
    assert "DWORD dwMapID, DWORD dwDropIndex, int nLevel, DWORD dwMapDropType" in " ".join(header.split())
    assert "m_mapMapID2DropList" not in header
    assert "m_mapMapDropID2MapDropTabItem.find" in source
    assert "m_mapMapDropID2DropList[dwDropIndex].find" in source
    assert "MAKE_INT64(dwMapDropID, nLevel)" in source
    assert "NpcMapDrop(pLootList, vecLooterList, m_pScene->m_dwMapID" in doodad


def test_map_drop_uninit_clears_target_maps():
    source = DROP_SOURCE.read_text()
    uninit = source.split("BOOL KDropCenter::UnInit", 1)[1].split(
        "BOOL KDropCenter::NpcMapDrop", 1
    )[0]

    # Regression: reload/shutdown must not retain target MapDrop state.
    assert "m_mapMapDropID2MapDropTabItem.clear()" in uninit
    assert "m_mapMapDropID2DropList[nDropIndex].clear()" in uninit


def test_map_list_exposes_target_map_drop_id_at_target_offset():
    header = MAP_LIST_HEADER.read_text()
    source = MAP_LIST_SOURCE.read_text()

    # Target DWARF: KMapParams.dwMapDropID is a DWORD at 0x5c; MapList.tab's
    # MapDrop column is that master-row ID, not a drop-list filename.
    assert "dwMapDropID" in header
    assert "offsetof(KMapParams, dwMapDropID) == 0x5C" in header
    assert 'GetInteger(nRowIndex, "MapDrop", 0' in source
    assert "szDropName" not in header
