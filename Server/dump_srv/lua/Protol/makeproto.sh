#!/bin/sh
/usr/bin/protoc -I../../../../Proto/ --plugin=../protobuf/plugin/protoc-gen-lua --lua_out=. ../../../../Proto/*.proto
#/usr/bin/protoc -I../../../proto/ -I/home/longmuquan/gitroot/protobuf-lua/protoc-plugin/ --plugin=/home/longmuquan/gitroot/protobuf-lua/protoc-plugin/protoc-gen-lua  --lua_out=.  ../../../proto/*.proto
#/usr/bin/protoc -I../../../../Proto/ -I/home/longmuquan/gitroot/protobuf-lua/protoc-plugin/ --plugin=/home/longmuquan/gitroot/protobuf-lua/protoc-plugin/protoc-gen-lua  --lua_out=. ../../../../Proto/*.proto

