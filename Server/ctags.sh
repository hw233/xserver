#!/bin/bash  
  
cat cscope.files | /usr/bin/ctags --c++-kinds=+p --fields=+iaSl --extra=+q -L -;  
#awk -F '"' '$2 ~ /^\tf/    {print $1 "\n"}' tags | awk '$1 ~ /^[a-zA-Z]/ {print "syn keyword cFunction " $1}' 1>  tags.vim ;  
#awk -F '"' '$2 ~ /^\td/	   {print $1 "\n"}' tags | awk '$1 ~ /^[a-zA-Z]/ {print "syn keyword cMacro " $1}'    1>> tags.vim ;  
#awk -F '"' '$2 ~ /^\tt/    {print $1 "\n"}' tags | awk '$1 ~ /^[a-zA-Z]/ {print "syn keyword cTypedef " $1}'  1>> tags.vim ;  
#awk -F '"' '$2 ~ /^\tv/    {print $1 "\n"}' tags | awk '$1 ~ /^[a-zA-Z]/ {print "syn keyword cGlobal " $1}'   1>> tags.vim ;
#awk -F '"' '$2 ~ /^\te/    {print $1 "\n"}' tags | awk '$1 ~ /^[a-zA-Z]/ {print "syn keyword cEnum " $1}'     1>> tags.vim ;  
#awk -F '"' '$2 ~ /^\tm/    {print $1 "\n"}' tags | awk '$1 ~ /^[a-zA-Z]/ {print "syn keyword cMember " $1}'   1>> tags.vim ;  
#awk -F '"' '$2 ~ /^\tc/    {print $1 "\n"}' tags | awk '$1 ~ /^[a-zA-Z]/ {print "syn keyword cClass " $1}'    1>> tags.vim ;  
#awk -F '"' '$2 ~ /^\ts/    {print $1 "\n"}' tags | awk '$1 ~ /^[a-zA-Z]/ {print "syn keyword cStruct " $1}'   1>> tags.vim ;  

