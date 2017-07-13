#!/bin/bash

cd `dirname $0`

echo 'CREATE DATABASE IF NOT EXISTS `'"xgame_global"'`;'
echo 'use`'"xgame_global"'`;'

echo 'DROP TABLE IF EXISTS `'"servers"'`;'
echo 'CREATE TABLE `'"servers"'` ('
echo '	`server_id` int NOT NULL COMMENT '"'"服务器ID"'"','
echo '	`mysql_host` varchar(30) NOT NULL COMMENT '"'"mysql数据库主机名"'"','
echo '	`mysql_port` int NOT NULL COMMENT '"'"mysql数据库端口"'"','
echo '	`mysql_user` varchar(30) NOT NULL COMMENT '"'"mysql数据库用户名"'"','
echo '	`mysql_pwd` varchar(30) NOT NULL COMMENT '"'"mysql数据库用户密码"'"','
echo '	`mysql_dbname` varchar(30) NOT NULL COMMENT '"'"mysql数据库名字"'"','
echo '	`redis_host` varchar(30) NOT NULL COMMENT '"'"redis数据库主机名"'"','
echo '	`redis_port` int NOT NULL COMMENT '"'"redis数据库端口"'"','
echo '	PRIMARY KEY (`server_id`)'
echo ") ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;"

