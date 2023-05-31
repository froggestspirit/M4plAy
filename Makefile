C ?= i686-linux-gnu-gcc

CFLAGS := -O2 -Wall

LDFLAGS := -lSDL2

SRCS := src/main.c src/cgb_audio.c src/m4a.c src/m4a_tables.c src/music_player.c src/sound_mixer.c


.PHONY: all clean

all: M4plAy
	@:

M4plAy: $(SRCS)
	$(C) $(CFLAGS) $(SRCS) -o $@ $(LDFLAGS)

clean:
	$(RM) M4plAy M4plAy.exe