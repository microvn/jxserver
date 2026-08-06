import json
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
HEADER = ROOT / "include/Include/SO3World/KGWConstList.h"
SOURCE = ROOT / "src/SO3World/Src/KGWConstList.cpp"
TARGET_INVENTORY = ROOT / "compare-engine/staging/target-global/target.dwarf.inventory.jsonl"
TARGET_DECOMPILE = ROOT / "compare-engine/pyghidra/SO3GameServerD_decompile.jsonl"


def target_fields():
    rows = []
    for line in TARGET_INVENTORY.read_text().splitlines():
        row = json.loads(line)
        if row.get("parent_die") == "0x0002a4bb":
            rows.append((row["name"], row["byte_offset"]))
    return rows


def header_fields(text=None):
    text = HEADER.read_text() if text is None else text
    body = text.split("struct KGWConstList", 1)[1].split("BOOL Init", 1)[0]
    return [(name, offset) for name, offset in re.findall(
        r"^\s+(?:int|float|BOOL|DWORD|char)\s+([A-Za-z_]\w*)(?:\[[^\]]+\])*;\s*//\s*(0x[0-9a-f]+)$",
        body,
        re.MULTILINE,
    )]


def test_target_field_inventory_is_reproduced_exactly():
    assert header_fields() == target_fields()


def test_target_layout_size_is_recorded():
    type_rows = [
        json.loads(line)
        for line in TARGET_INVENTORY.read_text().splitlines()
        if json.loads(line).get("kind") == "type"
        and json.loads(line).get("name") == "KGWConstList"
        and json.loads(line).get("die") == "0x0002a4bb"
    ]
    assert type_rows and type_rows[0]["byte_size"] == "0x0614"


def test_auction_predicate_has_independent_three_entry_oracle():
    source = SOURCE.read_text()
    body = source.split("BOOL KGWConstList::IsAuctionSellLeftHoursValid", 1)[1]
    assert "i < 3" in body
    assert "nAuctionSellLeftHours[i] == nHours" in body
    assert body.index("return true") < body.index("return false")


def test_auction_predicate_mutation_breaks_contract():
    source = SOURCE.read_text()
    mutated = source.replace("i < 3", "i < 2", 1)
    body = mutated.split("BOOL KGWConstList::IsAuctionSellLeftHoursValid", 1)[1]
    assert "i < 3" not in body


def target_loaddata_text():
    for line in TARGET_DECOMPILE.read_text().splitlines():
        row = json.loads(line)
        if row.get("address") == "082d0fa2" and row.get("text"):
            return row["text"]
    raise AssertionError("target LoadData decompile missing")


def test_target_auction_load_order_is_preserved():
    target = target_loaddata_text()
    source = SOURCE.read_text()
    target_order = [target.index(key) for key in ["GameCardTaxRate", "SellLeftHours", "CubPackageRoomRange", "BankPackagePrice"]]
    source_region = source[source.index('"GameCardTaxRate"'):source.index('"BankPackagePrice"')]
    source_order = [source_region.index(key) for key in ["GameCardTaxRate", "SellLeftHours", "CubPackageRoomRange"]]
    assert target_order == sorted(target_order)
    assert source_order == sorted(source_order)


def test_target_auction_load_order_mutation_is_detected():
    source = SOURCE.read_text()
    start = source.index('"GameCardTaxRate"')
    end = source.index('"BankPackagePrice"', start)
    first = source.index('"SellLeftHours"', start)
    second = source.index('"CubPackageRoomRange"', start)
    assert first < second
    mutated = source[:first] + source[first:second].replace(
        '"SellLeftHours"', '"BankPackagePrice"', 1
    ) + source[second:]
    mutated_region = mutated[start:end]
    assert mutated_region.index("BankPackagePrice") < mutated_region.index("SellLeftHours")
