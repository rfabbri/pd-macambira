all: noize colortv subtract xbend bend gol shuffle green tile rgbcopy copyvert black rgb hist setbits xshred sonogram vf2fs fs2vf swap 2colors plot makesliders
	dir *.dll

# FLAGS:
#
# <opts!!!>

FLAGS = /LD /Gd /GD /Ox

EFFECT = /export:perform_effect
COPY = /export:perform_copy
BOTH = /export:perform_effect /export:perform_copy

noize:
	cl noize.c $(FLAGS) /link $(BOTH)

colortv:
	cl colortv.c $(FLAGS) /link $(BOTH)

subtract:
	cl subtract.cpp $(FLAGS) /link $(EFFECT)

xbend:
	cl xbend.c $(FLAGS) /link $(BOTH)

bend:
	cl bend.c $(FLAGS) /link $(BOTH)

gol:
	cl gol.c $(FLAGS) /link $(EFFECT)

shuffle:
	cl shuffle.c $(FLAGS) /link $(EFFECT)

green:
	cl green.cpp $(FLAGS) /link $(EFFECT)

tile:
	cl tile.cpp $(FLAGS) /link $(COPY)

rgbcopy:
	cl rgbcopy.cpp $(FLAGS) /link $(COPY)

copyvert:
	cl copy_vert.c $(FLAGS) /link $(COPY)

black:
	cl black.c $(FLAGS) /link $(BOTH)

rgb:
	cl rgb.c $(FLAGS) /link $(EFFECT)

hist:
	cl hist.cpp $(FLAGS) /link $(EFFECT)

setbits:
	cl setbits.c $(FLAGS) /link $(EFFECT)

sonogram:
	cl sonogram.cpp $(FLAGS) /link $(EFFECT)

xshred:
	cl xshred.c $(FLAGS) /link $(COPY)

vf2fs:
	cl vf2fs.c $(FLAGS) /I..\Externals /link $(EFFECT)

fs2vf:
	cl fs2vf.c $(FLAGS) /I..\Externals /link $(EFFECT)

swap:
	cl swap.cpp $(FLAGS) /link $(COPY)

2colors:
	cl 2colors.cpp $(FLAGS) /link $(EFFECT)

plot:
	cl plot.cpp $(FLAGS) /link $(EFFECT)

makesliders:
	cl makesliders.c $(FLAGS) /link $(EFFECT)
	copy makesliders.dll ms.dll
