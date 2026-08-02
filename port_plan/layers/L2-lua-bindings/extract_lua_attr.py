#!/usr/bin/env python3
"""Extract the exe LUA_ATTRIBUTE_TYPE array (KLuaConst{char* name, DWORD value})
directly from SO3GameServer, no hand transcription. Array @0x084ca1a0, terminated
by {0,0}. Output measurements/oracle/_lua_attr.tsv (name<TAB>value)."""
import os, struct, re
from pathlib import Path
os.environ.setdefault("GHIDRA_INSTALL_DIR", "/opt/homebrew/Cellar/ghidra/12.1.2/libexec")
os.environ["JAVA_HOME"] = "/opt/homebrew/opt/openjdk@21"
import pyghidra
pyghidra.start()

ROOT = Path(os.environ.get("JX3_BUILD_ROOT", Path(__file__).resolve().parents[3]))
EXE = os.environ.get("TARGET_BINARY")
PROJ = os.environ.get("GHIDRA_PROJECT")
OUT = Path(__file__).resolve().parent / "evidence/_lua_attr.tsv"
ARR = int(os.environ.get("LUA_ATTRIBUTE_ARRAY", "0x084ca1a0"), 0)
if not EXE or not PROJ:
    raise SystemExit("set TARGET_BINARY and GHIDRA_PROJECT before extracting Lua attributes")
OUT.parent.mkdir(parents=True, exist_ok=True)

with pyghidra.open_program(EXE, analyze=False, project_location=PROJ, project_name="jx3lua") as flat:
    prog = flat.getCurrentProgram(); mem = prog.getMemory()
    space = prog.getAddressFactory().getDefaultAddressSpace()
    def rd(va, n):
        a = space.getAddress(va); out = bytearray()
        for _ in range(n):
            out.append(mem.getByte(a) & 0xff); a = a.add(1)
        return bytes(out)
    def cstr(va):
        a = space.getAddress(va); out = bytearray()
        for _ in range(256):
            b = mem.getByte(a) & 0xff
            if b == 0: break
            out.append(b); a = a.add(1)
        return out.decode("latin-1")
    entries = []
    va = ARR
    for _ in range(600):
        rec = rd(va, 8)
        name_ptr, value = struct.unpack("<II", rec)
        if name_ptr == 0 and value == 0: break
        name = cstr(name_ptr)
        assert re.match(r'^[A-Z0-9_]+$', name), (hex(va), name, value)
        entries.append((name, value)); va += 8
    with open(OUT, "w") as f:
        for n, v in entries:
            f.write(f"{n}\t{v}\n")
    print("extracted", len(entries), "entries ->", OUT)
    print("sample:", entries[:3], "...", entries[-2:])
