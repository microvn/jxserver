#!/usr/bin/env python3
"""[drift 2.5.2] Data-fix: 4 NPC/BOSS skill scripts declare tSkillData literal with
fewer entries than the skill's MaxLevel (tab), so our per-level loader (1..MaxLevel)
hits tSkillData[level]=nil -> "index a nil value" (13 lines). NPC skills only cast
level 1 in-game, so levels >tSkillData are unused; extend tSkillData to MaxLevel by
copying the last entry. Idempotent. Run on the deploy tree (host, GBK/ext4).

Usage: python3 fix_npc_tskilldata.py /path/to/镜像端/extracted/root
"""
import sys, os, re, csv

ROOT = sys.argv[1] if len(sys.argv) > 1 else "/root/jx3/镜像端/extracted/root"
SKILL_IDS = {"838", "844", "855", "4413"}   # confirmed failing (DBGML)

tab = os.path.join(ROOT, "settings/skill/skills.tab").encode()
rows = list(csv.reader(open(tab, encoding="latin-1"), delimiter="\t"))
h = rows[0]
idc, sic, mic = h.index("SkillID"), h.index("ScriptFile"), h.index("MaxLevel")

def extend(path_b, maxlv):
    raw = open(path_b, "rb").read().decode("latin-1")
    m = re.search(r"(tSkillData\s*=\s*\{)(.*?)(\n\};)", raw, re.S)
    if not m:
        print("  no tSkillData block:", path_b); return False
    body = m.group(2)
    entries = re.findall(r"[ \t]*\{[^}]*\},?[^\n]*", body)
    n = len(entries)
    if n >= maxlv:
        print("  already ok (%d>=%d)" % (n, maxlv)); return False
    last = entries[-1].rstrip()
    if not last.endswith(","):
        # ensure separator
        last = re.sub(r"\}(\s*(--[^\n]*)?)$", r"},\1", last)
    add = "\n".join(["\t{nDamageBase = 0, nDamageRand = 0, nCostMana = 0},\t\t--[drift 2.5.2] pad level %d" % i
                     for i in range(n + 1, maxlv + 1)])
    newbody = body.rstrip("\n") + "\n" + add
    out = raw[:m.start(2)] + newbody + raw[m.end(2):]
    open(path_b, "wb").write(out.encode("latin-1"))
    print("  extended %d -> %d" % (n, maxlv)); return True

for r in rows[2:]:
    if idc < len(r) and r[idc] in SKILL_IDS:
        sf = r[sic]; maxlv = int(r[mic])
        p = os.path.join(ROOT.encode(), b"scripts/skill/" + sf.replace("\\", "/").encode("latin-1"))
        print("id=%s maxlv=%d %s" % (r[idc], maxlv, sf.encode("latin-1").decode("gbk", "replace")))
        if os.path.isfile(p):
            extend(p, maxlv)
        else:
            print("  FILE NOT FOUND:", p)
