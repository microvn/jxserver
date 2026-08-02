#!/usr/bin/env python3
"""L4 — enum-value drift auditor. v2.5 DWARF enums vs 2010 source enums.
Catches value drift that name-coverage (L1) is blind to (e.g. KAI_ACTION_KEY eak*,
AIType, protocol-id enums). Streams llvm-dwarfdump once, caches to coverage/v25_enums.json.
Usage: enum_audit.py [--force]   (writes coverage/enum_drift.tsv)"""
import sys, os, re, json, subprocess
from pathlib import Path

ROOT=os.environ.get("JX3_BUILD_ROOT", str(Path(__file__).resolve().parents[3]))
DW=os.environ.get("V25",str(Path(ROOT).parent / "jx3_dwarf" / "SO3GameServerD"))
DWARFDUMP=os.environ.get("DWARFDUMP","/opt/homebrew/opt/llvm/bin/llvm-dwarfdump")
OUT=str(Path(__file__).resolve().parent / "evidence")
CACHE=f"{OUT}/v25_enums.json"
SRCDIRS=[f"{ROOT}/src/SO3World/Src", f"{ROOT}/include/Include"]
os.makedirs(OUT, exist_ok=True)

def extract_v25():
    """Stream dwarfdump --debug-info; collect enum tag -> {member:value}. Dedup by tag."""
    enums={}
    p=subprocess.Popen([DWARFDUMP,"--debug-info",DW], stdout=subprocess.PIPE, text=True, errors="replace")
    cur=None; cur_name=None; in_member=False; mem_name=None
    NAME=re.compile(r'DW_AT_name\s+\("([^"]*)"\)')
    CVAL=re.compile(r'DW_AT_const_value\s+\((-?0x[0-9a-fA-F]+|-?\d+)\)')
    def close():
        nonlocal cur,cur_name
        if cur and cur_name and cur_name not in enums:
            enums[cur_name]=cur
        cur=None; cur_name=None
    for line in p.stdout:
        if "DW_TAG_" in line:
            if "DW_TAG_enumeration_type" in line:
                close(); cur={}; cur_name=None; in_member=False
            elif "DW_TAG_enumerator" in line:
                in_member=(cur is not None); mem_name=None
            else:
                close(); in_member=False
            continue
        m=NAME.search(line)
        if m:
            if in_member: mem_name=m.group(1)
            elif cur is not None and cur_name is None: cur_name=m.group(1)
            continue
        m=CVAL.search(line)
        if m and in_member and mem_name is not None and cur is not None:
            try: cur.setdefault(mem_name, int(m.group(1),0))
            except ValueError: pass
            in_member=False
    close()
    p.wait()
    return enums

def parse_source_enums():
    """Best-effort C enum parse from source. Returns tag -> {member:value}. Skips
    members with non-integer initializers (flagged)."""
    enums={}; skipped=set()
    blk=re.compile(r'\benum\s+(?:class\s+)?(\w+)\s*(?::\s*\w+\s*)?\{([^}]*)\}', re.S)
    for d in SRCDIRS:
        for root,_,files in os.walk(d):
            if "/Test/" in root or "/Robot/" in root: continue
            for fn in files:
                if not fn.endswith((".h",".hpp",".cpp",".c")): continue
                try:
                    txt=open(os.path.join(root,fn),encoding="latin-1").read()
                except Exception: continue
                txt=re.sub(r'/\*.*?\*/','',txt,flags=re.S)
                txt=re.sub(r'//[^\n]*','',txt)
                for m in blk.finditer(txt):
                    tag=m.group(1); body=m.group(2)
                    vals={}; nxt=0; ok=True
                    for item in body.split(','):
                        item=item.strip()
                        if not item: continue
                        if '=' in item:
                            nm,rhs=item.split('=',1); nm=nm.strip(); rhs=rhs.strip()
                            try: v=int(rhs,0)
                            except ValueError:
                                skipped.add(tag); ok=False; break
                            vals[nm]=v; nxt=v+1
                        else:
                            vals[item]=nxt; nxt+=1
                    if ok and vals: enums.setdefault(tag,vals)
    return enums, skipped

def main():
    force="--force" in sys.argv
    if force or not os.path.exists(CACHE):
        sys.stderr.write("[L4] extracting v25 enums from DWARF (streaming, ~1min)...\n")
        v25=extract_v25(); json.dump(v25,open(CACHE,"w"))
    else:
        v25=json.load(open(CACHE))
    src,skipped=parse_source_enums()

    drift=[]      # (tag, member, v25val, srcval)
    only_v25=[]   # (tag, member, v25val)  member missing in source
    for tag,mem in src.items():
        if tag not in v25: continue
        vm=v25[tag]
        for name,sv in mem.items():
            if name in vm and vm[name]!=sv:
                drift.append((tag,name,vm[name],sv))
        for name,vv in vm.items():
            if name not in mem:
                only_v25.append((tag,name,vv))

    with open(f"{OUT}/enum_drift.tsv","w") as f:
        f.write("kind\tenum\tmember\tv25\tsource\n")
        for t,n,vv,sv in sorted(drift): f.write(f"VALUE_DRIFT\t{t}\t{n}\t{vv}\t{sv}\n")
        for t,n,vv in sorted(only_v25): f.write(f"MEMBER_ONLY_V25\t{t}\t{n}\t{vv}\t-\n")
    common=set(src)&set(v25)
    print(f"[L4] enums: v25={len(v25)} source={len(src)} common={len(common)} "
          f"value-drift={len(drift)} member-only-v25={len(only_v25)} src-skipped(expr)={len(skipped)}")
    if drift:
        print("  top value-drift enums:", ", ".join(sorted({d[0] for d in drift})[:12]))

if __name__=="__main__": main()
