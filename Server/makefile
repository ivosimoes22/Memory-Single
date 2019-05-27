CC = gcc
CFLAGS = -g -Wall
LINKER_FLAGS_SERVER = -pthread -lm 
OBJ_SERVER = server

$(OBJ_SERVER): server.o board_library.o
	$(CC) $(CFLAGS) -o $(OBJ_SERVER) server.o board_library.o $(LINKER_FLAGS_SERVER)

server.o: server.c board_library.h
	$(CC) -c $(CFLAGS) server.c

board_library.o: board_library.c board_library.h
	$(CC) -c $(CFLAGS) board_library.c

clean::
	rm -f *.o core server*~
