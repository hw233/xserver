#!/bin/sh
svn log -l35 | grep jacktang | sed 's/\(^r[0-9]*\) .*/-\1/g' | xargs svn log -v | grep ".*\.[h|cpp]" | sed 's/^ *[AM] \/Server\///g' | sort | unique 11.log 
