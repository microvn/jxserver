#!/usr/bin/env python3
"""Aggregate L1-L7 evidence into port_plan/layers/COVERAGE.md.
The dashboard: %-done per layer/system + top ranked gaps. Milestone = query this."""
import sys, os
from pathlib import Path
LAYERS=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else Path(__file__).resolve().parent

def read_tsv(p):
    if not os.path.exists(p): return []
    with open(p) as f: return [ln.rstrip("\n").split("\t") for ln in f if ln.strip()]

cpp=read_tsv(LAYERS / "L1-symbol-coverage/evidence/cpp_by_system.tsv")
lua=read_tsv(LAYERS / "L2-lua-bindings/evidence/lua_gap_ranked.tsv")
data=read_tsv(LAYERS / "L3-data-config/evidence/data_gaps.tsv")
enum=read_tsv(LAYERS / "L4-enum-contracts/evidence/enum_drift.tsv")
struct=read_tsv(LAYERS / "L5-serialized-layouts/evidence/struct_sizes.tsv")
trig=read_tsv(LAYERS / "L6-lua-triggers/evidence/trigger_gap.tsv")
callee=read_tsv(LAYERS / "L7-logic-drift/evidence/callee_drift.tsv")

lua_called=[r for r in lua if len(r)==2 and r[1].isdigit() and int(r[1])>0]
cal_rows=[r for r in callee[1:] if len(r)>=7]
cal_common=len(cal_rows)
cal_cand=[r for r in cal_rows if float(r[1])>=0.34]
cal_verified=[r for r in cal_rows if r[6]=="Y"]
enum_drift=[r for r in enum[1:] if len(r)==5 and r[0]=="VALUE_DRIFT"]
enum_only=[r for r in enum[1:] if len(r)==5 and r[0]=="MEMBER_ONLY_V25"]
dead_trig=[r for r in trig[1:] if len(r)==2 and "dead-trigger" in r[1]]

lines=[]
w=lines.append
w("# COVERAGE — port completeness dashboard (auto-generated)")
w("")
w("Re-run: `bash port_plan/layers/run_all.sh`. Source of truth = binaries + scripts + boot log, NOT memory.")
w("Milestone planning = query the tables below (highest-demand gap on the core-loop tier first).")
w("")
# L1
w("## L1 — C++ symbol coverage (per system, tier C=core-loop first)")
w("")
if cpp:
    w("| tier | system | v25 | have | missing | cov% |")
    w("|---|---|--:|--:|--:|--:|")
    for r in cpp:
        if len(r)==6: w(f"| {r[0]} | {r[1]} | {r[2]} | {r[3]} | {r[4]} | {r[5]} |")
w("")
# L2
w("## L2 — Lua binding gaps (engine-provided in v2.5, missing in ours), ranked by script demand")
w("")
w(f"Total engine-binding gaps: **{len(lua)}**; called by content (will nil at runtime): **{len(lua_called)}**.")
w("These catch pure-content features (a script calls it, our engine doesn't provide it) with zero reliance on memory.")
w("")
w("| # | missing binding | script calls |")
w("|--:|---|--:|")
for i,r in enumerate(lua_called[:40],1):
    w(f"| {i} | `{r[0]}` | {r[1]} |")
w("")
# L3
w("## L3 — data/tab gaps (from latest boot log; boot-time reads only)")
w("")
if data and data[0] and data[0][0]!="no-log":
    w("| count | error class |")
    w("|--:|---|")
    for r in data[:25]:
        if len(r)==2: w(f"| {r[0]} | `{r[1]}` |")
else:
    w("_no boot log available (bring cluster up + boot to populate)._")
w("")
# L4
w("## L4 — enum-value drift (v2.5 DWARF vs 2010 source)")
w("")
w(f"Value-drift members: **{len(enum_drift)}** across enums; members only-in-v25: **{len(enum_only)}**.")
w("Name-coverage (L1) is blind to these; a wrong ordinal = nil binding / wrong packet id / broken AI.")
w("")
w("| enum | member | v25 | source |")
w("|---|---|--:|--:|")
CORE_ENUM=("KAI_ACTION_KEY","AIType","GS_CLIENT_PROTOCOL","CLIENT_GS_PROTOCOL","SKILL_KIND_TYPE","ATTRIBUTE_TYPE","KR2S_PROTOCOL","INTERNAL_PROTOCOL")
core=[r for r in enum_drift if any(c in r[1] for c in CORE_ENUM)]
for r in core[:20]: w(f"| `{r[1]}` | {r[2]} | {r[3]} | {r[4]} |")
w("")
w(f"_showing core-path enums; full list in enum_drift.tsv ({len(enum_drift)} rows)._")
w("")
# L5
w("## L5 — serialized struct sizes (packet + DB blob) — v2.5 reference")
w("")
n_ref=len([r for r in struct[1:] if len(r)==4])
w(f"Serialized-contract structs catalogued from v2.5 DWARF: **{n_ref}** (authoritative byte sizes in struct_sizes.tsv).")
w("These layouts MUST match byte-for-byte when ported (free RAM offsets do not). Ours is built with")
w("thin debug info, so a global auto-diff is not possible; **verify ours per-feature with a static_assert")
w("compile-probe** (pattern: `port_plan/layers/L5-serialized-layouts/s2c_sizeof_probe.cpp`). Rebuild ours with `-g3` to enable auto-diff.")
w("")
# L6
w("## L6 — trigger-path gaps (pure-content features never fired)")
w("")
w(f"Callbacks the v2.5 engine fires that ours does not: **{len(trig)-1}**; of which a content script")
w(f"defines the handler (= silently DEAD feature): **{len(dead_trig)}**. This catches Lua-only content")
w("(events, activities) that a C++ class-diff can never see.")
w("")
w("| dead-trigger callback (content waits, engine never fires) |")
w("|---|")
for r in dead_trig[:25]: w(f"| `{r[0]}` |")
w("")
w("## L7 — logic-drift candidates (same name, changed body) + correctness floor")
w("")
if cal_common:
    ident=sum(1 for r in cal_rows if float(r[1])==0)
    w(f"Functions present in BOTH binaries: **{cal_common}**. Callee-set identical: **{ident} ({100*ident//cal_common}%)**; ")
    w(f"logic-drift CANDIDATES (callee div>=0.34): **{len(cal_cand)} ({100*len(cal_cand)//cal_common}%)**; ")
    w(f"behaviorally VERIFIED (ledger): **{len(cal_verified)} ({100.0*len(cal_verified)/cal_common:.1f}%)**.")
    w("")
    w("**Read carefully:** the ~45% candidate figure is NOISY (compiler/inlining inflates it; constructors dominate")
    w("false positives) and the identical-% overstates safety (leaf/getters trivially match). The ONLY trustworthy")
    w("number is the VERIFIED ledger. It means: **L1 name-coverage % is NOT correctness %** — a present function may")
    w("carry stale 2010 logic v2.5 rewrote. Grow `port_plan/layers/L7-logic-drift/evidence/logic_verified.txt` as functions are oracle/decompile-verified at port time.")
    w("")
w("## How to read / definition of 'done'")
w("- L1 low cov% on a **C**-tier system = engine methods content/combat needs are missing.")
w("- L2 high-call gap = binding many scripts depend on; port it to unblock the most content.")
w("- L4 core-enum drift = fix ordinals FIRST (breaks bindings/packets/AI silently).")
w("- L6 dead-trigger = the pure-Lua feature is present but never runs; needs the C++ fire-site.")
w("- 'Done' for a system = L1 cov high + L2 gaps=0 + L4 no core drift + L5 sizes verified + L6 no dead-trigger + L3 clean. NOT 'log looks clean'.")
w("")
w("## Known blind spots (measured gaps in the MEASURER itself — visible, not silent)")
w("- **L1 measures NAME presence, not LOGIC.** A method ported with wrong behavior counts as covered.")
w("  Correctness is only caught by oracle-mirror / decompile-diff when porting each function.")
w("- **L1 misses** free functions (non-class), inline/header-only (no emitted symbol), templates.")
w("- **L2** only sees `Lua`-prefixed bindings; misses non-convention registrations and Lua CONSTANT tables.")
w("- **L3** is boot-log only: tables loaded on-demand (dungeon/activity) and silent schema drift are NOT seen.")
w("- **L5** ours-side is per-feature compile-probe, not global (ours debug is thin).")
w("- **L6** dead-trigger count is a FLOOR: script handlers defined as `X.OnFoo=function` are missed by the grep.")
w("- **Not yet measured:** config-key/section drift (see `port_plan/layers/L3-data-config/diff_loaddata.py`), save/load blob VERSIONING,")
w("  client-side assets. Add layers here as needed — do not treat absence as coverage.")

with open(LAYERS / "COVERAGE.md", "w") as f: f.write("\n".join(lines)+"\n")
print("report:", LAYERS / "COVERAGE.md",
      "| lua-called-gaps:", len(lua_called), "| enum-drift:", len(enum_drift),
      "| dead-triggers:", len(dead_trig), "| data-gaps:", len(data))
