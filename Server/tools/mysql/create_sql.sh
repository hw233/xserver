cd `dirname $0`

if [ $# -lt 1 ];then
	echo 'pelase input argv[1]=server_id'
	exit 0
fi

server_id=$1

echo 'CREATE DATABASE IF NOT EXISTS `'"xgame$server_id"'`;'
echo 'use`'"xgame$server_id"'`;'

auto_val=`echo $server_id|awk '{ printf "0x%x%08x\n", $0, 578}'|xargs printf "%d"`
echo 'DROP TABLE IF EXISTS `'"player"'`;'
echo 'CREATE TABLE `'"player"'` ('
echo '	`open_id` int(11) NOT NULL,'
echo '	`player_id` bigint(21) NOT NULL AUTO_INCREMENT,'
echo '	`player_name` varchar(50) COLLATE utf8_unicode_ci NOT NULL,'
echo '	`lv` int(4) DEFAULT '"'"1"'"','
echo '	`job` int(1) DEFAULT '"'"0"'"','
echo '	`logout_time` datetime DEFAULT NULL,'
echo '	`create_time` datetime DEFAULT NULL,'
echo '	`comm_data` blob,'
echo '  `chengjie_rest` bigint(20) DEFAULT '0','
echo '	PRIMARY KEY (`player_id`),'
echo '	UNIQUE KEY `player_name` (`player_name`),'
echo '	KEY `open_id` (`open_id`)'
echo ") ENGINE=MyISAM AUTO_INCREMENT=$auto_val DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;"

echo 'DROP TABLE IF EXISTS `'"showitem"'`;'
echo 'create table showitem(id INT NOT NULL AUTO_INCREMENT, type INT NOT NULL, data BLOB,PRIMARY KEY ( id ));'

echo 'DROP TABLE IF EXISTS `'"client_data"'`;'
echo 'CREATE TABLE `'"client_data"'` ('
echo '	`player_id` bigint(21) NOT NULL,'
echo '	`data` blob,'
echo '	PRIMARY KEY (`player_id`)'
echo ") ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;"

auto_val=`echo $server_id|awk '{ printf "0x%x%08x\n", $0, 165 }'|xargs printf "%d"`
echo 'DROP TABLE IF EXISTS `'"mail"'`;'
echo 'CREATE TABLE `'"mail"'` ('
echo '	`mail_id` bigint(21) NOT NULL AUTO_INCREMENT,'
echo '	`player_id` bigint(21) NOT NULL,'
echo '	`state` int(11) DEFAULT 0,'
echo '	`extract` tinyint(1) DEFAULT '"'"0"'"','
echo '	`read` tinyint(1) DEFAULT' "'"0"'"','
echo '	`time` datetime DEFAULT NULL,'
echo '	`data` blob,'
echo '	PRIMARY KEY (`mail_id`),'
echo '	KEY `player_id` (`player_id`)'
echo ") ENGINE=MyISAM AUTO_INCREMENT=$auto_val DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;"

echo 'DROP TABLE IF EXISTS `'"chengjie"'`;'
echo 'CREATE TABLE `chengjie` ('
echo ' `id` bigint(21) NOT NULL AUTO_INCREMENT,'
echo ' `data` blob,'
echo ' `complete` tinyint(1) DEFAULT '0','
echo ' PRIMARY KEY (`id`)'
echo ") ENGINE=MyISAM AUTO_INCREMENT=3 DEFAULT CHARSET=utf8;"

auto_val=`echo $server_id|awk '{print lshift($1, 20)}'`
echo 'DROP TABLE IF EXISTS `'"guild"'`;'
echo 'CREATE TABLE `'guild'`('
echo '	`guild_id`  int  NOT NULL AUTO_INCREMENT COMMENT '"'"帮会id,自动增涨"'"','
echo '	`icon` int NOT NULL COMMENT '"'"帮会图标"'"','
echo '	`name` varchar(100) COLLATE utf8_unicode_ci NOT NULL COMMENT '"'"帮会名称"'"','
echo '	`master_id` bigint NOT NULL COMMENT '"'"会长"'"','
echo '	`level` int NOT NULL  COMMENT '"'"等级"'"','
echo '	`popularity` int NOT NULL  COMMENT '"'"人气"'"','
echo '	`approve_state` int NOT NULL  COMMENT '"'"审批开关"'"','
echo '	`recruit_state` int NOT NULL  COMMENT '"'"招募开关"'"','
echo '	`recruit_notice` varchar(601) COLLATE utf8_unicode_ci NOT NULL COMMENT '"'"招募宣言"'"','
echo '	`announcement` varchar(601) COLLATE utf8_unicode_ci NOT NULL COMMENT '"'"帮会公告"'"','
echo '	`comm_data` blob NOT NULL COMMENT  '"'"打包数据"'"','
echo '	PRIMARY KEY (`guild_id`)'
echo ") ENGINE=MyISAM AUTO_INCREMENT=$auto_val DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;"

echo 'DROP TABLE IF EXISTS `'"guild_player"'`;'
echo 'CREATE TABLE `'guild_player'`('
echo '	`player_id` bigint NOT NULL COMMENT '"'"玩家ID"'"','
echo '	`guild_id`  int  NOT NULL COMMENT '"'"帮会ID"'"','
echo '	`comm_data` blob NOT NULL COMMENT  '"'"打包数据"'"','
echo '	PRIMARY KEY (`player_id`),'
echo '	KEY (`guild_id`)'
echo ") ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;"

echo 'DROP TABLE IF EXISTS `'"guild_join"'`;'
echo 'CREATE TABLE `'guild_join'`('
echo '	`player_id`  bigint(21)  NOT NULL COMMENT '"'"玩家ID"'"','
echo '	`guild_id` int(21) NOT NULL COMMENT '"'"公会ID"'"','
echo '	`apply_time` datetime NOT NULL COMMENT '"'"申请时间"'"','
echo '	KEY  (`player_id`),'
echo '	KEY  (`guild_id`)'
echo ') ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;'

echo 'DROP TABLE IF EXISTS `'"game_global"'`;'
echo 'CREATE TABLE `'game_global'`('
echo '	`key`  int  NOT NULL COMMENT '"'"键值"'"','
echo '	`comm_data` blob NOT NULL COMMENT  '"'"打包数据"'"','
echo '	PRIMARY KEY (`key`)'
echo ') ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;'

