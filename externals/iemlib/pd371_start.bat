@echo off

set PD_PATCH=%1
set PD_INSTALL=C:\Programme\pd-0.37-1\bin
set PD_AUDIO=-r 44100 -audiobuf 80 -sleepgrain 10 -channels 2
set PD_MIDI=-nomidi
set PD_OPTIONS=-font 10
set PD_PATH=-path C:/Programme/pd-0.37-1/iemabs -path C:/Programme/pd-0.37-1/externs
set PD_LIB1=-lib iemlib1 -lib iemlib2 -lib iem_mp3 -lib iem_t3_lib -lib zexy -lib Gem
set PD_LIB2=-lib iem_ambi -lib iem_bin_ambi -lib iem_eq -lib iem_lms -lib iem_matrix
set PD_LIB3=-lib iem_spec2 -lib iem_delay -lib iem_roomsim -lib iem_tab -lib iemgui

@echo starting pd ...
%PD_INSTALL%\pd %PD_AUDIO% %PD_MIDI% %PD_OPTIONS% %PD_PATH% %PD_LIB1% %PD_LIB2% %PD_LIB3% %PD_PATCH%
