@echo off

set PD_PATCH=%1
set PD_INSTALL=C:\Programme\pd\bin
set PD_AUDIO=-r 44100 -audiobuf 80 -sleepgrain 10 -channels 2
set PD_MIDI=-nomidi
set PD_OPTIONS=-font 10
set PD_PATH=-path C:/Programme/pd/iemabs -path C:/Programme/pd/externs
set PD_LIB=-lib iemlib1 -lib iemlib2 -lib iem_mp3 -lib iem_t3_lib

@echo starting pd ...
%PD_INSTALL%\pd %PD_AUDIO% %PD_MIDI% %PD_OPTIONS% %PD_PATH% %PD_LIB% %PD_PATCH%
