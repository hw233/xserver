#!/bin/sh
find . -type f -name memerr*  | xargs grep -H ERROR | awk -F":" '{print $1}'
