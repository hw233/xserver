#!/bin/sh
rm -rf config/server
rm -rf Proto
rm -rf Server
cp /home/jacktang/svnroot/XGame/Server . -a
cp /home/jacktang/svnroot/XGame/Proto . -a
cp /home/jacktang/svnroot/XGame/config/server  config/ -a
rm -f Server/CMakeCache.txt
rm -rf  Server/CMakeFiles
svn log -l1 https://192.168.2.8/svn/XGame > version.log
