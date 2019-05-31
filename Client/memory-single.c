#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "UI_library.h"
#include "communications.h"
#include "thread_client.h"

//Função Main --> Contem o main loop do jogo
//e iniciação do UI

int main(int argc, char *argv[])
{
	int server_dim;
	SDL_Event event;
	done = 0;

	//Chack number of arguments
	if (argc <2)
	{
    printf("second argument should be server address\n");
    exit(-1);
	}

	if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) 
	{
		printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		exit(-1);
	}

	if(TTF_Init()==-1) 
	{
		printf("TTF_Init: %s\n", TTF_GetError());
		exit(2);
	}

	//Ligar ao servidor
	initSocket(argv[1]);

	//Recebe a dimensão vinda do servidor
	server_dim = getDimension();

	if (server_dim == 0)
	{
		close(sock_fd);
		exit(0);
	}

	//Cria a thread para ler as jogadas vindas do servidor
	create_board_window(75*server_dim, 75*server_dim, server_dim);

	//Create the thread
	pthread_create(&read_thread, NULL, readPlays, (void *)(&server_dim));
	
	while (!done)
	{
		while (SDL_PollEvent(&event)) 
		{
			switch (event.type) 
			{
				case SDL_QUIT: 
				{
					//Sair do jogo
					done = SDL_TRUE;
					break;
				}
				case SDL_MOUSEBUTTONDOWN:
				{
					int board_x, board_y;
					get_board_card(event.button.x, event.button.y, &board_x, &board_y);

					printf("click (%d %d) -> (%d %d)\n", event.button.x, event.button.y, board_x, board_y);
				
					//Envia-se as coordenadas para o servidor
					sendPlay(board_x, board_y);
				}
			}
		}
	}
	close_board_windows();
  close(sock_fd);
	exit(1);
}