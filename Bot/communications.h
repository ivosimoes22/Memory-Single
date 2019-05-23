#ifndef COM_LIBRARY_H
#define COM_LIBRARY_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>

typedef struct board_place{
  char v[3];
  int locked;             //Flag se a peça está bloqueada (Acertou a segunda jogada)
} board_place;

// Variaveis Globais para comunicar com o servidor
board_place *board;		
int sock_fd;
struct sockaddr_in server_addr;

//Declaração de funções
void initSocket(char *);
void* readServer(void *);
int getDimension();
void getBoard(int dim);
void sendPlay(int x, int y);
void* readPlays(void *arg);
void checkWinner();

#endif
