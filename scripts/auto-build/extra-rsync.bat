cd \msys\1.0\home\pd\

REM reset perms
echo y|cacls c:\msys\1.0\home\pd\auto-build /C /T /G pd:F everyone:R 

REM Cygwin rsync seems to be unhappy with SVN's .svn file permissions, so
REM ignore SVN files first to get all the 'meat'
rsync -av --progress --whole-file --exclude='*inv\**' --cvs-exclude --timeout=60 rsync://128.238.56.50/distros/pd-extended/ /home/pd/auto-build/pd-extended/

sleep 60

REM now get the SVN changes, this might fail a lot, especially on '.svn/entries'
rsync -av --progress --whole-file --exclude='*inv\**' --timeout=60 rsync://128.238.56.50/distros/pd-extended/ /home/pd/auto-build/pd-extended/

REM reset perms
echo y|cacls c:\msys\1.0\home\pd\auto-build /C /T /G pd:F everyone:R 

