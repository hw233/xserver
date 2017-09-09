#!/usr/bin/python
# coding: UTF-8
import sys
import re

filename = 'conn_srv.0'
if len(sys.argv) > 1:
    filename = sys.argv[1]

input = open(filename, 'r')
for line in input:
    print "%s" % line
    
    
input.close()         
    
    
