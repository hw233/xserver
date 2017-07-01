#!/bin/sh
killall -9 conn_srv
killall -9 login_srv
killall -9 game_srv
killall -9 db_srv

ulimit -c unlimited

cd conn_srv
./conn_srv -d
cd ../db_srv
./db_srv -d
sleep 2
cd ../login_srv
./login_srv -d
cd ../game_srv
./game_srv -d -r
cd ..

