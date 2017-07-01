#!/bin/sh
cd `dirname $0`
#kill -9 `cat game_srv/pid.txt`
#sleep 2
#ulimit -c unlimited
#
#cd db_srv
#if [ ! -d "/proc/`cat pid.txt`" ]; then
#	./db_srv -d
#fi
#
#cd ../game_srv
#./game_srv -d -r


ulimit -c unlimited

# user1信号, 断开game_srv连接
#kill -10 `cat conn_srv/pid.txt`


# 等待game_srv退出
if [ -f 'game_srv/pid.txt' ];then
	if [ -d "/proc/`cat game_srv/pid.txt`" ];then
		kill -10 `cat conn_srv/pid.txt`
	fi

        until [ ! -d "/proc/`cat game_srv/pid.txt`" ]
        do
		
                echo 'sleep 1'
                sleep 1
		
	done
fi

cd db_srv
if [ ! -d "/proc/`cat pid.txt`" ]; then
       ./db_srv -d
fi

cd ../game_srv
./game_srv -d -r


