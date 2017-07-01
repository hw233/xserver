cd `dirname $0`

if [ $# -lt 1 ];then
	echo 'pelase input argv[1]=server_id'
	exit 0
fi

server_id=$1

echo 'CREATE DATABASE IF NOT EXISTS `'"flow_record$server_id"'`;'
echo 'use`'"flow_record$server_id"'`;'

echo 'DROP TABLE IF EXISTS `'"client_to_conn_srv"'`;'
echo 'CREATE TABLE `'"client_to_conn_srv"'` ('
echo '	`msg_id` int(11) DEFAULT 0,'
echo '	`msg_num` int(11) DEFAULT 0,'
echo '	`msg_size` int(11) DEFAULT 0,'
echo '	`msg_sum_num` int(11) DEFAULT 0,'
echo '	`msg_sum_size` int(11) DEFAULT 0,'
echo '	PRIMARY KEY (`msg_id`)'
echo ") ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;"


echo 'DROP TABLE IF EXISTS `'"conn_srv_to_client"'`;'
echo 'CREATE TABLE `'"conn_srv_to_client"'` ('
echo '	`msg_id` int(11) DEFAULT 0,'
echo '	`msg_num` int(11) DEFAULT 0,'
echo '	`msg_size` int(11) DEFAULT 0,'
echo '	`msg_sum_num` int(11) DEFAULT 0,'
echo '	`msg_sum_size` int(11) DEFAULT 0,'
echo '	PRIMARY KEY (`msg_id`)'
echo ") ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;"


echo 'DROP TABLE IF EXISTS `'"other_srv_to_conn_srv"'`;'
echo 'CREATE TABLE `'"other_srv_to_conn_srv"'` ('
echo '	`msg_id` int(11) DEFAULT 0,'
echo '	`msg_num` int(11) DEFAULT 0,'
echo '	`msg_size` int(11) DEFAULT 0,'
echo '	`msg_sum_num` int(11) DEFAULT 0,'
echo '	`msg_sum_size` int(11) DEFAULT 0,'
echo '	PRIMARY KEY (`msg_id`)'
echo ") ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;"


echo 'DROP TABLE IF EXISTS `'"conn_srv_to_other_srv"'`;'
echo 'CREATE TABLE `'"conn_srv_to_other_srv"'` ('
echo '	`msg_id` int(11) DEFAULT 0,'
echo '	`msg_num` int(11) DEFAULT 0,'
echo '	`msg_size` int(11) DEFAULT 0,'
echo '	`msg_sum_num` int(11) DEFAULT 0,'
echo '	`msg_sum_size` int(11) DEFAULT 0,'
echo '	PRIMARY KEY (`msg_id`)'
echo ") ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;"


echo 'DROP TABLE IF EXISTS `'"game_srv_msg_time"'`;'
echo 'CREATE TABLE `'"game_srv_msg_time"'` ('
echo '	`msg_id` int(11) DEFAULT 0,'
echo '	`msg_num` int(11) DEFAULT 0,'
echo '	`msg_time` bigint(21) DEFAULT 0,'
echo '	PRIMARY KEY (`msg_id`)'
echo ") ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;"
