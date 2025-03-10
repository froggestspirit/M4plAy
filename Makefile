C ?= gcc
CFLAGS := -Wall
AR ?= ar

LDFLAGS := -lm -lportaudio -L./ -lm4play

LIB_SRCS := src/*.c

SRCS := main.cpp

HEADERS := m4play.h

.PHONY: all clean

all: libm4play.a M4plAy123 libm4play.so tidy
	@:

libm4play.a: $(LIB_SRCS)
	$(C) $(CFLAGS) $(LIB_SRCS) -c
	$(AR) -cvq libm4play.a *.o

libm4play.so: $(LIB_SRCS)
	$(C) $(CFLAGS) -fPIC -shared -o libm4play.so $(LIB_SRCS) -lm -lportaudio

M4plAy123: $(SRCS) $(HEADERS)
	g++ $(CFLAGS) $(SRCS) -o $@ $(LDFLAGS)

tidy:
	$(RM) *.o

clean:
	$(RM) M4plAy123 M4plAy123.exe libm4play.a libm4play.so *.o
