import re
from pathlib import Path


ROOT = Path(__file__).parents[1]


def read_source(relative_path):
    return (ROOT / relative_path).read_text(encoding="utf-8", errors="replace")


def test_activity_state_lua_constant_matches_target():
    source = read_source("src/SO3World/Src/KLuaConstList.cpp")

    assert "KLuaConst LUA_ACTIVITY_STATE[]" in source
    assert re.search(r'\{"NORMAL_OFF",\s*0\}', source)
    assert re.search(r'\{"DELAY_OFF",\s*1\}', source)
    assert re.search(r'\{"NORMAL_ON",\s*2\}', source)
    assert re.search(r'\{"RECOVER_ON",\s*3\}', source)
    assert re.search(r'\{"ACTIVITY_STATE",\s*LUA_ACTIVITY_STATE\}', source)


def test_copy_index_surface_is_registered_and_target_shaped():
    base_funcs = read_source("src/SO3World/Src/KBaseFuncList.cpp")
    world_header = read_source("include/Include/SO3World/KSO3World.h")
    world_source = read_source("src/SO3World/Src/KSO3World.cpp")

    assert "LuaGetAllCopyIndexByMapID" in base_funcs
    assert re.search(
        r'\{"GetAllCopyIndexByMapID",\s*LuaGetAllCopyIndexByMapID\}',
        base_funcs,
    )
    assert "GetAllCopyIndexByMapID" in world_header
    assert "m_SceneSet.Traverse" in world_source
    assert "m_dwMapID" in world_source
    assert "m_nCopyIndex" in world_source


def test_copy_index_empty_and_error_paths_return_lua_nil():
    source = read_source("src/SO3World/Src/KBaseFuncList.cpp")
    function = re.search(
        r"int LuaGetAllCopyIndexByMapID\(Lua_State\* L\)(.*?)(?=\n\s*/\*|\n\s*int |\Z)",
        source,
        re.DOTALL,
    )

    assert function, "LuaGetAllCopyIndexByMapID definition is missing"
    body = function.group(1)
    assert "Lua_PushNil(L);" in body
    assert "return 1;" in body
    assert body.index("Lua_PushNil(L);") < body.index("return 1;")
