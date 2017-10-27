#!/bin/bash

rm -rf pack_dir
mkdir -p pack_dir/lua_data
mkdir -p pack_dir/map_res
mkdir -p pack_dir/game_srv/logs
mkdir -p pack_dir/conn_srv/logs
mkdir -p pack_dir/login_srv/logs
mkdir -p pack_dir/friend_srv/logs
mkdir -p pack_dir/db_srv/logs
mkdir -p pack_dir/mail_srv/logs
mkdir -p pack_dir/guild_srv/logs
mkdir -p pack_dir/rank_srv/logs
mkdir -p pack_dir/doufachang_srv/logs
mkdir -p pack_dir/trade_srv/logs
mkdir pack_dir/excel_data

cp game_srv/game_srv pack_dir/game_srv/
cp game_srv/log4crc pack_dir/game_srv/

mkdir pack_dir/game_srv/so_game_srv/
cp game_srv/so_game_srv/libgamesrv.so pack_dir/game_srv/so_game_srv/
mkdir pack_dir/proto
cp proto/libproto.so pack_dir/proto

cp conn_srv/conn_srv pack_dir/conn_srv/
cp conn_srv/log4crc pack_dir/conn_srv/

cp login_srv/login_srv pack_dir/login_srv/
cp login_srv/log4crc pack_dir/login_srv/

cp friend_srv/friend_srv pack_dir/friend_srv/
cp friend_srv/log4crc pack_dir/friend_srv/

cp db_srv/db_srv pack_dir/db_srv/
cp db_srv/log4crc pack_dir/db_srv/

cp mail_srv/mail_srv pack_dir/mail_srv/
cp mail_srv/log4crc pack_dir/mail_srv/

cp guild_srv/guild_srv pack_dir/guild_srv/
cp guild_srv/log4crc pack_dir/guild_srv/

cp rank_srv/rank_srv pack_dir/rank_srv/
cp rank_srv/log4crc pack_dir/rank_srv/

cp doufachang_srv/doufachang_srv pack_dir/doufachang_srv/
cp doufachang_srv/log4crc pack_dir/doufachang_srv/

cp trade_srv/trade_srv pack_dir/trade_srv/
cp trade_srv/log4crc pack_dir/trade_srv/

cp *.sh  pack_dir
cp lua_data/*.lua pack_dir/lua_data/
cp lua_data/*.dat pack_dir/map_res/
cp excel_data/1.spb pack_dir/excel_data/
cp tools pack_dir/  -a
tar zcvf pack.tgz pack_dir

