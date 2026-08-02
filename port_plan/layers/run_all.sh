#!/bin/bash
# Port completeness harness — runs all layers, writes port_plan/layers/COVERAGE.md.
# Minimal & re-runnable. Reuses the existing symbol pipeline; extends with Lua+data layers.
#   L1 C++ symbol coverage   (binary vs oracle)         -> cpp_by_system.tsv
#   L2 Lua binding coverage  (engine-provide vs demand)  -> lua_gap_ranked.tsv
#   L3 data/tab gaps         (free boot-log signal)      -> data_gaps.tsv
# Milestone planning = query these outputs; no human memory needed.
set -euo pipefail
export LC_ALL=C
LAYER_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
ROOT=${JX3_BUILD_ROOT:-$(cd "$LAYER_DIR/../.." && pwd)}
: "${HOST:?set HOST to the build/deploy SSH target}"
: "${LOGDIR:?set LOGDIR to the remote SO3GameServer log directory}"
OUT=$LAYER_DIR/L3-data-config/evidence
mkdir -p "$OUT"

FORCE=${1:-}   # pass --force to re-extract DWARF caches (enum/struct)
echo "########## L1 C++ symbol coverage ##########"; bash "$LAYER_DIR/L1-symbol-coverage/cpp_coverage.sh"
echo "########## L2 Lua binding coverage ##########"; bash "$LAYER_DIR/L2-lua-bindings/lua_coverage.sh"
echo "########## L4 enum-value drift ##########"; python3 "$LAYER_DIR/L4-enum-contracts/enum_audit.py" $FORCE
echo "########## L5 struct-size (packet/DB) ##########"; python3 "$LAYER_DIR/L5-serialized-layouts/struct_audit.py" $FORCE
echo "########## L6 trigger-path ##########"; bash "$LAYER_DIR/L6-lua-triggers/trigger_audit.sh"
echo "########## L7 logic-drift candidates (callee-diff) ##########"; python3 "$LAYER_DIR/L7-logic-drift/callee_audit.py" $FORCE

echo "########## L3 data/tab gaps (latest boot log) ##########"
ssh -o ConnectTimeout=15 "$HOST" "LC_ALL=C bash -s" > "$OUT/data_gaps.tsv" <<REMOTE
export LC_ALL=C
F=\$(ls -t $LOGDIR/*/*.log 2>/dev/null | head -1)
[ -z "\$F" ] && { echo "no-log"; exit 0; }
# normalize row/col/ids, group by table+error kind
grep -aE '<ERROR' "\$F" | grep -aE ':Get(Integer|String|Float|Data)\(.*failed|Failed to open file|GetData' \
 | sed -E 's/^[0-9-]+<ERROR[^>]*>: //; s/[0-9]+/N/g' | sort | uniq -c | sort -rn | awk '{c=\$1;\$1="";print c"\t"substr(\$0,2)}'
REMOTE
echo "data-gap classes: $(wc -l <"$OUT/data_gaps.tsv")"

python3 "$LAYER_DIR/gen_coverage_report.py" "$LAYER_DIR"
echo "== wrote $LAYER_DIR/COVERAGE.md =="
