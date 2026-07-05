#!/usr/bin/env bash
# ============================================================================
# JX3 v2.5.2 — deploy the freshly-built SO3GameServer into the real data tree
# (镜像端/extracted/root) and boot it to see how far runtime gets.
# Runs inside the jx3build (centos7 / m32) container so glibc/locale match.
# Copies to SO3GameServer_ours — never overwrites the original stripped 2.5.2
# binary (that is our Ghidra ground-truth reference).
# ============================================================================
set -uo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
JX3ROOT="$(cd "$HERE/.." && pwd)"
DEPLOY="$JX3ROOT/镜像端/extracted/root"
IMAGE="${1:-jx3build}"
SECS="${2:-60}"

[ -f "$HERE/SO3GameServer" ] || { echo "no binary at $HERE/SO3GameServer — run ./build.sh"; exit 1; }
[ -d "$DEPLOY" ] || { echo "deploy tree missing: $DEPLOY"; exit 1; }

docker run --rm --platform linux/amd64 -v "$HERE":/work -v "$DEPLOY":/deploy "$IMAGE" bash -c '
set -uo pipefail
localedef -c -f GBK -i zh_CN zh_CN.gbk 2>/dev/null || true
cp /work/SO3GameServer /deploy/SO3GameServer_ours
chmod +x /deploy/SO3GameServer_ours
cd /deploy
echo "=== DEPLOY RUN (timeout '"$SECS"'s) ==="
LC_ALL=zh_CN.gbk LD_LIBRARY_PATH=. timeout '"$SECS"' ./SO3GameServer_ours 2>&1 | head -120
echo "=== EXIT ==="
'
