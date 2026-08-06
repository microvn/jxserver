import importlib.util
import json
import sys
import hashlib
from collections import Counter
from pathlib import Path


ROOT = Path(__file__).parents[1]
SCRIPT = ROOT / "compare-engine/tools/compare/compare_abi_probe.py"
SPEC = importlib.util.spec_from_file_location("compare_abi_probe", SCRIPT)
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)
PORT_SPEC = importlib.util.spec_from_file_location("port_compare", ROOT / "compare-engine/tools/compare/port_compare.py")
PORT_MODULE = importlib.util.module_from_spec(PORT_SPEC)
sys.modules[PORT_SPEC.name] = PORT_MODULE
PORT_SPEC.loader.exec_module(PORT_MODULE)
TARGET_SPEC = importlib.util.spec_from_file_location("compare_target_source", ROOT / "compare-engine/tools/compare/compare_target_source.py")
TARGET_MODULE = importlib.util.module_from_spec(TARGET_SPEC)
sys.modules[TARGET_SPEC.name] = TARGET_MODULE
TARGET_SPEC.loader.exec_module(TARGET_MODULE)


def inventory(rows):
    return [json.dumps(row) for row in rows]


def test_layout_facts_join_type_and_members():
    facts = MODULE.layout_facts_from_lines(
        inventory(
            [
                {"kind": "type", "die": "0x10", "name": "KWorldSettings", "byte_size": "0x20"},
                {"kind": "field", "parent_die": "0x10", "name": "m_First", "type_ref": "0x21", "byte_offset": "0x0"},
                {"kind": "field", "parent_die": "0x10", "name": "m_Second", "type_ref": "0x22", "byte_offset": "0x1c"},
            ]
        ),
        "KWorldSettings",
    )

    assert facts["sizeof"] == "0x20"
    assert facts["members"]["m_Second"]["offset"] == "0x1c"
    assert facts["members"]["m_First"]["type_ref"] == "0x21"


def test_compare_layout_reports_size_and_offset_drift():
    target = {
        "sizeof": "0x20",
        "members": {"m_First": {"offset": "0x0"}, "m_Second": {"offset": "0x1c"}},
    }
    candidate = {
        "sizeof": "0x24",
        "members": {"m_First": {"offset": "0x4"}, "m_Second": {"offset": "0x20"}},
    }

    result = MODULE.compare_layout(target, candidate)

    assert result["status"] == "DRIFT"
    assert result["sizeof"]["status"] == "DRIFT"
    assert result["members"]["m_First"]["status"] == "DRIFT"


def test_compare_layout_normalizes_zero_padded_hex_offsets():
    result = MODULE.compare_layout(
        {"sizeof": "0x614", "members": {"m_Field": {"offset": "0x5b4"}}},
        {"sizeof": "0x0614", "members": {"m_Field": {"offset": "0x05b4"}}},
    )

    assert result["sizeof"]["status"] == "MATCH"
    assert result["members"]["m_Field"]["offset"]["status"] == "MATCH"
    assert result["members"]["m_Field"]["status"] == "MATCH"


def test_canonical_event_key_normalizes_assert_and_logging_wrappers():
    assert TARGET_MODULE.canonical_event_key("__assert_fail") == "assert"
    assert TARGET_MODULE.canonical_event_key("KGLOG_PROCESS_ERROR") == "log"
    assert TARGET_MODULE.canonical_event_key("_Z11KGLogPrintf14KGLOG_PRIORITYPKcz@plt") == "log"
    assert TARGET_MODULE.source_event_key("KG_COM_RELEASE") == ""


def test_type_shape_compares_by_canonical_shape_not_raw_die_or_hex_spelling():
    result = MODULE.compare_layout(
        {
            "sizeof": "0x4",
            "members": {
                "m": {"offset": "0x0", "type_ref": "0x10", "type_shape": {"kind": "DW_TAG_structure_type", "name": "S", "byte_size": "0x4", "width": "0x4"}},
            },
        },
        {
            "sizeof": "0x04",
            "members": {
                "m": {"offset": "0x00", "type_ref": "0x99", "type_shape": {"kind": "struct", "name": "S", "byte_size": "0x04", "width": "0x04"}},
            },
        },
    )
    assert result["status"] == "MATCH"
    assert result["members"]["m"]["target_type_ref"] != result["members"]["m"]["candidate_type_ref"]


def test_target_call_facts_keep_logging_as_a_normalized_event(tmp_path):
    calls = tmp_path / "calls.jsonl"
    calls.write_text(
        json.dumps({
            "caller_address": "0x10",
            "caller_name": "KGWConstList::Init()",
            "callee_address": "0x20",
            "callee_name": "_Z11KGLogPrintf14KGLOG_PRIORITYPKcz@plt",
            "callsite": "0x30",
            "kind": "direct-call",
        }) + "\n",
        encoding="utf-8",
    )
    symbols = tmp_path / "symbols.jsonl"
    symbols.write_text("", encoding="utf-8")
    facts = TARGET_MODULE.target_facts(calls, symbols, "KGWConstList")
    assert facts["KGWConstList::Init"]["call_order"] == ["log"]


def test_compare_layout_missing_candidate_member_is_unresolved():
    result = MODULE.compare_layout(
        {"sizeof": "0x20", "members": {"m_First": {"offset": "0x0"}}},
        {"sizeof": "0x20", "members": {}},
    )

    assert result["status"] == "UNRESOLVED"
    assert result["members"]["m_First"]["status"] == "UNRESOLVED"


def test_count_compare_distinguishes_presence_from_order():
    result = PORT_MODULE.status_from_counts(Counter(["Init", "UnInit"]), Counter(["UnInit", "Init"]))
    assert result["status"] == "MATCH"


def test_candidate_manifest_validates_allowlist_hashes(tmp_path):
    candidate = tmp_path / "candidate"
    candidate.mkdir()
    source = candidate / "generated.h"
    source.write_text("int x;\n", encoding="utf-8")
    manifest = tmp_path / "candidate-manifest.json"
    manifest.write_text(json.dumps({
        "candidate_tree": str(candidate),
        "allowed_write_paths": ["generated.h"],
        "read_dependencies": [],
        "files": {"generated.h": hashlib.sha256(source.read_bytes()).hexdigest()},
    }), encoding="utf-8")
    result = PORT_MODULE.validate_candidate_manifest(manifest)
    assert result["status"] == "PASS"
    source.write_text("int changed;\n", encoding="utf-8")
    assert PORT_MODULE.validate_candidate_manifest(manifest)["status"] == "TOOLING_BLOCKED"


def test_full_export_recovers_class_method_key_without_call_row():
    row = {
        "name": "Init",
        "qualified_name": "BOOL __thiscall Init(KWorldSettings * this)",
    }
    assert PORT_MODULE._class_method_from_full_row(row, "KWorldSettings") == "KWorldSettings::Init"


def test_presence_is_drift_when_both_origins_are_known_but_differ(tmp_path):
    target_calls = tmp_path / "calls.jsonl"
    target_symbols = tmp_path / "symbols.jsonl"
    target_calls.write_text("{\"kind\":\"binary\"}\n", encoding="utf-8")
    target_symbols.write_text("{\"kind\":\"binary\"}\n", encoding="utf-8")
    target_facts = tmp_path / "target.jsonl"
    target_facts.write_text(json.dumps({"function": "KWorldSettings::Init"}) + "\n", encoding="utf-8")
    source_root = tmp_path / "src"
    source_root.mkdir()
    extractor = tmp_path / "extract.py"
    extractor.write_text("", encoding="utf-8")
    # Exercise the status rule directly; extraction itself is covered by the
    # existing semantic extractor tests.
    assert PORT_MODULE.status_from_counts(Counter(["Init"]), Counter(["Init", "UnInit"]))["status"] == "DRIFT"


def test_compare_readiness_selftest_is_local_and_passes():
    result = PORT_MODULE.run_selftest()
    assert result["status"] == "PASS"
    assert all(result["checks"].values())


def test_source_extractor_includes_header_inline_bodies(tmp_path):
    header = tmp_path / "KExample.h"
    header.write_text("inline void KExample::Load() { m_ready = 1; }\n", encoding="latin-1")
    extractor = tmp_path / "extractor.py"
    extractor.write_text(
        "import re\n"
        "FUNCTION_RE = re.compile(r'(?P<owner>KExample)::(?P<name>Load)\\s*\\([^)]*\\)\\s*\\{')\n"
        "def mask_comments_and_strings(text): return text\n",
        encoding="utf-8",
    )
    facts = TARGET_MODULE.source_facts(tmp_path, extractor, "KExample")
    assert "KExample::Load" in facts
    assert facts["KExample::Load"]["source_file"].endswith("KExample.h")


def test_cached_abi_provenance_detects_source_or_target_change(tmp_path):
    for directory in ("include", "src", "compat"):
        (tmp_path / directory).mkdir()
    header = tmp_path / "include" / "KExample.h"
    header.write_text("struct KExample {};\n", encoding="latin-1")
    target = tmp_path / "target.jsonl"
    target.write_text("target\n", encoding="utf-8")
    expected = MODULE.candidate_provenance(tmp_path, "KExample", "include/KExample.h", "gnu++98", [], target)
    assert MODULE.provenance_diff(expected, expected) == {}
    changed = dict(expected)
    changed["target_input_sha256"] = "stale"
    assert "target_input_sha256" in MODULE.provenance_diff(expected, changed)


def test_mismatch_table_contains_all_rows_without_truncation(tmp_path):
    report = {
        "dimensions": {
            "abi": {"status": "DRIFT", "members": {
                f"m_{index}": {"status": "DRIFT", "left": index, "right": index + 1}
                for index in range(12)
            }},
        },
    }
    output = tmp_path / "mismatch-table.md"
    PORT_MODULE.write_mismatch_table(report, output)
    text = output.read_text(encoding="utf-8")
    assert text.count("ABI_TYPE_DRIFT") == 13  # aggregate row plus 12 member rows
    assert "m_11" in text
