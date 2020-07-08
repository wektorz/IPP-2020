/** @file
 * inteface of dynamic array.
 */

#ifndef DYNAM_ARRAY
#define DYNAM_ARRAY
//needed for direct access to length
struct dynamic_array{
    uint32_t *a;
    int length;
    int size;
    bool ok;
};
typedef struct dynamic_array darray;

/**
 * creates new emmpty dynamic array containing uint32 numbers.
 */
darray* new_darray();

/**
 * add an element to array.
 */
void add_darray(darray *d,uint32_t x);

/**
 * removes all memory taken by darray.
 * @returns NULL (proposed usage d=free_darray(d))
 */ 
darray* free_darray(darray *d);

/**
 * function which tries to reads line as darray.
 * if there are some improper expresions returns NULL
 */ 
darray* read_numbers_from_line(int *z);

#endif