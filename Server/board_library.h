#ifndef BOARD_LIBRARY_H
#define BOARD_LIBRARY_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

typedef struct Color
{
    int r;
    int g;
    int b;
}Color;

typedef struct board_place
{
  char v[3];
  int revealed;           //Flag se peça está revelada ou não
  int locked;             //Flag se a peça está bloqueada (Acertou a segunda jogada)
  int wrong;              //Flag se a peça está com as letras vermelhas(errou a segunda jogagda)
  int first;              //Flag se a peça está revelada, mas na primeira jogada
  struct Color color;
}board_place;

typedef struct play_response
{
  int code;
  int play1[2];
  int play2[2];
  char str_play1[3], str_play2[3];
  struct Color color;
}play_response;

//Variaveis usadas no board_place.c
int dim_board;
board_place *board;
int n_corrects;
int alloc;              //Flaq para que cada vez que se reinicia um jogo nao voltar a alocar memoria para a board
pthread_mutex_t *mux;   //Vetor de mutex por cada linha

//Declaração de funções
char * get_board_place_str(int i, int j);
void init_board(int dim);
play_response board_play(int x, int y, int play1[2], int play2[2], int jogada, Color color, int *score);
int linear_conv(int i, int j);

#endif