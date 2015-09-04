SRCS=EjdbDatastore.cpp \
     KyotoCabinetDatastore.cpp \
     LevelDbDatastore.cpp \
     UnQLiteDatastore.cpp \

LIB=nosqlwrappers.a

include ../../../../Makefile.include
-include $(patsubst %.cpp,%.P,$(patsubst %.c,%.P,$(SRCS)))
