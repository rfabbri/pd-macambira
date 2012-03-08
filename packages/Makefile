#==============================================================================#
#
# Centralized cross-platform build system 
#
# see for instructions: http://puredata.org/docs/developer/build
#  <hans@at.or.at>
#
#==============================================================================#

CWD := $(shell pwd)

cvs_root_dir := $(shell cd $(CWD)/.. && pwd)
DESTDIR = $(CWD)/build
BUILDLAYOUT_DIR = $(CWD)


## default target
all: pd abstractions doc externals
	@echo "Complete build succeeded!"

include $(BUILDLAYOUT_DIR)/Makefile.buildlayout

#==============================================================================#
#
# GET VERSIONS FOR RELEVANT SOFTWARE
#
#==============================================================================#


CYCLONE_MAJOR_VERSION := $(shell grep CYCLONE_VERSION \
	$(externals_src)/miXed/cyclone/build_counter | cut -d ' ' -f 3 | \
	cut -d '"' -f 2)
CYCLONE_RELEASE := $(shell grep CYCLONE_RELEASE \
	$(externals_src)/miXed/cyclone/build_counter | cut -d ' ' -f 3 | \
	cut -d '"' -f 2)
CYCLONE_BUILD := $(shell grep CYCLONE_BUILD \
	$(externals_src)/miXed/cyclone/build_counter | cut -d ' ' -f 3 | \
	cut -d '"' -f 2)
CYCLONE_VERSION := $(CYCLONE_MAJOR_VERSION) $(CYCLONE_RELEASE) $(CYCLONE_BUILD)

GEM_VERSION := $(shell sed -n 's|^AC_INIT.\[Gem\],\[\([0-9]\).*|\1|p' $(gem_src)/configure.ac).$(shell sed -n 's|^AC_INIT.\[Gem\],\[[0-9]\.\([0-9][0-9]*\).*|\1|p' $(gem_src)/configure.ac)

MAXLIB_VERSION := $(shell grep "define VERSION" \
	$(externals_src)/maxlib/maxlib.c | cut -d '"' -f 2)

OSCX_VERSION := $(shell grep "define VERSION" $(externals_src)/OSCx/src/OSC.c | \
	cut -d '"' -f 2)

PDP_VERSION := $(shell grep PDP_VERSION= $(externals_src)/pdp/configure.ac | \
	cut -d '=' -f 2)

ZEXY_VERSION := $(shell grep VERSION $(externals_src)/zexy/src/zexy.h | \
	grep -v _VERSION | cut -d ' ' -f 3 | cut -d '"' -f 2)


# these are sent to all of the various Makefiles so that they all copy their
# output to the same directory tree
DEST_PATHS = BUILDLAYOUT_DIR=$(BUILDLAYOUT_DIR) \
				cvs_root_dir=$(cvs_root_dir) \
				DESTDIR=$(DESTDIR) \
				prefix=$(prefix) \
				libpddir=$(libpddir) \
				pkglibdir=$(libpddir) \
				OPT_CFLAGS="$(OPT_CFLAGS)" \
				UNAME=$(UNAME) \
				STRIP="$(STRIP)"


#==============================================================================#
#
# BUILD TARGETS
#
#==============================================================================#

#------------------------------------------------------------------------------
# abstractions
abstractions:


#------------------------------------------------------------------------------
# doc
doc:


#------------------------------------------------------------------------------
# externals
externals: 
	$(MAKE) -C $(externals_src) $(DEST_PATHS)


#------------------------------------------------------------------------------
# pd

PD_CONFIGURE_FLAGS = --prefix=$(prefix)

ifeq ($(OS_NAME),darwin)
  PD_CONFIGURE_FLAGS += --enable-jack
else
  ifeq ($(OS_NAME),linux)
    PD_CONFIGURE_FLAGS += --enable-jack --enable-alsa
  else
    PD_CONFIGURE_FLAGS += 
  endif
endif

PD_BUILD_FLAGS = 

HAVE_AUTOGEN_SH := $(wildcard $(pd_src)/autogen.sh)
# test for the new build system
ifeq (autogen.sh, $(findstring autogen.sh,$(HAVE_AUTOGEN_SH)))
#----------------------------------------------------------------------------#
# new autotools build system
pd: set_version
	cd $(pd_src) \
		&& ./autogen.sh \
		&& ./configure $(PD_CONFIGURE_FLAGS) \
		&& make $(DEST_PATHS) $(PD_BUILD_FLAGS) CFLAGS="-O2 -g"

PD_NAME=Pd
pd_install: pd
	$(MAKE) -C $(pd_src) $(DEST_PATHS) $(PD_BUILD_FLAGS) install
	install -d $(DESTDIR)$(manualsdir)/$(PD_NAME)
	install -p $(pd_src)/src/notes.txt $(DESTDIR)$(manualsdir)/$(PD_NAME)
else
#----------------------------------------------------------------------------#
# old build system
PD_AUTOCONF = autoconf
$(pd_src)/src/configure: $(pd_src)/src/configure.in
	cd $(pd_src)/src/ && \
	if [ -f acinclude.m4 ] ; then aclocal ; fi && \
	$(PD_AUTOCONF) && \
	if grep -q AC_CONFIG_HEADERS configure.in ; then autoheader ; fi

# this is not used yet because MinGW doesn't use ./configure && make yet
#$(pd_src)/src/makefile: $(pd_src)/src/makefile.in
#	cd $(pd_src)/src/ && ./configure $(PD_CONFIGURE_FLAGS)

# this line has the "-" to support the current makefile.mingw hack
pd: $(pd_src)/src/configure set_version
	-cd $(pd_src)/src && ./configure $(PD_CONFIGURE_FLAGS) && \
		make OPT_CFLAGS="" depend  && \
		make $(DEST_PATHS) $(PD_BUILD_FLAGS)

PD_NAME=Pd
pd_install: pd
	$(MAKE) -C $(pd_src)/src $(DEST_PATHS) $(PD_BUILD_FLAGS) install
	install -d $(DESTDIR)$(manualsdir)/$(PD_NAME)
	install -p $(pd_src)/src/notes.txt $(DESTDIR)$(manualsdir)/$(PD_NAME)

endif


#==============================================================================#
#
# INSTALL TARGETS
#
#==============================================================================#

# this is used for installing into a pre-build Pd binary
extended_install: abstractions_install doc_install \
externals_install readme_install welcome_install \
license_install noncvs_install pd_startup_install


#------------------------------------------------------------------------------
# install
install: pd_install extended_install
	@echo "Complete install succeeded!"

#------------------------------------------------------------------------------
# install_devel
devel_install: pd_devel_install
	@echo "Complete install succeeded!"

#------------------------------------------------------------------------------
# abstractions_install
abstractions_install:
	$(MAKE) -C $(abstractions_src) $(DEST_PATHS) install

#------------------------------------------------------------------------------
# doc_install
doc_install: 
	$(MAKE) -C $(doc_src) $(DEST_PATHS) install


#------------------------------------------------------------------------------
# extensions_install
extensions_install: 
	$(MAKE) -C $(extensions_src) $(DEST_PATHS) install


#------------------------------------------------------------------------------
# externals_install
externals_install: 
	$(MAKE) -C $(externals_src) $(DEST_PATHS) install

#------------------------------------------------------------------------------
# noncvs_install
# this is for including pre-compiled binaries in a build
noncvs_install:
	install -d $(DESTDIR)$(objectsdir)
	test ! -d $(packages_src)/noncvs/$(OS_NAME)/extra/ || \
		(cd $(packages_src)/noncvs/$(OS_NAME)/extra/ && \
		tar --exclude .svn --exclude .git --exclude CVS --create * | \
		tar --extract --directory $(DESTDIR)$(objectsdir) )

#------------------------------------------------------------------------------
# make loaders start automatically by being in new 'startup' folder
pd_startup_install:
	install -d $(DESTDIR)$(libpddir)/startup
	cd $(DESTDIR)$(libpddir)/startup && \
		$(LN_S) -f ../extra/libdir/libdir.$(EXTENSION) 0.libdir.$(EXTENSION) && \
		$(LN_S) -f ../extra/vanilla/list.$(EXTENSION) 1.list.$(EXTENSION) && \
		$(LN_S) -f ../extra/vanilla/ vanilla && \
		$(LN_S) -f ../extra/extra/ extra && \
		$(LN_S) -f ../extra/tclpd/ tclpd && \
		$(LN_S) -f ../extra/pdlua/ pdlua


#==============================================================================#
#
# GENERATE TEXT FILES FOR PACKAGE
#
#==============================================================================#

LICENSE_FILE = $(DESTDIR)$(manualsdir)/$(PD_NAME)/License.html
license_install:
  # generate HTML version of License
	install -d $(DESTDIR)$(manualsdir)/$(PD_NAME)
	-rm $(LICENSE_FILE)
	touch $(LICENSE_FILE)
	echo "<html><body>" >> "$(LICENSE_FILE)"
	echo "<h3>(Parts of this package can be used under " >> "$(LICENSE_FILE)"
	echo "<a href="Pd-LICENSE.txt">Pd&#146;s BSD license</a>)</h3>" >> "$(LICENSE_FILE)"
	echo "<font size=\"-1\">" >> "$(LICENSE_FILE)"
	cat "$(packages_src)/gpl-3.0.txt" | sed -e 's/^$$/\<P\>/g' >> "$(LICENSE_FILE)"
	echo "</font></body></html>" >> $(LICENSE_FILE)
# Pd's license file
	install -p "$(pd_src)/LICENSE.txt" "$(DESTDIR)$(manualsdir)/$(PD_NAME)/Pd-LICENSE.txt"



WELCOME_FILE = $(DESTDIR)$(manualsdir)/$(PD_NAME)/Welcome.html
welcome_install:
	install -d $(DESTDIR)$(manualsdir)/$(PD_NAME)
	-rm $(WELCOME_FILE)
	touch $(WELCOME_FILE)
	echo "<html><head>" >> $(WELCOME_FILE)
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"http://puredata.org/ploneCustom.css\" />" >> $(WELCOME_FILE)
	echo "</head>" >> $(WELCOME_FILE)
	echo "<body><p><p>" >> $(WELCOME_FILE)
	echo "<center><img src=\"\">" >> $(WELCOME_FILE)
	echo "<h2>Version $(PD_VERSION)</h2>" >> $(WELCOME_FILE)
	echo "<p>written by Miller S. Puckette</p></center>" >> $(WELCOME_FILE)
	echo "<font size=\"-1\">" >> $(WELCOME_FILE)
	echo "<p>`grep -A9 ACKNOWLEDG $(pd_src)/README.txt`</p>" >> $(WELCOME_FILE)
	echo "</font>" >> $(WELCOME_FILE)
	echo "</body></html>" >> $(WELCOME_FILE)



README_FILE = $(DESTDIR)$(manualsdir)/$(PD_NAME)/ReadMe.html
readme_install: 
	install -d $(DESTDIR)$(manualsdir)/$(PD_NAME)
	echo $(CYCLONE_RELEASE)
	-rm $(README_FILE)
	touch $(README_FILE)
	echo "<html>" >> $(README_FILE)
	echo "<head>" >> $(README_FILE)
	echo "<meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\" />" >> $(README_FILE)
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"http://puredata.info/plone.css\" />" >> $(README_FILE)
	echo "</head>" >> $(README_FILE)
	echo "<body>" >> $(README_FILE)
	echo "<h2>Pure Data $(PD_VERSION)</h2>" 	>> $(README_FILE)
	echo "<p>Pd is a real-time, graphical programming language for media processing.  It provides an environment for audio analysis, synthesis, and processing, with a rich set of multimedia capabilities.  You can get Pd for Linux, Windows, MacOS X, BSD, or IRIX.</p>" >> $(README_FILE)
	echo "<p>For more information, go to: <a href=\"http://puredata.info\" target=\"pd\">http://puredata.info</a></p>" >> $(README_FILE)
	echo "<h3>Installation</h3>" >> $(README_FILE)	
ifeq ($(OS_NAME),linux)
	echo "<p></p>" >> $(README_FILE)
	echo "<p></p>" >> $(README_FILE)
endif
ifeq ($(OS_NAME),darwin)
	echo "<p>To install Pd, drag the $(PD_APP_NAME).app into your <b>/Applications</b> folder or to the shortcut in the disk image.  (<i>You might need to put the old one in the trash before copying this one to <b>/Applications</b></i>)</p>" >> $(README_FILE)	
	echo "<p>By default, most of the included libraries are loaded at startup.  To change this, set your own preferences in the <b>Startup...</b> Preferences pane usings the <b>Save all settings</b> button.   If you want to start with the default preferences, throw away the Pd preferences file <strong>~/Library/Preferences/org.puredata.pdextended.plist</strong> (<code>~</code> <i>means your home folder</i>), or run this command in the Terminal.app: </p><p><code>rm ~/Library/Preferences/org.puredata.pdextended.plist</code></p><p>The Preferences panels in Pd are currently buggy, so you might have better luck with the Apple utilities: <code>defaults read org.puredata.pdextended</code>, or, <b>Property List Editor</b>, which is freely available as part of <a href=\"http://developer.apple.com/tools/xcode/download/\" target=\"_blank\">XCode</a> or the <a href=\"http://www.apple.com/support/downloads/serveradmintools104.html\" target=\"_blank\">Server Tools</a>.</p>" >> $(README_FILE)
	echo "If you want to use PDP on Mac OS X 10.4/Tiger or 10.3/Panther, you will need to install X11 (X11 comes installed with 10.5/Leopard).  It comes on the install CD/DVD that your computer came with.  For more detail, see <a href=\"http://www.simplehelp.net/2006/10/22/how-to-install-x11-in-os-x/\" target=\"_blank\">How to install X11 in OS X</a> or <a href=\"http://puredata.info/docs/faq/macosx\" target=\"_blank\">How do I install Pd on MacOS X?</a>." >> $(README_FILE)
endif
ifeq ($(OS_NAME),windows)
	echo "<p>To reset the default settings, and make sure that default libraries are loaded when Pd runs, " >> $(README_FILE)
	echo "double-click <code>C:\Program Files\pd\bin\pd-settings.reg</code> to import the settings to the registry.  WARNING: this will overwrite any existing Pd preferences!</p>" >> $(README_FILE)
	echo "<h3>ASIO4ALL</h3>" >> $(README_FILE)	
	echo "<p><a href=\"http://www.asio4all.com/\" target=\"_blank\">ASIO4ALL</a> is a cost-free <a href=\"http://en.wikipedia.org/wiki/Audio_Stream_Input/Output\" target=\"_blank\">ASIO</a> audio driver.  Is it highly recommended for use with Pd.  Download and install it, then when you launch Pd, choose the <strong>ASIO</strong> option from the <strong>Media</strong> menu.  Using ASIO4ALL should mean less crashes and troubles, as well as better audio performance." >> $(README_FILE)
	echo "</p>" >> $(README_FILE)
endif
	echo "<h3>Installing Externals, Objects, and Help files</h3>" >> $(README_FILE)
	echo "<p>" >> $(README_FILE)
	echo "If you would like to install other externals, objects, help files, etc. there are special folders that Pd-extended uses, which are listed below.  If the folder does not exist, you should create it.  You can find out more details about this by reading that FAQ:  <a href=\"http://puredata.info/docs/faq/how-do-i-install-externals-and-help-files\" target=\"_blank\">How do I install externals and help files?</a>." >> $(README_FILE)
	echo "<dl>" >> $(README_FILE)
ifeq ($(OS_NAME),darwin)
	echo "<dt>Only for the current user account</dt>" >> $(README_FILE)
	echo "<dd>~/Library/Pd</dd>" >> $(README_FILE)
	echo "<dt>For all user accounts on the computer</dt>" >> $(README_FILE)
	echo "<dd>/Library/Pd</dd>" >> $(README_FILE)
endif
ifeq ($(OS_NAME),linux)
	echo "<dt>Only for the current user account</dt>" >> $(README_FILE)
	echo "<dd>~/pd-externals/</dd>" >> $(README_FILE)
	echo "<dt>For all user accounts on the computer</dt>" >> $(README_FILE)
	echo "<dd>/usr/local/lib/pd-externals</dd>" >> $(README_FILE)
endif
ifeq ($(OS_NAME),windows)
	echo "<dt>Only for the current user account</dt>" >> $(README_FILE)
	echo "<dd><a href=\"file:///%UserProfile%/Application%20Data\">%UserProfile%\Application Data\Pd</a></dd>" >> $(README_FILE)
	echo "<dt>For all user accounts on the computer</dt>" >> $(README_FILE)
	echo "<dd><a href=\"file:///%CommonProgramFiles%/\">%ProgramFiles%\Common Files\Pd</a></dd>" >> $(README_FILE)
endif
	echo "</dl>" >> $(README_FILE)
	echo "</p>" >> $(README_FILE)
	echo "<h3>Flext Binaries have been removed from Pd-extended</h3>" >> $(README_FILE)	
	echo "<p>As of release 0.41.4 of Pd-extended, the whole package is built from source every night.  That means that some objects have been removed from the package because they were not being built from source (objects like [pool], [msd], [flashserver], etc.).  You can download these objects from the original authors or <a href=\"http://puredata.info/docs/faq/how-do-i-download-the-flext-binaries-that-used-to-be-included-in-pd-extended\" target=\"_blank\">get the files from the 0.40.3 Pd-extended release</a>." >> $(README_FILE)
	echo "</p>" >> $(README_FILE)
	echo "<h3>License</h3>" >> $(README_FILE)	
	echo "<p>" >> $(README_FILE)
	echo 'This package is released under the <a href="http://www.gnu.org/copyleft/gpl.html" target="gpl">GNU GPL</a>.  The Pd core and some other included code is originally available with a <a href="http://pure-data.cvs.sourceforge.net/pure-data/pd/LICENSE.txt">BSD license</a> from the Pd CVS on SourceForge.' >> $(README_FILE)
	echo "</p>" >> $(README_FILE)
	echo "<h3>Patented Algorithms</h3>" >> $(README_FILE)
	echo "<p>" >> $(README_FILE)
	echo "This package may contain software that is covered by patents in certain countries, like the U.S. and Germany.  In order to use this software you must have the proper license.  Below is a list of the known software packages that are covered by patents in some countries:" >> $(README_FILE)
	echo "</p>" >> $(README_FILE)
	echo "<ul>" >> $(README_FILE)
	echo '<li><a href="http://en.wikipedia.org/wiki/MP3" target="license">MP3 aka MPEG-1 Part 3 Layer 3</a></li>' >> $(README_FILE)
	echo '<li><a href="http://en.wikipedia.org/wiki/MPEG-2" target="license">MPEG-2</a></li>' >> $(README_FILE)
	echo '<li><a href="http://en.wikipedia.org/wiki/MPEG-4#Licensing" target="license">MPEG-4</a></li>' >> $(README_FILE)
	echo "</ul>" >> $(README_FILE)
	echo '<p>Please consider trying to get rid of software patents in your country: <a href="http://www.nosoftwarepatents.com" target="nsp">http://www.nosoftwarepatents.com</a></p>' >> $(README_FILE)
	echo "<h3>Pure Data CVS Developers</h3>" >> $(README_FILE)	
# this may seem whack, but it generates the list of developers from the SourceForge site:
	curl 'http://sourceforge.net/project/memberlist.php?group_id=55736' | grep -A2 -e '<td>' | sed 's|\(href="\)|target="w" \1http://sourceforge.net|' >> $(README_FILE)
	echo "</p>" >> $(README_FILE)
	echo "<p>" >> $(README_FILE)
	echo "Many others not listed have contributed their time and effort, this is just a list of the current developers in the SourceForge project.  But really, every Pd user is a developer and is encouraged to contribute to the CVS repository." >> $(README_FILE)
	echo "</p>" >> $(README_FILE)
	echo "<h3>Included Libraries</h3>" >> $(README_FILE)	
	echo "<p>To find out about which libraries are included in Pd-extended, see <a href=\"https://puredata.info/docs/LibrariesInPdExtended\" target=\"_blank\">Libraries in Pd-extended</a>.  Here's some of the libraries that are included:</p>" >> $(README_FILE)
	echo "<ul>" >> $(README_FILE)
	echo "<li>cyclone: $(CYCLONE_VERSION)" >> $(README_FILE)
	echo "<li>gem: $(GEM_VERSION)" >> $(README_FILE)
	echo "<li>maxlib: $(MAXLIB_VERSION)" >> $(README_FILE)
	echo "<li>oscx: $(OSCX_VERSION)" >> $(README_FILE)
	echo "<li>pdp: $(PDP_VERSION)" >> $(README_FILE)
	echo "<li>zexy: $(ZEXY_VERSION)" >> $(README_FILE)
	echo "</ul>" >> $(README_FILE)
	echo "<p>(this package was built on `date`)</p>" >> $(README_FILE)
	echo "</body></html>" >> $(README_FILE)



#==============================================================================#
#
## FINAL ASSEMBLY
#
#==============================================================================#


#----------------------------------------------------------------------------
# clean up the documentation before packaging
doc_format:
# clean out cruft files
	-find $(DESTDIR) -name .DS_Store -delete
	-find $(DESTDIR) -name '*.*.bak' -delete
	-find $(DESTDIR) -name .svn -print0 | xargs -0 rm -rf


#==============================================================================#
#
# DEVELOPER TARGETS
#
#==============================================================================#

set_version:
# change Pd's version number to reflect the extended build
# this needs the complete_version_defines patch to work
	cd $(pd_src)/src/ && \
		sed 's|^\(#define PD_TEST_VERSION "\).*"|\1$(PD-EXTENDED_VERSION)"|' m_pd.h > \
			m_pd.h.tmp && mv m_pd.h.tmp m_pd.h

unset_version:
# change the version number back to the original 
# this needs the complete_version_defines patch to work
	cd $(pd_src)/src && \
		sed 's|^\(#define PD_TEST_VERSION ".*\)$(PD-EXTENDED_VERSION_PREFIX).*"|\1"|' \
			m_pd.h > m_pd.h.tmp && \
		mv m_pd.h.tmp m_pd.h

#==============================================================================#
#
# CLEAN TARGETS
#
#==============================================================================#
abstractions_clean:
	-$(MAKE) -C $(abstractions_src) $(DEST_PATHS) clean

doc_clean:

externals_clean:
	-$(MAKE) -C $(externals_src) $(DEST_PATHS) clean

pd_clean:
	-$(MAKE) -C $(pd_src)/src $(DEST_PATHS) clean

# these targets are all from Makefile.buildlayout: install_clean cruft_clean
clean: abstractions_clean doc_clean externals_clean pd_clean 
	echo "Complete clean finished."


distclean: cruft_clean
	$(MAKE) -C $(abstractions_src) distclean
	$(MAKE) -C $(doc_src) distclean
	$(MAKE) -C $(extensions_src) distclean
	$(MAKE) -C $(externals_src) distclean
	-$(MAKE) -C $(pd_src) distclean

test_locations:
	@echo "PD_VERSION: $(PD_VERSION)"
	@echo "PD-EXTENDED_VERSION: $(PD-EXTENDED_VERSION)"
	@echo "CWD $(CWD)"
	@echo "DESTDIR $(DESTDIR)"
	@echo "PREFIX $(prefix)"
	@echo "BINDIR  $(bindir)"
	@echo "LIBDIR  $(libdir)"
	@echo "OBJECTSDIR  $(objectsdir)"
	@echo "PDDOCDIR  $(pddocdir)"
	@echo "LIBPDDIR  $(libpddir)"
	@echo "LIBPDBINDIR  $(libpdbindir)"
	@echo "HELPDIR  $(helpdir)"
	@echo "MANUALSDIR  $(manualsdir)"
	@echo "EXAMPLESDIR  $(examplesdir)"
	@echo "HAVE_AUTOGEN_SH --$(HAVE_AUTOGEN_SH)--"
	@echo "LN_S $(LN_S)"

.PHONY = abstractions doc externals pd pd_install pd_startup_install extended_install install devel_install abstractions_install doc_install extensions_install externals_install noncvs_install license_install welcome_install readme_install doc_format set_version unset_version abstractions_clean doc_clean externals_clean pd_clean clean distclean test_locations
