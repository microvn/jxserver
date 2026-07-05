#!/usr/bin/env python3
"""RE SO3GameCenter handshake reject (:736). Scan raw memory for KGLOG __PRETTY_FUNCTION__
strings, find referencing funcs, decompile. Reopen analyzed project (analyze=False)."""
import os
os.environ.setdefault("GHIDRA_INSTALL_DIR", "/opt/homebrew/Cellar/ghidra/12.1.2/libexec")
os.environ["JAVA_HOME"] = "/opt/homebrew/opt/openjdk@21"
import pyghidra
pyghidra.start()

EXE  = "/Volumes/ExData/game/jx3/镜像端/extracted/root/SO3GameCenter"
PROJ = "/private/tmp/claude-501/-Volumes-ExData-game-jx3/8c29d561-e7a2-4447-8a95-6903d68bd924/scratchpad/ghidra-center-re"

with pyghidra.open_program(EXE, analyze=False, project_location=PROJ, project_name="jx3center_re") as flat:
    from ghidra.app.decompiler import DecompInterface
    from ghidra.util.task import ConsoleTaskMonitor
    prog = flat.getCurrentProgram()
    fm = prog.getFunctionManager()
    mem = prog.getMemory()
    ref = prog.getReferenceManager()
    st  = prog.getSymbolTable()
    print("functions:", fm.getFunctionCount())

    def find_all(s):
        pat = s.encode("latin-1")
        out = []
        addr = mem.findBytes(prog.getMinAddress(), pat, None, True, ConsoleTaskMonitor())
        while addr is not None:
            out.append(addr)
            nxt = addr.add(1)
            addr = mem.findBytes(nxt, pat, None, True, ConsoleTaskMonitor())
            if len(out) > 20: break
        return out

    deci = DecompInterface(); deci.openProgram(prog)
    mon = ConsoleTaskMonitor()

    def refs_to(addr):
        fns = set()
        for r in ref.getReferencesTo(addr):
            f = fm.getFunctionContaining(r.getFromAddress())
            if f: fns.add(f)
        return fns

    def dump_refs(label):
        addrs = find_all(label)
        print(f"\n===== string '{label}': found at {[str(a) for a in addrs]} =====")
        seen = set()
        for a in addrs:
            for f in refs_to(a):
                if f.getEntryPoint() in seen: continue
                seen.add(f.getEntryPoint())
                res = deci.decompileFunction(f, 90, mon)
                if res and res.getDecompiledFunction():
                    print(f"\n######## {f.getName()} @ {f.getEntryPoint()} ########")
                    print(res.getDecompiledFunction().getC()[:8000])

    for lbl in ("void KGameServer::ProcessNetwork()", "KGameServer::OnHandshakeRequest",
                "KGameServer::DoHandshakeRespond"):
        dump_refs(lbl)
