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
	cd $(EXTERNALS_ROOT) && \
		etags --append \
			~/code/libhid-svn/*/*.[ch] \
			~/code/libusb/*.[ch] \
			/Users/hans/Documents/Research/HID/example\ code/CrossHID/*/*.h \
			/Users/hans/Documents/Research/HID/example\ code/CrossHID/*/*.cpp

#etags_Darwin:
#	etags --append /sw/include/*.h /sw/include/*/*.h

etags_Linux:
	etags --append libhid-svn/*/*.? 


