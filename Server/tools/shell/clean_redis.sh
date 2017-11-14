#!/bin/bash

cd `dirname $0`

if [ $# -lt 1 ];then
	echo 'pelase input argv[1]=server_id'
	exit 0
fi

server_id=$1
host=127.0.0.1
port=6379

function del_redis() {
    if [ "$1" == "" ]; then
#	echo "not del redis"
	return 0
    fi
#    echo $@

    for i in $@; do
#	echo "$i" | awk '{print "del " $0}' ;
	echo "$i" | awk '{print "del " $0}' | redis-cli -h ${host} -p ${port}
    done

#    echo "$1" | awk '{print "del " $0}' 
#    
}


t=`redis-cli -h ${host} -p ${port} keys  "server*_${server_id}"`
del_redis $t

t=`redis-cli -h ${host} -p ${port} keys  "s${server_id}_*"`
del_redis $t

t=`redis-cli -h ${host} -p ${port} keys  "*${server_id}"`
del_redis $t

#redis-cli -h ${host} -p ${port} --scan --pattern "server*_${server_id}" | awk '{print "del " $0}' | redis-cli -h ${host} -p ${port}
#redis-cli -h ${host} -p ${port} --scan --pattern "s${server_id}_*" | awk '{print "del " $0}' | redis-cli -h ${host} -p ${port}
#redis-cli -h ${host} -p ${port} --scan --pattern "*${server_id}" | awk '{print "del " $0}' | redis-cli -h ${host} -p ${port}
