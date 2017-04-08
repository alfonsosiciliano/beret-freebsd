# Public Domain - NO WARRANTY

#CC = clang
CCFLAGS = `sdl-config --cflags` -Wall -g
INCLUDEDIR = ./
LDFLAGS = `sdl-config --libs` -lSDLmain -lSDL_image -lSDL_ttf -lSDL_mixer -lm
OUTPUT = beret
SOURCES = game.c physics.c thing.c
OBJECTS=${SOURCES:.c=.o}

all : ${OUTPUT}

clean:
	rm -f ${OUTPUT} *.o *~

${OUTPUT}: ${OBJECTS}
	${CC} ${LDFLAGS} ${OBJECTS} -o ${OUTPUT}

.c.o:
	${CC} -I${INCLUDEDIR} ${CCFLAGS} -c ${.IMPSRC} -o ${.TARGET}

