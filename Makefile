CC          := mipsel-linux-gcc
STRIP       := mipsel-linux-strip

SYSROOT     := $(shell $(CC) --print-sysroot)
SDL1_CFLAGS := $(shell $(SYSROOT)/usr/bin/sdl-config --cflags) -DSDL_1
SDL1_LIBS   := $(shell $(SYSROOT)/usr/bin/sdl-config --libs) -lSDL_ttf
SDL2_CFLAGS := $(shell $(SYSROOT)/usr/bin/sdl2-config --cflags) -DSDL_2
SDL2_LIBS   := $(shell $(SYSROOT)/usr/bin/sdl2-config --libs) -lSDL2_ttf

OBJS        := sdl-1.2.o sdl-2.o
HEADERS     :=

INCLUDE     := -I.
DEFS        +=

CFLAGS       = -Wall -Wno-unused-variable \
               -O2 -fomit-frame-pointer $(DEFS) $(INCLUDE)
LDFLAGS     := -lshake

.PHONY: all opk

all: input-test-sdl-1.2 input-test-sdl-2

include Makefile.rules

input-test-sdl-1.2: sdl-1.2.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(SDL1_CFLAGS) $(SDL1_LIBS) -o $@ $^

input-test-sdl-2: sdl-2.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(SDL2_CFLAGS) $(SDL2_LIBS) -o $@ $^

sdl-1.2.o: sdl.c
	$(CC) $(CFLAGS) $(SDL1_CFLAGS) -o $@ -c $<

sdl-2.o: sdl.c
	$(CC) $(CFLAGS) $(SDL2_CFLAGS) -o $@ -c $<

opk: input-test.opk

input-test.opk: input-test-sdl-1.2 input-test-sdl-2
	$(SUM) "  OPK     $@"
	$(CMD)rm -rf .opk_data
	$(CMD)cp -r data .opk_data
	$(CMD)cp $^ .opk_data
	$(CMD)mksquashfs .opk_data $@ -all-root -noappend -no-exports -no-xattrs -no-progress >/dev/null

# The two below declarations ensure that editing a .c file recompiles only that
# file, but editing a .h file recompiles everything.
# Courtesy of Maarten ter Huurne.

# Each object file depends on its corresponding source file.
$(C_OBJS): %.o: %.c

# Object files all depend on all the headers.
$(OBJS): $(HEADERS)
