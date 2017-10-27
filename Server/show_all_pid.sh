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
)

for srv_name in ${srv_list[*]}
do
	if [ -f "${srv_name}/pid.txt" ]; then
	    echo -en "${srv_name}:  \e[33m`cat "${srv_name}/pid.txt"`\e[0m" | awk '{printf "%-20s %-5s\n", $1, $2}' 
	else
	    echo -en "${srv_name}:  \e33m0\e[0m" | awk '{printf "%-20s %-5s\n", $1, $2}' 
	fi
done

