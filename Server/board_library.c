#include "board_library.h"

int linear_conv(int i, int j)
{
  return j*dim_board+i;
}
char * get_board_place_str(int i, int j){
  //printf("\n%d, %d, %d ->%s\n",i, j, linear_conv(i,j), board[linear_conv(i, j)].v);
  return board[linear_conv(i, j)].v;
}

void init_board(int dim)
{
  int count  = 0;
  int i, j;
  char * str_place;
  dim_board = dim;
  char c1, c2;

  if (alloc == 0)
  {
    board = malloc(sizeof(board_place)* dim *dim);
    alloc = 1;
  }
  for( i=0; i < (dim_board*dim_board); i++)
  {
    board[i].v[0] = '\0';
    board[i].revealed = 0;
    board[i].first = 0;
    board[i].wrong = 0;
    board[i].locked = 0;
  }

  for (c1 = 'a' ; c1 < ('a'+dim_board); c1++)
  {
    for (c2 = 'a' ; c2 < ('a'+dim_board); c2++)
    {
      do
      {
        i = random()% dim_board;
        j = random()% dim_board;
        str_place = get_board_place_str(i, j);
        //printf("%d %d -%s-\n", i, j, str_place);
      }
      while(str_place[0] != '\0');
      
      str_place[0] = c1;
      str_place[1] = c2;
      str_place[2] = '\0';
      
      do
      {
        i = random()% dim_board;
        j = random()% dim_board;
        str_place = get_board_place_str(i, j);
        //printf("%d %d -%s-\n", i, j, str_place);
      }
      while(str_place[0] != '\0');
      
      str_place[0] = c1;
      str_place[1] = c2;
      str_place[2] = '\0';
      count += 2;
      
      if (count == dim_board*dim_board)
        return;
    }
  }
}

play_response board_play(int x, int y, int play1[2], int wrongplay[4], int jogada, Color color, int *score)
{
  play_response resp;
  char* first_str = get_board_place_str(play1[0], play1[1]);
  char* secnd_str = get_board_place_str(x, y);

  pthread_mutex_lock(&mux[x]);
  if (board[linear_conv(x,y)].revealed == 0)
  {
    if (jogada == 1)
    {
      board[linear_conv(x,y)].revealed = 1;
      pthread_mutex_unlock(&mux[x]);
      board[linear_conv(x,y)].first = 1;
      board[linear_conv(x,y)].color = color;
      resp.code = 1;
      play1[0] = x;
      play1[1] = y;
      resp.play1[0] = x;
      resp.play1[1] = y;
      strcpy(resp.str_play1, get_board_place_str(x, y)); 
    }
    else
    {
      //acertou
      if (strcmp(first_str, secnd_str) == 0)
      {
        board[linear_conv(x,y)].revealed = 1;
        pthread_mutex_unlock(&mux[x]);
        board[linear_conv(x,y)].locked = 1;
        board[linear_conv(x,y)].color = color;
        board[linear_conv(play1[0], play1[1])].locked = 1;
        board[linear_conv(play1[0], play1[1])].first = 0;
        n_corrects += 2;
        (*score)++;
        if (n_corrects == dim_board* dim_board)
        {
          resp.code = 3;
        }
        else
        {
          resp.code = 2;
        }
      }
      //errou
      else
      {
        board[linear_conv(x,y)].revealed = 1;
        pthread_mutex_unlock(&mux[x]);
        board[linear_conv(x,y)].wrong = 1;
        board[linear_conv(play1[0], play1[1])].wrong = 1;
        board[linear_conv(play1[0], play1[1])].first = 0;
        board[linear_conv(x,y)].color = color;
        //Guarda as coordenadas das duas joagadas erradas
        wrongplay[0] = play1[0];
        wrongplay[1] = play1[1];
        wrongplay[2] = x;
        wrongplay[3] = y;
        resp.code = -2;
      }
      resp.play1[0] = play1[0];
      resp.play1[1] = play1[1];
      resp.play2[0] = x;
      resp.play2[1] = y;
      strcpy(resp.str_play1, first_str);
      strcpy(resp.str_play2, secnd_str);
    } 
  }
  else
  {
    pthread_mutex_unlock(&mux[x]);
    //jogada 1 em carta revelada
    if (jogada == 1)
    {
      resp.code = -20;
    }
    //jogada 2 em carta revelada
    else
    {
      pthread_mutex_lock(&mux[play1[0]]);
      if (board[linear_conv(play1[0],play1[1])].locked != 1)
      {
        board[linear_conv(play1[0],play1[1])].revealed = 0;
        pthread_mutex_unlock(&mux[play1[0]]);
        board[linear_conv(play1[0],play1[1])].first = 0;
        resp.play1[0]= play1[0];
        resp.play1[1]= play1[1];
        resp.code = -4;
      }
      else
      {
        pthread_mutex_unlock(&mux[play1[0]]);
      }
      
    }
  }
  //pthread_mutex_unlock(&mux[x]);
  return resp;
}