#!/bin/bash
# L6 — trigger-path audit. Pure-Lua features (a seasonal event, an activity) run only
# if C++ FIRES their script callback. L2 catches bindings a script CALLS; L6 catches the
# inverse: callbacks the v2.5 engine invokes into Lua that OUR engine never fires -> the
# content's script exists but is never triggered = silently dead. Memory-free.
#   v25_cb  = callback-name string literals in the v2.5 binary (On*/Remote_On*/GS_On*).
#   ours_cb = same string literals present in OUR source (what our C++ fires).
#   GAP = v25_cb \ ours_cb, cross-referenced with scripts that DEFINE the handler.
set -euo pipefail
export LC_ALL=C
TOOL_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
ROOT=${JX3_BUILD_ROOT:-$(cd "$TOOL_DIR/../../.." && pwd)}
V25=${V25:-$(cd "$ROOT/.." && pwd)/jx3_dwarf/SO3GameServerD}
: "${HOST:?set HOST to the build/deploy SSH target}"
: "${SCRIPTS:?set SCRIPTS to the remote script directory}"
OUT=$TOOL_DIR/evidence
WORK=$(mktemp -d); trap 'rm -rf "$WORK"' EXIT
mkdir -p "$OUT"
SRCDIRS="$ROOT/src/SO3World/Src $ROOT/include/Include"
CBRE='^(On|Remote_On|GS_On|GC_On|Client_On|Server_On)[A-Z][A-Za-z0-9_]+$'

echo "[L6] v25 engine callback strings..."
strings -n 5 "$V25" | grep -aE "$CBRE" | sort -u > "$WORK/v25_cb.txt"
echo "[L6] ours source callback string literals..."
grep -rhIoaE '"[A-Za-z_][A-Za-z0-9_]*"' $SRCDIRS 2>/dev/null | tr -d '"' | grep -aE "$CBRE" | sort -u > "$WORK/ours_cb.txt"
comm -23 "$WORK/v25_cb.txt" "$WORK/ours_cb.txt" > "$WORK/gap.txt"
echo "[L6] v25 callbacks=$(wc -l <"$WORK/v25_cb.txt")  ours=$(wc -l <"$WORK/ours_cb.txt")  gap=$(wc -l <"$WORK/gap.txt")"

echo "[L6] cross-ref: which gap callbacks are DEFINED by content scripts (=dead trigger)..."
# ask host which gap names appear as 'function <name>' in scripts
ssh -o ConnectTimeout=15 "$HOST" "LC_ALL=C grep -rhIoaE 'function[[:space:]]+[A-Za-z_][A-Za-z0-9_.:]*' '$SCRIPTS' 2>/dev/null | sed -E 's/function[[:space:]]+//; s/[.:].*//' | sort -u" > "$WORK/script_defs.txt"
# emit: callback  defined_by_script(yes/no)
: > "$OUT/trigger_gap.tsv"
echo -e "callback\tdefined_in_scripts" >> "$OUT/trigger_gap.tsv"
while IFS= read -r cb; do
  if grep -qxF "$cb" "$WORK/script_defs.txt"; then echo -e "$cb\tYES(dead-trigger)"; else echo -e "$cb\tno"; fi
done < "$WORK/gap.txt" | sort -t$'\t' -k2,2 >> "$OUT/trigger_gap.tsv"

DEAD=$(grep -c "YES(dead-trigger)" "$OUT/trigger_gap.tsv" || true)
echo "=== L6 top gap callbacks (engine fires in v25, ours doesn't) ==="
grep "YES(dead-trigger)" "$OUT/trigger_gap.tsv" | head -20
echo "... total gap=$(($(wc -l <"$OUT/trigger_gap.tsv")-1))  of which content defines a handler (dead-trigger): $DEAD"
