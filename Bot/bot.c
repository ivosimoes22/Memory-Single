#include "communications.h"

//Função Main --> Contem o main loop do jogo
//e iniciação do UI

int main(int argc, char *argv[])
{
	done = 0;
	
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

	int board_x, board_y;

	//Loop do jogo
	while (!done)
	{
			board_x = random()%server_dim;
			board_y = random()%server_dim;
			sendPlay(board_x, board_y);
			usleep(100);
	}
  close(sock_fd);
	exit(0);
}