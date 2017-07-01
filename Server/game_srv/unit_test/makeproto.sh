#!/bin/sh
protoc  -I../../../Proto  --python_out=.  ../../../Proto/*.proto
