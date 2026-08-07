#!/bin/bash
# Materialise the PVP tier (Zone / Arena / Battlefield) into the deploy tree.
# Idempotent. Source = stock 2012 package, never rebuilt by us, so binaries keep
# their original names and a sha256 manifest is written next to them.
#
# Only latin/ASCII filenames are installed (DECISION §N2b: GBK names die on rsync).
#
# Usage: [SRC=...] [DST=...] deploy/pvp-install.sh
set -eu
SD="$(cd "$(dirname "$0")" && pwd)"
SRC=${SRC:-"$SD/../../download/PVPServer"}
DST=${DST:-"$SD/../../镜像端/extracted/root"}

test -d "$SRC" || { echo "SRC not found: $SRC" >&2; exit 1; }
test -d "$DST" || { echo "DST not found: $DST" >&2; exit 1; }

# ZoneServer ships debug-only in this package; there is no stripped release build.
BINS="ZoneServerD SO3ArenaServer SO3BattlefieldServer"
DATA="ArenaGroup.tab KG_CSLogServerSaveMethod.tab"

for f in $BINS; do
  test -f "$SRC/$f" || { echo "missing binary: $SRC/$f" >&2; exit 1; }
  cp -p "$SRC/$f" "$DST/$f"; chmod +x "$DST/$f"
done
for f in $DATA; do cp -p "$SRC/$f" "$DST/$f"; done

mkdir -p "$DST/arena_scripts"
cp -p "$SRC/arena_scripts/main.lua" "$DST/arena_scripts/main.lua"

# Center-group tabs: rewrite the single group row to this cluster's center name
# (relay_settings.ini [Global] ServerName). Column count is preserved byte-exactly;
# the stock CN groups below it are left untouched (those centers never connect).
CENTER_NAME=${CENTER_NAME:-0}
for f in ZoneCenterGroupList.tab ZoneCenterGroupList_Arena.tab; do
  SRC_TAB="$SRC/$f" DST_TAB="$DST/$f" CN="$CENTER_NAME" python3 - <<'PY'
import os
src, dst, cn = os.environ['SRC_TAB'], os.environ['DST_TAB'], os.environ['CN']
# latin-1 round-trips arbitrary bytes; the file is GBK and must stay GBK.
lines = open(src, 'r', encoding='latin-1', newline='').read().split('\r\n')
cols = lines[1].split('\t')
cols[0] = cn
for i in range(1, len(cols)):
    cols[i] = ''
lines[1] = '\t'.join(cols)
open(dst, 'w', encoding='latin-1', newline='').write('\r\n'.join(lines))
PY
done

# Baseline ini in the tree, same pattern as gs_settings.ini/relay_settings.ini: present
# here so the binaries are runnable standalone (cluster.sh, manual runs), while
# cluster-vps.sh mounts freshly generated copies over them.
for f in zoneserver.ini arena_server.ini battlefield_server.ini; do
  cp -p "$SD/vps-conf/$f" "$DST/$f"
done
if [ -n "${MYSQL_ROOT_PASSWORD:-}" ]; then
  esc=$(printf '%s' "$MYSQL_ROOT_PASSWORD" | sed 's/[\\/&]/\\\\&/g')
  sed -i.bak "s/__MYSQL_ROOT_PASSWORD__/$esc/g" "$DST/arena_server.ini" && rm -f "$DST/arena_server.ini.bak"
  PW_NOTE="arena_server.ini: MySQL password filled from env"
else
  PW_NOTE="arena_server.ini: __MYSQL_ROOT_PASSWORD__ left unresolved (cluster-vps.sh substitutes it; fill by hand for cluster.sh/manual runs)"
fi

( cd "$DST" && shasum -a 256 $BINS 2>/dev/null || sha256sum $BINS ) > "$DST/pvp-manifest.sha256"

echo "PVP tier installed into $DST"
echo "  binaries : $BINS"
echo "  center   : CenterName1 = $CENTER_NAME in ZoneCenterGroupList{,_Arena}.tab"
echo "  manifest : pvp-manifest.sha256"
echo "  $PW_NOTE"
echo "note: RemoteCenterConstList.ini / CenterConstList.ini / settings/*.tab / libEngine_Lua5.so"
echo "      are NOT overwritten - the deploy tree already carries the 2.5.2 versions."
