include Makefile.config

all:	$(TARGET)

linux:	pdp_opengl.pd_linux

darwin:	pdp_opengl.pd_darwin

subdirs:
	make -C system
	make -C modules
	make -C include

clean:
	make -C system clean
	make -C modules clean
	make -C include clean
	rm -f pdp_opengl.pd_linux
	rm -f *~

pdp_opengl.pd_linux: subdirs
	rm -f pdp_opengl.pd_linux
	$(CC) -export_dynamic -shared -o pdp_opengl.pd_linux modules/*.o system/*.o $(LDFLAGS) -g

pdp_opengl.pd_darwin: subdirs
	rm -f pdp_opengl.pd_linux
	$(CC) -o pdp_opengl.pd_pd_darwin modules/*.o system/*.o $(LDFLAGS) -g -bundle -bundle_loader $(PD_EXECUTABLE)
