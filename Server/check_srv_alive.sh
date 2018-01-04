#!/bin/bash
cd `dirname $0`

srv_list=(
conn_srv
game_srv
login_srv
friend_srv
db_srv
mail_srv
guild_srv
rank_srv
doufachang_srv
trade_srv
#raid_srv
activity_srv
)
cwd=`pwd -P`
for srv_name in ${srv_list[*]}
do
    if [ ! -f "$srv_name/pid.txt" ] || [ ! -d "/proc/`cat $srv_name/pid.txt`" ]; then
       echo -e "$srv_name \e[31mdead\e[0m" | awk '{printf "%-20s %-5s\n", $1, $2}'       
       continue
    fi
    
    pid=`cat $srv_name/pid.txt`
    if [ -z "$pid" ]; then
       echo -e "$srv_name \e[31mdead\e[0m" | awk '{printf "%-20s %-5s\n", $1, $2}'       
       continue
    fi
    
    t3=`ls -l  /proc/$pid/exe 2>/dev/null | grep "\-> $cwd/$srv_name/$srv_name"`
    if [ -z "$t3" ]; then
	echo -e "$srv_name \e[31mdead\e[0m" | awk '{printf "%-20s %-5s\n", $1, $2}'       
    else
	echo -e "$srv_name \e[32malive\e[0m" | awk '{printf "%-20s %-5s\n", $1, $2}' 	
    fi
done


