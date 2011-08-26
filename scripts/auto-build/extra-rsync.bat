REM uses http://setacl.sourceforge.net/

cd \MinGW\msys\1.0\home\pd\

REM echo y|cacls c:\MinGW\msys\1.0\home\pd\auto-build /C /T /G pd:F everyone:R 

setacl -on c:\MinGW\msys\1.0\home\pd\auto-build -ot file -actn ace -ace "n:pd;p:full,write_owner;i:so,sc;m:set" -ace "n:everyone;p:read;i:so,sc;m:set"

REM Cygwin rsync seems to be unhappy with SVN's .svn file permissions, so
REM ignore SVN files first to get all the 'meat'
rsync -av --progress --whole-file --exclude='*inv\**' --cvs-exclude --timeout=60 rsync://128.238.56.50/distros/pd-extended/ /cygdrive/c/MinGW/msys/1.0/home/pd/auto-build/pd-extended/

sleep 60

REM now get the SVN changes, this might fail a lot, especially on '.svn/entries'
rsync -av --progress --whole-file --delete-before --exclude='*inv\**' --timeout=60 rsync://128.238.56.50/distros/pd-extended/ /cygdrive/c/MinGW/msys/1.0/home/pd/auto-build/pd-extended/
