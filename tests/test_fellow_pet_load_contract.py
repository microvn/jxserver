from pathlib import Path


ROOT = Path(__file__).parents[1]
SRC = ROOT / "src/SO3World/Src"


def read_gb18030(path):
    return path.read_bytes().decode("gb18030", errors="strict")


def test_v1_uses_target_world_and_elapsed_save_clock_pair():
    source = read_gb18030(SRC / "KFellowPetBox.cpp")

    assert "g_pSO3World->m_nCurrentTime" in source
    assert "nTimeNow - m_pPlayer->m_nLastSaveTime" in source
    assert "LoadTimeLimitInfo((const KPET_TIME_LIMIT_INFO_DB*)pbyOffset, nTimeNow, nWorldTime)" in source
    assert "LoadTimeLimitInfo((const KPET_TIME_LIMIT_INFO_DB*)pbyOffset, nTimeNow, nTimeNow)" not in source


def test_role_block_dispatch_forwards_exact_fellow_version_and_length():
    source = read_gb18030(SRC / "KPlayer.cpp")

    assert "case rbtFellowPetData:" in source
    assert "m_FellowPetBox.Load(pbyOffset, pBlock->dwLen, pBlock->dwVer)" in source
