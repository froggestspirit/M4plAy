C ?= gcc

CFLAGS := -O2 -Wall

LDFLAGS := -lSDL2

SRCS := src/*.c


.PHONY: all clean

all: M4plAy
	@:

M4plAy: $(SRCS)
	$(C) $(CFLAGS) $(SRCS) -o $@ $(LDFLAGS)

clean:
	$(RM) M4plAy M4plAy.exe