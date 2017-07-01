#!/bin/bash
cd `dirname $0`

if [ $# -lt 4 ];then
	echo 'please input argv[1]=server_id, argv[2]=client_port, argv[3]=shm_addr, argv[4]=player_num'
	exit
fi

mysql_host=127.0.0.1
mysql_db_pwd=123456
#logger_mysql_host=10.10.111.246
#logger_mysql_db_name=VGame_logger_android
#logger_mysql_db_pwd=ghz_mysql_888
conn_redis_port=6379
conn_redis_timeout=5000
conn_redis_ip=127.0.0.1

serverid=$1
port=$2
shm_addr=`printf "%d" ${3}`
player_num=$4
monster_num=`expr 100 + ${player_num} \* 10`
boss_num=`expr ${monster_num} / 20`

echo "/// SERVER ID"
echo "game_srv_id=${serverid}"
echo "/// 是否打开gm指令,外网运营环境不能打开"
echo "open_gm_cmd=0"
echo "/// mysql数据库配置"
echo "mysql_host=${mysql_host}"
echo "mysql_port=3306"
echo "mysql_db_name=xgame${serverid}"
echo "mysql_db_flow_record=flow_record${serverid}"
echo "mysql_db_user=root"
echo "mysql_db_pwd=${mysql_db_pwd}"

echo "/// redis数据库配置"
echo "conn_redis_port=${conn_redis_port}"
echo "conn_redis_timeout=${conn_redis_timeout}"
echo "conn_redis_ip=${conn_redis_ip}"

echo "//展示服务器和游戏服连接端口"
echo "item_srv_game_port=11555"
echo "//展示服务器连接端口"
echo "item_srv_port=11556"


echo "//客户端连接的端口，也是需要对外开放的tcp端口"
echo "conn_srv_client_port=${port}"
port=`expr ${port} + 1`
echo "//游戏服和连接服通讯的端口"
echo "conn_srv_game_port=${port}"
port=`expr ${port} + 1`
echo "//登录服和连接服通讯的端口"
echo "conn_srv_login_port=${port}"
port=`expr ${port} + 1`
echo "//排行榜服和连接服通讯的端口"
echo "conn_srv_rank_port=${port}"
port=`expr ${port} + 1`
echo "//DB服和游戏服通讯的端口"
echo "db_srv_port=${port}"
port=`expr ${port} + 1`
echo "//邮件服和连接服通讯的端口"
echo "conn_srv_mail_port=${port}"
port=`expr ${port} + 1`
echo "//排行版日奖励发送服务"
echo "conn_srv_rank_reward_port=${port}"
port=`expr ${port} + 1`
echo "//php管理端连接端口"
echo "conn_srv_php_admin_port=${port}"
port=`expr ${port} + 1`
echo "//好友服务器连接端口"
echo "conn_srv_friend_port=${port}"
port=`expr ${port} + 1`
echo "//公会服务器连接端口"
echo "conn_srv_guild_port=${port}"
port=`expr ${port} + 1`
echo "//游戏服务器web端口"
echo "game_srv_web_port=${port}"
port=`expr ${port} + 1`
echo "//调试服务器连接端口"
echo "conn_srv_dump_port=${port}"
port=`expr ${port} + 1`



echo "game_srv_tick_time = 50"

echo "/// 共享内存"
echo -n "game_srv_player_key=0x"
echo "obase=16;${shm_addr}" | bc
((shm_addr=${shm_addr}+1))
echo "game_srv_player_num=${player_num}"

echo -n "game_srv_equip_key=0x"
echo "obase=16;${shm_addr}" | bc
((shm_addr=${shm_addr}+1))
echo "game_srv_equip_num=`expr ${player_num} \* 10`"

echo -n "game_srv_raid_key=0x"
echo "obase=16;${shm_addr}" | bc
((shm_addr=${shm_addr}+1))
echo "game_srv_raid_num=`expr ${player_num} / 2`"

echo -n "game_srv_skill_key=0x"
echo "obase=16;${shm_addr}" | bc
((shm_addr=${shm_addr}+1))
echo "game_srv_skill_num=`expr ${player_num} \* 20`"

echo -n "game_srv_buff_key=0x"
echo "obase=16;${shm_addr}" | bc
((shm_addr=${shm_addr}+1))
echo "game_srv_buff_num=`expr ${player_num} \* 100`"

echo -n "game_srv_monster_key=0x"
echo "obase=16;${shm_addr}" | bc
((shm_addr=${shm_addr}+1))
echo "game_srv_monster_num=${monster_num}"

echo -n "game_srv_boss_key=0x"
echo "obase=16;${shm_addr}" | bc
((shm_addr=${shm_addr}+1))
echo "game_srv_boss_num=${boss_num}"

echo -n "game_srv_sight_space_key=0x"
echo "obase=16;${shm_addr}" | bc
((shm_addr=${shm_addr}+1))
echo "game_srv_sight_space_num=${player_num}"

echo -n "game_srv_team_key=0x"
echo "obase=16;${shm_addr}" | bc
((shm_addr=${shm_addr}+1))
echo "game_srv_team_num=${player_num}"

echo -n "game_srv_zhenying_raid_key=0x"
echo "obase=16;${shm_addr}" | bc
((shm_addr=${shm_addr}+1))
echo "game_srv_zhenying_raid_num=30"

echo -n "game_srv_guild_wait_raid_key=0x"
echo "obase=16;${shm_addr}" | bc
((shm_addr=${shm_addr}+1))
echo "game_srv_guild_wait_raid_num=200"

echo -n "global_data_key=0x"
echo "obase=16;${shm_addr}" | bc
((shm_addr=${shm_addr}+1))

echo -n "game_srv_partner_key=0x"
echo "obase=16;${shm_addr}" | bc
((shm_addr=${shm_addr}+1))
echo "game_srv_partner_num=`expr ${player_num} \* 100`"

echo -n "game_srv_truck_key=0x"
echo "obase=16;${shm_addr}" | bc
((shm_addr=${shm_addr}+1))
echo "game_srv_truck_num=${player_num}"


