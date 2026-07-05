#!/usr/bin/env python3
"""Diff KGWConstList::LoadData: exe 2.5.2 (decompiled) vs source 2010.
Ra danh sách (section,key) mỗi bên → key bỏ / thêm / dời-section."""
import os, re, sys
os.environ.setdefault("GHIDRA_INSTALL_DIR", "/opt/homebrew/Cellar/ghidra/12.1.2/libexec")
os.environ["JAVA_HOME"] = "/opt/homebrew/opt/openjdk@21"
import pyghidra
pyghidra.start()
from ghidra.app.decompiler import DecompInterface
from ghidra.util.task import ConsoleTaskMonitor

EXE = "/Volumes/ExData/game/jx3/镜像端/extracted/root/SO3GameServer"
PROJ = "/Volumes/ExData/game/jx3/linux-build/ghidra-project"
SRC = "/Volumes/ExData/game/jx3/linux-build/src/SO3World/Src/KGWConstList.cpp"
SRCH = "/Volumes/ExData/game/jx3/linux-build/src/SO3World/Src/KGWConstList.h"

# ---------- source 2010: (SECTION_MACRO, key) ----------
def section_defines():
    m = {}
    for f in (SRC, SRCH):
        try: t = open(f, encoding="latin-1").read()
        except: continue
        for name, val in re.findall(r'#define\s+(SECTION_[A-Z0-9_]+)\s+"([^"]+)"', t):
            m[name] = val
    return m

def src2010_pairs():
    t = open(SRC, encoding="latin-1").read()
    # chỉ trong hàm LoadData
    i = t.find("::LoadData"); t = t[i:]
    defs = section_defines()
    pairs = []
    for sec, key in re.findall(r'Get(?:Integer|String|Float)\s*\(\s*(SECTION_[A-Z0-9_]+|"[^"]+")\s*,\s*"([^"]+)"', t):
        sec = defs.get(sec, sec.strip('"'))
        pairs.append((sec, key))
    return pairs

# ---------- exe 2.5.2: decompile FUN_082d0fa2, resolve &DAT->string ----------
def exe252_pairs():
    with pyghidra.open_program(EXE, analyze=False, project_location=PROJ, project_name="jx3") as flat:
        prog = flat.getCurrentProgram()
        fm = prog.getFunctionManager()
        af = flat.getAddressFactory()
        listing = prog.getListing()
        # tìm hàm LoadData qua string anchor "AssistPresentContribution"
        target = None
        refmgr = prog.getReferenceManager()
        for d in listing.getDefinedData(True):
            if d.hasStringValue() and "AssistPresentContribution" in str(d.getValue()):
                for ref in refmgr.getReferencesTo(d.getAddress()):
                    fn = fm.getFunctionContaining(ref.getFromAddress())
                    if fn: target = fn; break
            if target: break
        if not target:
            print("!! không tìm thấy LoadData exe"); return []
        ifc = DecompInterface(); ifc.openProgram(prog)
        res = ifc.decompileFunction(target, 120, ConsoleTaskMonitor())
        c = res.getDecompiledFunction().getC()
        open(PROJ + "/LoadData_252.c", "w").write(c)  # lưu full để tham khảo

        def resolve(tok):
            tok = tok.strip()
            if tok.startswith('"'): return tok.strip('"')
            m = re.match(r'&?DAT_([0-9a-f]+)', tok)
            if m:
                try:
                    addr = af.getAddress(m.group(1))
                    dd = listing.getDataContaining(addr)
                    if dd and dd.hasStringValue(): return str(dd.getValue())
                    # đọc thô C-string
                    b = bytearray()
                    a = addr
                    for _ in range(64):
                        v = prog.getMemory().getByte(a) & 0xff
                        if v == 0: break
                        b.append(v); a = a.add(1)
                    return b.decode("latin-1")
                except Exception as e:
                    return tok
            return tok

        pairs = []
        # call vtable: (...)(param_2, SECTION, "KEY", ...)
        for sec_raw, key in re.findall(r'\(param_2,\s*(&?DAT_[0-9a-f]+|"[^"]*")\s*,\s*"([^"]+)"', c):
            pairs.append((resolve(sec_raw), key))
        return pairs

def norm(pairs):  # unique giữ thứ tự
    seen=set(); out=[]
    for p in pairs:
        if p not in seen: seen.add(p); out.append(p)
    return out

s2010 = norm(src2010_pairs())
s252  = norm(exe252_pairs())
set10 = {(s.upper(),k) for s,k in s2010}
set25 = {(s.upper(),k) for s,k in s252}
k10 = {k for s,k in s2010}; k25 = {k for s,k in s252}

print(f"\n===== SOURCE 2010 đọc {len(s2010)} (section,key) =====")
print(f"===== EXE 2.5.2 đọc {len(s252)} (section,key) =====\n")
print("### KEY source 2010 CÓ mà exe 2.5.2 BỎ (phải xoá khỏi reader để khớp 2012):")
for s,k in s2010:
    if k not in k25: print(f"   - [{s}] {k}")
print("\n### KEY exe 2.5.2 THÊM mà source 2010 chưa đọc (cần bổ sung reader):")
for s,k in s252:
    if k not in k10: print(f"   + [{s}] {k}")
print("\n### KEY chung nhưng DỜI SECTION (2010 → 2012):")
d10={k:s for s,k in s2010}; d25={k:s for s,k in s252}
for k in sorted(set(d10)&set(d25)):
    if d10[k].upper()!=d25[k].upper(): print(f"   ~ {k}: [{d10[k]}] → [{d25[k]}]")
print(f"\nfull decompile lưu: {PROJ}/LoadData_252.c")
