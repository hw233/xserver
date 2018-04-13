#!/bin/bash

srv_list=(
conn_srv
game_srv
login_srv
friend_srv
db_srv
mail_srv
guild_srv
rank_srv
doufachang_srv
trade_srv
activity_srv
)

rm -rf pack_dir
mkdir -p pack_dir/lua_data
mkdir -p pack_dir/map_res
mkdir pack_dir/excel_data

for srv_name in ${srv_list[*]}
do
    mkdir -p pack_dir/$srv_name/logs
    cp $srv_name/$srv_name pack_dir/$srv_name/
    cp $srv_name/log4crc pack_dir/$srv_name/
done

mkdir pack_dir/game_srv/so_game_srv/
cp game_srv/so_game_srv/libgamesrv.so pack_dir/game_srv/so_game_srv/
mkdir pack_dir/proto
cp proto/libproto.so pack_dir/proto

cp *.sh  pack_dir
cp lua_data/*.lua pack_dir/lua_data/
cp lua_data/*.dat pack_dir/map_res/
cp excel_data/1.spb pack_dir/excel_data/
cp raidsrv_config.lua pack_dir/
cp tools pack_dir/  -a
cp showurl.ini pack_dir/
tar zcvf pack.tgz pack_dir

