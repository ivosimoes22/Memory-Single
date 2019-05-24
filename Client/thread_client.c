#include "thread_client.h"
#include "communications.h"
#include "UI_library.h"

//Thread respoonsável por ler jogadas vindas do server

void* readPlays(void *dim)
{
    play_response resp;

    while(1)
    {
      read(sock_fd, &resp, sizeof(play_response));
			//printf("Code %d on play1 %d %d, play2 %d %d\n", resp.code, resp.play1[0], resp.play1[1], resp.play2[0], resp.play2[1]);

      switch (resp.code) 
			{
		  case 1:
				paint_card(resp.play1[0], resp.play1[1] , resp.color.r, resp.color.g, resp.color.b);	//Pinta a primeira jogada (Letras a cinzento e cor do
				write_card(resp.play1[0], resp.play1[1], resp.str_play1, 200, 200, 200);							// background vinda do servidor)
				break;

			case 2:
				paint_card(resp.play1[0], resp.play1[1] , resp.color.r, resp.color.g, resp.color.b);	//Pinta as letras a preto (Bloqueia as duas cartas iguais)
				write_card(resp.play1[0], resp.play1[1], resp.str_play1, 0, 0, 0);
				paint_card(resp.play2[0], resp.play2[1] , resp.color.r, resp.color.g, resp.color.b);
				write_card(resp.play2[0], resp.play2[1], resp.str_play2, 0, 0, 0);
				break;

			case 3:
				paint_card(resp.play1[0], resp.play1[1] , resp.color.r, resp.color.g, resp.color.b);	//Pinta as letras a preto (Bloqueia as duas cartas iguais)
				write_card(resp.play1[0], resp.play1[1], resp.str_play1, 0, 0, 0);										//mas na última jogada, e seguidamente verifica o score
				paint_card(resp.play2[0], resp.play2[1] , resp.color.r, resp.color.g, resp.color.b);
				write_card(resp.play2[0], resp.play2[1], resp.str_play2, 0, 0, 0);
				checkWinner(score_winner);
				break;

			case 4:
				printf("Check!\n");
				paint_card(resp.play1[0], resp.play1[1] , resp.color.r, resp.color.g, resp.color.b);	//Pinta as letras a preto (Bloqueia só uma carta). Utilizada 
				write_card(resp.play1[0], resp.play1[1], resp.str_play1, 0, 0, 0);										// para receber a board quando um cliente entra no jogo
				break;

			case 5:
				clear_board((*(int*)dim));																														//Da reset a board
				printf("\nNew game started!!\n");
				break;

			case -2:
				paint_card(resp.play1[0], resp.play1[1] , resp.color.r, resp.color.g, resp.color.b);	//Pinta as letras das duas peças a vermelho
				write_card(resp.play1[0], resp.play1[1], resp.str_play1, 255, 0, 0);
				paint_card(resp.play2[0], resp.play2[1] , resp.color.r, resp.color.g, resp.color.b);
				write_card(resp.play2[0], resp.play2[1], resp.str_play2, 255, 0, 0);
				break;

			case -3:
				paint_card(resp.play1[0], resp.play1[1] , 255, 255, 255);
				paint_card(resp.play2[0], resp.play2[1] , 255, 255, 255);
				break;
				
			case -4:
				paint_card(resp.play1[0], resp.play1[1] , 255, 255, 255);
				break;

			case -5:
				paint_card(resp.play1[0], resp.play1[1] , resp.color.r, resp.color.g, resp.color.b);	//Pinta as letras de uma peça a vermelho 
				write_card(resp.play1[0], resp.play1[1], resp.str_play1, 255, 0, 0);									// Utilizado para dar a board a novo jogador 
				break;																																								// quando este entra a meio do jogo
		}
	}
}