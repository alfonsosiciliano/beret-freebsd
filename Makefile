CFLAGS := $(shell sdl-config --cflags) -Wall -g
LDLIBS := $(shell sdl-config --libs) -lSDLmain -lSDL_image -lSDL_ttf -lSDL_mixer -lm

default: beret

clean:
	rm -f beret *.o

beret: game.o thing.o physics.o
	$(CC) $^ $(LDLIBS) -o $@

%.o: %.c %.h
