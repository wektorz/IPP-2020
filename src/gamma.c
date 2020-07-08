/** @file gamma.c
 * implementation of gamma.h functionality
 * @date 16.04.2020
*/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define ESC '\033'

/**  struct line
holds pointer to content of line
*/
struct line{
    int *line;
};

/**     struct gamma
 * holds information about state of the game:
 * width -board width
 * height -board height
 * players -amount of players
 * areas -maximum allowed number of areas for one player
 * row -pointer to array of lines, stores state of the board
 * pom -pointer to array of lines, stores information whenever the tile was visited in rescan
 * players_golden- pointer to array with information whenever player executed their golden move already
 * players_area -pointer to  array with information about each player current area count
 * players_tiles -pointer to  array with information about each players current tile count
 * */
struct gamma{
    uint32_t width;
    uint32_t height;
    uint32_t players;
    uint32_t  areas;
    struct line *row;
    struct line *pom;
    bool *players_golden;
    uint32_t *players_area;
    uint64_t *players_tiles;
};
typedef struct gamma gamma_t;


/**  
 * frees memory used by @p a pointers
 */
static void line_delete(struct line *a)
{
    if(a!=NULL)
    {
        if((a->line)!=NULL)
        {
            free(a->line);
        }
    }
}


void gamma_delete(gamma_t *g)
{
    if(g!=NULL)
    {
        if(g->row!=NULL)
        {
            for(uint32_t i=0;i<g->height;i++)
            {
                line_delete(&(g->row[i]));
            }
            free(g->row);
        }
        if(g->pom!=NULL)
        {
            for(uint32_t i=0;i<g->height;i++)
            {
                line_delete(&(g->pom[i]));
            }
            free(g->pom);
        }
        if(g->players_tiles!=NULL)free(g->players_tiles);
        if(g->players_area!=NULL)free(g->players_area);
        if(g->players_golden!=NULL)free(g->players_golden);
        free(g);
    }
}

/**     line_setup
 * function to alocate memory for line @p a array
 * and fill it with @p k
 * array is length of @p width
 */

static void line_setup(struct line *a,uint32_t width,uint32_t k)
{
    a->line=malloc(width*sizeof(uint32_t));
    if(a->line!=NULL)
    {
        for(uint32_t i=0;i<width;i++)
        {
            a->line[i]=k;
        }
    }
}

/** allocate memory for array of lines.
 * array is length of @p height
 */
static struct line* row_setup(uint32_t height)
{
    struct line* result;
    result=malloc(height*sizeof(struct line));
    return result;
}

/** allocate memory for array containg counts of each player current areas.
 * index for player k is k-1
 * array length is @p players
 */
static uint32_t* players_area_setup(uint32_t players)
{
    uint32_t *result;
    result=calloc(players*sizeof(uint32_t),sizeof(uint32_t));
    return result;
}

/** 
 * allocate memory for array containg counts of each player current tiles
 *  index for player k is k-1
 * array length is @p players
 */
static uint64_t* players_tiles_setup(uint32_t players)
{
    uint64_t *result;
    result=calloc(players*sizeof(uint64_t),sizeof(uint64_t));
    return result;
}

/** 
 * allocate memory for array containg bool value of true for each player
 * index for player k is k-1
 * array length is @p players
 */
static bool* players_bool_setup(uint32_t players)
{
    bool *result;
    result=malloc(players*sizeof(bool));
    if(result!=NULL)
    {
        for(uint32_t i=0;i<players;i++)
        {
            result[i]=true;
        }
    }
    return result;
}

/** 
 * allocate memory for structure gamma and initialize arrays inside of it.
 * @param[in] width   – board width, positive number
 * @param[in] height  – board height, positive number
 * @param[in] players – number of players, positive
 * @param[in] areas   – max amount of areas taken by one player
 */
static gamma_t* gamma_setup(uint32_t width, uint32_t height,
                            uint32_t players, uint32_t areas)
{
    gamma_t* result;
    result=malloc(sizeof *result);
    if(result!=NULL)
    {
        result->height=height;
        result->width=width;
        result->players=players;
        result->areas=areas;
        result->row=row_setup(height);
        result->pom=row_setup(height);
        result->players_area=players_area_setup(players);
        result->players_tiles=players_tiles_setup(players);
        result->players_golden=players_bool_setup(players);
        if( result->row==NULL || result->pom==NULL || result->players_area==NULL
                || result->players_tiles==NULL || result->players_golden==NULL )
            {
                gamma_delete(result);
                result=NULL;
            }
    }

    return result;
}


gamma_t* gamma_new(uint32_t width, uint32_t height,
                   uint32_t players, uint32_t areas)
{
    gamma_t *p=NULL;
    bool ok=true;
    if(width>0 && height>0 && players>0 && areas>0)
    {
        p=gamma_setup(width,height,players,areas);
        if(p!=NULL && p->row!=NULL && p->pom!=NULL)
        {
           for(uint32_t i=0;i<height;i++)
            {
                line_setup(&(p->row[i]),width,'.');
                if(p->row[i].line==NULL)ok=false;
            }
            for(uint32_t i=0;i<height;i++)
            {
                line_setup(&(p->pom[i]),width,0);
                if(p->pom[i].line==NULL)ok=false;
            }
            if(ok==false)
            {
                gamma_delete(p);
                p=NULL;
            }
        }
    }
    return p;
}



/**
 *  checks if tile < @p x , @p y >  is a part of the board @p g .
 * @p g game which state is to be changed
 * @p x tile x coordinate
 * @p y tile y coordinate
 */
static bool valid_tile(gamma_t *g,uint32_t x,uint32_t y)
{
    return ((x< g->width)&&(y< g->height));
}

/** 
 * returns tile < @p x , @p y > value for valid tiles
 * and returns 0 for invalid tiles.
 * @p g game which state is to be changed
 * @p x tile x coordinate
 * @p y tile y coordinate
 */
static uint32_t tile_value(gamma_t *g,uint32_t x,uint32_t y)
{
    if(valid_tile(g,x,y))return g->row[y].line[x];
    else return 0;
}

/** 
 * checks if @p player is vaild for @p g .
 * @p g game which state is to be changed
 * @p player player number
 */
static bool valid_player(gamma_t *g,uint32_t player)
{
    return ((player>0)&&(player<= g->players));
}

/** 
 * sets @p g->pom arrays values to 0 (unvisited).
 * @p g -game which state is to be changed
 */
static void area_dfs_cleanup(gamma_t *g)
{
    for(uint32_t i=0;i<g->height;i++)
    {
        for(uint32_t j=0;j<g->width;j++)g->pom[i].line[j]=0;
    }
}


/** 
 * basic stack implementation
 * availible : pop and push
 * @p x tile x coordinate
 * @p y tile y coordinate
 * @p next is pointer to lower elements in stack
 */
struct stack{
    uint32_t x;
    uint32_t y;
    struct stack *next;
};

/** adds tile coordinates to stack.
 * @p s -pointer to stack pointer
 * @p x -tile x coordinate
 * @p y -tile y coordinate
 * @p r -pointer to stack size
 */
static void push(struct stack **s,uint32_t x,uint32_t y,uint64_t *r)
{
    struct stack *a=NULL;
    a=malloc(sizeof *a);
    if(a==NULL)exit(1);
    a->x=x;
    a->y=y;
    a->next=*s;
    *s=a;
    *r=*r+1;
}
/** copies tile from top of the stack and removes stack top node.
 * @p s -pointer to stack pointer
 * @p x -pointer to coordinate x of tile
 * @p y -pointer to coordinate y of tile
 * @p r -pointer to stack size
 *  */
static void pop(struct stack **s,uint32_t *x,uint32_t *y,uint64_t *r)
{
    struct stack *a=*s;
    *x=a->x;
    *y=a->y;
    *r=*r-1;
    *s=a->next;
    free(a);
}


/** 
 * marks all tiles in the same area as starting point to visited state 
 *  (tile in g->pom =1).
 * @param g- pointer to current game state
 * @param x- current tile column (indexed from 0)
 * @param y- current tile line (indexed from 0)
 * @param p- value of starting tile (one on which the function is called first)
 */
static void area_dfs(gamma_t *g,uint32_t x,uint32_t y,uint32_t p)
{
    uint64_t r=0;
    struct stack *s=NULL;
    push(&s,x,y,&r);
    g->pom[y].line[x]=1;
    while(r>0)
    {
        pop(&s,&x,&y,&r);
        if(tile_value(g,x+1,y)==p && (g->pom[y].line[x+1]==0))
        {
            push(&s,x+1,y,&r);
            g->pom[y].line[x+1]=1;
        }
        if(tile_value(g,x-1,y)==p && (g->pom[y].line[x-1]==0))
        {
            push(&s,x-1,y,&r);
            g->pom[y].line[x-1]=1;
        }
        if(tile_value(g,x,y+1)==p && (g->pom[y+1].line[x]==0))
        {
            push(&s,x,y+1,&r);
            g->pom[y+1].line[x]=1;
        }
        if(tile_value(g,x,y-1)==p && (g->pom[y-1].line[x]==0))
        {
            push(&s,x,y-1,&r);
            g->pom[y-1].line[x]=1;
        }
    }
}

/** 
 * update area count for @p player in @p g .
 * @p g -game which state is to be changed
 * @p player -player number
 */
static void area_rescan_one_player(gamma_t *g,uint32_t player)
{
    uint32_t k=0;
    g->players_area[player-1]=0;
    for(uint32_t i=0;i<g->height;i++)
    {
        for(uint32_t j=0;j<g->width;j++)
        {
            k=tile_value(g,j,i);
            if((k=='0'+player)&&(g->pom[i].line[j]==0))
            {
                area_dfs(g,j,i,k);
                g->players_area[player-1]++;
            }
        }
    }
    area_dfs_cleanup(g);
}

/** 
 * checks if one of the tiles next to tile < @p x, @p y >
 * belong to @p player .
 * @p g -game which state is to be changed
 * @p player -player number
 * @p x -coordinate x of checked tile
 * @p y -coordinate y of checked tile
 */
bool neigbours(gamma_t *g,uint32_t player, uint32_t x, uint32_t y)
{
    bool result=false;
    if(tile_value(g,x+1,y)==player+'0')result=true;
    else if(tile_value(g,x-1,y)==player+'0')result=true;
    else if(tile_value(g,x,y+1)==player+'0')result=true;
    else if(tile_value(g,x,y-1)==player+'0')result=true;
    return result;
}


bool gamma_move(gamma_t *g, uint32_t player, uint32_t x, uint32_t y)
{
    bool result=false;
    if(g!=NULL && tile_value(g,x,y)=='.'&&valid_player(g,player))
    {
        g->row[y].line[x]='0'+player;
        if(neigbours(g,player,x,y)==false)
        {
            g->players_area[player-1]++;
        }
        if(g->players_area[player-1] == g->areas+1)area_rescan_one_player(g,player);
        if(g->players_area[player-1] <= g->areas)
        {
            g->players_tiles[player-1]++;
            result=true;
        }
        else
        {
            g->row[y].line[x]='.';
            if(neigbours(g,player,x,y)==false)g->players_area[player-1]--;
        }

    }
    return result;
}


/** 
 * update area count for @p player1 and @p player2 in @p g .
 * @p g -game which state is to be changed
 * @p player1 -player1 number
 * @p player2 -player2 number
 */
static void area_rescan_two_players(gamma_t *g,uint32_t player1,uint32_t player2)
{
    uint32_t k=0;
    g->players_area[player1-1]=0;
    g->players_area[player2-1]=0;

    for(uint32_t i=0;i<g->height;i++)
    {
        for(uint32_t j=0;j<g->width;j++)
        {
            k=tile_value(g,j,i);
            if((k=='0'+player1)&&(g->pom[i].line[j]==0))
            {
                area_dfs(g,j,i,k);
                g->players_area[player1-1]++;
            }
            else if((k=='0'+player2)&&(g->pom[i].line[j]==0))
            {
                area_dfs(g,j,i,k);
                g->players_area[player2-1]++;
            }
        }
    }
    area_dfs_cleanup(g);
}


bool gamma_golden_move(gamma_t *g, uint32_t player, uint32_t x, uint32_t y)
{
    bool result=false;
    if(g!=NULL)
    {
        uint32_t k=tile_value(g,x,y);

        if(valid_player(g,player)&& g->players_golden[player-1]
           && k!='.'&& (k-'0')!=player && k!=0)
        {
            uint32_t p1=g->players_area[k-'1'];
            uint32_t p2=g->players_area[player-1];
            g->row[y].line[x]='0'+player;
            area_rescan_two_players(g,k-'0',player);
            if((g->players_area[player-1] <= g->areas)
               && (g->players_area[k-'1'] <= g->areas))
            {
                g->players_golden[player-1]=false;
                g->players_tiles[player-1]++;
                g->players_tiles[k-'1']--;
                result=true;
            }
            else
            {
                g->row[y].line[x]=k;
                g->players_area[player-1]=p2;
                g->players_area[k-'1']=p1;
            }
        }
    }
    return result;
}
bool gamma_weak_golden_possible(gamma_t *g, uint32_t player)
{
    bool result=false;
    if(g!=NULL && valid_player(g,player) && g->players_golden[player-1])
    {
        uint64_t s=0;
        for(uint32_t i=0;i<g->players;i++)
        {
            if(i!=player-1)s=s+g->players_tiles[i];
        }
        if(s>0)result=true;
    }
    return result;
}
bool gamma_golden_possible(gamma_t *g, uint32_t player)
{
    bool possible=false;
    if(gamma_weak_golden_possible(g,player))
    {
        /*jezeli gracz ma mniej obszarow niż wartosc maksymalna i inni maja pola to zawsze
         mozna wziac pole ktore nie rozdzieli pola innego gracza*/
        area_rescan_one_player(g,player);
        if(g->players_area[player-1]<g->areas)
        {
            possible=true;
        }
        else
        {
            uint32_t i=0;
            while(i< g->height && !possible)
            {
                uint32_t j=0;
                while(j< g->width && !possible)
                {
                    int k=g->row[i].line[j];
                    uint32_t p1=0;
                    if(valid_player(g,k-'0'))
                    {
                        p1=g->players_area[k-'1'];
                    }
                    uint32_t p2=g->players_area[player-1];
                    if(neigbours(g,player, j, i) && gamma_golden_move(g,player,j,i))
                    {
                        possible=true;
                        g->players_golden[player-1]=true;
                        g->players_tiles[player-1]--;
                        g->players_tiles[k-'1']++; 
                        g->row[i].line[j]=k;
                        g->players_area[player-1]=p2;
                        g->players_area[k-'1']=p1;
                    }
                    j++;
                }
                i++;
            }
        }
    }
    return possible;
}

uint64_t gamma_busy_fields(gamma_t *g, uint32_t player)
{
    if(g!=NULL && valid_player(g,player))return g->players_tiles[player-1];
    else return 0;
}

/** 
 * count amount of free tiles nex to to @p player tiles in board of @p g .
 * @p g -game which state is to be changed
 * @p player -player number
 */
static uint64_t empty_fields_next_player(gamma_t *g,uint32_t player)
{
    uint64_t result=0;
    if(g!=NULL)
    {
        for(uint32_t i=0;i<g->height;i++)
        {
            for(uint32_t j=0;j<g->width;j++)
            {
                if(g->row[i].line[j]=='.')
                {
                    int p=0;
                    if(tile_value(g,j+1,i)=='0'+player)p=1;
                    if(tile_value(g,j-1,i)=='0'+player)p=1;
                    if(tile_value(g,j,i+1)=='0'+player)p=1;
                    if(tile_value(g,j,i-1)=='0'+player)p=1;
                    if(p==1)result++;
                }
            }
        }
    }
    return result;
}


uint64_t gamma_free_fields(gamma_t *g, uint32_t player)
{
    uint64_t result=0;
    if(g!=NULL && valid_player(g,player))
    {
        if(g->players_area[player-1]==g->areas)area_rescan_one_player(g,player);
        if(g->players_area[player-1]<g->areas)
        {
            uint64_t s=0;
            for(uint32_t i=0;i<g->players;i++)
            {
                s=s+g->players_tiles[i];
            }
            result=g->height*g->width-s;
        }
        else
        {
            result=empty_fields_next_player(g,player);
        }
    }
    return result;
}


/**
 * sets fragment of string to printout to represantation of given tile
*/
void tile_print(gamma_t *g,uint32_t y, uint32_t x,char *s,int length)
{
    uint32_t p=g->row[y].line[x];
    if(p!='.')
    {
        p=p-'0';
        while(p>0)
        {
            s[length-1]='0'+p%10;
            p=p/10;
            length--;
        }
        while(length>0)
        {
            s[length-1]=' ';
            length--;
        }
    }
    else
    {
        s[length-1]='.';
        length--;
        while(length>0)
        {
            s[length-1]=' ';
            length--;
        }
    }
    
}
char* gamma_board(gamma_t *g)
{
    char *result=NULL;
    if(g!=NULL)
    {
        if(g->players<10)
        {
            result=malloc((g->height* (g->width+1)+1) *sizeof(char));
            if(result!=NULL)
            {
                for(uint32_t i=0;i<g->height;i++)
                {
                    for(uint32_t j=0;j<g->width;j++)result[(g->width+1)*(g->height-i-1)+j]=g->row[i].line[j];
                    result[(g->width+1)*(g->height-i)-1]='\n';
                }
                result[g->height*(g->width+1)]=0;
            }
        }
        else
        {
            int k=g->players;
            int length=0;
            while(k>0)
            {
                k=k/10;
                length++;
            }
            result=malloc((g->height*(g->width*(length+1)+1)+1)*sizeof(char));
            if(result!=NULL)
            {
                for(uint32_t i=0;i<g->height;i++)
                {
                    for(uint32_t j=0;j<g->width;j++)
                    {
                        tile_print(g ,i ,j ,result +(g->height-i-1) *(g->width *(length+1) +1) +j*(length+1), length);
                        result[ (g->height-i-1) *(g->width* (length+1) +1) +j *(length+1) +length]='|';
                    }
                    result[(g->width*(length+1)+1)*(g->height-i)-1]='\n';
                }
                result[g->height*(g->width*(length+1)+1)]=0;
            }
        }
        
    }
    return result;
}
/** @brief write inverse ANSI code to char table starting at index *i.
 */ 
void write_inverse(char* c,int *i)
{
    //4
    c[*i]=ESC;//27
    (*i)++;
    c[*i]='[';//91
    (*i)++;
    c[*i]='7';//55
    (*i)++;
    c[*i]='m';//109
    (*i)++;
    
}
/** @brief write end inverse ANSI code to char table starting at index *i.
 */ 
void write_end_inverse(char *c,int *i)
{
    //5
    c[*i]=ESC; 
    (*i)++;
    c[*i]='['; 
    (*i)++;
    c[*i]='2'; 
    (*i)++;
    c[*i]='7'; 
    (*i)++;
    c[*i]='m'; 
    (*i)++;
    
}
/** @brief write set backgound to black ANSI code to char table starting at index *i.
 */ 
void write_background_black_color(char *c,int *i)
{
    //5
    c[*i]=ESC;
    (*i)++;
    c[*i]='[';
    (*i)++;
    c[*i]='4';
    (*i)++;
    c[*i]='0';
    (*i)++;
    c[*i]='m';
    (*i)++;
    
}
/** @brief write set backgound to green ANSI code to char table starting at index *i.
 */ 
void write_background_green_color(char *c,int *i)
{
    //5
    c[*i]=ESC;
    (*i)++;
    c[*i]='[';
    (*i)++;
    c[*i]='4';
    (*i)++;
    c[*i]='2';
    (*i)++;
    c[*i]='m';
    (*i)++;
    
}
/** @brief write tile content to char table.
 * @p c pointer to char table
 * @p i pointer to int used as iterator over c
 * @p n tile content
 * @p length length of tile to write
 */ 
void write_tile(char *c,int *i,int n,int length)
{
    int p=length;
    if(n=='.')
    {
        for(int j=0;j<length-1;j++)
        {
            c[(*i)+j]=' ';
        }
        c[(*i)+length-1]='.';
    }
    else
    {
        n=n-'0';
        while(n>0)
        {
            length--;
            c[(*i)+length]=n%10+'0';
            n=n/10;
        }
        while(length>0)
        {
            length--;
            c[(*i)+length]=' ';
        }
    }
    (*i)=(*i)+p;
}
/** @brief prints gamma board with less than 10 players ingame for inteactive_mode.
 * @p g pointer to gamma structure
 * @p current_player current player number
 * @p cursor_x horizontal position of cursor 
 * @p cursor_y vertical postion of cursor 
 */ 
char* gamma_board_interactive_less_than_10(gamma_t *g,uint32_t current_player, uint32_t cursor_x,uint32_t cursor_y)
{
    int player= current_player;
    char *result=NULL;
    uint64_t size= g->height *(g->width+1) +10 + 10*g->players_tiles[player-1];
    result= malloc(size* sizeof(char));
    if(result!=NULL)
    {
        int i=0;
        for(uint64_t j=0; j < g->height; j++)
        {
            uint64_t y=g->height-j-1;
            for(uint64_t x=0; x< g->width; x++)
            {
                if(g->row[y].line[x]-'0'==player)
                {
                    write_background_green_color(result,&i);
                }
                if(y==cursor_y && x==cursor_x)
                {
                    write_inverse(result,&i);
                }

                result[i]= g->row[y].line[x];
                i++;

                if(y==cursor_y && x==cursor_x)
                {
                    write_end_inverse(result,&i);
                }   
                if(g->row[y].line[x]-'0'==player)
                {
                    write_background_black_color(result,&i);
                }
            }
            result[i]='\n';
            i++;
        }
        result[i]=0;
    }
    return result;
}
/** @brief prints gamma board with at least 10 players ingame for inteactive_mode.
 * @p g pointer to gamma structure
 * @p current_player current player number
 * @p cursor_x horizontal position of cursor 
 * @p cursor_y vertical postion of cursor 
 */ 
char* gamma_board_interactive_more_than_9(gamma_t *g,uint32_t current_player, uint32_t cursor_x,uint32_t cursor_y)
{
    int player= current_player;
    char *result=NULL;
    int k=g->players;
    int length=0;
    while(k>0)
    {
        k=k/10;
         length++;
    }
    uint64_t size= g->height* (g->width *(length+1) +1) +10 +10*g->players_tiles[player-1];
    result= malloc(size* sizeof(char));
    if(result!=NULL)
    {
        int i=0;
        for(uint64_t j=0; j < g->height; j++)
        {
            uint64_t y=g->height-j-1;
            for(uint64_t x=0; x< g->width; x++)
            {
                if(g->row[y].line[x]-'0'==player)
                {
                    write_background_green_color(result, &i);
                }
                if(y==cursor_y && x==cursor_x)
                {
                    write_inverse(result,&i);
                }
                write_tile(result, &i, g->row[y].line[x], length);
                if(y==cursor_y && x==cursor_x)
                {
                    write_end_inverse(result, &i);
                } 
                if(g->row[y].line[x]-'0'==player)
                {
                    write_background_black_color(result, &i);
                }
                result[i]='|';
                i++;
            }
        result[i]='\n';
        i++;
        }
        result[i]=0;
    }
    return result;
}

char*  gamma_board_interactive(gamma_t *g,uint32_t current_player, uint32_t cursor_x,uint32_t cursor_y)
{
    char *result=NULL;
    if(g!=NULL)
    {
        if(g->players<10)
        {
            result= gamma_board_interactive_less_than_10(g, current_player, cursor_x, cursor_y);
        }
        else
        {
            result= gamma_board_interactive_more_than_9(g, current_player, cursor_x, cursor_y);
        }
    }
    return result;
}
