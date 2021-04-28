#include <nusys.h>
#include <malloc.h>
#include "vector.h"

#define INITIAL_CAPACITY 8
#define min(x,y) (((x)<(y))?(x):(y))

struct _vector
{
    value_type *array;
    int size;
    int capacity;
};

vector vector_create() 
{
    vector v = (vector)malloc(sizeof(struct _vector)); 
    
    if (v == NULL) return NULL;
    
    v->size = 0; 
    v->capacity = INITIAL_CAPACITY; 
    v->array = (value_type *)malloc(sizeof(value_type) * v->capacity); 
    
    if (v->array == NULL) return NULL;
    
    return v;
}

void vector_destroy(vector v) 
{
    if (v == NULL) return;

    free(v->array); 
    free(v);
}

void vector_double_capacity(vector v) 
{
    if (v == NULL) return;

    int new_capacity = 2 * v->capacity; 
    
    value_type *new_array = (value_type *)malloc(sizeof(value_type) * new_capacity); 
    
    if (new_array == NULL) return;
    
    for (int i = 0; i < v->size; i++)
    {
        new_array[i] = v->array[i];
    }
    
    free(v->array); 
    v->array = new_array; 
    v->capacity = new_capacity;
}

void vector_half_capacity(vector v) 
{
    if (v == NULL) return;

    if (v->capacity <= INITIAL_CAPACITY) return;
    
    int new_capacity = v->capacity / 2; 
    
    value_type *new_array = (value_type *)malloc(sizeof(value_type) * new_capacity); 
    
    if (new_array == NULL) return;
    
    for (int i = 0; i < min(v->size, new_capacity); i++)
    {
        new_array[i] = v->array[i];
    }
    
    free(v->array); 
    v->array = new_array; 
    v->capacity = new_capacity; 
    v->size = min(v->size, new_capacity);
}

void vector_add(vector v, value_type value)
{
    if (v == NULL) return;

    if (v->size >= v->capacity)
    {
        vector_double_capacity(v);
    }
    
    v->array[v->size++] = value;
}

value_type vector_get(vector v, int i) 
{
    if (v == NULL) return NULL;

    if (i < 0 || i >= v->size) return NULL;
    
    return v->array[i];
}

void vector_put(vector v, int i, value_type value) 
{
    if (v == NULL) return;

    if (i < 0 || i >= v->size) return;
    
    v->array[i] = value;
}

void vector_add_at(vector v, int i, value_type value) 
{
    if (v == NULL) return;

    if (i < 0 || i >= v->size) return;
    
    if (v->size >= v->capacity)
    {
        vector_double_capacity(v);
    }
    
    for (int j = v->size; j > i; j--)
    {
        v->array[j] = v->array[j - 1];
    }
    
    v->array[i] = value; 
    v->size++;
}

value_type vector_remove_at(vector v, int i) 
{
    if (v == NULL) return NULL;

    if (i < 0 || i >= v->size) return NULL;
    
    value_type ret = v->array[i]; 
    
    for (int j = i + 1; j < v->size; j++)
    {
        v->array[j - 1] = v->array[j];
    }
    
    v->size--; 
    
    if (4 * v->size < v->capacity)
    {
        vector_half_capacity(v);
    }
    
    return ret;
}

int vector_is_empty(vector v) 
{ 
    if (v == NULL) return 1;

    return v->size == 0;
}

int vector_size(vector v) 
{
    if (v == NULL) return 0;

    return v->size;
}

void vector_clear(vector v) 
{
    if (v == NULL) return;

    v->size = 0; 
    
    while (v->capacity > INITIAL_CAPACITY)
    {
        vector_half_capacity(v);
    }
}
