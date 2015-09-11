-include ../../Makefile.include
ROOT_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

# lib name, version
LIBNAME=ejdb-1
VERSION=1.2.10
SOURCE=v$(VERSION)
ARCHIVE=$(SOURCE).tar.gz
BASE_URL := https://github.com/Softmotions/ejdb/archive

ifeq ($(CROSS_COMPILING), yes)
  DEPS = ../../Makefile.include
else
  ifeq ($(PLATFORM),)
    PLATFORM = native
    TARBALLS_LOCATION = $(ROOT_DIR)
    RETRIEVE_TOOL := curl -Ls --create-dirs -f -O
    ARCHIVE_TOOL := tar --strip-components=1 -xf
  endif
endif

LIBDYLIB=$(PLATFORM)/build/src/lib$(LIBNAME).a

all: .installed-$(PLATFORM) $(PREFIX)/lib/lib$(LIBNAME).a

$(PREFIX)/lib/lib$(LIBNAME).a:
	@$(MAKE) -C $(PLATFORM)/build install

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

$(LIBDYLIB): $(PLATFORM)
	@if test -f $(LIBDYLIB); then :; else \
	   cd $(PLATFORM)/build && cmake -DCMAKE_INSTALL_PREFIX=$(PREFIX) -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_SHARED_LIBS=0 -DPACKAGE_TGZ=0 -DPACKAGE_ZIP=0 .. && $(MAKE); \
	 fi

.installed-$(PLATFORM): $(LIBDYLIB)
	$(MAKE) -C $(PLATFORM)/build install
	touch $@

clean:
	$(MAKE) -C $(PLATFORM)/build clean
	rm -f .installed-$(PLATFORM)

distclean::
	rm -rf $(PLATFORM) .installed-$(PLATFORM)
