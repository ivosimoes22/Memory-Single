CC = gcc
CFLAGS = -g -Wall
LINKER_FLAGS_CLIENT = -pthread 
OBJ_CLIENT = bot

$(OBJ_CLIENT): bot.o communications.o
	$(CC) $(CFLAGS) -o $(OBJ_CLIENT) bot.o communications.o $(LINKER_FLAGS_CLIENT)

bot.o: bot.c communications.h
	$(CC) -c $(CFLAGS) bot.c

communications.o: communications.c communications.h
	$(CC) -c $(CFLAGS) communications.c

clean::
	rm -f *.o core bot *~
