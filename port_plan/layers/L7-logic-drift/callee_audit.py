#!/usr/bin/env python3
"""L7 — logic-drift candidate flagger (same name, updated body). The 6 structural
layers are blind to a function whose NAME is present but whose LOGIC v2.5 changed.
L7 disassembles both binaries and compares each common function's CALLEE SET.
A diverged callee set = candidate logic drift (does NOT confirm; inlining differs by
compiler, so this RANKS candidates, per-function verify still needed).
Also emits the logic-verified ledger view: everything is 'unverified' until oracle/
decompile-checked, so real correctness% << name-coverage%.
Usage: callee_audit.py [--force]  (writes coverage/callee_drift.tsv)"""
import sys, os, re, json, subprocess
from pathlib import Path

ROOT=os.environ.get("JX3_BUILD_ROOT", str(Path(__file__).resolve().parents[3]))
V25=os.environ.get("V25",str(Path(ROOT).parent / "jx3_dwarf" / "SO3GameServerD"))
OBJDUMP=os.environ.get("OBJDUMP","/opt/homebrew/opt/llvm/bin/llvm-objdump")
HOST=os.environ.get("HOST")
OURS_REMOTE=os.environ.get("OURS_REMOTE")
OUT=str(Path(__file__).resolve().parent / "evidence")
LEDGER=str(Path(__file__).resolve().parent / "evidence/logic_verified.txt")
os.makedirs(OUT, exist_ok=True)

NOISE=re.compile(r'^std::|^__gnu|^__cxx|^operator|\(anonymous|^_|~|^KG_|^K_tag')
HDR=re.compile(r'^[0-9a-f]+\s+<(.+)>:')          # function header:  4012f0 <Name>:
CALL=re.compile(r'\b(call|callq|jmp|bl)\b.*<([^>+]+)(?:\+0x[0-9a-f]+)?>')

def norm(n):  # strip args/return, keep Class::method
    n=n.split("(")[0].strip()
    return n

def is_proj(n):
    return "::" in n and not NOISE.search(n)

def parse(cmd, shell=False):
    """stream objdump -d -C; return {func: set(callees)} for project funcs."""
    fmap={}
    p=subprocess.Popen(cmd, stdout=subprocess.PIPE, text=True, errors="replace", shell=shell)
    cur=None
    for line in p.stdout:
        m=HDR.match(line)
        if m:
            fn=norm(m.group(1)); cur=fn if is_proj(fn) else None
            if cur is not None: fmap.setdefault(cur,set())
            continue
        if cur is None: continue
        m=CALL.search(line)
        if m:
            t=norm(m.group(2))
            if is_proj(t) and t!=cur: fmap[cur].add(t)
    p.wait()
    return fmap

def main():
    if not HOST or not OURS_REMOTE:
        raise SystemExit("set HOST and OURS_REMOTE before running L7")
    force="--force" in sys.argv
    cache=f"{OUT}/callee_maps.json"
    if force or not os.path.exists(cache):
        sys.stderr.write("[L7] disassembling v25 (local, ~min)...\n")
        v25=parse([OBJDUMP,"-d","-C","--no-show-raw-insn",V25])
        sys.stderr.write("[L7] disassembling ours (ssh)...\n")
        ours=parse(f"ssh -o ConnectTimeout=20 {HOST} \"objdump -d -C --no-show-raw-insn '{OURS_REMOTE}'\"", shell=True)
        json.dump({"v25":{k:sorted(v) for k,v in v25.items()},
                   "ours":{k:sorted(v) for k,v in ours.items()}}, open(cache,"w"))
    else:
        d=json.load(open(cache)); v25={k:set(v) for k,v in d["v25"].items()}; ours={k:set(v) for k,v in d["ours"].items()}

    verified=set()
    if os.path.exists(LEDGER):
        verified={l.strip() for l in open(LEDGER) if l.strip() and not l.startswith("#")}

    common=sorted(set(v25)&set(ours))
    rows=[]
    for fn in common:
        a,b=v25[fn],ours[fn]
        if not a and not b:
            div=0.0
        else:
            u=a|b; div=len(a^b)/len(u) if u else 0.0
        rows.append((fn,div,len(a),len(b),len(a-b),len(b-a)))
    rows.sort(key=lambda r:-r[1])

    THR=0.34
    flagged=[r for r in rows if r[1]>=THR]
    with open(f"{OUT}/callee_drift.tsv","w") as f:
        f.write("function\tdivergence\tv25_callees\tours_callees\tonly_v25\tonly_ours\tverified\n")
        for fn,div,na,nb,ov,oo in rows:
            f.write(f"{fn}\t{div:.2f}\t{na}\t{nb}\t{ov}\t{oo}\t{'Y' if fn in verified else ''}\n")

    ncommon=len(common)
    print(f"[L7] common functions (name in BOTH binaries): {ncommon}")
    print(f"[L7] callee-set identical (div=0): {sum(1 for r in rows if r[1]==0)} "
          f"({100.0*sum(1 for r in rows if r[1]==0)/ncommon if ncommon else 0:.0f}%)")
    print(f"[L7] LOGIC-DRIFT CANDIDATES (div>={THR}): {len(flagged)} "
          f"({100.0*len(flagged)/ncommon if ncommon else 0:.0f}% of common)  <- same name, likely stale body")
    print(f"[L7] logic-VERIFIED (ledger): {len(verified)} "
          f"({100.0*len(verified)/ncommon if ncommon else 0:.1f}% of common)  <- real correctness floor")
    print("  top drift candidates:", ", ".join(r[0] for r in flagged[:10]))

if __name__=="__main__": main()
