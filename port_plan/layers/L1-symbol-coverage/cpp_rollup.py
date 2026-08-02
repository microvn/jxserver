#!/usr/bin/env python3
"""Roll up per-class v25/ours/missing counts into per-system coverage.
Args: v25_cls.tsv ours_cls.tsv miss_cls.tsv out_dir
Writes out_dir/cpp_missing_by_class.tsv and out_dir/cpp_by_system.tsv; prints summary."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from systems import system_of, SYSTEM_TIER
from collections import defaultdict

def load(p):
    d={}
    with open(p) as f:
        for ln in f:
            c,n=ln.rstrip("\n").split("\t"); d[c]=int(n)
    return d

v25=load(sys.argv[1]); ours=load(sys.argv[2]); miss=load(sys.argv[3]); OUT=sys.argv[4]

# only classes present in the oracle and that we track (have real work)
rows=[]
for c in sorted(v25):
    m=miss.get(c,0)
    if m==0: continue
    rows.append((system_of(c), c, v25[c], ours.get(c,0), m))

with open(f"{OUT}/cpp_missing_by_class.tsv","w") as f:
    f.write("system\tclass\tv25\tours\tmissing\n")
    for sysn,c,v,o,m in sorted(rows, key=lambda r:(-r[4],)):
        f.write(f"{sysn}\t{c}\t{v}\t{o}\t{m}\n")

by=defaultdict(lambda:[0,0,0])  # system -> [v25, ours_of_v25, missing]  (ours capped at v25 for %)
for sysn,c,v,o,m in rows:
    b=by[sysn]; b[0]+=v; b[1]+=min(o,v); b[2]+=m

with open(f"{OUT}/cpp_by_system.tsv","w") as f:
    f.write("tier\tsystem\tv25\thave\tmissing\tcoverage%\n")
    tot=[0,0,0]
    order=sorted(by, key=lambda s:(SYSTEM_TIER.get(s,"Z"), -by[s][2]))
    for s in order:
        v,h,m=by[s]; tot[0]+=v; tot[1]+=h; tot[2]+=m
        cov=100.0*h/v if v else 0
        f.write(f"{SYSTEM_TIER.get(s,'?')}\t{s}\t{v}\t{h}\t{m}\t{cov:.0f}\n")
    covt=100.0*tot[1]/tot[0] if tot[0] else 0
    f.write(f"-\tTOTAL\t{tot[0]}\t{tot[1]}\t{tot[2]}\t{covt:.0f}\n")

print("=== C++ symbol coverage by system (tier C=core-loop first) ===")
print(f"{'tier':4} {'system':20} {'v25':>5} {'have':>5} {'miss':>5} {'cov%':>5}")
for s in sorted(by, key=lambda s:(SYSTEM_TIER.get(s,'Z'), -by[s][2])):
    v,h,m=by[s]; print(f"{SYSTEM_TIER.get(s,'?'):4} {s:20} {v:5} {h:5} {m:5} {100.0*h/v if v else 0:5.0f}")
v,h,m=[sum(by[s][i] for s in by) for i in range(3)]
print(f"{'':4} {'TOTAL':20} {v:5} {h:5} {m:5} {100.0*h/v if v else 0:5.0f}")
