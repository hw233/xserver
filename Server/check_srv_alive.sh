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
)

for srv_name in ${srv_list[*]}
do
	if [ -f "$srv_name/pid.txt" ] && [ -d "/proc/`cat $srv_name/pid.txt`" ]; then
		echo -e "$srv_name \e[32malive\e[0m" | awk '{printf "%-20s %-5s\n", $1, $2}' 
	else
		echo -e "$srv_name \e[31mdead\e[0m" | awk '{printf "%-20s %-5s\n", $1, $2}'
	fi
done


