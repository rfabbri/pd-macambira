#!/usr/bin/python

import sys
import re

f = open(sys.argv[1], 'r')
for line in f.readlines():
    if re.match('^ALL_LINGUAS = (.*)', line):
        for po in line.split('=')[1].strip().rstrip().split(' '):
            s = po.split('_')
            if len(s) == 2:
                iso = s[0] + '_' + s[1].upper()
            else:
                iso = po
            print '<string>' + iso + '</string>',
