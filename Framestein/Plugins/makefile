all:	noize colortv subtract xbend bend gol shuffle green \
	tile rgbcopy copyvert black rgb hist setbits xshred \
	sonogram vf2fs fs2vf swap 2colors plot makesliders \
	convolution compare darken deinterlace difference interlace lighten \
	multiply overlay screen shadowcaster softlight \
	rgbseek modgain traffic constrain eclipse eclipse02 eclipse03 \
	cga keyscreen rene cutout fromage rgbavg rgbavg02 \
	rowca argument-passing lightspeed merge_additive merge_mod ping \
	fader

	dir *.dll

# FLAGS:
#
# <opts!!!>

FLAGS = /LD /Gd /GD /Ox

# does this suck or what? how to do it better?
EFFECT = /export:info /export:perform_effect
COPY = /export:info /export:perform_copy
BOTH = /export:info /export:perform_effect /export:perform_copy

noize:
	cl noize.c $(FLAGS) /link $(BOTH)

colortv:
	cl colortv.c $(FLAGS) /link $(BOTH)

subtract:
	cl subtract.cpp $(FLAGS) /GX /link $(EFFECT)

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
	cl vf2fs.c $(FLAGS) /I..\Patches /link $(EFFECT)

fs2vf:
	cl fs2vf.c $(FLAGS) /I..\Patches /link $(EFFECT)

swap:
	cl swap.cpp $(FLAGS) /link $(COPY)

2colors:
	cl 2colors.cpp $(FLAGS) /link $(EFFECT)

plot:
	cl plot.cpp $(FLAGS) /link $(EFFECT)

makesliders:
	cl makesliders.c $(FLAGS) /link $(EFFECT)
	copy makesliders.dll ms.dll

#
# PixelPack by Olaf Matthes
#

convolution:
	cl convolution.c $(FLAGS) /link $(EFFECT)

compare:
	cl compare.c $(FLAGS) /link $(COPY)

darken:
	cl darken.c $(FLAGS) /link $(COPY)

deinterlace:
	cl deinterlace.c $(FLAGS) /link $(EFFECT)

difference:
	cl difference.c $(FLAGS) /link $(COPY)

interlace:
	cl interlace.c $(FLAGS) /link $(COPY)

lighten:
	cl lighten.c $(FLAGS) /link $(COPY)

multiply:
	cl multiply.c $(FLAGS) /link $(COPY)

overlay:
	cl overlay.c $(FLAGS) /link $(COPY)

screen:
	cl screen.c $(FLAGS) /link $(COPY)

shadowcaster:
	cl shadowcaster.c $(FLAGS) /link $(BOTH)

softlight:
	cl softlight.c $(FLAGS) /link $(COPY)

#
# </PixelPack>
#

#
# PeRColate
#

rgbseek:
	cl rgbseek.c $(FLAGS) /link $(EFFECT)

modgain:
	cl modgain.c $(FLAGS) /link $(EFFECT)

traffic:
	cl traffic.c $(FLAGS) /link $(EFFECT)

constrain:
	cl constrain.c $(FLAGS) /link $(EFFECT)

eclipse:
	cl eclipse.c $(FLAGS) /link $(EFFECT)

eclipse02:
	cl eclipse02.c $(FLAGS) /link $(COPY)

eclipse03:
	cl eclipse03.c $(FLAGS) /link $(EFFECT)

cga:
	cl cga.c $(FLAGS) /link $(EFFECT)

keyscreen:
	cl keyscreen.c $(FLAGS) /link $(COPY)

rene:
	cl rene.c $(FLAGS) /link $(COPY)

cutout:
	cl cutout.c $(FLAGS) /link $(COPY)

fromage:
	cl fromage.c $(FLAGS) /link $(COPY)

rgbavg:
	cl rgbavg.c $(FLAGS) /link $(EFFECT)

rgbavg02:
	cl rgbavg02.c $(FLAGS) /link $(COPY)

#
# </PeRColate>
#

rowca:
	cl rowca.cpp $(FLAGS) /GX /link $(EFFECT)

argument-passing:
	cl argument-passing.cpp $(FLAGS) /GX /link $(EFFECT)

lightspeed:
	cl lightspeed.c $(FLAGS) /link $(EFFECT)

merge_additive:
	cl merge_additive.cpp $(FLAGS) /GX /link $(COPY)

merge_mod:
	cl merge_mod.cpp $(FLAGS) /GX /link $(COPY)

ping:
	cl ping.c $(FLAGS) /link $(EFFECT)

fader:
	cl fader.cpp $(FLAGS) /GX /link $(COPY)
