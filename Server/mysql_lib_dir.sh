#!/bin/sh
mysql_config --variable=pkglibdir | awk '{printf $1}'
