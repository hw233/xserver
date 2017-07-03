cd `dirname $0`


echo 'CREATE DATABASE IF NOT EXISTS `'"xgame_global"'`;'
echo 'use`'"xgame_global"'`;'

echo 'DROP TABLE IF EXISTS `'"setting"'`;'
echo 'CREATE TABLE `'"setting"'` ('
echo '	`open_id` int NOT NULL COMMENT '"'"账号ID"'"','
echo '	`data` blob NOT NULL COMMENT '"'"设置数据包"'"','
echo '	PRIMARY KEY (`open_id`)'
echo ") ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;"

