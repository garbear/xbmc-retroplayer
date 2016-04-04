#
# To add new process overrides, add a new subdir corresponding to your platform in overrides/
# see configure.ac for CORE_SYSTEM_NAME and CORE_SYSTEM_VARIANT for known platforms.
# If needed add your new platform to configure.ac first.
#

INCLUDES += -I/home/garrett/Documents/steamlink-sdk/examples/kodi/kodi-src/xbmc/cores/VideoPlayer

OVERRIDES = $(wildcard overrides/linux/*.cpp) $(wildcard overrides//*.cpp)
ifneq ($(strip $(OVERRIDES)),)
  CXXFLAGS += -DPLATFORM_OVERRIDE
endif

SRCS  = ProcessInfo.cpp $(OVERRIDES)
LIB = Process.a

include /home/garrett/Documents/steamlink-sdk/examples/kodi/kodi-src/Makefile.include
-include $(patsubst %.cpp,%.P,$(patsubst %.c,%.P,$(SRCS)))
