SRCS=ButtonKeyHandler.cpp \
     DefaultController.cpp \
     JoystickDriverPrimitive.cpp \
     JoystickEasterEgg.cpp \
     JoystickMonitor.cpp \
     JoystickTranslator.cpp \

LIB=input_joysticks.a

include ../../../Makefile.include
-include $(patsubst %.cpp,%.P,$(patsubst %.c,%.P,$(SRCS)))
