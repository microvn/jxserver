from pathlib import Path


ROOT = Path(__file__).parents[1]
DB_HEADER = ROOT / "include/Include/KRoleDBDataDef.h"
CLASS_HEADER = ROOT / "src/SO3World/Src/KCampActiveStat.h"
CLASS_SOURCE = ROOT / "src/SO3World/Src/KCampActiveStat.cpp"
PLAYER_HEADER = ROOT / "src/SO3World/Src/KPlayer.h"
PLAYER_SOURCE = ROOT / "src/SO3World/Src/KPlayer.cpp"


def test_target_camp_db_layout_is_fixed():
    text = DB_HEADER.read_text(encoding="utf-8")
    assert "struct KCAMP_ACTIVE_STAT_DB" in text
    assert "time_t nRestTime;" in text
    assert "byIsActiveBeforeWeek;" in text
    assert "byIsActive;" in text
    assert "sizeof(KCAMP_ACTIVE_STAT_DB) == 0x10" in text


def test_target_camp_class_layout_and_contract_are_present():
    header = CLASS_HEADER.read_text(encoding="utf-8")
    source = CLASS_SOURCE.read_text(encoding="utf-8")
    assert "sizeof(KCampActiveStat) == 0x18" in header
    assert "BOOL Load(BYTE* pbyData, size_t uDataLen);" in header
    assert "BOOL Save(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize);" in header
    assert "uDataLen == sizeof(KCAMP_ACTIVE_STAT_DB)" in source
    assert "uBufferSize >= sizeof(Data)" in source
    assert "*puUsedSize = sizeof(Data);" in source


def test_role_block_29_is_wired_without_touching_other_services():
    header = PLAYER_HEADER.read_text(encoding="gb18030", errors="replace")
    source = PLAYER_SOURCE.read_text(encoding="gb18030", errors="replace")
    assert '#include "KCampActiveStat.h"' in header
    assert "KCampActiveStat    m_CampActiveStat;" in header
    assert "case rbtCampActiveStat:" in source
    assert "m_CampActiveStat.Load(pbyOffset, pBlock->dwLen)" in source
    assert "m_CampActiveStat.Save, rbtCampActiveStat" in source
    assert "m_CampActiveStat.Init(this)" in source
    assert "m_CampActiveStat.UnInit()" in source
