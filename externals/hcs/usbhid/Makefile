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
	etags *.[ch] ../../../pd/src/*.[ch] \
		~/code/libhid-svn/*/*.[ch] \
		/sw/include/hid*.h /sw/include/usb*.h \
		/System/Library/Frameworks/IOKit.framework/Headers/hid/*.h \
		/Users/hans/Documents/Research/HID/example\ code/CrossHID/*/*.h \
		/Users/hans/Documents/Research/HID/example\ code/CrossHID/*/*.cpp \
		../hid/HID\ Utilities\ Source/*.[ch]

#etags_Darwin:
#	etags --append /sw/include/*.h /sw/include/*/*.h

etags_Linux:
	etags --append libhid-svn/*/*.? 


