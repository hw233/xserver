#!/bin/sh
export ASAN_OPTIONS=detect_leaks=true:detect_stack_use_after_return=true:check_initialization_order=true:log_path=./memerr.log
#export LSAN_OPTIONS=suppressions=/home/jacktang/svnroot/XGame/Server/suppr.txt
export LSAN_OPTIONS=suppressions=../suppr.txt
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
./game_srv -d

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

cd ../doufachang_srv
./doufachang_srv -d

cd ..
./show_all_pid.sh 
./check_srv_alive.sh

a=`find . -type f -name memerr*  | xargs grep  -H ERROR | awk -F":" '{print $1}'`
if [ -z "$a" ]; then
    find . -type f -name "memerr*" | xargs rm -f    
else
    find . -type f -name "memerr*" | grep -v "$a" | xargs rm -f    
fi    

