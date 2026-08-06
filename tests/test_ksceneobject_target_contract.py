#!/usr/bin/env python3
"""Target-derived static contracts for standalone KSceneObject."""
from pathlib import Path
import re
import unittest


ROOT = Path(__file__).resolve().parents[1]
CORE = ROOT / "src/SO3World/Src/KSceneObject.cpp"
HEADER = ROOT / "src/SO3World/Src/KSceneObject.h"
LUA = ROOT / "src/SO3World/Src/KLuaSceneObject.cpp"


def body(text, signature):
    start = text.index(signature)
    brace = text.index("{", start)
    depth = 0
    for index in range(brace, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[brace + 1:index]
    raise AssertionError("unbalanced body: %s" % signature)


def ordered(text, fragments):
    positions = [text.index(fragment) for fragment in fragments]
    if positions != sorted(positions):
        raise AssertionError("out of target order: %r" % fragments)


class KSceneObjectTargetContract(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.core = CORE.read_text(encoding="latin-1")
        cls.header = HEADER.read_text(encoding="latin-1")
        cls.lua = LUA.read_text(encoding="latin-1")

    def test_init_target_state_order_and_constants(self):
        init = body(self.core, "BOOL KSceneObject::Init()")
        fragments = [
            "m_nX                = 0;", "m_nY                = 0;", "m_nZ                = 0;",
            "m_nTouchRange       = 0;", "m_nXCell            = 0;", "m_nYCell            = 0;",
            "m_nMass             = DEFAULT_MASS;", "m_nWidth            = 1;",
            "m_nLength           = 1;", "m_nHeight           = 1;", "m_nGameLoop         = 0;",
            "m_nFaceDirection    = 0;", "m_nPitchDirection   = 0;", "m_pCell             = NULL;",
            "m_pRegion           = NULL;", "m_pScene            = NULL;", "return true;",
        ]
        ordered(init, fragments)
        self.assertIn("#define DEFAULT_MASS\t\t\t128", (ROOT / "include/Include/SO3World/Global.h").read_text(encoding="latin-1"))
        broken = init.replace("m_nMass             = DEFAULT_MASS;", "m_nMass             = 0;")
        with self.assertRaises(ValueError):
            ordered(broken, fragments)

    def test_ctor_uninit_loop_and_server_setcell_contract(self):
        ctor = body(self.core, "KSceneObject::KSceneObject()")
        ordered(ctor, ["m_RegionObjNode.m_pSceneObject = this;", "m_pCell = NULL;", "m_pRegion = NULL;", "m_pScene = NULL;"])
        self.assertEqual(body(self.core, "void KSceneObject::UnInit()").strip(), "")
        loop = body(self.core, "BOOL KSceneObject::CheckGameLoop(void)")
        loop_fragments = ["m_nGameLoop < g_pSO3World->m_nGameLoop", "m_nGameLoop = g_pSO3World->m_nGameLoop;", "return true;", "return false;"]
        ordered(loop, loop_fragments)
        setcell = body(self.core, "void KSceneObject::SetCell(KCell* pDstCell)")
        server_prefix = setcell.split("#ifdef _CLIENT", 1)[0]
        self.assertIn("m_pCell = pDstCell;", server_prefix)
        self.assertNotIn("OnUpdateAllRelation", server_prefix)
        broken = loop.replace("m_nGameLoop = g_pSO3World->m_nGameLoop;", "m_nGameLoop = 0;")
        with self.assertRaises(ValueError):
            ordered(broken, loop_fragments)

    def test_coordinate_lua_and_range_order(self):
        coords = body(self.core, "BOOL KSceneObject::GetAbsoluteCoordinate")
        ordered(coords, ["*pnX = m_nX;", "*pnY = m_nY;", "*pnZ = m_nZ;", "return true;"])
        lua_coords = body(self.lua, "int KSceneObject::LuaGetAbsoluteCoordinate")
        lua_fragments = ["Lua_PushNumber(L, m_nX);", "Lua_PushNumber(L, m_nY);", "Lua_PushNumber(L, m_nZ);", "nResult = 3;"]
        ordered(lua_coords, lua_fragments)
        lua_cell = body(self.lua, "int KSceneObject::LuaGetCell")
        ordered(lua_cell, ["if (m_pCell)", "m_pCell->LuaGetObj(L);", "return 1;", "return 0;"])
        range_body = body(self.header, "inline BOOL g_InRange(KSceneObject* pObjA, KSceneObject* pObjB, int nRange)")
        ordered(range_body, ["assert(pObjA);", "assert(pObjB);", "pObjA->GetAbsoluteCoordinate", "pObjB->GetAbsoluteCoordinate", "ZPOINT_TO_XYPOINT(nZa)", "ZPOINT_TO_XYPOINT(nZb)", "nRange)", "bResult = true;"])
        self.assertIn("#define ZPOINT_TO_XYPOINT(Z)            ((Z) / 8)", (ROOT / "include/Include/SO3GlobalDef.h").read_text(encoding="latin-1"))
        broken = lua_coords.replace("Lua_PushNumber(L, m_nY);", "Lua_PushNumber(L, m_nZ);")
        with self.assertRaises(ValueError):
            ordered(broken, lua_fragments)

    def test_target_layout_declaration_order(self):
        declaration = self.header[self.header.index("class KSceneObject"):self.header.index("inline BOOL g_InRange")]
        ordered(declaration, [
            "m_nX;", "m_nY;", "m_nZ;", "m_nTouchRange;", "m_nXCell;", "m_nYCell;",
            "m_nMass;", "m_nWidth;", "m_nLength;", "m_nHeight;", "m_nFaceDirection;",
            "m_nPitchDirection;", "m_nGameLoop;", "m_pCell;", "m_pRegion;", "m_pScene;",
            "m_RegionObjNode;",
        ])
        self.assertRegex(declaration, re.compile(r"virtual\s+void\s+SetCell\(KCell\* pDstCell\);"))

    def test_target_weak_accessor_macro_binds_direct_fields(self):
        luna = (ROOT / "include/Include/Luna.h").read_text(encoding="latin-1")
        def macro_contract(candidate):
            self.assertIn("int get##__VAR_NAME__(void) {return m_n##__VAR_NAME__;};", candidate)
            self.assertIn("void set##__VAR_NAME__(int nValue)\t{m_n##__VAR_NAME__ = nValue;}", candidate)
        macro_contract(luna)
        for field in ("X", "Y", "Z", "TouchRange", "XCell", "YCell", "Mass", "Width", "Length", "Height", "FaceDirection", "PitchDirection"):
            self.assertIn("DECLARE_LUA_INTEGER(%s);" % field, self.header)
        broken = luna.replace("m_n##__VAR_NAME__ = nValue;", "m_n##__VAR_NAME__ = 0;")
        with self.assertRaises(AssertionError):
            macro_contract(broken)


if __name__ == "__main__":
    unittest.main()
