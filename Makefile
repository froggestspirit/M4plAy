CC ?= gcc

CFLAGS = -Wall -std=c11 -O2

LIBS = -lm -lportaudio

SRCS = src/main.c src/m4a.c src/m4a_tables.c src/sound_mixer.c

.PHONY: all clean

all: M4plAy
	@:

M4plAy: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $@ $(LDFLAGS) $(LIBS)

clean:
	$(RM) M4plAy M4plAy.exe