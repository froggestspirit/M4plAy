PREFIX := i686-linux-gnu-
OBJCOPY := $(PREFIX)objcopy
CC := $(PREFIX)gcc
AS := $(PREFIX)as
# Native Linux build
CFLAGS :=
ASFLAGS := --defsym
PLATFORM_LIBS := -lSDL2 #-lm -lportaudio

SDL_DIR := 
ASM_PSEUDO_OP_CONV := sed -e 's/\.4byte/\.int/g;s/\.2byte/\.short/g'

export CPP := $(PREFIX)cpp
export LD := $(PREFIX)ld

TITLE       := POKEMON EMER
GAME_CODE   := BPEE
MAKER_CODE  := 01
REVISION    := 0

SHELL := /bin/bash -o pipefail

C_SUBDIR = src
ASM_SUBDIR = asm
DATA_SRC_SUBDIR = src/data
DATA_ASM_SUBDIR = data
SONG_SUBDIR = sound/songs
MID_SUBDIR = sound/songs/midi
SAMPLE_SUBDIR = sound/direct_sound_samples

C_BUILDDIR = $(OBJ_DIR)/$(C_SUBDIR)
ASM_BUILDDIR = $(OBJ_DIR)/$(ASM_SUBDIR)
DATA_ASM_BUILDDIR = $(OBJ_DIR)/$(DATA_ASM_SUBDIR)
SONG_BUILDDIR = $(OBJ_DIR)/$(SONG_SUBDIR)
MID_BUILDDIR = $(OBJ_DIR)/$(MID_SUBDIR)

ASFLAGS := 

GCC_VER := $(shell $(CC) -dumpversion)

CC1 	:= $(shell $(PREFIX)gcc --print-prog-name=cc1) -quiet
override CFLAGS += 
ROM		:= pokeemerald.elf
OBJ_DIR	:= build/pc

CPPFLAGS := -iquote include -iquote -Wno-trigraphs -I/usr/include/SDL2 -D_REENTRANT

LDFLAGS = -Map ../../$(MAP)

LIB := $(LIBPATH) -lgcc -lc

SHA1 := $(shell { command -v sha1sum || command -v shasum; } 2>/dev/null) -c
AIF := tools/aif2pcm/aif2pcm
MID := tools/mid2agb/mid2agb
SCANINC := tools/scaninc/scaninc
PREPROC := tools/preproc/preproc

TOOLDIRS := $(filter-out tools/agbcc tools/binutils,$(wildcard tools/*))
TOOLBASE = $(TOOLDIRS:tools/%=%)
TOOLS = $(foreach tool,$(TOOLBASE),tools/$(tool)/$(tool))

MAKEFLAGS += --no-print-directory

# Clear the default suffixes
.SUFFIXES:
# Don't delete intermediate files
.SECONDARY:
# Delete files that weren't built properly
.DELETE_ON_ERROR:

# Secondary expansion is required for dependency variables in object rules.
.SECONDEXPANSION:

.PHONY: all rom clean tidy tools mostlyclean clean-tools $(TOOLDIRS)

infoshell = $(foreach line, $(shell $1 | sed "s/ /__SPACE__/g"), $(info $(subst __SPACE__, ,$(line))))

# Build tools when building the rom
# Disable dependency scanning for clean/tidy/tools
ifeq (,$(filter-out all rom,$(MAKECMDGOALS)))
$(call infoshell, $(MAKE) tools)
else
NODEP := 1
endif

C_SRCS_IN := $(wildcard $(C_SUBDIR)/*.c $(C_SUBDIR)/*/*.c $(C_SUBDIR)/*/*/*.c)
C_SRCS := $(foreach src,$(C_SRCS_IN),$(if $(findstring .inc.c,$(src)),,$(src)))
C_OBJS := $(patsubst $(C_SUBDIR)/%.c,$(C_BUILDDIR)/%.o,$(C_SRCS))

C_ASM_SRCS += $(wildcard $(C_SUBDIR)/*.s $(C_SUBDIR)/*/*.s $(C_SUBDIR)/*/*/*.s)
C_ASM_OBJS := $(patsubst $(C_SUBDIR)/%.s,$(C_BUILDDIR)/%.o,$(C_ASM_SRCS))

ASM_SRCS := $(wildcard $(ASM_SUBDIR)/*.s)
ASM_OBJS := $(patsubst $(ASM_SUBDIR)/%.s,$(ASM_BUILDDIR)/%.o,$(ASM_SRCS))

# get all the data/*.s files EXCEPT the ones with specific rules
REGULAR_DATA_ASM_SRCS := $(wildcard $(DATA_ASM_SUBDIR)/*.s)

DATA_ASM_SRCS := $(wildcard $(DATA_ASM_SUBDIR)/*.s)
DATA_ASM_OBJS := $(patsubst $(DATA_ASM_SUBDIR)/%.s,$(DATA_ASM_BUILDDIR)/%.o,$(DATA_ASM_SRCS))

SONG_SRCS := $(wildcard $(SONG_SUBDIR)/*.s)
SONG_OBJS := $(patsubst $(SONG_SUBDIR)/%.s,$(SONG_BUILDDIR)/%.o,$(SONG_SRCS))

MID_SRCS := $(wildcard $(MID_SUBDIR)/*.mid)
MID_OBJS := $(patsubst $(MID_SUBDIR)/%.mid,$(MID_BUILDDIR)/%.o,$(MID_SRCS))

OBJS     := $(C_OBJS) $(ASM_OBJS) $(DATA_ASM_OBJS) $(SONG_OBJS) $(MID_OBJS)
OBJS_REL := $(patsubst $(OBJ_DIR)/%,%,$(OBJS))

SUBDIRS  := $(sort $(dir $(OBJS)))

AUTO_GEN_TARGETS :=

$(shell mkdir -p $(SUBDIRS))

all: rom

tools: $(TOOLDIRS)

$(TOOLDIRS):
	@$(MAKE) -C $@ CC=$(HOSTCC) CXX=$(HOSTCXX)

rom: $(ROM)

clean: mostlyclean

clean-tools:
	@$(foreach tooldir,$(TOOLDIRS),$(MAKE) clean -C $(tooldir);)

mostlyclean: tidy
	rm -f $(SAMPLE_SUBDIR)/*.bin
	rm -f $(MID_SUBDIR)/*.s
	find . \( -iname '*.1bpp' -o -iname '*.4bpp' -o -iname '*.8bpp' -o -iname '*.gbapal' -o -iname '*.lz' -o -iname '*.rl' -o -iname '*.latfont' -o -iname '*.hwjpnfont' -o -iname '*.fwjpnfont' \) -exec rm {} +
	rm -f $(AUTO_GEN_TARGETS)

tidy:
	rm -f $(ROM) $(ELF) $(MAP)
	rm -r $(OBJ_DIR)

include songs.mk

%.s: ;
%.aif: ;

sound/%.bin: sound/%.aif ; $(AIF) $< $@

%src/platform/sdl2.o: CFLAGS += -O3


ifeq ($(NODEP),1)
$(C_BUILDDIR)/%.o: c_dep :=
else
$(C_BUILDDIR)/%.o: c_dep = $(shell [[ -f $(C_SUBDIR)/$*.c ]] && $(SCANINC) -I include -I tools/agbcc/include -I $(C_SUBDIR)/$*.c)
endif

ifeq ($(DINFO),1)
override CFLAGS += -g -O0
else
override CFLAGS += -O3
endif

$(C_BUILDDIR)/%.o : $(C_SUBDIR)/%.c $$(c_dep)
	@$(CPP) $(CPPFLAGS) $< -o $(C_BUILDDIR)/$*.i
	@$(PREPROC) $(C_BUILDDIR)/$*.i charmap.txt | $(CC1) $(CFLAGS) -o $(C_BUILDDIR)/$*.s
	@echo -e ".text\n\t.align\t2, 0\n" >> $(C_BUILDDIR)/$*.s
	$(AS) $(ASFLAGS) -o $@ $(C_BUILDDIR)/$*.s


# The dep rules have to be explicit or else missing files won't be reported.
# As a side effect, they're evaluated immediately instead of when the rule is invoked.
# It doesn't look like $(shell) can be deferred so there might not be a better way.


ifeq ($(NODEP),1)
$(ASM_BUILDDIR)/%.o: $(ASM_SUBDIR)/%.s
	$(AS) $(ASFLAGS) -o $@ $<
else
define ASM_DEP
$1: $2 $$(shell $(SCANINC) -I include -I "" $2)
	$$(AS) $$(ASFLAGS) -o $$@ $$<
endef
$(foreach src, $(ASM_SRCS), $(eval $(call ASM_DEP,$(patsubst $(ASM_SUBDIR)/%.s,$(ASM_BUILDDIR)/%.o, $(src)),$(src))))
endif

ifeq ($(NODEP),1)
$(DATA_ASM_BUILDDIR)/%.o: $(DATA_ASM_SUBDIR)/%.s
	$(PREPROC) $< charmap.txt | $(CPP) -I include | $(ASM_PSEUDO_OP_CONV) | $(AS) $(ASFLAGS) -o $@
else
define DATA_ASM_DEP
$1: $2 $$(shell $(SCANINC) -I include -I "" $2)
	$$(PREPROC) $$< charmap.txt | $$(CPP) -I include | $$(ASM_PSEUDO_OP_CONV) | $$(AS) $$(ASFLAGS) -o $$@
endef
$(foreach src, $(REGULAR_DATA_ASM_SRCS), $(eval $(call DATA_ASM_DEP,$(patsubst $(DATA_ASM_SUBDIR)/%.s,$(DATA_ASM_BUILDDIR)/%.o, $(src)),$(src))))
$(foreach src, $(C_ASM_SRCS), $(eval $(call DATA_ASM_DEP,$(patsubst $(C_SUBDIR)/%.s,$(C_BUILDDIR)/%.o, $(src)),$(src))))
endif

$(SONG_BUILDDIR)/%.o: $(SONG_SUBDIR)/%.s
	$(ASM_PSEUDO_OP_CONV) $< | $(AS) $(ASFLAGS) -I sound -o $@

$(ROM): $(C_OBJS)
	$(CC) $(CFLAGS) $^ $(PLATFORM_LIBS) -static-libgcc -o $@