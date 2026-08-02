#!/usr/bin/env bash
# Switch the VPS GS between the immutable stock oracle and one candidate binary.
set -euo pipefail

ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
VPS_HOST=${VPS_HOST:-jx3-vps-2005}
REMOTE_ROOT=${REMOTE_ROOT:-/root/jx3}
REMOTE_DEPLOY="$REMOTE_ROOT/镜像端/extracted/root"
REMOTE_CLUSTER="$REMOTE_ROOT/linux-build/deploy/cluster-vps.sh"
STOCK_BIN=${STOCK_BIN:-SO3GameServer_STOCK_DONTREMOVE}
STOCK_SHA=${STOCK_SHA:-3002bf4ad08d4c0fb9d4fc10f2d66b1f6cd8f70d398ff792136a3b7d5c416f11}
CANDIDATE_PREFIX=${CANDIDATE_PREFIX:-SO3GameServer_CANDIDATE}

die(){ echo "error: $*" >&2; exit 1; }
q(){ printf '%q' "$1"; }
remote(){ ssh "$VPS_HOST" "$@"; }
require_runtime_env(){
  : "${PUBIP:?set PUBIP to the public server IP}"
  : "${MYSQL_ROOT_PASSWORD:?set MYSQL_ROOT_PASSWORD before switching GS}"
}

verify_stock(){
  local line sha
  line=$(remote "sha256sum $(q "$REMOTE_DEPLOY/$STOCK_BIN")")
  sha=${line%% *}
  test "$sha" = "$STOCK_SHA" || die "stock hash mismatch: $line"
  echo "stock=$STOCK_BIN sha256=$sha"
}

switch_remote(){
  local bin=$1
  require_runtime_env
  remote "GSBIN=$(q "$bin") PUBIP=$(q "$PUBIP") MYSQL_ROOT_PASSWORD=$(q "$MYSQL_ROOT_PASSWORD") bash $(q "$REMOTE_CLUSTER") restart-gs"
}

stock(){
  verify_stock
  switch_remote "$STOCK_BIN"
}

candidate(){
  local src=${1:-}
  if test -z "$src"; then
    src=$(ls -1t "$ROOT"/SO3GameServer_CANDIDATE_* 2>/dev/null | grep -v '\\(build\\.log\\|manifest\\)$' | head -1 || true)
  fi
  test -n "$src" || die "no timestamped candidate artifact found; run deploy/build-candidate.sh"
  test -f "$src" || die "candidate binary not found: $src"
  file "$src" | grep -q 'ELF 32-bit' || die "candidate is not an ELF32 binary: $src"

  local name="$CANDIDATE_PREFIX"_$(date +%Y%m%d_%H%M%S)
  local dst="$REMOTE_DEPLOY/$name"
  scp "$src" "$VPS_HOST:$dst" >/dev/null
  remote "chmod 755 $(q "$dst"); sha256sum $(q "$dst")"
  echo "candidate=$name source=$src"
  switch_remote "$name"
}

status(){
  remote "docker inspect -f '{{.Name}} status={{.State.Status}} restart={{.RestartCount}}' jx3gs jx3center 2>/dev/null || true"
  remote "docker top jx3gs -eo pid,args 2>/dev/null || true"
  remote "find $(q "$REMOTE_DEPLOY") -maxdepth 1 -type f -name 'SO3GameServer*' -printf '%f %s bytes\\n' | sort"
}

verify(){
  verify_stock
  status
}

logs(){
  remote "tail -n 120 /tmp/jx3gs.log 2>/dev/null || true"
}

usage(){
  cat <<EOF
Usage:
  $0 stock                 Switch to immutable stock GS
  $0 candidate [BINARY]   Upload and switch to timestamped candidate
  $0 status                Show service, process, and available GS artifacts
  $0 verify                Verify stock hash and show active topology
  $0 logs                  Show latest GS launcher log

Candidate default: newest $ROOT/SO3GameServer_CANDIDATE_<commit>_<datetime>
Requires PUBIP and MYSQL_ROOT_PASSWORD for stock/candidate switching.
EOF
}

case ${1:-} in
  stock) stock ;;
  candidate) candidate "${2:-}" ;;
  status) status ;;
  verify) verify ;;
  logs) logs ;;
  *) usage; exit 2 ;;
esac
