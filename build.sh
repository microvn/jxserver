#!/usr/bin/env bash
# ============================================================================
# JX3 v2.5.2 Linux server — COMPILE + LINK  (runs inside jx3build container)
# ----------------------------------------------------------------------------
# Reproduces the state reached in session 8d215a3e:
#   ~188/194 SO3World compile, link undefined 1332 -> ~20.
# Toolchain: centos:7 / gcc 4.8.5 / -m32 (old libstdc++ ABI matches the 2012 .so).
#
# Known residual undefined (documented, NOT scaffold bugs):
#   CRC32 (write standard impl), 3 singles KMissile/KPathFinder/KTrackList,
#   a lingering old-AI header ref to pin. Socket framing/security is provided
#   by the verified i386 libs/libcommon.a archive (Wave 1b R11); the old
#   common_recon/kg_socket.cpp pass-through must not be compiled.
#
# Env blocker on Apple Silicon: OrbStack's emulated x86 VM is UNSTABLE under the
# full 188-file load ("unexpected EOF" / VM exit) -> gives incomplete compiles.
# For a reliable full build+link use a NATIVE x86_64 Linux host (VPS / real PC)
# with docker centos:7. The scaffold + this script are host-portable.
# ============================================================================
set -uo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
IMAGE="${1:-jx3build}"

# DEAD set (18 file) — old class-based AI + tàn dư của nó. Server thật (binary 2012)
# chỉ chứa VM AI (KAIAction/KAILogic/KAIVM/...), 0 class-AI (verify bằng strings).
# +KAI_Player: kế thừa KAIBase, vắng binary -> chính nó tạo 18 KAIBase::* undefined ở link.
# +KMissile/KPathFinder/KTrackList: chết (forward-decl/chỉ-KAIBase-dùng/no-caller, vắng binary)
# -> loại thay vì vá phỏng đoán. Xem DECISION.md §B2,§D.
DEAD="KAIBase KAIBaseTactics KAIControl KAIEvent KLuaAIEvent KAISkillSelect \
KAI_Negative KAI_Critter KAI_Monk KAI_Positive KAI_Retardate KAI_Wolf KAI_WolfKing KAI_Wood \
KAI_Player KMissile KPathFinder KTrackList KAIParamTemplateList \
KGodServer KGod"

docker run --rm --platform linux/amd64 -v "$HERE":/work "$IMAGE" bash -c '
set -uo pipefail
cd /work
# NOTE: the real final build (session 8d215a3e, cmd 306) did NOT create any
# include/Base/Engine|Common dir-symlinks — capital-case includes ("Engine/X.h")
# resolve through mkshim (-Ishim) instead. On the case-INSENSITIVE macOS mount
# those symlinks collide with the existing engine/ common/ dirs and make
# OrbStack virtiofs flake ("No such file" races), so they are intentionally gone.

M32="-m32 -I/usr/include/c++/4.8.2/i686-redhat-linux"
INC="-Isrc/SO3GameServer -Isrc/SO3World -Isrc/SO3World/Src -Ishim -Icompat \
-Iinclude/Include -Iinclude/Include/SO3World -Iinclude/Base -Iinclude/Base/engine \
-Iinclude/Base/common -Iinclude/Base/lua5 \
-Isrc/SO3Represent/Src -Iinclude/Include/SO3Represent"
FLAGS="$M32 -include compat/prelude.h -c -w -fpermissive -std=gnu++98 -D__linux -D_SERVER -D_STANDALONE $INC"
DEAD="'"$DEAD"'"

rm -rf obj; mkdir obj
ok=0; fail=0
for f in src/SO3World/Src/*.cpp; do
  b=$(basename "$f" .cpp)
  echo "$DEAD" | grep -qw "$b" && continue
  if g++ $FLAGS "$f" -o obj/"$b".o 2>/dev/null; then ok=$((ok+1)); else fail=$((fail+1)); echo "  FAIL $b"; fi
done
for f in src/SO3GameServer/Main.cpp src/SO3GameServer/KSO3GameServer.cpp \
         src/SO3GameServer/KGameServerEyes.cpp src/SO3GameServer/stdafx.cpp; do
  b=$(basename "$f" .cpp)
  g++ $FLAGS "$f" -o obj/glue_"$b".o 2>/dev/null && ok=$((ok+1)) || { fail=$((fail+1)); echo "  FAIL glue $b"; }
done
# [R7] The leaked common archive is the source of truth for socket/security,
# including AcceptSecurity/ConnectSecurity and KG_SecuritySocketStream.  Do not
# compile the old pass-through reconstruction: merely linking the archive is
# insufficient if a recon object satisfies those symbols first.
for m in crc32_shim; do
  g++ $FLAGS src/common_recon/$m.cpp -o obj/recon_$m.o 2>/dev/null && ok=$((ok+1)) || echo "  FAIL recon $m"
done
echo "=== COMPILE: ok=$ok fail=$fail  objects=$(ls obj/*.o 2>/dev/null | wc -l) ==="

echo "=== LINK ==="
# Xuất ra /work (= linux-build/ trên host, persistent) thay vì /tmp trong container.
g++ -m32 obj/*.o libs/libcommon.a -L libs -lEngine_Lua5D -lSO3EnumConvertorD -lSO3ItemHouseD \
    -llzo2 -ldl -lpthread -o /work/SO3GameServer 2>/tmp/le
echo "link exit=$?  undefined refs: $(grep -c "undefined reference" /tmp/le)"
echo "--- distinct undefined symbols ---"
grep "undefined reference" /tmp/le | sed -E "s/.*undefined reference to .//; s/.$//" | c++filt | sort -u | head -60
[ -f /work/SO3GameServer ] && { echo "=== BINARY PRODUCED ==="; file /work/SO3GameServer; }
'
