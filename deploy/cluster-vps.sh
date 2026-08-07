#!/bin/bash
# JX3 3-tier cluster — PUBLIC/VPS mode (host networking, stock binaries OK).
#
# Vì sao khác cluster.sh: binary JX3 dùng CHUNG một giá trị [Player] IP cho cả
# bind lẫn "advertise" (IP mà server bảo client nối tới để vào thế giới game).
# Không sửa được code (dùng SO3GameServer gốc) => cách duy nhất để 1 IP vừa
# bind-được vừa advertise-đúng vừa reachable từ ngoài là cho container dùng CHUNG
# network namespace của host (--network host), nơi eth0 đã sẵn public IP.
#   - gs_settings [Player] IP = $PUBIP  -> bind eth0 thật + advertise đúng
#   - gateway   [Player] IP = (trống)   -> bind mọi interface (0.0.0.0)
#   - mọi link nội bộ (Relay/MySQL/Log) -> 127.0.0.1 (chung host netns)
# MySQL vẫn là CONTAINER (không cài gì lên host), chỉ nghe 127.0.0.1:3306.
# --cap-add=NET_ADMIN BẮT BUỘC: host-net không kế thừa cap; center cần
# CAP_NET_ADMIN cho setsockopt(SO_RCVBUFFORCE) ở accept-validator (blocker §R9),
# thiếu => EPERM => RST => "Game center lost".
#
# Config gốc trong tree KHÔNG bị đụng: script sinh 3 file ini từ vps-conf/*.ini
# (thay __PUBIP__) rồi bind-mount ĐÈ lên /deploy chỉ trong container.
#
# FIREWALL (tự làm, script không đụng iptables của host):
#   chỉ mở inbound TCP 5004 (gateway) + 3113 (gameserver).
#   CHẶN 9001/5003/3306/5100 khỏi internet — host-net không có cô lập Docker,
#   firewall là tuyến bảo vệ DUY NHẤT.
#
# Usage: [PUBIP=1.2.3.4] [GSBIN=SO3GameServer_STOCK_DONTREMOVE] cluster-vps.sh up|down|status|logs
set -u
: "${PUBIP:?set PUBIP to the public server IP}"
: "${MYSQL_ROOT_PASSWORD:?set MYSQL_ROOT_PASSWORD before starting the cluster}"
PUBIP=$PUBIP
GSBIN=${GSBIN:-SO3GameServer_STOCK_DONTREMOVE} # stock; rebuild: explicit candidate name
DEPLOY=/root/jx3/镜像端/extracted/root
DBROOT=${DBROOT:-/root/jx3/mysql56-data}
SD="$(cd "$(dirname "$0")" && pwd)"
CONF="$SD/vps-conf"
RUN="$SD/vps-conf/.run"

gen_conf(){
  mkdir -p "$RUN"
  cp "$CONF/gateway.ini" "$CONF/gs_settings.ini" "$CONF/relay_settings.ini" \
     "$CONF/zoneserver.ini" "$CONF/arena_server.ini" "$CONF/battlefield_server.ini" "$RUN/"
  sed -i "s/__PUBIP__/$PUBIP/g" "$RUN/gs_settings.ini"
  esc_mysql_password=$(printf '%s' "$MYSQL_ROOT_PASSWORD" | sed 's/[\\/&]/\\\\&/g')
  sed -i "s/__MYSQL_ROOT_PASSWORD__/$esc_mysql_password/g" "$RUN/relay_settings.ini" "$RUN/arena_server.ini"
}
ensure_dbroot(){
  mkdir -p "$DBROOT"
}
MNT(){ echo "-v $RUN/gateway.ini:/deploy/gateway.ini -v $RUN/gs_settings.ini:/deploy/gs_settings.ini -v $RUN/relay_settings.ini:/deploy/relay_settings.ini -v $RUN/zoneserver.ini:/deploy/zoneserver.ini -v $RUN/arena_server.ini:/deploy/arena_server.ini -v $RUN/battlefield_server.ini:/deploy/battlefield_server.ini"; }

# PVP tier (opt out with PVP=0). Zone is a hub: center/arena/battlefield all dial OUT
# to it on 9111, so 9111 is the only new listener. Zone ships debug-only in the 2012
# package, hence ZoneServerD.
PVP=${PVP:-1}
PVP_BINS="ZoneServerD SO3ArenaServer SO3BattlefieldServer"
pvp_ready(){
  [ "$PVP" = 1 ] || return 1
  for b in $PVP_BINS; do [ -x "$DEPLOY/$b" ] || return 1; done
  return 0
}
pvp_warn(){
  echo "PVP tier SKIPPED: missing $(for b in $PVP_BINS; do [ -x "$DEPLOY/$b" ] || printf '%s ' "$b"; done)in $DEPLOY"
  echo "  fix: run deploy/pvp-install.sh on the machine holding download/PVPServer, then sync the deploy tree"
}

# 5003 = center<->GS, 9111 = zone hub. Both bind wide; keep them off the internet.
fw_on(){
  for p in 5003 9111; do
    iptables -C INPUT -p tcp --dport $p ! -i lo -j DROP 2>/dev/null || iptables -I INPUT -p tcp --dport $p ! -i lo -j DROP
  done; }
fw_off(){ for p in 5003 9111; do iptables -D INPUT -p tcp --dport $p ! -i lo -j DROP 2>/dev/null; done; }

run(){ # $1=container name  $2=binary
  # Binary TỰ daemon-hoá: launcher fork ra daemon rồi parent exit 0 sau 0-2s.
  # => KHÔNG foreground/exec được (PID1 thoát 0 => container thoát => mất daemon,
  # crash-loop). Chạy nền rồi GIÁM SÁT: PID1(bash) sống chừng nào daemon còn;
  # daemon chết => vòng lặp thoát => container thoát => docker (--restart) bật lại.
  # Thay cho "sleep 100000" cũ (giữ container sống MÙ QUÁNG kể cả khi game đã chết).
  # pgrep -x khớp comm (<=15 ký tự) để KHÔNG tự-khớp cmdline của chính bash.
  local m; m=$(echo "$2" | cut -c1-15)
  docker run -d --name "$1" --network host --cap-add=NET_ADMIN --restart unless-stopped \
    -v "$DEPLOY":/deploy $(MNT) jx3build bash -c \
    "localedef -c -f GBK -i zh_CN zh_CN.gbk 2>/dev/null||true; cd /deploy; export LC_ALL=zh_CN.gbk LD_LIBRARY_PATH=.; ./$2 >/tmp/$1.log 2>&1; sleep 10; while pgrep -x '$m' >/dev/null 2>&1; do sleep 15; done" >/dev/null
}
up_mysql(){
  ensure_dbroot
  docker rm -f jx3mysql 2>/dev/null
  docker run -d --name jx3mysql --network host --cap-add=NET_ADMIN --restart unless-stopped \
    -v "$DBROOT":/var/lib/mysql \
    -e MYSQL_ROOT_PASSWORD="$MYSQL_ROOT_PASSWORD" -e MYSQL_DATABASE=jx3_25 \
    mysql:5.6 --lower-case-table-names=1 --max-allowed-packet=20M --bind-address=127.0.0.1 >/dev/null
  echo -n "mysql init"; for i in $(seq 1 40); do sleep 2; docker exec -e MYSQL_PWD="$MYSQL_ROOT_PASSWORD" jx3mysql mysqladmin -uroot ping 2>/dev/null|grep -q alive && break; echo -n .; done; echo
  docker exec -e MYSQL_PWD="$MYSQL_ROOT_PASSWORD" jx3mysql mysql -uroot -e "GRANT ALL PRIVILEGES ON *.* TO 'root'@'127.0.0.1' IDENTIFIED BY '$MYSQL_ROOT_PASSWORD'; GRANT ALL PRIVILEGES ON *.* TO 'root'@'%' IDENTIFIED BY '$MYSQL_ROOT_PASSWORD'; FLUSH PRIVILEGES;" 2>/dev/null
}
up_app(){
  gen_conf
  docker rm -f jx3zone jx3arena jx3bf jx3center jx3gw jx3gs 2>/dev/null
  if pvp_ready; then
    run jx3zone ZoneServerD; echo "zone up"; sleep 4     # hub must precede its clients
  elif [ "$PVP" = 1 ]; then pvp_warn; fi
  run jx3center SO3GameCenter; echo "center up"; sleep 16
  run jx3gw SO3Gateway;        echo "gateway up"; sleep 8
  run jx3gs "$GSBIN";          echo "gameserver up ($GSBIN)"
  if pvp_ready; then
    run jx3arena SO3ArenaServer;       echo "arena up (db jx3_tf self-provisions)"
    run jx3bf    SO3BattlefieldServer; echo "battlefield up"
  fi
  fw_on; echo "firewall: 5003+9111 blocked from public (5004+3113 open)"
}
down_app(){
  docker rm -f jx3zone jx3arena jx3bf jx3center jx3gw jx3gs 2>/dev/null
  fw_off
  echo down-app
}
restart_gs(){
  gen_conf
  docker rm -f jx3gs 2>/dev/null
  run jx3gs "$GSBIN"
  echo "gameserver restarted ($GSBIN)"
}

case "${1:-}" in
 up)
  echo "PUBIP=$PUBIP  GSBIN=$GSBIN  (client nối login tới $PUBIP:5004)"
  down_app
  docker rm -f jx3mysql 2>/dev/null
  up_mysql
  up_app
  ;;
 up-app)
  echo "PUBIP=$PUBIP  GSBIN=$GSBIN  (app tier only)"
  up_app
  ;;
 down) down_app; docker rm -f jx3mysql 2>/dev/null; echo down;;
 down-app) down_app;;
 restart-gs)
  restart_gs
  ;;
 status)
  docker ps --filter name=jx3 --format "{{.Names}}: {{.Status}}"
  for c in jx3center jx3gw jx3gs jx3zone jx3arena jx3bf; do
    docker inspect "$c" >/dev/null 2>&1 || continue
    echo -n "$c procs="; docker exec "$c" pgrep -c "SO3|ZoneServer" 2>/dev/null||echo 0
  done
  if [ "$PVP" = 1 ] && ! pvp_ready; then pvp_warn; fi
  ;;
 logs) cd "$DEPLOY"; for d in SO3GameCenter SO3Gateway SO3GameServer; do echo "=== $d ==="; L=$(ls -t logs/$d/*/*.log 2>/dev/null|head -1); grep -avE "]:Get" "$L" 2>/dev/null|tail -8; done
  for c in jx3zone jx3arena jx3bf; do docker inspect "$c" >/dev/null 2>&1 || continue; echo "=== $c ==="; docker exec "$c" tail -8 "/tmp/$c.log" 2>/dev/null; done;;
 pvp-logs)  # the bind handshake lives here: "[ModuleN] '<name>' bind!" vs "Name Conflict"
  for c in jx3zone jx3arena jx3bf; do echo "=== $c ==="; docker exec "$c" grep -aE "bind|Conflict|Protocol mismatch|Connect to Zone|Module" "/tmp/$c.log" 2>/dev/null | tail -15; done
  echo "=== center ZoneClient ==="; cd "$DEPLOY"; L=$(ls -t logs/SO3GameCenter/*/*.log 2>/dev/null|head -1); grep -aE "ZoneClient|\[Zone\]" "$L" 2>/dev/null|tail -15;;
 *) echo "usage: [PUBIP=1.2.3.4] [MYSQL_ROOT_PASSWORD=...] [GSBIN=SO3GameServer_STOCK_DONTREMOVE] [PVP=0] $0 up|up-app|down|down-app|restart-gs|status|logs|pvp-logs";;
esac
