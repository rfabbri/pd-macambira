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

configure: generate.sh
	./generate.sh
