#!/usr/bin/env python3
"""Probe EXE stripped (SO3GameServer 2.5.2): định vị hàm config-reader qua string-anchor,
decompile để xem chất lượng THẬT (khác .so có DWARF). Nhắm đúng hàm version-drift."""
import os, sys
os.environ.setdefault("GHIDRA_INSTALL_DIR", "/opt/homebrew/Cellar/ghidra/12.1.2/libexec")
os.environ["JAVA_HOME"] = "/opt/homebrew/opt/openjdk@21"
import pyghidra
pyghidra.start()
from ghidra.app.decompiler import DecompInterface
from ghidra.util.task import ConsoleTaskMonitor

exe = "/Volumes/ExData/game/jx3/镜像端/extracted/root/SO3GameServer"
ANCHOR = sys.argv[1] if len(sys.argv) > 1 else "AssistPresentContribution"  # key đặc thù config-reader

PROJ = "/Volumes/ExData/game/jx3/linux-build/ghidra-project"
os.makedirs(PROJ, exist_ok=True)
with pyghidra.open_program(exe, analyze=True,
                          project_location=PROJ, project_name="jx3") as flat:
    prog = flat.getCurrentProgram()
    fm = prog.getFunctionManager()
    total = fm.getFunctionCount()
    named = sum(1 for f in fm.getFunctions(True)
                if not f.getName().startswith(("FUN_","thunk_")) and not f.isExternal())
    print(f"=== EXE SO3GameServer: {total} hàm, {named} có tên (RTTI/import), {total-named} vô danh (FUN_) ===")

    # tìm string ANCHOR + hàm tham chiếu nó
    refmgr = prog.getReferenceManager()
    listing = prog.getListing()
    hits = []
    for d in listing.getDefinedData(True):
        if not d.hasStringValue():
            continue
        v = d.getValue()
        if v and ANCHOR in str(v):
            addr = d.getAddress()
            for ref in refmgr.getReferencesTo(addr):
                fn = fm.getFunctionContaining(ref.getFromAddress())
                if fn: hits.append(fn)
    hits = list(dict.fromkeys(hits))
    print(f"\n=== string '{ANCHOR}' được tham chiếu bởi {len(hits)} hàm ===")
    for f in hits[:5]:
        print(f"  {f.getName()} @ {f.getEntryPoint()}  ({f.getBody().getNumAddresses()} bytes)")

    if not hits:
        print("không tìm thấy anchor — thử string khác"); sys.exit(0)

    target = hits[0]
    print(f"\n=== DECOMPILE (config-reader 2.5.2): {target.getName()} ===")
    ifc = DecompInterface(); ifc.openProgram(prog)
    res = ifc.decompileFunction(target, 120, ConsoleTaskMonitor())
    if res.decompileCompleted():
        c = res.getDecompiledFunction().getC()
        # in gọn: chỉ phần đọc config (dòng chứa string key / section)
        print(c[:4500])
    else:
        print("FAILED:", res.getErrorMessage())
