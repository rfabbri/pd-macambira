#!/usr/bin/python
# -*- coding: utf-8 -*-

import subprocess, sys, socket, time, os, re, time, smtplib, signal
import random

class PdTest():

    def __init__(self):
        self.rand = random.SystemRandom()

    def find_pdexe(self, rootdir):
        # start with the Windows/Mac OS X location
        exe = pdrootdir + '/bin/pd'
        if not os.path.isfile(exe):
            # try the GNU/Linux location
            exe = pdrootdir+'../../bin/pd'
            if not os.path.isfile(exe):
                print "ERROR: can't find pd executable"
                exit
        return os.path.realpath(exe)

    def make_netreceive_patch(self, port):
        filename = '/tmp/.____pd_netreceive_'+str(self.port)+'____.pd'
        fd = open(filename, 'w')
        fd.write('#N canvas 222 130 454 304 10;')
        fd.write('#X obj 201 13 import vanilla;')
        fd.write('#X obj 111 83 netreceive ' + str(self.port) + ' 0 old;')
        fd.write('#X obj 111 103 loadbang;')
        fd.write('#X obj 111 123 print netreceive_patch;')
    # it would be nice to have this patch tell us when it is closed...
    #    fd.write('#X obj 211 160 tof/destroysend pd;')
    #    fd.write('#X obj 211 160 closebang;')
    #    fd.write('#X obj 211 180 print CLOSE;')
        fd.write('#X connect 2 0 3 0;')
    #    fd.write('#X connect 3 0 4 0;')
        fd.close()
        self.netreceive_patch = filename

    def send_to_socket(self, message):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(5)
        try:
            s.connect(('localhost', self.port))
            s.send(message)
            s.close()
        except socket.timeout:
            print "socket timed out while sending"

    def send_to_pd(self, message):
        self.send_to_socket('; pd ' + message + ';\n')

    def open_patch(self, filename):
        dir, file = os.path.split(filename)
        self.send_to_pd('open ' + file + ' ' + dir)

    def close_patch(self, filename):
        dir, file = os.path.split(filename)
        self.send_to_pd('; pd-' + file + ' menuclose')

    def launch_pd(self):
        pdexe = self.find_pdexe(pdrootdir)
        p = subprocess.Popen([pdexe, '-nogui', '-stderr', '-open', self.netreceive_patch],
                             stdout=subprocess.PIPE, stderr=subprocess.STDOUT, 
                             close_fds=True);
        line = p.stdout.readline()
        while line != 'netreceive_patch: bang\n':
            line = p.stdout.readline()
        return p

    def quit_pd(self, process):
        self.send_to_pd('quit')
        time.sleep(1)
        try:
            os.kill(process.pid, signal.SIGTERM)
        except OSError:
            print 'OSError on SIGTERM'
        time.sleep(1)
        try:
            os.kill(process.pid, signal.SIGKILL)
        except OSError:
            print "OSError on SIGKILL"


    #---------- list of lines to ignore ----------#
    def remove_ignorelines(self, list):
        ignorelines = [
            'expr, expr~, fexpr~ version 0.4 under GNU General Public License \n',
            'fiddle version 1.1 TEST4\n',
            'sigmund version 0.07\n',
            'bonk version 1.5\n'
            'pique 0.1 for PD version 23\n',
            'this is pddplink 0.1, 3rd alpha build...\n',
            'beware! this is tot 0.1, 19th alpha build...\n',
            'foo: you have opened the [loadbang] help document\n',
            'print: bang\n',
            'print: 207\n',
            'print: 2 1\n',
            'obj3\n',
            'obj4 34\n',
            'initial_bang: bang\n',
            '\n'
            ]
        ignorepatterns = [
            'ydegoyon@free.fr',
            'olaf.matthes@gmx.de',
            'Olaf.*Matthes',
            '[a-z]+ v0\.[0-9]',
            'IOhannes m zm',
            'part of zexy-',
            'Pd: 0.43.1-extended',
            'based on sync from jMax'
            ]
        for ignore in ignorelines:
            try:
                list.remove(ignore)
            except ValueError:
                pass
        for line in list:
            for pattern in ignorepatterns:
                m = re.search('.*' + pattern + '.*', line)
                while m:
                    try:
                        list.remove(m.string)
                        m = re.search('.*' + pattern + '.*', line)
                    except ValueError:
                        break
        return list


    def runtest(self, log, root, filename):
        patchoutput = []
        patch = os.path.join(root, filename)
        self.port = int(self.rand.random() * 10000) + int(self.rand.random() * 10000) + 40000
        self.make_netreceive_patch(self.port)
        p = self.launch_pd()
        try:
            self.open_patch(patch)
            time.sleep(5)
            self.close_patch(patch)
            self.quit_pd(p)
        except socket.error:
            patchoutput.append('socket.error')                 
        while True:
            line = p.stdout.readline()
            m = re.search('EOF on socket', line)
            if not m and line:
                patchoutput.append(line)
            else:
                break
        patchoutput = self.remove_ignorelines(patchoutput)
        if len(patchoutput) > 0:
            log.write('\n\n__________________________________________________\n')
            log.write('loading: ' + patch + '\n')
            for line in patchoutput:
                log.write(line)
            log.flush()

#---------- main()-like thing ----------#

try:
    pdrootdir = sys.argv[1]
except IndexError:
    print 'only one arg: root dir of pd'
    sys.exit(2)

test = PdTest()

now = time.localtime(time.time())
date = time.strftime('20%y-%m-%d', now)
datestamp = time.strftime('20%y-%m-%d_%H.%M.%S', now)

outputfilename = 'load_every_help_' + socket.gethostname() + '_' + datestamp + '.log'
outputfile = '/tmp/' + outputfilename
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


# make the email report
fromaddr = 'pd@pdlab.idmi.poly.edu'
toaddr = 'hans@at.or.at'
mailoutput = []
mailoutput.append('From: ' + fromaddr + '\n')
mailoutput.append('To: ' + toaddr + '\n')
mailoutput.append('Subject: load_every_help ' + datestamp + '\n\n\n')
mailoutput.append('______________________________________________________________________\n\n')
mailoutput.append('Complete log:\n')
mailoutput.append('http://autobuild.puredata.info/auto-build/' + date + '/logs/'
                  + outputfilename + '\n')


# upload the log file to the autobuild website
rsyncfile = 'rsync://128.238.56.50/upload/' + date + '/logs/' + outputfilename
try:
    p = subprocess.Popen(['rsync', '-ax', outputfile, rsyncfile],
                         stdout=subprocess.PIPE, stderr=subprocess.STDOUT).wait()
except:
    mailoutput.append('rsync upload of the log failed!\n')
    mailoutput.append(''.join(p.stdout.readlines()))


mailoutput.append('______________________________________________________________________\n\n')
server = smtplib.SMTP('in1.smtp.messagingengine.com')
logoutput = []
fd = open(outputfile, 'r')
for line in fd:
    logoutput.append(line)
server.sendmail(fromaddr, toaddr, ''.join(mailoutput + logoutput))
server.quit()
