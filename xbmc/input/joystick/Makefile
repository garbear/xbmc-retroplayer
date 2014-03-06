SRCS=ActionObserver.cpp \
     Joystick.cpp \
     JoystickAxis.cpp \
     JoystickButton.cpp \
     JoystickHat.cpp \
     JoystickManager.cpp \
     MouseObserver.cpp \

LIB=joystick.a

include ../../../Makefile.include
-include $(patsubst %.cpp,%.P,$(patsubst %.c,%.P,$(SRCS)))
