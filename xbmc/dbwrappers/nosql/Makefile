SRCS=DenormalizedDatabase.cpp \
     DocumentGroup.cpp \
     GenericDocumentStore.cpp \
     KyotoCabinetDatastore.cpp \
     LevelDbDatastore.cpp \

LIB=nosql.a

include ../../../Makefile.include
-include $(patsubst %.cpp,%.P,$(patsubst %.c,%.P,$(SRCS)))
