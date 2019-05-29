#include "communications.h"

//Função Main --> Contem o main loop do jogo
//e iniciação do UI

int jogada=1;

int main(int argc, char *argv[])
{
	//int i,j,k,l;
	int done = 0;
	int coord[2];
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
	printf("Dim %d\n", server_dim);

	int board_x, board_y;

	while (!done)
	{
			board_x = rand()%server_dim;
			board_y = rand()%server_dim;
			coord[0] = board_x;
			coord[1] = board_y;
    	//write(sock_fd, &coord, 2*sizeof(int));
			sendPlay(board_x, board_y);
			//sleep(1);
	}
	printf("fim\n");
	free(board);
  close(sock_fd);
	exit(0);
}