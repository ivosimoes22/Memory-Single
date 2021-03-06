#include "communications.h"

void initSocket(char *address)
{
    sock_fd= socket(AF_INET, SOCK_STREAM, 0);

    if (sock_fd == -1)
    {
        perror("socket: ");
        exit(-1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port= htons(3000);
	inet_aton(address, &server_addr.sin_addr);

    if(connect(sock_fd, (const struct sockaddr *) &server_addr, sizeof(server_addr)) == -1)
    {
		printf("Error connecting\n");
		exit(-1);
	}
    printf("Connected to the server\n");   
}

int getDimension()
{
    int dim;
    if (read(sock_fd, &dim, sizeof(dim)) < 1)
    {
        done = 1;
        return 0;
    }
    return dim;
}

void sendPlay(int x, int y)
{
	int coord[2] = {x, y};
    if (write(sock_fd, &coord, sizeof(coord)) < 1)
    {
        done = 1;
    }
}

void checkWinner()
{
	int result[2];

    //Recebe do servidor o próprio score e impreme-o no ecran
	if (read(sock_fd, &result, sizeof(result)) < 1)
    {
        done = 1;
        return;
    }
	printf("\nYour score is %d!!\n", result[0]);


	if (result[1] == 1)
		printf("\nYou have the max score!! You won the game!!\n");
	else
		printf("\nYou lost! Better luck next time :)\n");

	return;
}
