#!/bin/bash
pid=`cat pid.txt`
core_file_name=`ls -lt /tmp/core.game_srv.*.$pid | head -1 | awk '{print $9}'`
#echo $core_file_name
`cp $core_file_name core_bak/`
`cp so_game_srv/*.so core_bak/`
`cp game_srv core_bak/`
`cp logs/game_srv.0 core_bak/`
`chmod a+r core_bak/*`
