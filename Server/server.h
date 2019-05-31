#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <signal.h>
#include <semaphore.h>

#include "board_library.h"

//------------------------Declaração de struct para cada cliente--------------------------

typedef struct Client_node
{
    pthread_t client_thr;       //thread do cliente correspondente
    pthread_t timer_thread;     //thread da função contar 5 segundos
    pthread_t sec2_thread;      //thread da função contar 2 segundos
    sem_t sem_5;                //Semáforo para controlar os 5s
    sem_t sem_2;                //Semaforo para controlar os 2s
    struct Color color;         //Cor do cliente
    int player_fd;              //inteiro correspondente à socket do cliente
    int id;                     //indice do cliente 
    int jogada;                 //Variavel para saber se é a primeira ou a segunda jogada
    int *score;                 //Mantem o score de cada cliente
    int coord[2];               //Coordenadas que recebe do cliente
    int play1[2];               //Variavel que guarda a primeira jogada
    int wrongplay[4];           //Guarda as coordenadas das duas jogadas se jogou mal
    int sec2_state;             //Flag que bloqueia o cliene quando erra a 2ª jogada
    int exit_all;               //Flag que quando a thread principal sai, mata todas as outras threads associadas  
    struct Client_node *next;   //pointeiro para o proximo jogador
    struct Client_node *prev;   //ponteiro para o jogador anterior
}Client_node;

//----------------------------------------------------------------------------------------


//------------------------------Declaração de variaveis globais---------------------------
sem_t sem_10;
pthread_t sec10_thread;
int dimension;                      //Dimensão da board
int client_index = 1;               //Guarda o nr total de indice de cliente 
int n_clientes = 0;                 //Numero de clientes ligados
int score_winner = 0;               //Guarda o max score entre todos clientes conectados
Client_node *client_list = NULL;    //Lista de clientes ligados ao servidor
int game_locked = 0;                //Flag que indica se o jogo está bloqueado quando acaba (a espera que acabe os 10s)
pthread_rwlock_t rwlock;            //Varivel para o read/write lock
//-----------------------------------------------------------------------------------------


//-------------------------------Declaração de funções------------------------------------- 

void *thread_client(void *arg);
Client_node* insertClient(Client_node* head, int player_fd);
void deleteClient(Client_node* current);
void* wait5s(void * arg);
void Reset_game();
void getBoardState(Client_node* current);
void ctrl_c_callback_handler(int signum);
Color generateColor();
void checkMaxScore();
void informWinner();
int exit_game(Client_node* current);
void* thread_func_2(void *arg);
void* thread_func_10(void *arg);
void sendAllPlayers(Client_node* current, play_response resposta);
void initializeMutex(int dim);

//-----------------------------------------------------------------------------------------


#endif