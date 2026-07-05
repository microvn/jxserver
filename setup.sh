#!/usr/bin/env bash
# ============================================================================
# JX3 (剑网3) v2.5.2 Linux server — BUILD PREP (source now lives in git)
# ----------------------------------------------------------------------------
# The source tree (src/ include/ compat/ devenv/) is git-tracked and is the
# SOURCE OF TRUTH. To fix something: edit the file, commit. Do NOT regenerate
# source here.
#
# This script only materialises the build INPUTS that are gitignored because
# they are binary or generated:
#   1. the 3 reused engine .so   (copied from 镜像端, binary, not in git)
#   2. Source/Common/* bridge    (relative symlinks, generated)
#   3. shim/                      (case-insensitive #include symlinks, generated)
#
# History: the tree was originally assembled from scattered leak trees + hand
# patches by bootstrap-from-leak.sh (kept for provenance). That pipeline is
# frozen into the baseline commit; endgame.py's edits are baked into source.
# ============================================================================
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
JX3ROOT="$(cd "$HERE/.." && pwd)"
SOROOT="$JX3ROOT/镜像端/extracted/root"   # the 3 reused .so (D variants)

echo "== 0. sanity: git source + binary inputs present =="
for p in "$HERE/src/SO3World/Src" "$HERE/include/Include" "$HERE/mkshim.py" \
         "$SOROOT/libEngine_Lua5D.so" "$SOROOT/libSO3EnumConvertorD.so" "$SOROOT/libSO3ItemHouseD.so"; do
  [ -e "$p" ] || { echo "  MISSING: $p"; exit 1; }
done
echo "  ok (source .cpp: $(ls "$HERE/src/SO3World/Src/"*.cpp | wc -l | tr -d ' '))"

echo "== 1. reused engine .so (binary input from 镜像端) =="
mkdir -p "$HERE/libs"
cp "$SOROOT/libEngine_Lua5D.so" "$SOROOT/libSO3EnumConvertorD.so" "$SOROOT/libSO3ItemHouseD.so" "$HERE/libs/"
for f in "$HERE/libs/"*.so; do
  printf "  %-26s " "$(basename "$f")"
  python3 -c "d=open('$f','rb').read(20);print('ELF'+('32' if d[4]==1 else '64'),'i386' if d[18]==3 else hex(d[18]))"
done

echo "== 2. original-layout bridge (Include/ headers use ../../Source/Common/... paths) =="
mkdir -p "$HERE/Source/Common"
ln -sfn "../../src/SO3World"     "$HERE/Source/Common/SO3World"
ln -sfn "../../src/SO3Represent" "$HERE/Source/Common/SO3Represent"
echo "  Source/Common/{SO3World,SO3Represent} -> src/"

echo "== 3. case-insensitive #include shim =="
python3 "$HERE/mkshim.py"

echo
echo "BUILD PREP READY at $HERE"
echo "next:  ./build.sh"
