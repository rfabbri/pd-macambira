nmake
cd ..
cd extra

del *.dll

cd bonk~
nmake pd_nt
move bonk~.dll ..
cd ..

cd choice
nmake pd_nt
move choice.dll ..
cd ..

cd expr~
nmake pd_nt
move expr.dll ..
cd ..

cd fiddle~
nmake pd_nt
move fiddle~.dll ..
cd ..

cd loop~
nmake pd_nt
move loop~.dll ..
cd ..

cd lrshift~
nmake pd_nt
move lrshift~.dll ..
cd ..

cd pique
nmake pd_nt
move pique.dll ..
cd ..


