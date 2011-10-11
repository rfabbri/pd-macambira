#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import time
import socket
import os
import re
from pdtest import PdTest, TestLog

try:
    pdrootdir = sys.argv[1]
except IndexError:
    print 'only one arg: root dir of pd'
    sys.exit(2)

test = PdTest(pdrootdir)

now = time.localtime(time.time())
date = time.strftime('20%y-%m-%d', now)
datestamp = time.strftime('20%y-%m-%d_%H.%M.%S', now)

outputfile = '/tmp/load_every_help_' + socket.gethostname() + '_' + datestamp + '.log'
fd = open(outputfile, 'w')
fd.write('load_every_help\n')
fd.write('========================================================================\n')
fd.flush()

extradir = os.path.join(pdrootdir, 'extra')
for root, dirs, files in os.walk(extradir):
    for name in files:
        m = re.search(".*-help\.pd$", name)
        if m:
            test.runtest(fd, root, name)

docdir = os.path.join(pdrootdir, 'doc')
for root, dirs, files in os.walk(docdir):
    for name in files:
        m = re.search(".*\.pd$", name)
        if m:
            test.runtest(fd, root, name)

fd.close()


# upload the load and send the email report
log = TestLog(outputfile)
log.upload()
log.email(subject='load_every_help ' + datestamp)
