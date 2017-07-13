#!/bin/sh
cd `dirname $0`
#sleep 2
filename="`pwd`/server_info.ini"

# user1信号, 断开game_srv连接
kill -10 `cat conn_srv/pid.txt`

# 等待game_srv退出
if [ -f 'game_srv/pid.txt' ];then
	until [ ! -d "/proc/`cat game_srv/pid.txt`" ]
	do  
		echo 'sleep 1'
		sleep 1   
	done  
fi

# conn_srv退出
kill -9 `cat conn_srv/pid.txt`

# db_srv 退出
kill -9 `cat db_srv/pid.txt`

# 清理共享内存
grep -F 'key' $filename | grep -v 'zhuizhai_key'|awk -F'=' '{print $NF}'|while read key;do ipcrm -M $key ;done

ulimit -c unlimited

cd conn_srv
./conn_srv -d

cd ../db_srv
if [ ! -f 'pid.txt' ] || [ ! -d "/proc/`cat pid.txt`" ]; then
	./db_srv -d
fi

sleep 2

cd ../game_srv
./game_srv -d -t

cd ../login_srv
./login_srv -d

cd ../friend_srv
./friend_srv -d

cd ../mail_srv
./mail_srv -d

cd ../guild_srv
./guild_srv -d

cd ../rank_srv
./rank_srv -d

cd ..
./show_all_pid.sh 
./check_srv_alive.sh

