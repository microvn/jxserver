#!/usr/bin/env python3
"""Compare compiler-noise between two builds of ours (gcc4.8.5 vs gcc4.4.7) against
the v2.5 oracle (gcc4.3.2). Tests Gemini's hypothesis: does building ours with a gcc
close to the target's compiler REDUCE the callee-set divergence (= compiler noise),
making logic-drift detection cleaner? Apples-to-apples: same function set for both.
Args: <oldgcc_callees.tsv>  (func<TAB>callee lines from obj_gcc44/*.o)
Reads coverage/callee_maps.json (v25 + ours-4.8.5 maps from L7)."""
import sys, json, os
from pathlib import Path
ROOT=os.environ.get("JX3_BUILD_ROOT", str(Path(__file__).resolve().parents[3]))
OUT=str(Path(__file__).resolve().parent / "evidence")
d=json.load(open(f"{OUT}/callee_maps.json"))
v25={k:set(v) for k,v in d["v25"].items()}
ours48={k:set(v) for k,v in d["ours"].items()}

import re
NOISE=re.compile(r'^std::|^__gnu|^__cxx|^operator|\(anonymous|^_|~|^KG_|^K_tag')
def norm(n): return n.split("(")[0].strip()
def is_proj(n): return "::" in n and not NOISE.search(n)
old={}
for ln in open(sys.argv[1]):
    p=ln.rstrip("\n").split("\t")
    if len(p)!=2: continue
    f=norm(p[0]); c=norm(p[1])
    if f!=c and is_proj(f) and is_proj(c): old.setdefault(f,set()).add(c)

def div(a,b):
    u=a|b
    return len(a^b)/len(u) if u else 0.0

# apples-to-apples: functions present in ALL THREE (old-gcc .o, ours-4.8.5, v25)
common=sorted(set(old)&set(ours48)&set(v25))
THR=0.34
def stats(buildmap,label):
    ident=cand=0
    for fn in common:
        dv=div(buildmap[fn], v25[fn])
        if dv==0: ident+=1
        if dv>=THR: cand+=1
    n=len(common)
    print(f"  {label:18} identical={ident:4} ({100*ident//n if n else 0}%)  drift-candidate(>= {THR})={cand:4} ({100*cand//n if n else 0}%)")
    return cand,n

print(f"[compare] functions present in ALL THREE builds: {len(common)}")
print(f"[compare] divergence vs v2.5 oracle (lower candidate% = less compiler noise = cleaner drift signal):")
c48,n=stats(ours48,"ours gcc4.8.5")
cold,_=stats(old,"ours gcc4.4.7")
if n:
    print(f"[compare] RESULT: gcc4.4.7 drift-candidates {100*cold//n}% vs gcc4.8.5 {100*c48//n}% "
          f"-> noise change {100*(cold-c48)//n:+d} pts on identical function set")
