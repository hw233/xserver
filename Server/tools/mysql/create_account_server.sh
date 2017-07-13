#!/bin/bash

cd `dirname $0`

echo 'CREATE DATABASE IF NOT EXISTS `'"xgame_global"'`;'
echo 'use`'"xgame_global"'`;'

echo 'DROP TABLE IF EXISTS `'"account_server"'`;'
echo 'CREATE TABLE `'"account_server"'` ('
echo '	`open_id` bigint NOT NULL COMMENT '"'"账号ID"'"','
echo '	`server_id` int NOT NULL COMMENT '"'"服务器ID"'"','
echo '	KEY (`open_id`)'
echo ") ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;"

