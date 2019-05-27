CC = gcc
CFLAGS = -g -Wall
LINKER_FLAGS_CLIENT = -lSDL2 -lSDL2_ttf -pthread 
OBJ_CLIENT = memory-single

$(OBJ_CLIENT): memory-single.o thread_client.o UI_library.o communications.o
	$(CC) $(CFLAGS) -o $(OBJ_CLIENT) memory-single.o thread_client.o UI_library.o communications.o $(LINKER_FLAGS_CLIENT)

memory-single.o: memory-single.c thread_client.h UI_library.h communications.h
	$(CC) -c $(CFLAGS) memory-single.c

thread_client.o: thread_client.c thread_client.h
	$(CC) -c $(CFLAGS) thread_client.c

UI_library.o: UI_library.c UI_library.h
	$(CC) -c $(CFLAGS) UI_library.c

communications.o: communications.c communications.h
	$(CC) -c $(CFLAGS) communications.c

clean::
	rm -f *.o core memory-single *~
