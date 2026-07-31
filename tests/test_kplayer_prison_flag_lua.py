import re
from pathlib import Path


ROOT = Path(__file__).parents[1]


def read_source(relative_path):
    return (ROOT / relative_path).read_text(encoding="utf-8", errors="replace")


def test_kplayer_prison_flag_has_target_lua_accessor_and_storage():
    header = read_source("src/SO3World/Src/KPlayer.h")

    assert re.search(r"\bBOOL\s+m_bPrisonFlag\s*;", header)
    assert "DECLARE_LUA_BOOL(PrisonFlag);" in header


def test_kplayer_prison_flag_is_registered_as_b_property():
    source = read_source("src/SO3World/Src/KLuaPlayer.cpp")

    assert re.search(
        r"REGISTER_LUA_BOOL\(KPlayer,\s*PrisonFlag\)",
        source,
    )
