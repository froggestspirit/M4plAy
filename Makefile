C ?= gcc

CFLAGS := -O2 -Wall

LDFLAGS := -lm -lportaudio

SRCS := src/main.c src/m4a.c src/m4a_tables.c src/sound_mixer.c

HEADERS := src/io_reg.h src/m4a.h src/m4a_internal.h src/sound_mixer.h

.PHONY: all clean

all: M4plAy
	@:

M4plAy: $(SRCS) $(HEADERS)
	$(C) $(CFLAGS) $(SRCS) -o $@ $(LDFLAGS)

clean:
	$(RM) M4plAy M4plAy.exe
