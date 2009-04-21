REM @echo off

cd \msys\1.0\home\pd\

REM C:\msys\1.0\bin\sh --login -i -c find /home/pd/auto-build/pd-extended/ -name entries 

REM Cygwin rsync seems to be unhappy with SVN's .svn file permissions, so
REM ignore SVN files first to get all the 'meat'
rsync -av --progress --exclude='*inv\**' --cvs-exclude --timeout=60 rsync://128.238.56.50/distros/pd-extended/ /home/pd/auto-build/pd-extended/
sleep 60
REM now get the SVN changes, this might fail a lot, especially on '.svn/entries'
rsync -av --progress --exclude='*inv\**' --timeout=60 rsync://128.238.56.50/distros/pd-extended/ /home/pd/auto-build/pd-extended/

