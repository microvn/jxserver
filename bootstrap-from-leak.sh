#!/usr/bin/env bash
# ============================================================================
# JX3 (剑网3) v2.5.2 Linux server — SCAFFOLD ASSEMBLY
# ----------------------------------------------------------------------------
# Rebuilds the scratchpad/build tree that used to live in /tmp (wiped on reset).
# Everything it needs survives on persistent disks:
#   - hand-written / reconstructed files -> _recovered/  (recovered from
#     Claude file-history of session 8d215a3e; the ONLY non-reproducible pieces)
#   - bulk source + headers -> $JX3ROOT/source/{jx3dev-master,JX3-AIO,...}
#   - the 3 reused engine .so -> $JX3ROOT/镜像端/extracted/root/
# linux-build/ is a sibling of source/ ; JX3ROOT = its parent (the jx3 root).
#
# Idempotent: wipes and re-lays src/ include/ libs/ shim/ each run, then
# overlays _recovered/ LAST so the reconstructed stubs win over the real
# headers (ExData is case-insensitive: kmemory.h overwrites KMemory.h — this
# is the documented mechanism that keeps the std::string-based KMemory stub
# instead of the pooled real one that regressed the build 178->53).
# ============================================================================
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"

# Everything is anchored to ONE root: the parent of this build dir. linux-build/
# sits as a sibling of source/ under the jx3 collection, so all extracted source
# is mapped in one place ($JX3ROOT/source) — relocate the whole collection and
# nothing here needs editing.
JX3ROOT="$(cd "$HERE/.." && pwd)"
SRC="$JX3ROOT/source"

# --- persistent sources (all derived from $JX3ROOT) -------------------------
JXDEV="$SRC/jx3dev-master"                                              # SO3World/Represent/glue + Include
BASEINC="$SRC/JX3-AIO/JX3_Download/Jx3D-master/source/Base/include"     # real Base headers
SOROOT="$JX3ROOT/镜像端/extracted/root"                                 # the 3 reused .so (D variants)
LUA5="$SRC/Sword3-FullSource-master/Jx3Full/Source/Source/base/base/DevEnv/Include/Lua5"  # KingSoft-packaged Lua 5.1 headers
FULLENG="$SRC/Sword3-FullSource-master/Jx3Full/Source/Source/base/base/Include/Engine"    # FullSource engine headers (KTextFilter.h etc. missing from Jx3D Base)

echo "== 0. sanity: sources present =="
for p in "$JXDEV/Source/Common/SO3World/Src" "$JXDEV/Include" "$BASEINC/engine" "$LUA5/lua.h" \
         "$SOROOT/libEngine_Lua5D.so" "$SOROOT/libSO3EnumConvertorD.so" "$SOROOT/libSO3ItemHouseD.so"; do
  [ -e "$p" ] || { echo "  MISSING: $p"; exit 1; }
done
echo "  all sources OK"

echo "== 1. clean generated dirs =="
rm -rf "$HERE/src" "$HERE/include" "$HERE/libs" "$HERE/shim" "$HERE/obj" \
       "$HERE/compat" "$HERE/devenv" "$HERE/Source" "$HERE/Dockerfile" "$HERE/mkshim.py"
mkdir -p "$HERE/src" "$HERE/include" "$HERE/libs" "$HERE/obj"

echo "== 2. bulk source trees =="
cp -R "$JXDEV/Source/Common/SO3World"      "$HERE/src/SO3World"
cp -R "$JXDEV/Source/Common/SO3Represent"  "$HERE/src/SO3Represent"
cp -R "$JXDEV/Source/Server/SO3GameServer" "$HERE/src/SO3GameServer"
echo "  SO3World .cpp: $(ls "$HERE/src/SO3World/Src/"*.cpp | wc -l | tr -d ' ')"

echo "== 3. header roots =="
cp -R "$JXDEV/Include" "$HERE/include/Include"
cp -R "$BASEINC"       "$HERE/include/Base"
# Lua 5.1 headers: laid out to satisfy BOTH include styles the sources use —
#   luaapi.h   ->  #include "lua.h"            (flat, via -Iinclude/Base/lua5)
#   KLuaScriptEx.h -> #include <lua5/lualib.h> (subdir, via -Iinclude/Base/lua5)
mkdir -p "$HERE/include/Base/lua5/lua5"
cp "$LUA5/"*.h "$HERE/include/Base/lua5/"
cp "$LUA5/"*.h "$HERE/include/Base/lua5/lua5/"
echo "  Include .h: $(find "$HERE/include/Include" -iname '*.h' | wc -l | tr -d ' ')  Base .h: $(find "$HERE/include/Base" -iname '*.h' | wc -l | tr -d ' ')"

echo "== 4. reused engine .so =="
cp "$SOROOT/libEngine_Lua5D.so" "$SOROOT/libSO3EnumConvertorD.so" "$SOROOT/libSO3ItemHouseD.so" "$HERE/libs/"
for f in "$HERE/libs/"*.so; do
  printf "  %-26s " "$(basename "$f")"
  python3 -c "d=open('$f','rb').read(20);print('ELF'+('32' if d[4]==1 else '64'),'i386' if d[18]==3 else hex(d[18]))"
done

echo "== 5. overlay recovered hand-written files (LAST — stubs must win) =="
# Dockerfile + mkshim.py at root
cp "$HERE/_recovered/Dockerfile" "$HERE/Dockerfile"
cp "$HERE/_recovered/mkshim.py"  "$HERE/mkshim.py"
# compat/, devenv/  (whole dirs, hand-written)
cp -R "$HERE/_recovered/compat"  "$HERE/compat"
cp -R "$HERE/_recovered/devenv"  "$HERE/devenv"
# reconstructed engine stubs overwrite real KMemory.h / KLuaScriptEx.h (case-insensitive FS)
cp "$HERE/_recovered/include/Base/engine/kmemory.h"     "$HERE/include/Base/engine/kmemory.h"
cp "$HERE/_recovered/include/Base/engine/KLuaScriptEx.h" "$HERE/include/Base/engine/KLuaScriptEx.h"
# reconstructed common networking layer
mkdir -p "$HERE/src/common_recon"
cp "$HERE/_recovered/src/common_recon/"*.cpp "$HERE/src/common_recon/"
# KTextFilter.h: real engine header the Jx3D Base tree lacks (KPlayerServer/
# KBaseFuncList/KLuaMailClient need it). Pull from FullSource engine headers.
cp "$FULLENG/KTextFilter.h" "$HERE/include/Base/engine/KTextFilter.h"
echo "  overlaid: Dockerfile mkshim.py compat/ devenv/ engine stubs + KTextFilter.h + common_recon"

echo "== 6. original-layout bridge =="
# Some Include/ headers use tree-relative includes like
#   #include "../../Source/Common/SO3World/Src/KPlayer.h"
# which only resolve when Include/ and Source/ are siblings (original layout).
# In the split scaffold they resolve via -Iinclude/Include ->
#   include/Include/../../Source/Common/... == /work/Source/Common/... .
# Bridge that path back into src/ with relative symlinks.
mkdir -p "$HERE/Source/Common"
ln -sfn "../../src/SO3World"     "$HERE/Source/Common/SO3World"
ln -sfn "../../src/SO3Represent" "$HERE/Source/Common/SO3Represent"
echo "  Source/Common/{SO3World,SO3Represent} -> src/"

echo "== 7. endgame patches (5 file SỐNG: _NAME_LEN include, netinet/in.h, KNpcTeam dead methods) =="
# Vá SAU khi copy source (upstream giữ nguyên). Chỉ vá file SỐNG; file CHẾT
# (KMissile/KPathFinder/KTrackList/KAI_Player) loại qua DEAD set trong build.sh.
# Xem DECISION.md §B2,§C,§D. Idempotent.
python3 "$HERE/patches/endgame.py" "$HERE"

echo "== 8. case-insensitive #include shim =="
python3 "$HERE/mkshim.py"

echo
echo "SCAFFOLD READY at $HERE"
echo "next:  docker build --platform linux/amd64 -t jx3build .   # (image already exists as jx3build:latest)"
echo "       ./build.sh"
