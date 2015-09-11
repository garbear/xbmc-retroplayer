-include ../../Makefile.include
DEPS = ../../Makefile.include Makefile Makefile

# lib name, version
LIBNAME=ejdb
VERSION=1.2.10
SOURCE=v$(VERSION)
ARCHIVE=$(SOURCE).tar.gz
BASE_URL := https://github.com/Softmotions/ejdb/archive

LIBDYLIB=$(PLATFORM)/build/src/lib$(LIBNAME)-1.a

all: .installed-$(PLATFORM)

$(TARBALLS_LOCATION)/$(ARCHIVE):
	cd $(TARBALLS_LOCATION); $(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE) $(DEPS)
ifeq ($(PREFIX),)
	@echo
	@echo "ERROR: please set PREFIX to the kodi install path e.g. make PREFIX=/usr/local"
	@exit 1
endif
	rm -rf $(PLATFORM); mkdir -p $(PLATFORM)/build
	cd $(PLATFORM); $(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	cd $(PLATFORM)/build; $(CMAKE) -DCMAKE_BUILD_TYPE=Debug -DBUILD_SHARED_LIBS=0 -DPACKAGE_TGZ=0 -DPACKAGE_ZIP=0 ..

$(LIBDYLIB): $(PLATFORM)
	$(MAKE) -C $(PLATFORM)/build

.installed-$(PLATFORM): $(LIBDYLIB)
	$(MAKE) -C $(PLATFORM)/build install
	touch $@

clean:
	$(MAKE) -C $(PLATFORM)/build clean
	rm -rf $(PLATFORM) .installed-$(PLATFORM)

distclean::
	rm -rf $(PLATFORM) .installed-$(PLATFORM)
