#!/bin/sh
/usr/bin/protoc  -I../../Proto  --python_out=.  ../../Proto/*.proto
/usr/bin/protoc  -I../proto  --python_out=.  ../proto/*.proto
