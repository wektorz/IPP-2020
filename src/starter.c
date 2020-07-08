/** @file 
 * @brief awaits proper comand to start the game.
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "gamma.h"
#include "dynamic_array.h"
#include "batchmode.h"
#include "interactivemode.h"

#define MAX_PROMPT ((unsigned int) 25)
/** @brief check if board will fit into terminal.
 * @p width board width
 * @p height board height
 * @p players amount of players ingame
 */ 
bool is_window_size_ok(uint32_t width,uint32_t height,uint32_t players)
{
    bool ok=false;
    struct winsize win;
    if (!(ioctl(STDOUT_FILENO, TIOCGWINSZ, &win) == -1))
    {
        if(players<10)
        {
            //height+1 because of ingame prompts
            if( height+1 < win.ws_row && width < win.ws_col)
            {
                if(width<=MAX_PROMPT+1)
                {
                ok=true;
                }
            }
        }
        else
        {
            int k=players;
            int length=0;
            while(k>0)
            {
                k=k/10;
                length++;
            }
            //(length+1) because of separator |
            if( height*length+1 < win.ws_row && width*(length+1) < win.ws_col)
            {
                if(width<=MAX_PROMPT+length)
                {
                ok=true;
                }
            }
        }
    }
    return ok;
}
int main()
{
    int input=' ';
    int line=0;
    gamma_t *g=NULL;
    darray *d=NULL;
    bool not_done=true;//was batchmode or interactive mode not called earlier
    bool not_ok=true;//used for checking parameters
    while(input!=EOF && not_done)
    {
        line++;
        input=getchar();
        switch (input)
        {
            case 'B':
                input= getchar();
                d= read_numbers_from_line(&input);
                not_ok=true;
                if(d!=NULL && d->length==4)
                {
                    g= gamma_new(d->a[0] ,d->a[1] ,d->a[2] ,d->a[3]);
                    if(g!=NULL)
                    {
                        printf("OK %d\n",line);
                        main_batch(g,&line);
                        gamma_delete(g);
                        not_ok=false;
                        not_done=false;
                    }
                }
                if(not_ok) fprintf(stderr,"ERROR %d\n",line);
                d=free_darray(d);
            break;

            case 'I':
                input= getchar();
                d= read_numbers_from_line(&input);
                not_ok=true;
                if(d!=NULL && d->length==4)
                {
                    g=gamma_new(d->a[0], d->a[1], d->a[2], d->a[3]);
                    if(g!=NULL)
                    {
                        if(is_window_size_ok(d->a[0], d->a[1], d->a[2]))
                        {
                            printf("OK %d\n",line);
                            main_interactive(g);
                            not_ok=false;
                            not_done=false;
                        }
                        else
                        {
                            printf("Terminal is too small :(\n");
                        }
                        gamma_delete(g);
                    }
                    
                }
                if(not_ok) fprintf(stderr,"ERROR %d\n",line);
                d=free_darray(d);
            break;

            case '\n':break;
            case EOF: break;
            default:
                if(input!='#') 
                {
                    fprintf(stderr,"ERROR %d\n",line);
                }
                while(input!=EOF && input!='\n')
                {
                    input= getchar();
                }
            break;
        }
    }
    
}