#!/bin/bash
# L1 — C++ symbol coverage (re-runnable). Reuses port_plan/layers/inventory/extract_symbols.sh.
# Metric: methods defined in the v2.5.2 DWARF oracle binary but NOT in ours = missing.
# ours binary lives on the build host; v25 DWARF is local. Rolls up per gameplay system.
# Output: port_plan/layers/L1-symbol-coverage/evidence/ + prints per-system % to stdout.
set -euo pipefail
export LC_ALL=C
TOOL_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
ROOT=${JX3_BUILD_ROOT:-$(cd "$TOOL_DIR/../../.." && pwd)}
V25=${V25:-$(cd "$ROOT/.." && pwd)/jx3_dwarf/SO3GameServerD}
: "${HOST:?set HOST to the build/deploy SSH target}"
: "${OURS_REMOTE:?set OURS_REMOTE to the rebuilt binary on the remote host}"
OUT=$TOOL_DIR/evidence
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
mkdir -p "$OUT"

norm_awk='($2=="T"||$2=="t") && $3 ~ /^_ZN/ {print $3}'
filt_noise='^std::|^__gnu|^__cxx|^_|^operator|<|\(anonymous|\.cold|\.part|\.constprop|\.isra'

echo "[L1] extracting v25 oracle symbols (local DWARF)..."
if c++filt --help 2>&1 | grep -q 'LLVM'; then CXXFILT="c++filt -n -p"; else CXXFILT="c++filt -p"; fi
nm "$V25" 2>/dev/null | awk "$norm_awk" | $CXXFILT \
  | grep -vE "$filt_noise" | grep '::' | sort -u > "$WORK/v25.txt"

echo "[L1] extracting ours symbols (ssh $HOST)..."
ssh -o ConnectTimeout=10 "$HOST" "LC_ALL=C nm '$OURS_REMOTE' 2>/dev/null | awk '\$2==\"T\"||\$2==\"t\"{if(\$3 ~ /^_ZN/)print \$3}' | c++filt -p | grep -vE '$filt_noise' | grep '::' | LC_ALL=C sort -u" > "$WORK/ours.txt"

echo "[L1] v25=$(wc -l <"$WORK/v25.txt")  ours=$(wc -l <"$WORK/ours.txt")"
comm -23 "$WORK/v25.txt" "$WORK/ours.txt" > "$WORK/missing.txt"

# per-class v25 / ours / missing counts
awk -F'::' '{print $1}' "$WORK/v25.txt"  | sort | uniq -c | awk '{print $2"\t"$1}' > "$WORK/v25_cls.tsv"
awk -F'::' '{print $1}' "$WORK/ours.txt" | sort | uniq -c | awk '{print $2"\t"$1}' > "$WORK/ours_cls.tsv"
awk -F'::' '{print $1}' "$WORK/missing.txt" | sort | uniq -c | awk '{print $2"\t"$1}' > "$WORK/miss_cls.tsv"

cp "$WORK/missing.txt" "$OUT/cpp_missing_members.txt"
python3 "$TOOL_DIR/cpp_rollup.py" "$WORK/v25_cls.tsv" "$WORK/ours_cls.tsv" "$WORK/miss_cls.tsv" "$OUT"
