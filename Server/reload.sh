#!/bin/bash

if [ $# -lt 1 ];then
#	echo 'please input argv[1] , for example reload'
#	exit
    t="reload"
else
    t=$1
fi


port=`cat server_info.ini | grep game_srv_web_port | sed 's/game_srv_web_port=//g'`
#echo $port
command="curl http://127.0.0.1:${port}/$t"
#echo $command
${command}
