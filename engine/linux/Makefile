RESULT=shmup
CFLAGS=-I../src -Ilibpng -DLINUX `sdl-config --cflags` `pkg-config zlib --cflags` `pkg-config openal --cflags`
CFLAGS += -g
LDFLAGS=-lGL -lm -z `sdl-config --libs` `pkg-config zlib --libs` `pkg-config openal --libs`

DEPS=../src/camera.o \
     ../src/collisions.o \
     ../src/commands.o \
     ../src/config.o \
     ../src/dEngine.o \
     ../src/enemy.o \
     ../src/enemy_particules.o \
     ../src/entities.o \
     ../src/event.o \
     ../src/fht.o \
     ../src/filesystem.o \
     ../src/fx.o \
     ../src/io_interface.o \
     ../src/ItextureLoader.o \
     ../src/lee.o \
     ../src/lexer.o \
     ../src/lofb.o \
     ../src/log.o \
     ../src/material.o \
     ../src/math.o \
     ../src/matrix.o \
     ../src/md5.o \
     ../src/menu.o \
     ../src/music.o \
     ../src/netchannel.o \
     ../src/player.o \
     ../src/preproc.o \
     ../src/quaternion.o \
     ../src/renderer.o \
     ../src/renderer_fixed.o \
     ../src/renderer_progr.o \
     ../src/shab.o \
     ../src/sounds.o \
     ../src/sound_openAL.o \
     ../src/stats.o \
     ../src/text.o \
     ../src/texture.o \
     ../src/tha.o \
     ../src/timer.o \
     ../src/titles.o \
     ../src/trackmem.o \
     ../src/unzip.o \
     ../src/vis.o \
     ../src/wavfile.o \
     ../src/world.o \
     libpng/png.o \
     libpng/pngerror.o \
     libpng/pngget.o \
     libpng/pngmem.o \
     libpng/pngpread.o \
     libpng/pngread.o \
     libpng/pngrio.o \
     libpng/pngrtran.o \
     libpng/pngrutil.o \
     libpng/pngset.o \
     libpng/pngtrans.o \
     libpng/pngwio.o \
     libpng/pngwrite.o \
     libpng/pngwtran.o \
     libpng/pngwutil.o \
	 native.o \
     main.o

all: $(RESULT)

shmup: $(DEPS)
	gcc -o $@ $(LDFLAGS) $(DEPS)

%.o: %.c
	gcc -o $@ -c $(CFLAGS) $< 

.PHONY: clean
clean:
	rm -f $(RESULT) $(DEPS)

