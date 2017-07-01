#!/bin/sh
find . -type f -name "*.[hc]pp" > cscope.files
find . -type f -name "*.[hc]" >> cscope.files
cscope -bq
cat cscope.files | etags -

