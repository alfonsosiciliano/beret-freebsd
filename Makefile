CC= gcc
CFLAGS= -Wall -g
LFLAGS= -lSDLmain -lSDL -lSDL_image -lSDL_ttf -lSDL_mixer

default: beret

clean:
	rm beret *.o

beret: game.o thing.o physics.o
	$(CC) $(LFLAGS) -o $@ game.o thing.o physics.o

%.o: %.c %.h
