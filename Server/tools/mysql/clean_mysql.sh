#!/bin/bash

cd `dirname $0`

if [ $# -lt 1 ];then
	echo 'pelase input argv[1]=server_id'
	exit 0
fi

server_id=$1
host=127.0.0.1
port=3306
user=root
password=123456


mysql -h${host} -P${port} -u${user} -p${password} xgame${server_id} < clean_mysql.sql
