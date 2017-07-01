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
	echo -n "${srv_name}:  "
	if [ -f "${srv_name}/pid.txt" ]; then
		cat "${srv_name}/pid.txt"
		echo
	else
		echo "0"
	fi
done

