#!/usr/bin/env python3
"""Lean body-drift measure for SHARED functions (same name in both builds).
Opens the ghidriff-analyzed project (no re-import), decompiles each shared function
in ours + v2.5, normalizes the pseudo-C (the decompiler is the cross-compiler
normalizer), and scores similarity. Low score = old function whose BODY drifted
silently. Scoped by class-name prefix (arg) to stay fast.
Usage: bsim_lite.py <PREFIX> [max]      e.g. bsim_lite.py KCharacter:: 120
"""
import os, sys, re, difflib
os.environ.setdefault("GHIDRA_INSTALL_DIR", "/opt/homebrew/Cellar/ghidra/12.1.2/libexec")
os.environ.setdefault("JAVA_HOME", "/opt/homebrew/opt/openjdk@21")
import pyghidra
pyghidra.start()
from ghidra.base.project import GhidraProject
from ghidra.app.decompiler import DecompInterface
from ghidra.util.task import ConsoleTaskMonitor

PROJ_NAME=os.environ.get("GHIDRA_PROJECT_NAME")
PROJ_DIR=os.environ.get("GHIDRA_PROJECT_DIR")
if not PROJ_NAME or not PROJ_DIR:
    raise SystemExit("set GHIDRA_PROJECT_NAME and GHIDRA_PROJECT_DIR before running bsim_lite.py")
PREFIX=sys.argv[1] if len(sys.argv)>1 else "KCharacter::"
MAX=int(sys.argv[2]) if len(sys.argv)>2 else 120

proj=GhidraProject.openProject(PROJ_DIR, PROJ_NAME, True)
root=proj.getRootFolder()
progs={}
for df in root.getFiles():
    nm=df.getName()
    p=proj.openProgram("/", nm, True)
    if "ours" in nm: progs["ours"]=p
    elif "SO3GameServerD" in nm or "nodbg" in nm: progs["v25"]=p
print("programs:", {k:v.getName() for k,v in progs.items()}, file=sys.stderr)

def funcmap(p):
    m={}
    for f in p.getFunctionManager().getFunctions(True):
        try: n=f.getName(True)   # namespace-qualified: KCharacter::CheckDie
        except Exception: n=f.getName()
        m.setdefault(n, f)
    return m

def decomp_iface(p):
    di=DecompInterface(); di.openProgram(p); return di

NORMV=re.compile(r'\b(local_[0-9a-fx]+|[a-zA-Z]?Var[0-9]+|param_[0-9]+|in_[A-Za-z0-9]+|unaff_[A-Za-z0-9]+|extraout_[A-Za-z0-9]+|DAT_[0-9a-f]+|PTR_[0-9a-f]+|LAB_[0-9a-f]+|FUN_[0-9a-f]+|s_[A-Za-z0-9_]+)\b')
def norm(c):
    c=NORMV.sub("V",c); c=re.sub(r'0x[0-9a-f]+','N',c); c=re.sub(r'\s+',' ',c)
    return c

mon=ConsoleTaskMonitor()
diO=decomp_iface(progs["ours"]); diV=decomp_iface(progs["v25"])
fO=funcmap(progs["ours"]); fV=funcmap(progs["v25"])
shared=sorted(n for n in fO if n in fV and PREFIX in n and "::" in n)[:MAX]
print(f"shared {PREFIX} functions to score: {len(shared)}", file=sys.stderr)

def dc(di,f):
    r=di.decompileFunction(f,30,mon)
    if r and r.decompileCompleted():
        dcf=r.getDecompiledFunction()
        if dcf: return norm(dcf.getC() or "")
    return ""

rows=[]
for n in shared:
    a=dc(diO,fO[n]); b=dc(diV,fV[n])
    if not a or not b: continue
    ratio=difflib.SequenceMatcher(None,a,b).ratio()
    rows.append((ratio,n,len(a),len(b)))
rows.sort()
print(f"\n=== {PREFIX} body-similarity (LOW = likely drifted body). scored={len(rows)} ===")
print(f"{'sim':>5}  function")
for r,n,la,lb in rows:
    flag="  <-- DRIFT" if r<0.75 else ("  ~"if r<0.9 else "")
    print(f"{r:5.2f}  {n}{flag}")
drift=[r for r in rows if r[0]<0.75]
print(f"\nlikely-drifted (sim<0.75): {len(drift)}/{len(rows)} ({100*len(drift)//len(rows) if rows else 0}%)")
proj.close()
