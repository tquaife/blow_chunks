CC = gcc
CFLAGS = -g -Wall #-std=c99 #-Wall
LIBS =  -lm
INCLS = -I./
OBJ=blow_chunks.o blow_chunks_engine.o libchunky.o string_utils.o math_parser.o

blow_chunks: ${OBJ} 
		${CC} ${CFLAGS} ${OBJ} -o $@  ${INCLS} ${LIBS}

.c.o: $<
		$(CC) ${INCLS} $(CFLAGS) -c $<

clean:
		\rm -f *.o *~ *%
