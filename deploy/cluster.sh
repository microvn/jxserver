#!/bin/bash
# JX3 3-tier cluster boot (verified 2026-07-05). MySQL 5.6 in isolated netns;
# center/gateway/gameserver share that netns; 192.168.200.105 aliased on lo.
# Usage: cluster.sh up|down|status|logs
set -u
: "${MYSQL_ROOT_PASSWORD:?set MYSQL_ROOT_PASSWORD before starting the cluster}"
DEPLOY=/root/jx3/镜像端/extracted/root
GSBIN=${GSBIN:-SO3GameServer_ours}   # SO3GameServer_ours (rebuilt) | SO3GameServer (stock)
run(){ docker run -d --name $1 --network=container:jx3mysql -v "$DEPLOY":/deploy jx3build bash -c \
 "localedef -c -f GBK -i zh_CN zh_CN.gbk 2>/dev/null||true; cd /deploy; LC_ALL=zh_CN.gbk LD_LIBRARY_PATH=. ./$2 >/tmp/$1.log 2>&1 & sleep 100000" >/dev/null; }
case "$1" in
 up)
  docker rm -f jx3mysql jx3center jx3gw jx3gs 2>/dev/null
  docker run -d --name jx3mysql --cap-add=NET_ADMIN -e MYSQL_ROOT_PASSWORD="$MYSQL_ROOT_PASSWORD" -e MYSQL_DATABASE=jx3_25 \
    mysql:5.6 --lower-case-table-names=1 --max-allowed-packet=20M >/dev/null
  echo -n "mysql init"; for i in $(seq 1 40); do sleep 2; docker exec -e MYSQL_PWD="$MYSQL_ROOT_PASSWORD" jx3mysql mysqladmin -uroot ping 2>/dev/null|grep -q alive && break; echo -n .; done; echo
  docker exec -e MYSQL_PWD="$MYSQL_ROOT_PASSWORD" jx3mysql mysql -uroot -e "GRANT ALL PRIVILEGES ON *.* TO 'root'@'127.0.0.1' IDENTIFIED BY '$MYSQL_ROOT_PASSWORD'; GRANT ALL PRIVILEGES ON *.* TO 'root'@'%' IDENTIFIED BY '$MYSQL_ROOT_PASSWORD'; FLUSH PRIVILEGES;" 2>/dev/null
  PID=$(docker inspect -f "{{.State.Pid}}" jx3mysql); nsenter -t $PID -n ip addr add 192.168.200.105/32 dev lo 2>/dev/null
  run jx3center SO3GameCenter; echo "center up"; sleep 16
  run jx3gw SO3Gateway; echo "gateway up"; sleep 8
  run jx3gs $GSBIN; echo "gameserver up ($GSBIN)"
  ;;
 down) docker rm -f jx3mysql jx3center jx3gw jx3gs 2>/dev/null; echo down;;
 status) docker ps --filter name=jx3 --format "{{.Names}}: {{.Status}}"; for c in jx3center jx3gw jx3gs; do echo -n "$c procs="; docker exec $c pgrep -c "SO3" 2>/dev/null||echo 0; done;;
 logs) cd $DEPLOY; for d in SO3GameCenter SO3Gateway SO3GameServer; do echo "=== $d ==="; L=$(ls -t logs/$d/*/*.log 2>/dev/null|head -1); grep -avE "]:Get" "$L" 2>/dev/null|tail -8; done;;
 *) echo "usage: $0 up|down|status|logs";;
esac
