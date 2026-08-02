#!/usr/bin/env bash
# Build one committed source state and preserve the output as an immutable artifact.
set -euo pipefail

ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
IMAGE=${JX3_IMAGE:-jx3build}
TARGET=${JX3_TARGET_BIN:?set JX3_TARGET_BIN to the regular SO3GameServerD file}
test -f "$TARGET" || { echo "target is not a regular file: $TARGET" >&2; exit 2; }

if test -n "$(git -C "$ROOT" status --porcelain)" && test "${ALLOW_DIRTY:-0}" != 1; then
  echo "source tree is dirty; commit the change or set ALLOW_DIRTY=1 explicitly" >&2
  git -C "$ROOT" status --short >&2
  exit 2
fi

commit=$(git -C "$ROOT" rev-parse --short=12 HEAD)
stamp=$(date +%Y%m%d_%H%M%S)
artifact="$ROOT/SO3GameServer_CANDIDATE_${commit}_${stamp}"
log="$artifact.build.log"
manifest="$artifact.manifest"

JX3_TARGET_BIN="$TARGET" "$ROOT/build.sh" "$IMAGE" 2>&1 | tee "$log"
test -f "$ROOT/SO3GameServer" || { echo "build produced no SO3GameServer" >&2; exit 2; }
mv "$ROOT/SO3GameServer" "$artifact"
chmod 755 "$artifact"

binary_sha=$(shasum -a 256 "$artifact" | awk '{print $1}')
target_sha=$(shasum -a 256 "$TARGET" | awk '{print $1}')
cat > "$manifest" <<EOF
schema=jx3.candidate-artifact.v1
artifact=$(basename "$artifact")
source_commit=$commit
source_commit_full=$(git -C "$ROOT" rev-parse HEAD)
target_path=$TARGET
target_sha256=$target_sha
binary_sha256=$binary_sha
build_log=$(basename "$log")
created_at=$stamp
image=$IMAGE
EOF

printf 'artifact=%s\nsha256=%s\nmanifest=%s\n' "$artifact" "$binary_sha" "$manifest"
