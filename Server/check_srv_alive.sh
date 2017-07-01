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
)

for srv_name in ${srv_list[*]}
do
	if [ -f "$srv_name/pid.txt" ] && [ -d "/proc/`cat $srv_name/pid.txt`" ]; then
		echo -e "\033[0m$srv_name alive\033[0m" | awk '{printf "%-15s %-5s\n", $1, $2}' 
	else
		echo -e "\033[31m$srv_name dead\033[0m" | awk '{printf "%-15s %-5s\n", $1, $2}'
	fi
done


