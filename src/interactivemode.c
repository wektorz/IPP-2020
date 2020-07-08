/**
 * @file
 * implements interactive mode.
 * */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <termios.h>
#include <unistd.h>

#include "gamma.h"
#include "dynamic_array.h"

#define CTRL_D 4
#define SPACE 32
#define ESC '\033'
/** getchar for interactive mode.
 * input in real time.
 */
int i_getchar()
{
    //setup terminal
    struct termios old_terminal, new_terminal;
    if(tcgetattr(STDIN_FILENO, &old_terminal)==-1) exit(1);
    new_terminal=old_terminal;
    new_terminal.c_lflag &=~(ICANON);
    new_terminal.c_lflag &=~(ECHO);
    if(tcsetattr(STDIN_FILENO, TCSANOW, &new_terminal)==-1) exit(1);
    int z=getchar();
    //restore terminal
    if(tcsetattr(STDIN_FILENO, TCSANOW, &old_terminal)==-1) exit(1);
    return z;
}
/**hides cursor using ANSI.
 */ 
void hide_cursor()
{
    printf("\e[?25l");
}

/** prints board state
 */
void interactive_gamma_board(gamma_t* g,uint32_t player, uint32_t cursor_x,uint32_t cursor_y)
{
    printf("%c[2J",ESC);
    hide_cursor();
    printf("%c[0;0H",ESC);
    char *screen=gamma_board_interactive(g ,player, cursor_x, cursor_y);
    printf("%s",screen);
    free(screen);
}
/** prints results 
 */
void results(gamma_t* g)
{
    for(uint32_t i=0;i<g->players;i++)
    {
        printf("PLAYER %d %ld\n",i+1,g->players_tiles[i]);
    }
}

/** read cursor movement from input.
 * updates cursor postion
 */ 
void move_cursor(gamma_t *g,uint32_t* x,uint32_t* y,int *z)
{
    if(*z==ESC)
    {
        (*z)=i_getchar();
        if((*z)=='[')
        {
            (*z)=i_getchar();
            switch (*z)
            {
                case 'A': 
                if(*y +1<g->height)
                {
                    *y=*y +1;
                }break;

                case 'C' :
                if(*x +1<g->width)
                {
                    *x=*x +1;
                }break;

                case 'B': 
                if(*y>0)
                {
                    *y=*y -1;
                }break;

                case 'D': 
                if(*x>0)
                {
                    *x=*x -1;
                }break;

            }
        }
    }
}


/** update current_player to next valid player number
 */ 
void change_player(uint32_t* current_player,uint32_t players)
{
    *current_player=*current_player+1; 
    if(*current_player==players+1) *current_player=1;
}
/** update current_player to next player which can make a move.
 * @returns succes
 */ 
bool next_player(gamma_t *g,uint32_t* current_player)
{
    bool found=true;
    uint32_t h=*current_player;
    do
    {
        change_player(current_player,g->players);
        if(*current_player==h)found=false;
    } while (found && gamma_free_fields(g, *current_player)==0
            && !gamma_golden_possible(g, *current_player));
    if(*current_player==h && (gamma_free_fields(g, *current_player)>0
            || gamma_golden_possible(g, *current_player)))
            {
                found=true;
            }
    return found;
}

/**
 * read input as command
 * calls proper functions for inputs
 */ 
void interactive_command(gamma_t *g,int *z,bool *ok,uint32_t* x,uint32_t* y,uint32_t *current_player)
{
    switch(*z)
    {
        case EOF: 
            *ok=false;
        break;
        
        case CTRL_D:
            *ok=false;
        break;

        case SPACE://gamma_move
            if(gamma_move(g, *current_player,*x,*y))
            {
                *ok=next_player(g, current_player);
            }
        break;

        case 'c':case 'C'://player skip
            *ok=next_player(g, current_player);
        break;

        case 'g':case 'G'://golden_move
            if(gamma_golden_move(g, *current_player, *x, *y))
            {
                *ok=next_player(g, current_player);
            }
        break;

        default:move_cursor(g, x, y, z);break;
    }
    interactive_gamma_board(g, *current_player,*x,*y);
    printf("current player: %d\n", *current_player);
}
/**
 * changes terminal state,
 * main input loop
 */ 
void main_interactive(gamma_t* g)
{
    int z=' ';
    uint32_t cursor_x=0,cursor_y=0;
    bool ok=true;
    uint32_t current_player=1;

    

    interactive_gamma_board(g, current_player, cursor_x, cursor_y);

    while(ok)
    {
        z=i_getchar();
        interactive_command(g ,&z, &ok, &cursor_x, &cursor_y, &current_player);
    }
    
    //writing end state
    interactive_gamma_board(g, current_player, cursor_x, cursor_y);
    results(g);
    printf("%c[m",ESC);

}