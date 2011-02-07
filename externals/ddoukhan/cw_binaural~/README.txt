cw_binaural~: A binaural synthesis external

Authors: 
 David Doukhan <david.doukhan@gmail.com> - http://www.limsi.fr/Individu/doukhan
 Anne Sedes <sedes.anne@gmail.com>

For more details, see:
CW_binaural: a binaural Sythesis External for Pure Data
David Doukhan and Anne Sedes, PDCON09

* libsndfile should be installed on your machine in order to compile the external

* when using mingw compiler, you should install the gnu regex lib

* The external use try/catch statements. On some OS, it may cause PD to crash when exceptions are thrown. To solve this issue, go to file=>path=>startup, edit a new entry, and set cw_binaural~ to be the first external to be loaded.
For more details concerning the try/cath issue, see http://www.mail-archive.com/pd-dev@iem.at/msg06694.html
