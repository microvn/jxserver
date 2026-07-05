#!/usr/bin/env python3
"""Extract the exe LUA_ATTRIBUTE_TYPE array (KLuaConst{char* name, DWORD value})
directly from SO3GameServer, no hand transcription. Array @0x084ca1a0, terminated
by {0,0}. Output tools/_lua_attr.tsv (name<TAB>value)."""
import os, struct, re
os.environ.setdefault("GHIDRA_INSTALL_DIR", "/opt/homebrew/Cellar/ghidra/12.1.2/libexec")
os.environ["JAVA_HOME"] = "/opt/homebrew/opt/openjdk@21"
import pyghidra
pyghidra.start()

EXE  = "/Volumes/ExData/game/jx3/镜像端/extracted/root/SO3GameServer"
PROJ = "/Volumes/ExData/game/jx3/linux-build/ghidra-project"
OUT  = "/Volumes/ExData/game/jx3/linux-build/tools/_lua_attr.tsv"
ARR  = 0x084ca1a0

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
