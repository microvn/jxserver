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
# Usage: [PUBIP=1.2.3.4] [GSBIN=SO3GameServer] cluster-vps.sh up|down|status|logs
set -u
: "${PUBIP:?set PUBIP to the public server IP}"
: "${MYSQL_ROOT_PASSWORD:?set MYSQL_ROOT_PASSWORD before starting the cluster}"
PUBIP=$PUBIP
GSBIN=${GSBIN:-SO3GameServer}          # gốc: SO3GameServer | rebuild: SO3GameServer_ours
DEPLOY=/root/jx3/镜像端/extracted/root
SD="$(cd "$(dirname "$0")" && pwd)"
CONF="$SD/vps-conf"
RUN="$SD/vps-conf/.run"

gen_conf(){
  mkdir -p "$RUN"
  cp "$CONF/gateway.ini" "$CONF/gs_settings.ini" "$CONF/relay_settings.ini" "$RUN/"
  sed -i "s/__PUBIP__/$PUBIP/g" "$RUN/gs_settings.ini"
  esc_mysql_password=$(printf '%s' "$MYSQL_ROOT_PASSWORD" | sed 's/[\\/&]/\\\\&/g')
  sed -i "s/__MYSQL_ROOT_PASSWORD__/$esc_mysql_password/g" "$RUN/relay_settings.ini"
}
MNT(){ echo "-v $RUN/gateway.ini:/deploy/gateway.ini -v $RUN/gs_settings.ini:/deploy/gs_settings.ini -v $RUN/relay_settings.ini:/deploy/relay_settings.ini"; }

fw_on(){  # chặn cổng nội bộ 5003 (center bind 0.0.0.0) khỏi internet, chừa loopback cho GS↔center
  iptables -C INPUT -p tcp --dport 5003 ! -i lo -j DROP 2>/dev/null || iptables -I INPUT -p tcp --dport 5003 ! -i lo -j DROP; }
fw_off(){ iptables -D INPUT -p tcp --dport 5003 ! -i lo -j DROP 2>/dev/null; }

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

case "${1:-}" in
 up)
  docker rm -f jx3mysql jx3center jx3gw jx3gs 2>/dev/null
  gen_conf
  echo "PUBIP=$PUBIP  GSBIN=$GSBIN  (client nối login tới $PUBIP:5004)"
  docker run -d --name jx3mysql --network host --cap-add=NET_ADMIN --restart unless-stopped \
    -e MYSQL_ROOT_PASSWORD="$MYSQL_ROOT_PASSWORD" -e MYSQL_DATABASE=jx3_25 \
    mysql:5.6 --lower-case-table-names=1 --max-allowed-packet=20M --bind-address=127.0.0.1 >/dev/null
  echo -n "mysql init"; for i in $(seq 1 40); do sleep 2; docker exec -e MYSQL_PWD="$MYSQL_ROOT_PASSWORD" jx3mysql mysqladmin -uroot ping 2>/dev/null|grep -q alive && break; echo -n .; done; echo
  docker exec -e MYSQL_PWD="$MYSQL_ROOT_PASSWORD" jx3mysql mysql -uroot -e "GRANT ALL PRIVILEGES ON *.* TO 'root'@'127.0.0.1' IDENTIFIED BY '$MYSQL_ROOT_PASSWORD'; GRANT ALL PRIVILEGES ON *.* TO 'root'@'%' IDENTIFIED BY '$MYSQL_ROOT_PASSWORD'; FLUSH PRIVILEGES;" 2>/dev/null
  run jx3center SO3GameCenter; echo "center up"; sleep 16
  run jx3gw SO3Gateway;        echo "gateway up"; sleep 8
  run jx3gs "$GSBIN";          echo "gameserver up ($GSBIN)"
  fw_on; echo "firewall: 5003 blocked from public (5004+3113 open)"
  ;;
 down) docker rm -f jx3mysql jx3center jx3gw jx3gs 2>/dev/null; fw_off; echo down;;
 status) docker ps --filter name=jx3 --format "{{.Names}}: {{.Status}}"; for c in jx3center jx3gw jx3gs; do echo -n "$c procs="; docker exec "$c" pgrep -c "SO3" 2>/dev/null||echo 0; done;;
 logs) cd "$DEPLOY"; for d in SO3GameCenter SO3Gateway SO3GameServer; do echo "=== $d ==="; L=$(ls -t logs/$d/*/*.log 2>/dev/null|head -1); grep -avE "]:Get" "$L" 2>/dev/null|tail -8; done;;
 *) echo "usage: [PUBIP=1.2.3.4] [GSBIN=SO3GameServer] $0 up|down|status|logs";;
esac
