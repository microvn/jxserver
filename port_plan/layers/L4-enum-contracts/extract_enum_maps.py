#!/usr/bin/env python3
"""Extract the 3 EnumConvertor-owned string->int maps (ATTRIBUTE_TYPE,
REQUIRE_TYPE, KTONG_OPERATION_TYPE) from libSO3EnumConvertorD.so, authoritative
for 2.5.2. Built at load via make_pair("name", value); codegen emits adjacent:
    c7 44 24 08 <imm32>   ; mov [esp+8], value
    c7 44 24 04 <ptr32>   ; mov [esp+4], &"name"
Scan all initialized memory for that pair, resolve the string via memory, group
by name prefix. Output measurements/oracle/_enum_<NAME>.tsv (name<TAB>value sorted by value).
Run with ENUM_CONVERTOR_BINARY and GHIDRA_PROJECT set.
"""
import os, re, struct
from pathlib import Path
os.environ.setdefault("GHIDRA_INSTALL_DIR", "/opt/homebrew/Cellar/ghidra/12.1.2/libexec")
os.environ["JAVA_HOME"] = "/opt/homebrew/opt/openjdk@21"
import pyghidra
pyghidra.start()
from jpype import JArray, JByte

ROOT = Path(os.environ.get("JX3_BUILD_ROOT", Path(__file__).resolve().parents[3]))
SO = os.environ.get("ENUM_CONVERTOR_BINARY")
PROJ = os.environ.get("GHIDRA_PROJECT")
OUT = Path(__file__).resolve().parent / "evidence"
if not SO or not PROJ:
    raise SystemExit("set ENUM_CONVERTOR_BINARY and GHIDRA_PROJECT before extracting enum maps")
OUT.mkdir(parents=True, exist_ok=True)

with pyghidra.open_program(SO, analyze=False, project_location=PROJ, project_name="jx3") as flat:
    prog = flat.getCurrentProgram()
    mem  = prog.getMemory()
    space = prog.getAddressFactory().getDefaultAddressSpace()

    def read_cstring(va):
        try:
            a = space.getAddress(va)
        except Exception:
            return None
        out = bytearray()
        for _ in range(128):
            try: b = mem.getByte(a) & 0xff
            except Exception: return None
            if b == 0: break
            out.append(b); a = a.add(1)
        try: return out.decode("ascii")
        except Exception: return None

    pat = re.compile(rb'\xc7\x44\x24\x08(....)\xc7\x44\x24\x04(....)', re.DOTALL)
    pairs = []
    for blk in mem.getBlocks():
        if not blk.isInitialized(): continue
        n = int(blk.getSize())
        try:
            arr = JArray(JByte)(n)
            mem.getBytes(blk.getStart(), arr)
        except Exception as e:
            print("  block read fail", blk.getName(), e); continue
        raw = bytes((int(x) & 0xff) for x in arr)
        for m in pat.finditer(raw):
            val = struct.unpack("<i", m.group(1))[0]
            ptr = struct.unpack("<I", m.group(2))[0]
            s = read_cstring(ptr)
            if s and re.match(r'^[A-Za-z_][A-Za-z0-9_]*$', s) and len(s) >= 3:
                pairs.append((s, val))

    print("raw make_pair hits:", len(pairs))
    groups = {"ATTRIBUTE_TYPE": {}, "REQUIRE_TYPE": {}, "KTONG_OPERATION_TYPE": {}}
    other = {}
    for s, v in pairs:
        if   s.startswith("at"):  g = "ATTRIBUTE_TYPE"
        elif s.startswith("rqt"): g = "REQUIRE_TYPE"
        elif s.startswith("ktot"): g = "KTONG_OPERATION_TYPE"
        else: other.setdefault(s, v); continue
        groups[g].setdefault(s, v)

    for g, d in groups.items():
        items = sorted(d.items(), key=lambda kv: kv[1])
        with open(OUT / f"_enum_{g}.tsv", "w") as f:
            for name, val in items:
                f.write(f"{name}\t{val}\n")
        vals = [v for _, v in items]
        print(f"{g}: {len(items)} entries, range {min(vals) if vals else '-'}..{max(vals) if vals else '-'}")
    if other:
        sm = sorted(other.items(), key=lambda kv: kv[1])
        print(f"UNGROUPED: {len(other)} sample:", sm[:15])
print("done")
