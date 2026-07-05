#!/usr/bin/env python3
"""Pilot: chứng minh pyghidra decompile được .so JX3 (DWARF) — chạy trực tiếp, không cần MCP."""
import os, sys
os.environ.setdefault("GHIDRA_INSTALL_DIR", "/opt/homebrew/Cellar/ghidra/12.1.2/libexec")
os.environ["JAVA_HOME"] = "/opt/homebrew/opt/openjdk@21"

import pyghidra
pyghidra.start()

from ghidra.app.decompiler import DecompInterface
from ghidra.util.task import ConsoleTaskMonitor

so = sys.argv[1] if len(sys.argv) > 1 else \
    "/Volumes/ExData/game/jx3/镜像端/extracted/root/libSO3ItemHouseD.so"
want = sys.argv[2] if len(sys.argv) > 2 else None

with pyghidra.open_program(so, analyze=True) as flat:
    prog = flat.getCurrentProgram()
    fm = prog.getFunctionManager()
    funcs = [f for f in fm.getFunctions(True)]
    print(f"=== {os.path.basename(so)}: {len(funcs)} hàm ===")

    # hàm có tên thật (từ DWARF), bỏ thunk/extern
    named = [f for f in funcs if not f.isThunk() and not f.isExternal()
             and not f.getName().startswith("FUN_")]
    print(f"hàm có TÊN THẬT (DWARF): {len(named)} / {len(funcs)}")
    print("--- 20 hàm mẫu ---")
    for f in named[:20]:
        print(f"  {f.getEntryPoint()}  {f.getName()}")

    # chọn hàm để decompile
    target = None
    if want:
        for f in named:
            if want.lower() in f.getName().lower():
                target = f; break
    if not target:
        # chọn 1 hàm cỡ vừa, có tên đẹp
        cands = sorted([f for f in named if 200 < f.getBody().getNumAddresses() < 900],
                       key=lambda f: f.getName())
        target = cands[0] if cands else named[0]

    print(f"\n=== DECOMPILE: {target.getName()} @ {target.getEntryPoint()} ===")
    ifc = DecompInterface(); ifc.openProgram(prog)
    res = ifc.decompileFunction(target, 60, ConsoleTaskMonitor())
    if res.decompileCompleted():
        print(res.getDecompiledFunction().getC())
    else:
        print("decompile FAILED:", res.getErrorMessage())
