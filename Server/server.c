#include "server.h"

int main(int argc, char *argv[])
{
    int sock_temp;
    int sock_fd;
    struct sockaddr_in local_addr;

    //Armar o sinal CTRL C
    struct sigaction act;
	act.sa_flags = SA_SIGINFO;
	act.sa_handler = ctrl_c_callback_handler;
	sigaction(SIGINT, &act, NULL);
	
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

    //Incializar os read/write locks
    pthread_rwlock_init(&rwlock, NULL);

    //Iniciar o board
    init_board(dimension);
    n_corrects = 0;
    alloc = 0;

    printf("Waiting for players\n");

    //Cria uma thread para contar 10s no fim do jogo
    pthread_create(&sec10_thread, NULL, thread_func_10, NULL);
    
    //Loop para entrar jogadores no servidor
    while(1)
    { 
        //Liga-se um novo cliente
        sock_temp = accept(sock_fd, NULL, NULL);
        
        //Insere-se o novo cliente no topo da lista
        client_list = insertClient(client_list, client_index);

        //Associa-se a socket do cliente que entrou a uma variavel na estrutura do cliente na lista
        client_list->player_fd = sock_temp;

        //Incrementa-se o numero de clientes no servidor e o index
        client_index++; 
        n_clientes++;

        //Criam-se as threads para receber respostas do cliente, para contar os 5s e contar os 2s
        pthread_create(&client_list->client_thr, NULL, thread_client, (void*)client_list);
        pthread_create(&client_list->timer_thread, NULL, wait5s, (void*)client_list);
        pthread_create(&client_list->sec2_thread, NULL, thread_func_2, (void*)client_list);

        printf("Client %d connected\n", client_list->id);
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
    if (write(current->player_fd, &dimension, sizeof(dimension)) < 1)
    {
        exit_game(current);
        pthread_exit(NULL);
    }  
    //Funçao para quando um jogador entrar a meio do jogo receber a board
    getBoardState(current);
    
    while(1)
    { 
        //Recebe a joagada deste cliente
        read_val = read(current->player_fd, &current->coord, sizeof(current->coord));
        if (read_val < 1)
        {
            printf("Exiting client %d thread\n", current->id);
            break;
        }
        //Ignora todos os pedidos do cliente caso só esteja um jogador ligado
        //ou tenha errado a jogada (durante 2s)
        if (n_clientes < 2 || current->sec2_state == 1 || game_locked == 1) continue;

        //Processa a jogada
        resposta = board_play(current->coord[0], current->coord[1], current->play1, current->wrongplay, current->jogada, current->color, (current->score));

        if (resposta.code == -20) 
            continue;

        if (current->jogada == 1)
        {            
            current->jogada = 2;

            //mandar para thread para começar contar 5 segundos
            sem_post(&(current->sem_5));
        }
        else
        {
            current->jogada = 1;
            
            //mandar sinal para interromper o temporizador
            sem_post(&(current->sem_5));

            //Verfica se o jogador errou
            if (resposta.code == -2)
            {
                //Manda a thread dos 2s começar a contar
                sem_post(&(current->sem_2));

                //Bloqueia todas as respostas durante os 2s
                current->sec2_state = 1;
            }

            //Atualiza o score máximo
            else if (resposta.code == 2 || resposta.code == 3)
            {
                checkMaxScore(current);
            }  
        }

        // Verifica se o jogador ganhou
        if ((n_corrects == dimension*dimension))
        {
            //Bloqueia todas as respostas durante os 10s
            game_locked = 1;
            sem_post(&sem_10);
            
            //Informa os clientes quem ganhou
            informWinner();
        }

        //Envia a jogada a todos os clientes, incluindo ele próprio
        sendAllPlayers(current, resposta);
    }
    exit_game(current);
    pthread_exit(NULL);  
}

//Funçao para enviar a resposta a todos os clientes na lista

void sendAllPlayers(Client_node* current, play_response resposta)
{
    pthread_rwlock_rdlock(&rwlock);
    Client_node* aux = client_list;
      
    while(aux != NULL)
    {
        resposta.color = current->color;
        if (write(aux->player_fd, &resposta, sizeof(play_response)) < 1)
        {
            close(aux->player_fd);
        }
        aux = aux->next;

    }
    pthread_rwlock_unlock(&rwlock);
}

//Função para criar um novo no (cliente) e 
//inicializar as variaveis correspondentes

Client_node* insertClient(Client_node* head, int id)
{
    Client_node* new_client = (Client_node*)malloc(sizeof(Client_node));

    new_client->coord[0]=0;
    new_client->id = id;
    new_client->jogada = 1;
    new_client->color = generateColor();
    new_client->score = malloc(sizeof(int *));
    *(new_client->score)=0;
    new_client->sec2_state = 0;
    new_client->play1[0] = -1;
    new_client->exit_all = 0;

    pthread_rwlock_wrlock(&rwlock);

    //Novo nó aponta para o segundo da lista
    new_client->next = head;
    new_client->prev = NULL;

    if (head != NULL)
    {
        //No anterior aponta para o novo no
        head->prev = new_client;
    }
    pthread_rwlock_unlock(&rwlock);
    return new_client;
}

//Função para apagar um cliente da lista

void deleteClient(Client_node* current)
{
    Client_node* aux = NULL;

    if (current == NULL)
    {
        exit(-1);
    }

    pthread_rwlock_wrlock(&rwlock);

    //É a head
    if (current->prev == NULL)
    {
        if (current->next == NULL)
        {
            free(current->score);
            free(current);
            client_list=NULL;
            pthread_rwlock_unlock(&rwlock);
            return;
        }
        else
        {   
            //Guardar a nova cabeça
            aux = current->next;
            aux->prev = NULL;
            client_list = aux;          
            free(current->score);
            //apaga o nó
            free(current);
            pthread_rwlock_unlock(&rwlock);
            return;
        }
    }
    //Está no meio
    else if (current->next != NULL)
    {
        aux = current;
        current->prev->next = current->next;
        current->next->prev = current->prev;
        free(aux);
        pthread_rwlock_unlock(&rwlock);
        return;
    }
    //Esta no fim
    else
    {
        aux = current;
        current->prev->next = NULL;
        free(aux->score);
        free(aux);
    }
    pthread_rwlock_unlock(&rwlock);
}

//Função para a thread que conta os 10 segundos
//e dá reset ao jogo

void* thread_func_10(void *arg)
{
    sem_init(&(sem_10),0, 0);

    while(1)
    {
        sem_wait(&sem_10);
        sleep(10);
        Reset_game();
        game_locked = 0;
    }
}

//Função para a thread que conta os 2 segundos
//e volta a pintar as peças de branco

void* thread_func_2(void *arg)
{
    Client_node* current = (Client_node*)arg;
    play_response resposta;
    sem_init(&(current->sem_2),0, 0);

    while(1)
    {
        //Se o jogador erra espera 2s
        //e volta a pintar as peças de branco
        sem_wait(&(current->sem_2));

        if (current->exit_all == 1)
            break;

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
    pthread_exit(NULL);
}

//Funçao para a thread apagar a jogada passado 5 segundos

void* wait5s(void* arg)
{  
    Client_node* current = (Client_node*)arg;
    play_response resp;
    struct timespec ts;

    //Inicializar o semáforo
    sem_init(&(current->sem_5), 0, 0);

    //semáforo parado
    while(1)
	{	
        sem_wait(&current->sem_5);
        
        if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
        {
            exit(-1);
        }
        //Conta 5 segundos	
        ts.tv_sec += 5;	
        sem_timedwait(&current->sem_5, &ts);

        if (current->exit_all == 1)
            break;

        if (current->jogada == 2)
        {
            if(board[linear_conv(current->coord[0], current->coord[1])].locked != 1 && board[linear_conv(current->coord[0], current->coord[1])].first == 1)
            {
                //Atualização da resposta para apagar
                resp.code = -4;
                resp.play1[0] = current->coord[0];
                resp.play1[1] = current->coord[1];

                //Comunica a todos os jogadores que passaram 5 segundos
                //É necessário pintar a tile de branco novamente
                sendAllPlayers(current, resp); 

                //Atualiza o board e a jogada
                board[linear_conv(resp.play1[0], resp.play1[1])].first = 0;
                board[linear_conv(resp.play1[0], resp.play1[1])].revealed = 0;
                current->jogada = 1;
            }
        }
    }
    pthread_exit(NULL);
}

//Função para atualizaar o max score

void checkMaxScore()
{
    pthread_rwlock_rdlock(&rwlock);
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
    pthread_rwlock_unlock(&rwlock);
}

//Função para enviar o score a cada cliente
//e quem ganhou o jogo

void informWinner()
{
    Client_node* aux = NULL;
    int winner;

    pthread_rwlock_rdlock(&rwlock);
    aux = client_list;        
    while(aux != NULL)
    {
        //Envia o score a cada um dos jogadores
        if (write(aux->player_fd, &(*aux->score), sizeof(int)) < 1)
        {
            close(aux->player_fd);
        }

        if((*aux->score) == score_winner)
            winner = 1;
        else winner = 0;

        if(write(aux->player_fd, &winner, sizeof(int)) < 1)
        {
            close(aux->player_fd);
        }
        aux = aux->next;
    }
    pthread_rwlock_unlock(&rwlock);
}

//Função para dar reset ao jogo passados 10s

void Reset_game()
{   
    Client_node* current = client_list;
    play_response resp;

    init_board(dimension);
    n_corrects = 0;
    score_winner = 0;

    resp.code = 5;

    while (current != NULL)
    {
        write(current->player_fd, &resp, sizeof(play_response));
        *(current->score) = 0;
        current = current->next;
    }
}

//Função para quando um novo jogador entra
//a meio do jogo, receber o estado da board

void getBoardState(Client_node* current)
{
    int i,j;
    play_response resp;

    for (i = 0; i < dimension; i++)
    {
        for(j = 0; j < dimension; j++)
        {
            if (board[linear_conv(i,j)].revealed == 1)
            {
                if (board[linear_conv(i,j)].locked == 1)
                {
                    resp.code = 4;
                    resp.play1[0] = i;
                    resp.play1[1] = j;
                }
                else if (board[linear_conv(i,j)].wrong == 1)
                {
                    resp.code = -5;
                    resp.play1[0] = i;
                    resp.play1[1] = j;
                }
                else if(board[linear_conv(i,j)].first == 1)
                {
                    resp.code = 1;
                    resp.play1[0] = i;
                    resp.play1[1] = j;
                }
                resp.color = board[linear_conv(i, j)].color;
                strcpy(resp.str_play1, get_board_place_str(i, j));
                if (write(current->player_fd, &resp, sizeof(play_response)) < 1)
                {
                    close(current->player_fd);
                }
            }   
        }
    }
}

//Função para gerar uma cor random

Color generateColor()
{
    Color color;

    int r = random() % 256;
    int g = random() % 256;
    int b = random() % 256;

    //Devide-se todos os valores r,g,b por branco
    //para as cores ficarem mais claras

    color.r = floor((r + 255)/2);
    color.g = floor((g + 255)/2);
    color.b = floor((b + 255)/2);

    return color;
}

//Função para um jogador sair do server de forma "limpa"

int exit_game(Client_node* current)
{
    play_response resposta;

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

    //Decrementa o numero de clientes no server
    n_clientes--;

    //Manda mensagem a todas as threads relacionadas com
    //este cliente para as apagar

    current->exit_all = 1;
    sem_post(&current->sem_5);
    sem_post(&current->sem_5);
    sem_post(&(current->sem_2));
    pthread_mutex_consistent(&mux[current->coord[0]]);
    //pthread_mutex_unlock(&mux[current->coord[0]]);
    close(current->player_fd);
    deleteClient(current);
    return 0;
}

//Função para inicializar os mutex

void initializeMutex(int dim)
{
    int i;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST); 

    //Alocar a memoria para o vetor de mutex (Mutex por cada linha/coluna)
    mux = (pthread_mutex_t*)malloc(dim*sizeof(pthread_mutex_t));

    for(i = 0; i < dim; i++)
    {
        pthread_mutex_init(&mux[i], &attr);
    }
    pthread_mutexattr_destroy(&attr);
}

//Função que é chamada quando acciona o CTRL-C
//Limpa a memoria toda a alocada e o servidor é encerrado

void ctrl_c_callback_handler(int signum)
{
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
    free(mux);
	exit(0);
}






