-include ../../Makefile.include
DEPS = ../../Makefile.include Makefile Makefile

# lib name, version
LIBNAME=kyotocabinet
VERSION=1.2.76
SOURCE=$(VERSION)
ARCHIVE=$(LIBNAME)-$(SOURCE).tar.gz
BASE_URL := http://fallabs.com/kyotocabinet/pkg

LIBDYLIB=$(PLATFORM)/lib$(LIBNAME).a

all: .installed-$(PLATFORM)

$(TARBALLS_LOCATION)/$(ARCHIVE):
	cd $(TARBALLS_LOCATION); $(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE) $(DEPS)
ifeq ($(PREFIX),)
	@echo
	@echo "ERROR: please set PREFIX to the kodi install path e.g. make PREFIX=/usr/local"
	@exit 1
endif
	rm -rf $(PLATFORM); mkdir -p $(PLATFORM)
	cd $(PLATFORM); $(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	cd $(PLATFORM); patch -p1 < ../0001_Remove_TR1_headers.patch
	cd $(PLATFORM); patch -p1 < ../0002_Fix_return_value.patch
	cd $(PLATFORM); autoreconf -vif
	cd $(PLATFORM); ./configure --prefix=$(PREFIX) --disable-shared --enable-static

$(LIBDYLIB): $(PLATFORM)
	$(MAKE) -C $(PLATFORM)

.installed-$(PLATFORM): $(LIBDYLIB)
	$(MAKE) -C $(PLATFORM) install
	touch $@

clean:
	$(MAKE) -C $(PLATFORM) clean
	rm -rf $(PLATFORM) .installed-$(PLATFORM)

distclean::
	rm -rf $(PLATFORM) .installed-$(PLATFORM)
