#!/usr/bin/env python3
"""Decompile the DECRYPTED center (from runtime memory dump wrapped as ELF).
Find KGameServer::OnHandshakeRequest + ProcessNetwork via their __PRETTY_FUNCTION__
strings (now present after unpack), decompile them to see the 2.5.2-added :736 check."""
import os
os.environ.setdefault("GHIDRA_INSTALL_DIR", "/opt/homebrew/Cellar/ghidra/12.1.2/libexec")
os.environ["JAVA_HOME"] = "/opt/homebrew/opt/openjdk@21"
import pyghidra
pyghidra.start()

ELF="/private/tmp/claude-501/-Volumes-ExData-game-jx3/8c29d561-e7a2-4447-8a95-6903d68bd924/scratchpad/center_decrypted.elf"
PROJ="/private/tmp/claude-501/-Volumes-ExData-game-jx3/8c29d561-e7a2-4447-8a95-6903d68bd924/scratchpad/ghidra-center-dump"
os.makedirs(PROJ, exist_ok=True)

with pyghidra.open_program(ELF, analyze=True, project_location=PROJ, project_name="jx3center_dump") as flat:
    from ghidra.app.decompiler import DecompInterface
    from ghidra.util.task import ConsoleTaskMonitor
    prog=flat.getCurrentProgram(); fm=prog.getFunctionManager(); mem=prog.getMemory(); ref=prog.getReferenceManager()
    print("functions:", fm.getFunctionCount())
    mon=ConsoleTaskMonitor()

    def find_all(s):
        pat=s.encode("latin-1"); out=[]
        a=mem.findBytes(prog.getMinAddress(), pat, None, True, mon)
        while a is not None and len(out)<20:
            out.append(a); a=mem.findBytes(a.add(1), pat, None, True, mon)
        return out

    deci=DecompInterface(); deci.openProgram(prog)
    def refs_funcs(addr):
        s=set()
        for r in ref.getReferencesTo(addr):
            f=fm.getFunctionContaining(r.getFromAddress())
            if f: s.add(f)
        return s
    def dump(label):
        addrs=find_all(label)
        print(f"\n===== '{label}' @ {[str(a) for a in addrs]} =====")
        seen=set()
        for a in addrs:
            for f in refs_funcs(a):
                if f.getEntryPoint() in seen: continue
                seen.add(f.getEntryPoint())
                res=deci.decompileFunction(f,120,mon)
                if res and res.getDecompiledFunction():
                    print(f"\n######## {f.getName()} @ {f.getEntryPoint()} ########")
                    print(res.getDecompiledFunction().getC()[:9000])
    dump("void KGameServer::OnHandshakeRequest(BYTE*, size_t, int)")
    dump("void KGameServer::ProcessNetwork()")
