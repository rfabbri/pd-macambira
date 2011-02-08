cw_binaural~: A binaural synthesis external

Authors: 
 David Doukhan <david.doukhan@gmail.com> - http://perso.limsi.fr/doukhan
 Anne Sedes <sedes.anne@gmail.com>

For more details, see:
CW_binaural~: a binaural Synthesis External for Pure Data
David Doukhan and Anne Sedes, PDCON09

cw_binaural~ has been successfully compiled and tested on the following OS: Ubuntu Lucid Lynx, Intel MacOS 10.6, Windows 7.
Precompiled binaries should be ok for Intel Mac OS >= 10.4, Windows XP, Vista, 7

* To use precompiled external on mac, you should install universal binaries of libsndfile using macports:
sudo port install libsndfile +universal

* To use precompiled external on ubuntu, you should install libsndfile:
sudo apt-get install libsndfile

* libsndfile should be installed on your machine in order to compile the external

* when using mingw compiler, you should manually install the gnu regex lib

* The external use try/catch statements. On some OS, it may cause PD to crash when exceptions are thrown. To solve this issue, go to file=>path=>startup, edit a new entry, and set cw_binaural~ to be the first external to be loaded.
For more details concerning the try/cath issue, see http://www.mail-archive.com/pd-dev@iem.at/msg06694.html
