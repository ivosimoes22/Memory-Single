#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
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
    int player_fd;              //socket
    int id;                     //indice do cliente 
    int jogada;                 //Variavel para saber se é a primeira ou a segunda jogada
    int *score;                 //Mantem o score de cada cliente
    struct Color color;         //Cor do cliente
    sem_t *sem;                 //Semáforo para controlar os 5s
    int coord[2];               //Coordenadas que recebe do cliente
    int play1[2];               //Variavel que guarda a primeira jogada
    int wrongplay[4];           //Guarda as coordenadas das duas jogadas se jogou mal
    int sec2_state;            
    int exit_all;
    struct Client_node *next;   //pointeiro para o proximo jogador
    struct Client_node *prev;   //ponteiro para o jogador anterior
}Client_node;

//----------------------------------------------------------------------------------------


//------------------------------Declaração de variaveis globais---------------------------

int dimension;                      //Dimensão da board
int client_index = 1;               //Guarda o nr total de indice de cliente 
int n_clientes = 0;                 //Numero de clientes ligados
int score_winner = 0;               //Guarda o max score entre todos clientes conectados
Client_node *client_list = NULL;    //Lista de clientes ligados ao servidor
int game_locked = 0;                //Flag que indica se o jogo está bloqueado quando acaba (a espera que acabe os 10s)

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
void endGame();
int exit_game(Client_node* current);
void* thread_func(void *arg);
void sendAllPlayers(Client_node* current, play_response resposta);
void initializeMutex(int dim);

//-----------------------------------------------------------------------------------------


int main(int argc, char *argv[])
{
    int sock_temp;
    int sock_fd;
    Client_node *aux = NULL;
    struct sockaddr_in local_addr;

    //Armar o sinal CTRL C
    signal(SIGINT, ctrl_c_callback_handler);
	
	if(argc != 2)
    {
		exit(-1);
	}
	sscanf(argv[1], "%d", &dimension);
		
    // Verificar se a dimensão da board é um número par
    if (dimension % 2 != 0)
    {
        printf("A dimensão da board tem que ser um número par!\n");
        exit(-1);
    }			    

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd ==-1)
    {
        perror("socket: ");
        exit(-1);
    }

    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(3000);
    local_addr.sin_addr.s_addr= INADDR_ANY;

    int err = bind(sock_fd, (struct sockaddr*)&local_addr, sizeof(local_addr));
    if (err ==-1)
    {
        perror("bind: ");
        exit(-1);
    }

    printf("Socket 1 created and binded\n");
    listen(sock_fd, 5);

    //Inciar os mutex
    initializeMutex(dimension);

    //Iniciar o board
    init_board(dimension);
    n_corrects = 0;

    printf("Waiting for players\n");
    
    //Loop para entrar jogadores no servidor
    while(1)
    { 
        sock_temp = accept(sock_fd, NULL, NULL);
        client_list = insertClient(client_list, client_index);
        client_list->player_fd = sock_temp;

        printf("Client %d connected\n", client_list->id);
        client_index++, n_clientes++;
        pthread_create(&client_list->client_thr, NULL, thread_client, (void*)client_list);
        pthread_create(&client_list->timer_thread, NULL, wait5s, (void*)client_list);
        pthread_create(&client_list->sec2_thread, NULL, thread_func, (void*)client_list);
    }

    //Loop para esperar que todos os jogadores saiam do jogo
    aux = client_list;
    while(aux != NULL)
    {
        pthread_join(aux->client_thr, NULL);
        aux = aux->next;
    }

    close(sock_fd);
    exit(0);
}

void *thread_client(void *arg)
{
    int read_val;
    //Variavel para a thread ter conhecimento 
    //do seu cliente (que vem na lista de clientes)
    Client_node* current = ((Client_node*)arg);

    play_response resposta;

    //Enviar a dimensão da board para o cliente
    write(current->player_fd, &dimension, sizeof(dimension));

    //Funçao para quando um jogador entrar a meio do jogo receber a board
    getBoardState(current);
    
    while(1)
    { 
        //Verifica se o cliente saiu do jogo
        if (exit_game(current))
            break;

        //Recebe a joagada deste cliente
        read_val = read(current->player_fd, &current->coord, sizeof(current->coord));
        if (read_val != sizeof(current->coord))
        {
            printf("Error in read: %d\n", read_val);
            //exit(-1);
        }

        //printf("Check1\n");

        //Ignora todos os pedidos do cliente caso só esteja um jogador ligado
        //ou tenha errado a jogada (durante 2s)
        if (n_clientes < 2 || current->sec2_state == 1 || game_locked == 1) 
            continue;

        //Processa a jogada
        resposta = board_play(current->coord[0], current->coord[1], current->play1, current->wrongplay, current->jogada, current->color, (current->score));

        //printf("Coord %d, %d -- Rev %d ", current->coord[0], current->coord[1], board[linear_conv(current->coord[0],current->coord[1])].revealed);

        if (resposta.code == -20) 
            continue;

        //Envia a jogada a todos os clientes, incluindo ele próprio
        sendAllPlayers(current, resposta);

        if (current->jogada == 1)
        {    
            current->jogada = 2;

            //mandar para thread para começar contar 5 segundos
            sem_post(current->sem);
        }
        else
        {
            current->jogada = 1;

            //mandar sinal para interromper o temporizador
            sem_post(current->sem);

            //Verfica se o jogador errou
            if (resposta.code == -2)
                //Altera a variavel para outra thread processar
                current->sec2_state = 1;
        }

        // Verifica se o jogador ganhou
        if ((n_corrects == dimension*dimension))
        {
            game_locked = 1;
            endGame();
        }
    }


    n_clientes--;
    printf("Exiting client %d thread\n", current->id);
    current->exit_all = 1;
    sem_post(current->sem);
    sem_post(current->sem);
    close(current->player_fd);
    //deleteClient(current);
    pthread_exit(NULL);  
}

void sendAllPlayers(Client_node* current, play_response resposta)
{
    Client_node* aux = client_list;
      
    while(aux != NULL)
    {
        resposta.color = current->color;
        write(aux->player_fd, &resposta, sizeof(play_response));
        aux = aux->next;
    }
}

Client_node* insertClient(Client_node* head, int id)
{
    Client_node* new_client = (Client_node*)malloc(sizeof(Client_node));

    new_client->id = id;
    new_client->jogada = 1;
    new_client->sem = malloc(sizeof(sem_t*));
    new_client->color = generateColor();
    new_client->score = malloc(sizeof(int *));
    *(new_client->score)=0;
    new_client->sec2_state = 0;
    new_client->play1[0] = -1;
    new_client->coord[0] = -2;
    new_client->exit_all = 0;

    //Novo nó aponta para o segundo da lista
    new_client->next = head;
    new_client->prev = NULL;

    if (head != NULL)
    {
        //No anterior aponta para o novo no
        head->prev = new_client;
    }
    return new_client;
}

void deleteClient(Client_node* current)
{
    Client_node* aux = NULL;

    if (current == NULL)
        exit(-1);

    //É a head
    if (current->prev == NULL)
    {
        if (current->next == NULL)
        {
            free(current->sem);
            free(current);
            client_list=NULL;
            return;
        }
        else
        {   
            //Guardar a nova cabeça
            aux = current->next;
            aux->prev = NULL;
            client_list = aux;          

            //apaga o nó
            free(current->sem);
            free(current);
            return;
        }
    }
    //Está no meio
    else if (current->next != NULL)
    {
        aux = current;
        current->prev->next = current->next;
        current->next->prev = current->prev;
        free(aux->sem);
        free(aux);
        return;
    }
    //Esta no fim
    else
    {
        aux = current;
        current->prev->next = NULL;
        free(aux->sem);
        free(aux);
    }
}

void* thread_func(void *arg)
{
    Client_node* current = (Client_node*)arg;
    play_response resposta;

    while(1)
    {
        //Se o jogador erra espera 2s
        //e volta a pintar as peças de branco

        if (current->sec2_state == 1)
        {
            //Quando o jogador erra espera 2 segundos
            sleep(2);

            //Codigo para voltar a pintar as peças de branco
            resposta.code = -3;

            resposta.play1[0] = current->wrongplay[0];
            resposta.play1[1] = current->wrongplay[1];
            resposta.play2[0] = current->wrongplay[2];
            resposta.play2[1] = current->wrongplay[3];

            //Envia a todos os clientes
            sendAllPlayers(current, resposta);

            //Da update ao board
            board[linear_conv(resposta.play1[0], resposta.play1[1])].wrong = 0;
            board[linear_conv(resposta.play2[0], resposta.play2[1])].wrong = 0;
            board[linear_conv(resposta.play1[0], resposta.play1[1])].revealed = 0;
            board[linear_conv(resposta.play2[0], resposta.play2[1])].revealed = 0;

            current->sec2_state = 0;
        }

        //Quando o jogo acaba é necessário
        //bloquear todos os jogadores e reiniciar
        //o jogo ao fim de 10s

        else if (game_locked == 1)
        {
            sleep(10);
            Reset_game();
            game_locked = 0;
        }
        else if (current->exit_all == 1)
            break;
    }
    pthread_exit(NULL);
}

void* wait5s(void* arg)
{  
    int sem_value;
    Client_node* current = (Client_node*)arg;
    play_response resp;
    struct timespec ts;

    //Inicializar o semáforo
    sem_init(current->sem, 0, 0);

    //semáforo parado
    while(1)
	{	
        sem_wait(current->sem);
            
        sem_getvalue(current->sem, &sem_value);
        if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
        {
            exit(-1);
        }
        //Conta 5 segundos	
        ts.tv_sec += 5;	
        sem_timedwait(current->sem, &ts);
        sem_getvalue(current->sem, &sem_value);

        if (current->exit_all == 1) break;

        if (current->jogada == 2)
        {
            if(strcmp(get_board_place_str(current->coord[0], current->coord[1]), "") != 0)
            {
                //Atualização da resposta para apagar
                resp.code = -4;
                resp.play1[0] = current->coord[0];
                resp.play1[1] = current->coord[1];

                //Comunica a todos os jogadores que passaram 5 segundos
                //É necessário pintar a tile de branco novamente
                sendAllPlayers(current, resp);

                //Atualiza o board e a jogada
                current->jogada = 1;
                current->play1[0] = -1;
                board[linear_conv(resp.play1[0], resp.play1[1])].first = 0;
                board[linear_conv(resp.play1[0], resp.play1[1])].revealed = 0;
            }
        }
        sem_getvalue(current->sem, &sem_value);
    }
    pthread_exit(NULL);
}

void checkMaxScore()
{
    Client_node* aux = client_list;        
    while(aux != NULL)
    {
        //Atualiza o max score entre todos os jogadores
        if (*(aux->score) > score_winner)
        {
            score_winner=*(aux->score);
        }    
        aux = aux->next;
    }
}

void informWinner()
{
    Client_node* aux = client_list;
    int winner;
        
    while(aux != NULL)
    {
        //Envia o score a cada um dos jogadores
        write(aux->player_fd, &(*aux->score), sizeof(int));

        if((*aux->score) == score_winner)
            winner = 1;
        else winner = 0;

        write(aux->player_fd, &winner, sizeof(int));
        aux = aux->next;
    }
}

void endGame()
{
    checkMaxScore();
    informWinner();
}

void Reset_game()
{   
    Client_node* current = client_list;
    play_response resp;

    //Da reset a board
    init_board(dimension);

    n_corrects = 0;
    score_winner = 0;

    //Diz a todos os clientes para limparem a board
    resp.code = 5;
    while (current != NULL)
    {
        write(current->player_fd, &resp, sizeof(play_response));
        *(current->score) = 0;
        current = current->next;
    }
}

void getBoardState(Client_node* current)
{
    int i,j;
    play_response resp;

    for (i = 0; i < dimension; i++)
    {
        for(j = 0; j < dimension; j++)
        {
            //pthread_mutex_lock(&mux[i]);
            if (board[linear_conv(i,j)].revealed == 1)
            {
                if (board[linear_conv(i,j)].locked == 1)
                {
                    //pthread_mutex_unlock(&mux[i]);
                    resp.code = 4;
                    resp.play1[0] = i;
                    resp.play1[1] = j;
                }
                else if (board[linear_conv(i,j)].wrong == 1)
                {
                    //pthread_mutex_unlock(&mux[i]);
                    resp.code = -5;
                    resp.play1[0] = i;
                    resp.play1[1] = j;
                }
                else if(board[linear_conv(i,j)].first == 1)
                {
                    //pthread_mutex_unlock(&mux[i]);
                    resp.code = 1;
                    resp.play1[0] = i;
                    resp.play1[1] = j;
                }
                resp.color = board[linear_conv(i, j)].color;
                strcpy(resp.str_play1, get_board_place_str(i, j));
                write(current->player_fd, &resp, sizeof(play_response));
            }
            //pthread_mutex_unlock(&mux[i]);   
        }
    }
}

Color generateColor()
{
    Color color;

    int r = random() % 256;
    int g = random() % 256;
    int b = random() % 256;

    r = floor((r + 255)/2);
    g = floor((g + 255)/2);
    b = floor((b + 255)/2);

    color.r = r, color.g = g, color.b = b;
    return color;
}

int exit_game(Client_node* current)
{
    play_response resposta;

    if (current->coord[0] == -1 && current->coord[1] == -1)
    {
        //se sair depois de executar a primeira jogada
        if (current->jogada == 2)
        {
            //Atualização da resposta para apagar
            resposta.code = -4;
            resposta.play1[0] = current->play1[0];
            resposta.play1[1] = current->play1[1];
            board[linear_conv(resposta.play1[0], resposta.play1[1])].revealed = 0;
            board[linear_conv(resposta.play1[0], resposta.play1[1])].wrong = 0;
            sendAllPlayers(current, resposta);
        }
        return 1;
    }
    return 0;
}

void initializeMutex(int dim)
{
    int i;

    //Alocar a memoria para o vetor de mutex (Mutex por cada linha/coluna)
    mux = (pthread_mutex_t*)malloc(dim*sizeof(pthread_mutex_t));

    for(i = 0; i < dim; i++)
    {
        pthread_mutex_init(&mux[i], NULL);
    }
}

void ctrl_c_callback_handler(int signum)
{
    printf("\nCaught signal Ctr-C\n");

    Client_node* current = client_list;
    Client_node* aux;

    //Quando sai do servidor apaga todos
    //os clientes que ainda estiverem ligados
    while ((current!=NULL))
    {
        aux = current;
        current = current->next;
        deleteClient(aux);
    }
    free(board);
	exit(0);
}






