CC      ?= cc
ifeq ($(OS,Windows_NT)
	TARGET := output.exe
else
	TARGET := output
endif

PKG_CONFIG ?= pkg-config

CFLAGS  += $(shell $(pkg-config) --cflags sdl3)
LDLIBS += $(shell $(pkg-config) --libs sdl3)

SRC     = main.c

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $@ $(LDLIBS)

clean:
	rm -f $@
