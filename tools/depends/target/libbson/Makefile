-include ../../Makefile.include
ROOT_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

# lib name, version
LIBNAME=libbson
VERSION=1.1.10
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz
BASE_URL := https://github.com/mongodb/libbson/releases/download/1.1.10

ifeq ($(CROSS_COMPILING), yes)
  DEPS = ../../Makefile.include
else

  ifeq ($(PLATFORM),)
    PLATFORM = native
    TARBALLS_LOCATION = $(ROOT_DIR)
    BASE_URL := https://github.com/mongodb/libbson/releases/download/1.1.10
    RETRIEVE_TOOL := curl -Ls --create-dirs -f -O
    ARCHIVE_TOOL := tar --strip-components=1 -xf
  endif
endif

LIBDYLIB=$(PLATFORM)/.libs/$(LIBNAME)-1.0.a

all: .installed-$(PLATFORM) $(PREFIX)/lib/$(LIBNAME)-1.0.a

$(PREFIX)/lib/$(LIBNAME)-1.0.a:
	@$(MAKE) -C $(PLATFORM) install

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

$(LIBDYLIB): $(PLATFORM)
	@if test -f $(LIBDYLIB); then :; else \
          cd $(PLATFORM) && autoreconf -vif && ./configure --prefix=${PREFIX} --disable-shared --enable-static && ${MAKE} ;\
        fi

.installed-$(PLATFORM): $(LIBDYLIB)
	@echo "in installed"
	$(MAKE) -C $(PLATFORM) install
	touch $@

clean:
	$(MAKE) -C $(PLATFORM) clean
	rm -f .installed-$(PLATFORM)

distclean::
	rm -rf $(PLATFORM) .installed-$(PLATFORM)
