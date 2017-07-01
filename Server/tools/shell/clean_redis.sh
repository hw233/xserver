#!/bin/bash

cd `dirname $0`

if [ $# -lt 1 ];then
	echo 'pelase input argv[1]=server_id'
	exit 0
fi

server_id=$1
host=127.0.0.1
port=6379


redis-cli -h ${host} -p ${port} --scan --pattern "server*_${server_id}" | awk '{print "del " $0}' | redis-cli -h ${host} -p ${port}
redis-cli -h ${host} -p ${port} --scan --pattern "s${server_id}_*" | awk '{print "del " $0}' | redis-cli -h ${host} -p ${port}
