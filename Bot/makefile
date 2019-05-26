CC = gcc
CFLAGS = -g -Wall
LINKER_FLAGS_CLIENT =  -pthread 
OBJ_CLIENT = memory-single.out

$(OBJ_CLIENT): memory-single.o thread_client.o  communications.o
	$(CC) $(CFLAGS) -o $(OBJ_CLIENT) memory-single.o thread_client.o  communications.o $(LINKER_FLAGS_CLIENT)

memory-single.o: memory-single.c thread_client.h communications.h
	$(CC) -c $(CFLAGS) memory-single.c

thread_client.o: thread_client.c thread_client.h
	$(CC) -c $(CFLAGS) thread_client.c

communications.o: communications.c communications.h
	$(CC) -c $(CFLAGS) communications.c

clean::
	rm -f *.o core *.out *~
