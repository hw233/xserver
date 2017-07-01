#!/bin/sh
svn status ../Proto
svn status | grep -v  ".*\.d$" | grep -v ".*\/logs\/.*" | grep -v ".*pid\.txt$" | grep -v ".*cscope\..*" | grep -v ".*thirdlib\/.*" | grep -v ".*TAGS$" | grep -v ".*proto\/.*\.[hc]$"
