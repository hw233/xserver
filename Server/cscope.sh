#!/bin/sh
cd `dirname $0`
find . -type f -name "*.[hc]pp" > cscope.files
find . -type f -name "*.[hc]" >> cscope.files
cscope -bq
cat cscope.files | xargs ctags -e 

