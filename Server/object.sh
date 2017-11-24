#!/bin/bash

port=`cat server_info.ini | grep game_srv_web_port | sed 's/game_srv_web_port=//g'`
if [ $# -gt 0 ];then
    port=`cat server_info.ini | grep raid_srv_web_port | sed 's/raid_srv_web_port=//g'`    
fi

#echo $port
command="curl -s http://127.0.0.1:${port}/object"
#echo $command
object=`${command}`
echo ${object} | sed 's/<br><br>[ ]*/\n/g'
