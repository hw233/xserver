cd `dirname $0`

if [ $# -lt 1 ];then
	echo 'pelase input argv[1]=server_id'
	exit 0
fi

server_id=$1

echo 'CREATE DATABASE IF NOT EXISTS `'"xgame_name"'`;'
echo 'use`'"xgame_name"'`;'

auto_val=`echo $server_id|awk '{ printf "0x%x%08x\n", $0, 1}'|xargs printf "%d"`
echo 'DROP TABLE IF EXISTS `'"name$server_id"'`;'
echo 'CREATE TABLE `'"name$server_id"'` ('
echo '	`id` bigint(21) NOT NULL AUTO_INCREMENT,'
echo '	`sex` int(1) DEFAULT '"'"0"'"','
echo '	`name` varchar(50) COLLATE utf8_unicode_ci NOT NULL,'
echo '	PRIMARY KEY (`id`),'
echo '	UNIQUE KEY `name` (`name`),'
echo '	KEY `sex` (`sex`)'
echo ") ENGINE=MyISAM AUTO_INCREMENT=$auto_val DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;"

