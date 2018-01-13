#!/bin/sh
cd `dirname $0`

if [ $# -lt 1 ];then 
	echo 'please input argv[1]=server_id'
	exit 0
fi 
server_id=$1
./mysql/clean_mysql.sh ${server_id} 
./shell/clean_redis.sh ${server_id}
cd python/
./doufachang_init_player.py 0
