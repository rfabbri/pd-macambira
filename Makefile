TARGET=sigpack

default: 
	make -C ../ $(TARGET)

install:
	make -C ../ $(TARGET)_install

clean:
	make -C ../ $(TARGET)_clean
