#!/bin/sh
cd `dirname $0`
#sleep 2
filename="`pwd`/../server_info.ini"
grep -F 'key' $filename |awk -F'=' '{print $NF}'|while read key;do ipcrm -M $key ;done

