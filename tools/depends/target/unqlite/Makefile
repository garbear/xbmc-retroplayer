-include ../../Makefile.include
ROOT_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

# lib name, version
LIBNAME=unqlite
VERSION=1.1.6
SOURCE=unqlite-db-116
ARCHIVE=$(SOURCE).zip
ARCHIVE_TOOL=unzip
ARCHIVE_TOOL_FLAGS=
BASE_URL := http://unqlite.org/db

ifeq ($(CROSS_COMPILING), yes)
  DEPS = ../../Makefile.include
else
  ifeq ($(PLATFORM),)
    PLATFORM = native
    TARBALLS_LOCATION = $(ROOT_DIR)
    RETRIEVE_TOOL := curl -Ls --create-dirs -f -O
  endif
endif

LIBDYLIB=$(PLATFORM)/lib$(LIBNAME).a

all: .installed-$(PLATFORM) $(PREFIX)/lib/lib$(LIBNAME).a

$(PREFIX)/lib/lib$(LIBNAME).a:
	mkdir -p $(PREFIX)/lib
	mkdir -p $(PREFIX)/include
	mkdir -p $(PREFIX)/share/doc/unqlite
	cp $(LIBDYLIB) $(PREFIX)/lib
	cp $(PLATFORM)/unqlite.h $(PREFIX)/include
	cp $(PLATFORM)/license.txt $(PREFIX)/share/doc/unqlite

$(TARBALLS_LOCATION)/$(ARCHIVE):
	cd $(TARBALLS_LOCATION); $(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE) $(DEPS)
	rm -rf $(PLATFORM); mkdir -p $(PLATFORM)
	cd $(PLATFORM); $(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)

$(LIBDYLIB): $(PLATFORM)
	$(CC) -c -o $(PLATFORM)/unqlite.o $(PLATFORM)/unqlite.c
	$(AR) rcs $@ $(PLATFORM)/unqlite.o

.installed-$(PLATFORM): $(LIBDYLIB)
	mkdir -p $(PREFIX)/lib
	mkdir -p $(PREFIX)/include
	mkdir -p $(PREFIX)/share/doc/unqlite
	cp $(LIBDYLIB) $(PREFIX)/lib
	cp $(PLATFORM)/unqlite.h $(PREFIX)/include
	cp $(PLATFORM)/license.txt $(PREFIX)/share/doc/unqlite
	touch $@

clean:
	rm -f .installed-$(PLATFORM)

distclean::
	rm -rf $(PLATFORM) .installed-$(PLATFORM)
