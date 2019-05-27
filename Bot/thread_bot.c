#include "thread_bot.h"
#include "communications.h"

//Thread respoonsável por ler jogadas vindas do server

void* readPlays(void *arg)
{
  	play_response resp;
    int done = 0, dim=(*(int *)arg);

    while(done != 1)
    {
		read(sock_fd, &resp, sizeof(play_response));
		//printf("Code %d on play1 %d %d, play2 %d %d\n", resp.code, resp.play1[0], resp.play1[1], resp.play2[0], resp.play2[1]);

		switch (resp.code) 
		{
			case 1:
				//se a jogada nao estiver em memória
				if (strcmp (resp.str_play1, board[resp.play1[1]*dim +resp.play1[0]].v)!=0)
				{
					strcpy(board[resp.play1[1]*dim +resp.play1[0]].v, resp.str_play1);
				}
				break;
			
			case 2:
				board[resp.play1[1]*dim +resp.play1[0]].locked=1;
				board[resp.play2[1]*dim +resp.play2[0]].locked=1;
				break;

			case 3:
				board[resp.play1[1]*dim +resp.play1[0]].locked=1;
				board[resp.play2[1]*dim +resp.play2[0]].locked=1;
				checkWinner();
				break;

			case 4:
				if (strcmp (resp.str_play1, board[resp.play1[1]*dim +resp.play1[0]].v)!=0)
				{
					strcpy(board[resp.play1[1]*dim +resp.play1[0]].v, resp.str_play1);
				}
				break;
			
			case 5:
				for(int i=0; i < (dim*dim); i++)
				{
					board[i].v[0] = '\0';
					board[i].locked=0;
				}
				break;

			case -2:
				if (strcmp (resp.str_play2, board[resp.play2[1]*dim +resp.play2[0]].v)!=0)
				{
					strcpy(board[resp.play2[1]*dim +resp.play2[0]].v, resp.str_play2);
				}
				break;

			case -3:
				break;
				
			case -4:
				break;

			case -5:
				if (strcmp (resp.str_play1, board[resp.play1[1]*dim +resp.play1[0]].v)!=0)
				{
					strcpy(board[resp.play1[1]*dim +resp.play1[0]].v, resp.str_play1);
				}
				break;
		}
	}
  	pthread_exit(NULL);
}