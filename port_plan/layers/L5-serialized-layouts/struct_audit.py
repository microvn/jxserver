#!/usr/bin/env python3
"""L5 — struct byte-size audit for packet + DB-blob contracts (serialized layouts
that MUST match, unlike free RAM offsets). v2.5 DWARF byte_size = authoritative.
Compares against ours-DWARF where present (ours is built with thin debug info, so
coverage is partial; missing ones are flagged 'verify-by-compile-probe', not silent).
Usage: struct_audit.py [--force]   (writes coverage/struct_sizes.tsv)"""
import sys, os, re, json, subprocess
from pathlib import Path

ROOT=os.environ.get("JX3_BUILD_ROOT", str(Path(__file__).resolve().parents[3]))
DW=os.environ.get("V25",str(Path(ROOT).parent / "jx3_dwarf" / "SO3GameServerD"))
DWARFDUMP=os.environ.get("DWARFDUMP","/opt/homebrew/opt/llvm/bin/llvm-dwarfdump")
HOST=os.environ.get("HOST")
OURS_REMOTE=os.environ.get("OURS_REMOTE")
OUT=str(Path(__file__).resolve().parent / "evidence")
CACHE=f"{OUT}/v25_structs.json"
os.makedirs(OUT, exist_ok=True)

# names that denote a SERIALIZED contract (packet or DB blob) — the layouts that MUST match
# byte-for-byte (free RAM offsets are fine; these are not). Focused prefixes, low noise.
SERIAL=re.compile(r'(^S2C_|^C2S_|^G2C_|^C2G_|^R2S_|^S2R_|^GC_|^CG_|_PARAM$|_DB$|_DBDATA$|_DB_DATA$|DBData$|RoleData$|_SYNC$|_PACKAGE$|_HANDSHAKE)')

def stream_sizes(dumpcmd):
    sizes={}
    p=subprocess.Popen(dumpcmd, stdout=subprocess.PIPE, text=True, errors="replace", shell=isinstance(dumpcmd,str))
    NAME=re.compile(r'DW_AT_name\s+\("([^"]*)"\)')
    SIZE=re.compile(r'DW_AT_byte_size\s+\((-?0x[0-9a-fA-F]+|-?\d+)\)')
    cur=None; nm=None; sz=None
    def close():
        nonlocal cur,nm,sz
        if nm and sz is not None and nm not in sizes: sizes[nm]=sz
        cur=None; nm=None; sz=None
    for line in p.stdout:
        if "DW_TAG_" in line:
            if any(t in line for t in ("DW_TAG_structure_type","DW_TAG_class_type","DW_TAG_union_type")):
                close(); cur=True; nm=None; sz=None
            else:
                close()
            continue
        if cur:
            m=NAME.search(line)
            if m and nm is None: nm=m.group(1); continue
            m=SIZE.search(line)
            if m and sz is None:
                try: sz=int(m.group(1),0)
                except ValueError: pass
    close(); p.wait()
    return sizes

def main():
    if not HOST or not OURS_REMOTE:
        raise SystemExit("set HOST and OURS_REMOTE before running L5")
    force="--force" in sys.argv
    if force or not os.path.exists(CACHE):
        sys.stderr.write("[L5] extracting v25 struct sizes from DWARF (streaming)...\n")
        v25=stream_sizes([DWARFDUMP,"--debug-info",DW]); json.dump(v25,open(CACHE,"w"))
    else:
        v25=json.load(open(CACHE))
    sys.stderr.write("[L5] extracting ours struct sizes via ssh (thin debug)...\n")
    ours=stream_sizes(f"ssh -o ConnectTimeout=15 {HOST} \"llvm-dwarfdump --debug-info '{OURS_REMOTE}' 2>/dev/null || readelf --debug-dump=info '{OURS_REMOTE}' 2>/dev/null\"")

    serial=sorted(n for n in v25 if SERIAL.search(n))
    drift=[]; unknown=[]; ok=0
    for n in serial:
        if n in ours:
            if ours[n]!=v25[n]: drift.append((n,v25[n],ours[n]))
            else: ok+=1
        else:
            unknown.append((n,v25[n]))
    with open(f"{OUT}/struct_sizes.tsv","w") as f:
        f.write("status\tstruct\tv25_size\tours_size\n")
        for n,v,o in sorted(drift): f.write(f"SIZE_DRIFT\t{n}\t{v}\t{o}\n")
        for n,v in unknown: f.write(f"OURS_UNKNOWN\t{n}\t{v}\t?\n")
    print(f"[L5] serialized-contract structs (v25 reference)={len(serial)}  "
          f"size-match={ok}  SIZE_DRIFT={len(drift)}  ours-unknown={len(unknown)}")
    print(f"  NOTE: ours built with thin debug -> global size-diff not possible; struct_sizes.tsv is the")
    print(f"  v25 AUTHORITATIVE reference. Verify ours per-feature via static_assert compile-probe")
    print(f"  (pattern: port_plan/layers/L5-serialized-layouts/s2c_sizeof_probe.cpp). For automatic global diff, rebuild ours with -g3.")
    if drift:
        print("  drifted:", ", ".join(n for n,_,_ in drift[:15]))

if __name__=="__main__": main()
