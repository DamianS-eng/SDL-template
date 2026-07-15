CC      ?= cc
ifeq ($(OS),Windows_NT)
	RM := del /Q
	TARGET := output.exe
else
	RM := rm -f
	TARGET := output
endif

PKG_CONFIG ?= pkg-config

CFLAGS  += $(shell $(PKG-CONFIG) --cflags sdl3)
LDLIBS += $(shell $(PKG-CONFIG) --libs sdl3)

SRC     = main.c

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $@ $(LDLIBS)

clean:
	$(RM) $(TARGET)
