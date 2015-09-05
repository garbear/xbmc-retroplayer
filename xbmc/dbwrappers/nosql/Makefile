SRCS=GenericDocumentStore.cpp \
     KyotoCabinetDatastore.cpp \
     LevelDbDatastore.cpp \
     NoSqlDatabase.cpp \

LIB=nosql.a

include ../../../Makefile.include
-include $(patsubst %.cpp,%.P,$(patsubst %.c,%.P,$(SRCS)))
