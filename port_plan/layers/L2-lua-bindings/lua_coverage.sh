#!/bin/bash
# L2 — Lua binding coverage (hollow-detector), ACCURATE via binary oracle.
# The v2.5 engine registers Lua bindings as Lua-prefixed fns (REGISTER_LUA_FUNC ->
# &Class::Lua##Name; global tables -> free LuaName). So:
#   PROVIDE(v25)  = Lua-facing names in the DWARF oracle binary.
#   PROVIDE(ours) = same in our built binary.
#   ENGINE GAP    = PROVIDE(v25) \ PROVIDE(ours)  <- precise, no script-regex noise.
#   DEMAND        = how often scripts call each name (only RANKS the gap; noise tolerated).
# A gap called by content = a feature that WILL nil at runtime even if never reached in a test.
set -euo pipefail
export LC_ALL=C
TOOL_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
ROOT=${JX3_BUILD_ROOT:-$(cd "$TOOL_DIR/../../.." && pwd)}
V25=${V25:-$(cd "$ROOT/.." && pwd)/jx3_dwarf/SO3GameServerD}
: "${HOST:?set HOST to the build/deploy SSH target}"
: "${OURS_REMOTE:?set OURS_REMOTE to the rebuilt binary on the remote host}"
: "${SCRIPTS:?set SCRIPTS to the remote script directory}"
OUT=$TOOL_DIR/evidence
WORK=$(mktemp -d); trap 'rm -rf "$WORK"' EXIT
mkdir -p "$OUT"

# extract Lua-facing names from a binary's demangled symbols.
# match  (Class::)?LuaName  -> strip everything up to and incl "Lua" prefix, drop args.
lua_names() { # reads demangled symbol lines on stdin
  grep -oE '(::)?Lua[A-Z][A-Za-z0-9_]*' | sed -E 's/.*Lua//' | sort -u
}
if c++filt --help 2>&1 | grep -q 'LLVM'; then CXXFILT="c++filt -n -p"; else CXXFILT="c++filt -p"; fi

echo "[L2] PROVIDE(v25) from DWARF oracle..."
nm "$V25" 2>/dev/null | awk '($2=="T"||$2=="t"){print $3}' | $CXXFILT | lua_names > "$WORK/v25_lua.txt"
echo "[L2] PROVIDE(ours) via ssh..."
ssh -o ConnectTimeout=10 "$HOST" "LC_ALL=C nm '$OURS_REMOTE' 2>/dev/null | awk '\$2==\"T\"||\$2==\"t\"{print \$3}' | c++filt -p | grep -oE '(::)?Lua[A-Z][A-Za-z0-9_]*' | sed -E 's/.*Lua//' | LC_ALL=C sort -u" > "$WORK/ours_lua.txt"
echo "[L2] v25 lua-bindings=$(wc -l <"$WORK/v25_lua.txt")  ours=$(wc -l <"$WORK/ours_lua.txt")"

comm -23 "$WORK/v25_lua.txt" "$WORK/ours_lua.txt" > "$WORK/engine_gap.txt"

echo "[L2] DEMAND: script call counts (ssh)..."
ssh -o ConnectTimeout=15 "$HOST" "LC_ALL=C grep -rhIoaE '\b[A-Za-z_][A-Za-z0-9_]*[[:space:]]*\(' '$SCRIPTS' 2>/dev/null | sed -E 's/[[:space:]]*\(.*//' | sort | uniq -c | awk '{print \$2\"\t\"\$1}'" > "$WORK/demand.tsv"

# rank engine gap by demand
awk -F'\t' 'NR==FNR{d[$1]=$2;next}{print $1"\t"(($1 in d)?d[$1]:0)}' "$WORK/demand.tsv" "$WORK/engine_gap.txt" \
  | sort -t$'\t' -k2,2 -rn > "$OUT/lua_gap_ranked.tsv"

CALLED=$(awk -F'\t' '$2>0' "$OUT/lua_gap_ranked.tsv" | wc -l | xargs)
echo "=== L2: engine Lua-bindings MISSING in ours, ranked by script demand ==="
echo "name<TAB>script_call_count"
head -25 "$OUT/lua_gap_ranked.tsv"
echo "..."
echo "total engine-binding gaps: $(wc -l <"$OUT/lua_gap_ranked.tsv")  (of which called by scripts: $CALLED)"
