/** @file
 * implematation of batchmode
 */ 
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "gamma.h"
#include "dynamic_array.h"

/**
 * runs gamma_move for batchmode.
 * checks if amount of arguments in @param d is proper
 */ 
static void batch_gamma_move(gamma_t* g,int *line,darray *d)
{
    if(d!=NULL && d->length==3)
    {
        if(gamma_move(g,d->a[0],d->a[1],d->a[2]))printf("1\n");
        else printf("0\n");
    }
    else
    {
        fprintf(stderr,"ERROR %d\n",*line);
    }
}

/**
 * runs gamma_busy_fields for batchmode.
 * checks if amount of arguments in @param d is proper
 */ 
void batch_gamma_busy_fields(gamma_t* g,int *line,darray *d)
{
    if(d!=NULL && d->length==1)
    {
        printf("%ld\n",gamma_busy_fields(g,d->a[0]));
    }
    else
    {
        fprintf(stderr,"ERROR %d\n",*line);
    }
}
/**
 * runs gamma_free_fields for batchmode.
 * checks if amount of arguments in @param d is proper
 */ 
void batch_gamma_free_fields(gamma_t* g,int *line,darray *d)
{
    if(d!=NULL && d->length==1)
    {
        printf("%ld\n",gamma_free_fields(g,d->a[0]));
    }
    else
    {
        fprintf(stderr,"ERROR %d\n",*line);
    }
}

/**
 * runs gamma_golden_possible for batchmode.
 * checks if amount of arguments in @param d  is proper
 */ 
void batch_gamma_golden_possible(gamma_t* g,int *line,darray *d)
{
    if(d!=NULL && d->length==1)
    {
        if(gamma_golden_possible(g,d->a[0]))printf("1\n");
        else printf("0\n");
    }
    else
    {
        fprintf(stderr,"ERROR %d\n",*line);
    }
}

/**
 * runs gamma_golden_move for batchmode.
 * checks if amount of arguments in @param d is proper
 */ 
void batch_gamma_golden_move(gamma_t* g,int *line,darray *d)
{
    if(d!=NULL && d->length==3)
    {
        if(gamma_golden_move(g,d->a[0],d->a[1],d->a[2]))printf("1\n");
        else printf("0\n");
    }
    else
    {
        fprintf(stderr,"ERROR %d\n",*line);
    }
}

/**
 * runs gamma_board for batchmode.
 * checks if amount of arguments in @param d is proper
 */ 
void batch_gamma_board(gamma_t* g,int *line,darray *d)
{
    if(d!=NULL && d->size==0)
    {
        char* s=gamma_board(g);
        printf("%s",s);
        free(s);
    }
    else
    {
        fprintf(stderr,"ERROR %d\n",*line);
    }
}

/**
 * reads line as a command.
 */
void recognise_command(gamma_t* g,int *line,int *z)
 {
    *z=getchar();
    int k=*z;
    *line=*line+1;
    if((*z)!=EOF && (*z)!='\n')*z=getchar();
    darray* d=read_numbers_from_line(z); 
    switch(k)
    {
        case 'm':batch_gamma_move(g,line,d);break;
        case 'g':batch_gamma_golden_move(g,line,d);break;
        case 'b':batch_gamma_busy_fields(g,line,d);break;
        case 'f':batch_gamma_free_fields(g,line,d);break;
        case 'q':batch_gamma_golden_possible(g,line,d);break;
        case 'p':batch_gamma_board(g,line,d);break;
        default:
        if(k!='#' && k!='\n' && k!=EOF)
        {
            fprintf(stderr,"ERROR %d\n",*line);
        }
        break;
    }
    d=free_darray(d);
 }


void main_batch(gamma_t* g,int *line)
{
    int z=' ';
    while(z!=EOF)
    {
        recognise_command(g,line,&z);
    }
}