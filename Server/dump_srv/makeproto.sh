#!/bin/sh
protoc  -I../../Proto  --python_out=.  ../../Proto/*.proto
protoc  -I../proto  --python_out=.  ../proto/*.proto
