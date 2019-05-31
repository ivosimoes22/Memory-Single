#ifndef THREAD_CLIENT_H
#define THREAD_CLIENT_H

#include <pthread.h>

//Struct da resposta

typedef struct Color
{
    int r;
    int g;
    int b;
}Color;

typedef struct play_response
{
  int code; 
  int play1[2];
  int play2[2];
  char str_play1[3], str_play2[3];
  struct Color color;
} play_response;

//Declaração da ID da thread
pthread_t read_thread;

//Variveis globais
int done;

//Declaração da função thread que está a
//receber as respostas do server
void* readPlays(void *arg);

#endif