#!/usr/bin/env python
# -*- coding: utf-8 -*-

import xlrd
import random
import codecs
import sys

if len(sys.argv) < 2:
    print "please enter argv: [1]server_id"
    exit()
else:
    server_id = int(sys.argv[1])

#print server_id

workbook = xlrd.open_workbook('12文本.xlsx')

if workbook.nsheets < 2:
    print "sheet num error"
    exit()

worksheet = workbook.sheet_by_index(1)

num_rows = worksheet.nrows
num_cols = worksheet.ncols
if num_rows <= 5 or num_cols < 3:
    print "row or col num error"
    exit()

#print "sheet_name:%s, rows_num:%d, cols_num:%d" % (worksheet.name, num_rows, num_cols)

last_name = []
male_first_name = []
female_first_name = []

for cur_col in range(num_cols):
    for cur_row in range(num_rows):
        if cur_row < 5:
            continue

        cell = worksheet.cell_value(cur_row, cur_col)
        #if cur_row == 5:
            #print "value:%s" % (cell)

        if not cell.strip():
           break

       
        if cur_col == 0:
            last_name.append(cell)
        elif cur_col == 1:
            male_first_name.append(cell)
        elif cur_col == 2:
            female_first_name.append(cell)

print "last_name:%d, male_first_name:%d, female_first_name:%d" % (len(last_name), len(male_first_name), len(female_first_name))

all_name = set([])
male_name = []
female_name = []
for nlastname in range(len(last_name)):
    for nfirstname in range(len(male_first_name)):
        name = ''.join([last_name[nlastname], male_first_name[nfirstname]])
        if name not in all_name:
            male_name.append(name)
            all_name.add(name)

random.shuffle(male_name)
print "%s - %s, %d" % (male_name[0], male_name[-1], len(male_name))

for nlastname in range(len(last_name)):
    for nfirstname in range(len(female_first_name)):
        name = ''.join([last_name[nlastname], female_first_name[nfirstname]])
        if name not in all_name:
            female_name.append(name)
            all_name.add(name)

random.shuffle(female_name)
print "%s - %s, %d" % (female_name[0], female_name[-1], len(female_name))
print "all_name.size:%d" % (len(all_name))

def write_sql(server_id, pfile, lname, sex):
    #print "type pfile:%s" % (type(pfile))
    if not isinstance(server_id, int) or not isinstance(lname, list) or not isinstance(sex, int):
        print "args type error"
        return
    count = 0
    for name in lname:
        if count == 0:
            pfile.write("insert into name%d(sex, name) values (%d, '%s')" % (server_id, sex, name))
        else:
            pfile.write(",(%d, '%s')" % (sex, name))
        count = count + 1
        if count == 200:
            pfile.write(";\n")
            count = 0
    if count > 0:
        pfile.write(";\n")



output_file_name = "name%d.sql" % (server_id)
#print output_file_name
with codecs.open(output_file_name, 'w', 'utf-8') as pfile:
    write_sql(server_id, pfile, male_name, 1)
    write_sql(server_id, pfile, female_name, 2)

