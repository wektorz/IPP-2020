/** @file
 * imlements dynamic_array.h
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
struct dynamic_array{
    uint32_t *a;
    int length;
    int size;
    bool ok;
};
typedef struct dynamic_array darray;

darray* new_darray()
{
    darray* result=malloc(sizeof (darray));
    result->a=NULL;
    result->length=0;
    result->size=0;
    result->ok=true;
    return result;
}

void add_darray(darray *d,uint32_t x)
{
    if(d->length==d->size)
    {
        d->size=d->size*2+1;
        d->a=realloc(d->a, d->size * sizeof(uint32_t));
        if(d->a==NULL)exit(1);//emergency
    }
    d->a[d->length]=x;
    (d->length)++;
}

darray* free_darray(darray *d)
{
    if(d!=NULL)
    {
        if(d->a!=NULL)
        {
            free(d->a);
        }
        free(d);
    }
    return NULL;
}

/**
 * returns type of character.
 * 0 for digits
 * 1 for whitespaces
 * 2 for all other
 */
int character_type(int x)
{
    if((x>='0')&&(x<='9'))return 0;
    else if((x==' ')||(x=='\t')||(x=='\v')||(x=='\f')|| (x=='\r'))return 1;
    else return 2;
}
/**
 * definition of valid number (vnum)
 * has uint32 value and bool state
 */
struct valid_number{
    uint32_t n;
    bool ok;
};
typedef struct valid_number vnum;
/**
 * initilizes valid vnum to 0
 */
static vnum vnum_create()
{
    vnum result;
    result.n=0;
    result.ok=true;
    return result;
}
/**
 * reads number from input.
 * if input is not a valid number sets state of vnum to false
 * stores value of read number in vnum
 */
static vnum read_number(int *z)
{
    vnum result=vnum_create();
    if(character_type(*z)!=0)result.ok=false;
    do
    {
        if(((UINT32_MAX-(*z)+'0')/10>=result.n)&&character_type(*z)==0)
        {
        result.n=result.n*10+((*z)-'0');
        (*z)=getchar();
        }
        else
        {
            result.ok=false;
        }
    } while ((character_type(*z)==0) && result.ok);
    return result;
}
/**
 * ignores whitespaces from input
 */ 
static void ignore_whitespaces(int *z)
{
    while(((*z)!=EOF && (*z)!='\n') && (character_type(*z)==1))
    {
        (*z)=getchar();
    }
}
/**
 * end reading of bad line
 */
static void end_bad_line_read(int *z)
 {
    while((*z)!=EOF && (*z)!='\n')
    {
        *z=getchar();
    }
 }

darray* read_numbers_from_line(int *z)
{
    darray* result=new_darray();
    bool ok=true;
    if((*z)!='\n'&& character_type(*z)!=1)
    {
        ok=false;
    }
    ignore_whitespaces(z);
    while((*z)!=EOF && (*z)!='\n'&& character_type(*z)!=2 && ok)
    {
        vnum pom=vnum_create();
        pom=read_number(z);
        ignore_whitespaces(z);
        if(pom.ok)
        {
            add_darray(result,pom.n);
        }
        else
        {
            ok=false;
        }
    }
    if(ok==false || (character_type(*z)==2 && ((*z)!=EOF && (*z)!='\n')))
    {
        result=free_darray(result);
    }
    if((*z)!=EOF && (*z)!='\n')end_bad_line_read(z); 
    return result;
}
