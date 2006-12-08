TARGET := $(shell pwd | sed 's|.*/\(.*\)$$|\1|')
EXTERNALS_ROOT := $(shell pwd | sed 's|^\(/.*externals\).*|\1|')

default: 
	make -C $(EXTERNALS_ROOT) $(TARGET)

install:
	make -C $(EXTERNALS_ROOT) $(TARGET)_install

clean:
	make -C $(EXTERNALS_ROOT) $(TARGET)_clean

test_locations:
	make -C $(EXTERNALS_ROOT) test_locations

# for emacs
etags:
	etags ../../../pd/src/*.h *.[ch] linux/input.h 
	make etags_`uname -s`

etags_Darwin:
	etags -a HID\ Utilities\ Source/*.[ch] \
		/System/Library/Frameworks/ForceFeedback.framework/Headers/*.h \
		/System/Library/Frameworks/Carbon.framework/Headers/*.h  \
		/System/Library/Frameworks/IOKit.framework/Headers/hid*/*.[ch]

etags_Linux:
	etags -a /usr/include/*.h linux/input.h /usr/include/sys/*.h

etags_MINGW:
	etags -a /usr/include/*.h /usr/include/sys/*.h \
		/usr/local/include/*.h /usr/local/include/sys/*.h
