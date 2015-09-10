include ../../Makefile.include
DEPS= ../../Makefile.include Makefile

# lib name, version
LIBNAME=unqlite
VERSION=1.1.6
SOURCE=unqlite-db-116
ARCHIVE=$(SOURCE).zip
ARCHIVE_TOOL=unzip
ARCHIVE_TOOL_FLAGS=
BASE_URL := http://unqlite.org/db

LIBDYLIB=$(PLATFORM)/lib$(LIBNAME).a

all: .installed-$(PLATFORM)

$(TARBALLS_LOCATION)/$(ARCHIVE):
	cd $(TARBALLS_LOCATION); $(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE) $(DEPS)
	rm -rf $(PLATFORM); mkdir -p $(PLATFORM)
	cd $(PLATFORM); $(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)

$(LIBDYLIB): $(PLATFORM)
	$(CC) -c -o $(PLATFORM)/unqlite.o $(PLATFORM)/unqlite.c
	$(AR) rcs $@ $(PLATFORM)/unqlite.o

.installed-$(PLATFORM): $(LIBDYLIB)
	cp $(LIBDYLIB) $(PREFIX)/lib
	cp $(PLATFORM)/unqlite.h $(PREFIX)/include
	touch $@

clean:
	rm -rf $(PLATFORM) .installed-$(PLATFORM)

distclean::
	rm -rf $(PLATFORM) .installed-$(PLATFORM)
