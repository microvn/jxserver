import importlib.util
from pathlib import Path


ROOT = Path(__file__).parents[1]
SCRIPT = ROOT / "compare-engine/tools/compare/surface_extract.py"
SPEC = importlib.util.spec_from_file_location("surface_extract", SCRIPT)
MODULE = importlib.util.module_from_spec(SPEC)
assert SPEC and SPEC.loader
SPEC.loader.exec_module(MODULE)


def test_source_surface_covers_header_only_special_members_and_gcc_shapes(tmp_path):
    header = tmp_path / "KExample.h"
    header.write_text(
        '#pragma pack(push, 1)\n'
        '#include <stdint.h>\n'
        '#define _SERVER 1\n'
        '#if defined(_SERVER)\n'
        'struct KExample : public IBase {\n'
        '    KExample();\n'
        '    ~KExample();\n'
        '    int Load(const char* name) const;\n'
        '    unsigned char tail[0];\n'
        '};\n'
        '#endif\n',
        encoding="latin-1",
    )
    facts = MODULE.source_surface(tmp_path, "KExample")
    assert facts["packing"]["packed"] is True
    assert "<stdint.h" in facts["includes"]
    assert facts["variants"]
    assert "KExample::KExample" in facts["declarations"]
    assert facts["declarations"]["KExample::Load"]["const"] is True


def test_mask_comments_and_strings_preserves_code_positions_for_semantic_extractor():
    source = (
        '/* KFake::Ignored() { */\n'
        'KExample::Load("not // a comment", \'{\'); // KFake::Ignored()\n'
        'KExample::Save();\n'
    )

    masked = MODULE.mask_comments_and_strings(source)

    assert len(masked) == len(source)
    assert "KFake::Ignored" not in masked
    assert "KExample::Load" in masked
    assert "KExample::Save" in masked


def test_compare_surface_does_not_turn_missing_target_lanes_into_match():
    target = {
        "symbols": [{"demangled": "KExample::Load()"}],
        "special_members": {"target_symbols": ["KExample::~KExample()"]},
        "layout": {"types": [{"name": "KExample", "byte_size": "0x10"}], "inheritance": []},
        "protocol": [],
        "class": "KExample",
    }
    source = {
        "declarations": {"KExample::Load": {"function": "KExample::Load"}},
        "special_members": {"explicit": []},
        "types": {"KExample": {"kind": "struct", "bases": []}},
        "packing": {"packed": False},
        "variants": [],
        "includes": [],
        "macros": {},
        "constants": {"strings": [], "numbers": []},
        "enums": {},
        "protocol": [],
        "class": "KExample",
    }
    result = MODULE.compare_surface(target, source)
    assert result["special_members"]["status"] == "UNRESOLVED"
    assert result["compiler_contract"]["status"] == "UNRESOLVED"
