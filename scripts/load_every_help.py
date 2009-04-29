#!/usr/bin/python

import subprocess, sys, socket, time, os, re

try:
    pdrootdir = sys.argv[1]
except IndexError:
    print 'only one arg: root dir of pd'
    sys.exit(2)


bindir = pdrootdir + '/bin'
docdir = pdrootdir + '/doc/5.reference/maxlib'
pdexe = bindir + '/pd'
pdsendexe = bindir + '/pdsend'

PORT = 55555
netreceive_patch = '/tmp/.____pd_netreceive____.pd'


def make_netreceive_patch(filename):
    fd = open(filename, 'w')
    fd.write('#N canvas 222 130 454 304 10;')
    fd.write('#X obj 111 83 netreceive ' + str(PORT) + ' 0 old;')
    fd.write('#X obj 111 103 loadbang;')
    fd.write('#X obj 111 123 print netreceive_patch;')
    fd.write('#X connect 1 0 2 0;')
    fd.close()

def send_to_socket(message):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(('localhost', PORT))
    s.send(message)
    s.close()

def send_to_pd(message):
    send_to_socket('; pd ' + message + ';\n')

def open_patch(filename):
    dir, file = os.path.split(filename)
    send_to_pd('open ' + file + ' ' + dir)

def close_patch(filename):
    dir, file = os.path.split(filename)
    send_to_pd('; pd-' + file + ' menuclose')


def launch_pd():
    p = subprocess.Popen([pdexe, '-nogui', '-stderr', '-open', netreceive_patch],
                         stdout=subprocess.PIPE, stderr=subprocess.STDOUT, 
                         close_fds=True);
    line = p.stdout.readline()
    while line != 'netreceive_patch: bang\n':
        line = p.stdout.readline()
    return p

#---------- list of lines to ignore ----------#
def remove_ignorelines(list):
    ignorelines = [
        'expr, expr~, fexpr~ version 0.4 under GNU General Public License \n',
        'fiddle version 1.1 TEST4\n',
        'sigmund version 0.03\n',
        'pique 0.1 for PD version 23\n',
        'this is pddplink 0.1, 3rd alpha build...\n',
        '\n'
        ]
    ignorepatterns = [
        'ydegoyon@free.fr',
        'olaf.matthes@gmx.de',
        'Olaf.*Matthes',
        '[a-z]+ v0\.[0-9]',
        'IOhannes m zm',
        'part of zexy-'
        ]
    for ignore in ignorelines:
        try:
            list.remove(ignore)
        except ValueError:
            pass
    for line in list:
        for pattern in ignorepatterns:
            m = re.search('.*' + pattern + '.*', line)
            if m:
                try:
                    list.remove(m.string)
                except ValueError:
                    pass
    return list


#---------- main()-like thing ----------#

make_netreceive_patch(netreceive_patch)

mailoutput = []
for root, dirs, files in os.walk(docdir):
    #dirs.remove('.svn')
#    print "root: " + root
    for name in files:
        m = re.search(".*\.pd$", name)
        if m:
            patch = os.path.join(root, m.string)
            p = launch_pd()
            open_patch(patch)
            close_patch(patch)
            send_to_pd('quit')
            patchoutput = []
            line = p.stdout.readline()
            while line != 'EOF on socket 10\n':
                patchoutput.append(line) 
                line = p.stdout.readline()
            patchoutput = remove_ignorelines(patchoutput)
            if len(patchoutput) > 0:
                print 'loading: ' + name
                mailoutput.append('__________________________________________________')
                mailoutput.append('loading: ' + name)
                mailoutput.append('--------------------------------------------------')
                mailoutput.append(patchoutput)
                for line in patchoutput:
                    print '--' + line + '--'
