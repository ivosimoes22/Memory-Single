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

#include "thread_client.h"

// Variaveis Globais para comunicar com o servidor	
int sock_fd;
struct sockaddr_in server_addr;

//Declaração de funções
void initSocket(char*);
void* readServer(void*);
int getDimension();
void getBoard(int);
void sendPlay(int, int);
void* readPlays(void*);
void checkWinner();

#endif
