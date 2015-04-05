SRCS=GameClient.cpp \
     GameClientProperties.cpp \
     GamePeripheral.cpp \
     GamePeripheralFeature.cpp \
     GamePeripheralLayout.cpp \

LIB=gameaddons.a

include ../../../Makefile.include
-include $(patsubst %.cpp,%.P,$(patsubst %.c,%.P,$(SRCS)))
