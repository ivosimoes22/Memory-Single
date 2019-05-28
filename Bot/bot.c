#include "communications.h"
#include "thread_bot.h"

//Função Main --> Contem o main loop do jogo
//e iniciação do UI

int jogada=1;

int main(int argc, char *argv[])
{
	int i,j,k,l;
	int done = 0;

	//Chack number of arguments
	if (argc <2)
	{
    printf("second argument should be server address\n");
    exit(-1);
	}

	//Ligar ao servidor
	initSocket(argv[1]);

	//Recebe a dimensão vinda do servidor
	int server_dim = getDimension();

	board = malloc(sizeof(board_place)* server_dim *server_dim);
	for(int i=0; i < (server_dim*server_dim); i++)
	{
		board[i].v[0] = '\0';
		board[i].locked=0;
	}

	//Cria a thread para ler as jogadas vindas do servidor
	pthread_create(&read_thread, NULL, readPlays, (void *)(&server_dim) );

	int board_x, board_y, board_x1, board_y1;

	while (!done)
	{
			if(jogada==1)
			{/*
				for(i=0; i < server_dim; i++)
				{
					for(j=0; j < server_dim; j++)
					{
						if (strcmp ("", board[j*server_dim+i].v)!=0 && board[j*server_dim+i].locked == 0)
						{
							board_x=i;
							board_y=j;
							for(k=0; k < server_dim; k++)
							{
								for(l=0; l < server_dim; l++)
								{
									if (strcmp (board[l*server_dim+k].v, board[board_y*server_dim+board_x].v)==0 && board[l*server_dim+k].locked == 0 && l*server_dim+k!=board_y*server_dim+board_x)
									{
										board_x1=k;
										board_y1=l;
										do
										{
											//Envia-se as coordenadas para o servidor
											sendPlay(board_x, board_y);
											sleep(1);
											sendPlay(board_x1, board_y1);
											sleep(1);
										}
										while(board[l*server_dim+k].locked == 0);
									}
								}
							}
						}
					}
				}*/
				jogada = 2;
				board_x=rand()%server_dim;
				board_y=rand()%server_dim;
				sendPlay(board_x, board_y);
				//sleep(1);
			}
			if (jogada ==2)
			{
				jogada =1;
				board_x=rand()%server_dim;
				board_y=rand()%server_dim;
				sendPlay(board_x, board_y);
				//sleep(1);
			}
		}
	printf("fim\n");
	free(board);
  close(sock_fd);
}