CC = gcc
CFLAGS = -g #-std=c99 #-Wall
LIBS =  -lm
INCLS = -I./
OBJ=blow_chunks.o blow_chunks_engine.o libchunky.o string_utils.o

blow_chunks: ${OBJ} 
		${CC} ${CFLAGS} ${OBJ} -o $@  ${INCLS} ${LIBS}

.c.o: $<
		$(CC) ${INCLS} $(CFLAGS) -c $<

clean:
		\rm -f *.o *~ *%
