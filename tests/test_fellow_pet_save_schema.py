from pathlib import Path


ROOT = Path(__file__).parents[1]
SRC = ROOT / "src/SO3World/Src"


def read_source(path):
    return path.read_bytes().decode("gb18030", errors="replace")


HEADER = read_source(SRC / "KFellowPetBox.h")
SOURCE = read_source(SRC / "KFellowPetBox.cpp")


def test_fellow_pet_layout_and_map_key():
    assert "KPlayer* m_pPlayer" in HEADER
    assert "KCustomData<9> m_PetData" in HEADER
    assert "KPET_TIME_LIMIT_INFO_MAP m_PetTimeLimitInfoMap" in HEADER
    assert "KITEM_EXIST_TYPE eExistType" in HEADER
    assert "time_t nGenTime" in HEADER
    assert "time_t nMaxExistTime" in HEADER


def test_fellow_pet_save_wire_widths_and_lifecycle():
    assert "void KFellowPetBox::UnInit" in SOURCE
    assert "uBufferSize > 5" in SOURCE
    assert "pbyOffset += 6" in SOURCE
    assert "*(WORD*)(pbyBuffer + 4)" in SOURCE
    assert "pbyOffset[2]" in SOURCE
    assert "pbyOffset + 3" in SOURCE
    assert "pbyOffset + 7" in SOURCE
    assert "pbyOffset += 11" in SOURCE


def test_kplayer_owns_and_wires_fellow_pet_box():
    header = read_source(SRC / "KPlayer.h")
    source = read_source(SRC / "KPlayer.cpp")
    assert '#include "KFellowPetBox.h"' in header
    assert "KFellowPetBox       m_FellowPetBox" in header
    assert "m_FellowPetBox.Init(this)" in source
    assert "m_FellowPetBox.UnInit()" in source
    assert "m_FellowPetBox.Save" in source
