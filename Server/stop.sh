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
kill -9 `cat db_srv/pid.txt`

# 清理共享内存
grep -F 'key' $filename | grep -v 'zhuizhai_key'|awk -F'=' '{print $NF}'|while read key;do ipcrm -M $key ;done

