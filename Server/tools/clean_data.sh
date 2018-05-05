#!/bin/sh
cd `dirname $0`

if [ $# -lt 1 ];then 
    server_id=`cat ../server_info.ini | grep game_srv_id | sed 's/.*=\([0-9]*\)/\1/g'`
else
    server_id=$1
fi 

echo "server_id = $server_id"
./mysql/clean_mysql.sh ${server_id} 
./shell/clean_redis.sh ${server_id}
cd python/
./doufachang_init_player.py 0
