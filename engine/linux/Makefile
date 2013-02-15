EXECUTABLE     = shmup
INCLUDES       = ../src libpng

linux_SOURCES  := native.c main.c
linux_OBJECTS  := $(linux_SOURCES:.c=.o)

engine_SOURCES := $(wildcard ../src/*.c)
engine_OBJECTS := $(engine_SOURCES:.c=.o) 

libpng_SOURCES := $(wildcard libpng/*.c)
# Do not compile pngtest.c
libpng_SOURCES := $(filter-out libpng/pngtest.c,$(libpng_SOURCES))
libpng_OBJECTS := $(libpng_SOURCES:.c=.o)

CFLAGS   = -Wall -Wextra -Wmissing-prototypes -DLINUX $(addprefix -I,$(INCLUDES))
CFLAGS  += `sdl-config --cflags` `pkg-config zlib --cflags` `pkg-config openal --cflags`
LDFLAGS  = -lGL -lm -z `sdl-config --libs` `pkg-config zlib --libs` `pkg-config openal --libs` -lSDL_mixer

OBJECTS = $(linux_OBJECTS) $(engine_OBJECTS) $(libpng_OBJECTS)

all: $(EXECUTABLE)

.PHONY: debug
debug: CFLAGS += -g
debug: all

.PHONY: release
release: CFLAGS += -DRELEASE
release: all

shmup: $(OBJECTS)
	gcc -o $@ $(LDFLAGS) $^

%.o: %.c
	gcc -o $@ -c $(CFLAGS) $< 

.PHONY: clean
clean:
	rm -f $(EXECUTABLE) $(OBJECTS)

