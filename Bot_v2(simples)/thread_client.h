#ifndef THREAD_CLIENT_H
#define THREAD_CLIENT_H

#include <pthread.h>

//Struct da resposta

//É necessário declarar esta estrutura pois é usada na thread
//e só vamos usar o board_libray.c/board_library.h no server

typedef struct Color
{
    int r;
    int g;
    int b;
}Color;

typedef struct play_response
{
  int code; // 0 - filled
            // 1 - 1st play
            // 2 2nd - same plays
            // 3 END
            // -2 2nd - diffrent
  int play1[2];
  int play2[2];
  char str_play1[3], str_play2[3];
  struct Color color;
  int ok;
} play_response;

//Declaração da ID da thread
pthread_t read_thread;
int is_good;
int score_winner;


//Declaração da função thread que está a
//receber as respostas do server
void* readPlays(void *arg);


#endif