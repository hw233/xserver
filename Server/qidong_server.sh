#!/bin/sh 
./stop.sh 
make svnup
cmake .
make -j4
./restart.sh 
./cscope.sh 
./ctags.sh
