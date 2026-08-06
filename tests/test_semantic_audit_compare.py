import json
import subprocess
import sys
from pathlib import Path


SCRIPT = Path(__file__).parents[1] / "compare-engine/tools/compare/semantic_audit_compare.py"


def bundle(origin, *, events=None, abi=None, cfg=None):
    return {
        "schema": "jx3-audit-bundle.v1",
        "origin": origin,
        "provenance": {"origin": origin, "sha256": f"{origin}-sha"},
        "scope": {"dimensions": ["identity", "ordered_events", "cfg", "abi"]},
        "dimensions": {
            "identity": {"qualified_name": "KWorldSettings::UnInit"},
            "ordered_events": events or ["A", "B", "C"],
            "cfg": cfg or {"paths": ["entry", "success", "return"]},
            "abi": abi or {"sizeof": "0x10", "members": {"m_x": "0x0"}},
        },
    }


def run_compare(tmp_path, left, right):
    left_path = tmp_path / "left.json"
    right_path = tmp_path / "right.json"
    out_path = tmp_path / "diff.json"
    left_path.write_text(json.dumps(left), encoding="utf-8")
    right_path.write_text(json.dumps(right), encoding="utf-8")
    result = subprocess.run(
        [sys.executable, str(SCRIPT), str(left_path), str(right_path), "--out-json", str(out_path)],
        text=True,
        capture_output=True,
        check=False,
    )
    assert result.returncode == 0, result.stderr
    return json.loads(out_path.read_text(encoding="utf-8"))


def test_identical_bundles_pass(tmp_path):
    diff = run_compare(tmp_path, bundle("target"), bundle("candidate"))

    assert diff["verdict"] == "PASS"
    assert diff["dimensions"]["ordered_events"]["status"] == "MATCH"
    assert diff["dimensions"]["abi"]["status"] == "MATCH"


def test_order_drift_is_reported_with_first_mismatch(tmp_path):
    diff = run_compare(
        tmp_path,
        bundle("target", events=["A", "B", "C"]),
        bundle("candidate", events=["A", "C", "B"]),
    )

    assert diff["verdict"] == "BLOCKED"
    order = diff["dimensions"]["ordered_events"]
    assert order["status"] == "DRIFT"
    assert order["first_mismatch"]["index"] == 1
    assert order["first_mismatch"]["left"] == "B"
    assert order["first_mismatch"]["right"] == "C"


def test_missing_dimension_is_unresolved_not_pass(tmp_path):
    left = bundle("target")
    right = bundle("candidate")
    del right["dimensions"]["cfg"]

    diff = run_compare(tmp_path, left, right)

    assert diff["verdict"] == "BLOCKED"
    assert diff["dimensions"]["cfg"]["status"] == "UNRESOLVED"


def test_dimension_absent_on_both_sides_is_not_applicable(tmp_path):
    left = bundle("target")
    right = bundle("candidate")
    left["scope"]["dimensions"].append("wire")
    right["scope"]["dimensions"].append("wire")

    diff = run_compare(tmp_path, left, right)

    assert diff["dimensions"]["wire"]["status"] == "UNRESOLVED"


def test_unscoped_wire_is_not_applicable(tmp_path):
    diff = run_compare(tmp_path, bundle("target"), bundle("candidate"))

    assert diff["dimensions"]["wire"]["status"] == "NOT_APPLICABLE"
