SRCS=GameClient.cpp \
     GameFileLoader.cpp \
     GameManager.cpp \
     SerialState.cpp

LIB=games.a

include ../../Makefile.include
-include $(patsubst %.cpp,%.P,$(patsubst %.c,%.P,$(SRCS)))
